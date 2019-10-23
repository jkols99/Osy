# Team repository for NSWI004

**All assignments from now on require the MSIM emulator and the MIPS cross compiler toolchain. Please see
[the course website](https://d3s.mff.cuni.cz/teaching/nswi004) for the download and installation instructions.**

The individual assignments are stored in `ASSIGMENT-*.md` files, the source code
is otherwise organized in the same manner as for the last individual assignment.
Further assignments (and related code modifications) will be published as
commits to the parent repository (see *Forked from* link for this project).
You are expected to merge them yourselves. We will inform about such
updates via the mailing list and also on
[the course website](https://d3s.mff.cuni.cz/teaching/nswi004).

A simple guide for merging with upstream repository is available e.g.
[here](https://help.github.com/en/articles/syncing-a-fork) (even though
the guide is for GitHub, there is virtually no difference to GitLab in
this respect).

Each assignment will have its own suite of tests, their list will be in
`suite_as*.txt` files. They can be executed simply by typing

```shell
./tools/tester.py suite suite_as*.txt
```

Your `.gitlab-ci.yml` file contains CI configuration for your work. Check
that you always execute (and pass) even the suites for previous assignments.
When the assignment is finished, there should be no regressions and all
tests should be green.

We will evaluate the state of your repository (your `master` branch)
at the time of the deadline for each assignment (i.e. the last commit
on `master` before deadline). If you wish to use a different commit,
[tag it](https://git-scm.com/book/en/v2/Git-Basics-Tagging) with `as2`, `as3`
etc. tags (obviously they still have to refer to commit before the deadline).
