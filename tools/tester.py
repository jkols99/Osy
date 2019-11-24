#!/usr/bin/python3

# SPDX-License-Identifier: Apache-2.0
# Copyright 2019 Charles University


import argparse
import logging
import multiprocessing
import os
import re
import shutil
import subprocess
import sys
import threading
import time

CPU_COUNT = multiprocessing.cpu_count()


class TesterException(Exception):
    def __init__(self, message, details):
        Exception.__init__(self, message)
        self.details = details

class UnreachableCode(TesterException):
    def __init__(self):
        import inspect
        info = inspect.getframeinfo(inspect.stack()[1][0])
        position = '{}:{}'.format(info.filename, info.lineno)
        TesterException.__init__(self,
                                 'internal error at {}'.format(position),
                                 'reached unreachable code :-(')

class InThreadPopener(threading.Thread):
    def __init__(self, command, work_dir, log_filename, logger, stdin=None):
        threading.Thread.__init__(self)
        self.command = command
        self.work_dir = work_dir
        self.log_filename = log_filename
        self.logger = logger
        self.stdin = stdin
        self.output = []
        self.exit_code = None
        self.proc = None
        self.last_exception = None

    def run(self):
        try:
            with open(self.log_filename, 'wt') as log_file:
                self.proc = subprocess.Popen(self.command,
                                             stdin=self.stdin,
                                             stdout=subprocess.PIPE,
                                             stderr=subprocess.STDOUT,
                                             cwd=self.work_dir)
                for line in self.proc.stdout:
                    line = line.decode('utf-8')
                    line2 = line.rstrip()
                    self.output.append(line2)
                    self.logger.debug('[%s] %s', self.command[0], line2)
                    log_file.write(line)
            self.proc.wait()
            self.exit_code = self.proc.returncode
        except Exception as exc:
            self.last_exception = exc

    def forceful_wait(self, timeout):
        self.join(timeout)
        if self.is_alive():
            self.logger.error('Forcefully killing %s', self.command)
            self.exit_code = -1
            self.forcefully_terminate()
            self.join(1)
            if self.is_alive():
                self.logger.critical('Failed to kill %s', self.command)
            raise TesterException('{} timed-out'.format(self.command[0]),
                                  'timed-out after {} seconds'.format(timeout))

    def forcefully_terminate(self):
        for _ in range(5):
            try:
                if self.proc.poll() is not None:
                    break
                self.proc.terminate()
                if self.proc.poll() is not None:
                    break
                time.sleep(1)
                if self.proc.poll() is not None:
                    break
                self.proc.kill()
            except OSError as exc:
                self.logger.debug(exc)
                # Probably (fingers crossed) a race between poll() and kill()

def run_command_with_logging(command, work_dir, log_filename, logger, stdin=None, timeout=600):
    proc = InThreadPopener(command, work_dir, log_filename, logger, stdin)
    proc.start()
    proc.forceful_wait(timeout)
    if proc.last_exception:
        raise TesterException('execution of {} failed'.format(command[0]),
                              str(proc.last_exception))
    return (proc.exit_code, proc.output)

def prepare_empty_build_dir(test_name):
    test_name_clean = re.sub(r'[^\w\-_]', '_', test_name.replace('/', '__'))
    dir_name = os.path.join(os.getcwd(), '_build_' + test_name_clean)
    try:
        shutil.rmtree(dir_name)
    except FileNotFoundError:
        pass
    os.mkdir(dir_name)
    return dir_name


def run_kernel_test(test_descriptor, extra_arguments):
    parts = test_descriptor.split(':')
    name = parts[0]

    if (len(parts) > 1) and parts[1].startswith('m'):
        memory_size = int(parts[1][1:])
    else:
        memory_size = None

    logger = logging.getLogger('K/{}'.format(name))
    build_dir = prepare_empty_build_dir('kernel/{}'.format(test_descriptor))
    logger.debug('Will use build directory %s.', build_dir)

    configure_args = ['--kernel-test={}'.format(name)]
    if memory_size is not None:
        configure_args.append('--memory-size={}'.format(memory_size))
    for i in extra_arguments['configure']:
        configure_args.append(i)

    logger.info('Configuring (%s)...', ' '.join(configure_args))
    (exit_code, _) = run_command_with_logging(
        ['../configure.py', '--verbose'] + configure_args,
        build_dir,
        os.path.join(build_dir, 'configure.log'),
        logger,
        timeout=60,
    )
    if exit_code != 0:
        raise TesterException('configuration failed', 'see configure.log')

    logger.info('Building ...')
    (exit_code, _) = run_command_with_logging(
        ['make', '-j{}'.format(CPU_COUNT)],
        build_dir,
        os.path.join(build_dir, 'make.log'),
        logger,
        timeout=120,
    )
    if exit_code != 0:
        raise TesterException('build failed', 'see make.log')

    msim_log = os.path.join(build_dir, 'msim.log')

    logger.info('Running MSIM ...')
    (exit_code, output) = run_command_with_logging(
        ['msim'],
        build_dir,
        msim_log,
        logger,
        timeout=240,
    )
    if exit_code != 0:
        raise TesterException('MSIM execution failed', 'see msim.log')

    found_test_passed = False
    for line in output:
        if line == 'Test passed.':
            found_test_passed = True
            break
    if not found_test_passed:
        raise TesterException('test failed', 'see msim.log')

    logger.info('Checking test output ...')
    (exit_code, _) = run_command_with_logging(
        ['../tools/check_output.py'],
        build_dir,
        os.path.join(build_dir, 'report.log'),
        logger,
        stdin=open(msim_log, 'r'),
        timeout=30,
    )

    if exit_code != 0:
        raise TesterException('test failed', 'see report.log')

def get_suite_tests(suite_filename):
    with open(suite_filename, 'r') as suite:
        for line in suite:
            line = line.strip()
            if line.startswith('#') or (line == ''):
                continue
            parts = line.split()
            if len(parts) != 2:
                raise TesterException('invalid suite file {}'.format(suite_filename),
                                      'invalid line {}'.format(line))
            yield (parts[0], parts[1])


def run_tests(tests, extra_arguments):
    logger = logging.getLogger('run_tests')

    report = []
    for test in tests:
        try:
            if test['type'] == 'kernel':
                run_kernel_test(test['name'], extra_arguments)
                report.append({
                    'name': test['name'],
                    'status': 'passed'
                })
            else:
                raise TesterException('unknown test type {} for {}'.format(
                    test['type'], test['name']), '')
        except TesterException as exc:
            logger.error('Test %s failed: %s (%s).', test['name'], exc, exc.details)
            report.append({
                'name': test['name'],
                'status': 'failed',
                'message': str(exc),
            })
    return report

def all_tests_passed(report):
    for result in report:
        if result['status'] != 'passed':
            return False
    return True


def print_report(report):
    logger = logging.getLogger('report')
    logger.info('')
    logger.info('Test run report:')
    logger.info('')

    count_passed = 0
    count_total = 0
    for result in report:
        if result['status'] == 'passed':
            logger.info(' - %s passed', result['name'])
            count_passed = count_passed + 1
        elif result['status'] == 'failed':
            logger.info(' - %s FAILED (%s)', result['name'], result['message'])
        else:
            raise UnreachableCode()

        count_total = count_total + 1

    logger.info('')
    count_failures = count_total - count_passed
    if count_failures == 0:
        logger.info('All tests passed.')
    else:
        logger.info('There were failures: %d passed, %d failed.', count_passed, count_failures)


def main():
    common_args = argparse.ArgumentParser(add_help=False)
    common_args.add_argument('--verbose',
                             default=False,
                             dest='verbose',
                             action='store_true',
                             help='Be verbose.')
    common_args.add_argument('--toolchain',
                             default=None,
                             dest='toolchain_dir',
                             help='Toolchain directory.')

    args = argparse.ArgumentParser(description='Run NSWI004 tests')
    args.set_defaults(action='help')
    args.set_defaults(logging_level=logging.INFO)
    args_sub = args.add_subparsers(help='Select what to do.')

    args_help = args_sub.add_parser('help', help='Show this help.')
    args_help.set_defaults(action='help')

    args_kernel = args_sub.add_parser('kernel', help='Run kernel test.', parents=[common_args])
    args_kernel.set_defaults(action='kernel')
    args_kernel.add_argument('test_names',
                             metavar='TEST_NAME',
                             nargs='+',
                             help='Kernel test names.')

    args_suite = args_sub.add_parser('suite', help='Run whole test suite.', parents=[common_args])
    args_suite.set_defaults(action='suite')
    args_suite.add_argument('suite_files',
                            metavar='SUITE_DESCRIPTOR_FILE',
                            nargs='+',
                            help='Filename with test suite description.')

    if len(sys.argv) < 2:
        class HelpConfig:
            def __init__(self):
                self.action = 'help'
        config = HelpConfig()
    else:
        config = args.parse_args()

    if config.action == 'help':
        args.print_help()
        return 0

    if config.verbose:
        config.logging_level = logging.DEBUG
    logging.basicConfig(format='[%(asctime)s %(name)-25s %(levelname)7s] %(message)s',
                        level=config.logging_level)

    extra_arguments = {
        'configure': [],
    }
    if config.toolchain_dir is not None:
        extra_arguments['configure'].append('--toolchain')
        extra_arguments['configure'].append(os.path.realpath(config.toolchain_dir))

    tests = []
    if config.action == 'kernel':
        for test in config.test_names:
            tests.append({
                'type': 'kernel',
                'name': test
            })
    elif config.action == 'suite':
        for suite in config.suite_files:
            for (test_type, test_name) in get_suite_tests(suite):
                tests.append({
                    'type': test_type,
                    'name': test_name
                })
    else:
        raise UnreachableCode()

    report = run_tests(tests, extra_arguments)

    print_report(report)

    return 0 if all_tests_passed(report) else 1


if __name__ == "__main__":
    sys.exit(main())
