Lab 1: Time Travel Shell
========================

Contributors:
-------------
Name | UID | Email
--- | --- | ---
Georgi Baghdasaryan | 603 875 284 | baghdasaryan@ucla.edu  
Sixiang Gu | 903 943 192 | ntgsx92@gmail.com

Lab Description
---------------

### Introduction:
Many of the shell scripts have command sequences that look like the following
(though the actual commands are proprietary):

```shell
sort < a | cat b - | tr A-Z a-z > c
sort -k2 d - < a | uniq -c > e
diff a c > f
```

In this example, the standard 
[POSIX](http://pubs.opengroup.org/onlinepubs/9699919799/ "IEEE POSIX")
shell executes the code serially: it waits for the command in the first line
to finish before starting the command in the second line. The main goal is to
speed this up by running commands in parallel when it is safe to do so. In
this example, it is safe to start running the second command before the first
one finishes, because the second command does not read any file that the first
command writes. However, the third command cannot be started until the first
one finishes, because the first command writes to file _c_ that the third
command reads.

Lab's goal is to write a prototype for a shell that runs code like the above
considerably faster than standard shells do, by exploiting the abovementioned
parallelism.

To simplify things, this prototype will exploit parallelism only at the top
level, such as in the examples shown above. It will not support parallelize
subcommands. For example, this prototype executes the following commands in
sequence, without parallelizing them, because all the commands are subsidiary
to the parentheses at the top level:

```shell
(sort < a | cat b - | tr A-Z a-z > c
 sort -k2 d - < a | uniq -c > e
 diff a c > f)
```

Shell scripts executed by this program all follow these rules:
* They limit themselves to a small subset of the shell syntax, described in
[Shell syntax subset](#shell-syntax-subset) below.
* Simple commands in scripts have limited behavior, described in [Time travel
limitations on computations](#time-travel-limitations-on-computations) below.
* In some cases they behave in special ways, as described in [Don't care
behaviors](#dont-care-behaviors) below. 

### Some details on syntax and semantics of the shell subset described above

The "time travel" feature of this shell is feasible partly because of the
restricted subset of the Linux shell.

#### Shell syntax subset

This program supports only the following small subset of the standard [POSIX
shell grammar](http://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#tag_18_10):
* Words, consisting of a maximal sequence of one or more adjacent characters
that are ASCII letters (either upper or lower case), digits, or any of: _! % +
, - . / : @ ^ __
* The following eight special tokens: _; | && || ( ) < >_
* Simple commands, which are sequences of one or more words. The first word is
the file to be executed.
* Subshells, which are complete commands surrounded by _(_ _)_.
* Commands, which are simple commands or subshells followed by I/O
redirections. An I/O redirection is possibly empty, or _<_ followed by a word,
or _>_ followed by a word, or _<_ followed by a word followed by _>_ followed
by a word.
* Pipelines, which are one or more commands separated by _|_.
* And-ors, which are one or more pipelines separated by _&&_ or _||_. The _&&_
and _||_ operators are left-associative and have the same operator precedence.
* Complete commands, which are one or more and-ors each separated by a
semicolon or newline, and which are optionally followed by a semicolon. An
entire shell script is a complete command.
* Comments, each consisting of a _#_ that is not immediately preceded by an
ordinary token, followed by characters up to (but not including) the next
newline.
* White space consisting of space, tab, and newline. Newline is special: as
described above, it can substitute for semicolon. Also, although white space
can ordinarily appear before and after any token, the only tokens that newlines
can appear before are _(_, _)_, and the first words of simple commands.
Newlines may follow any special token other than _<_ and _>_.

If shell's input does not fall within the above subset, it outputs to stderr a
syntax error message that starts with the line number and a colon, and then
exits with code 1.

Also, shell can have undefined behavior if any of the following features are
used:
* [Shell reserved words](http://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#tag_18_04)
such as _!_, _{_, _if_, and _function_, when used as the first word of a command.
* Commands that invoke
[special built-in utilities](http://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#tag_18_14)
such as _break_, _._, and _exit_.
* A token consisting entirely of digits, immediately before _<_ or _>_ (for
example, as in the command "_cat 2>/dev/null_").
* Two adjacent left parentheses _((_ - see 
[Token Recognition](http://pubs.opengroup.org/onlinepubs/007904875/xrat/xcu_chap02.html#tag_02_02_03)
for why. 

#### Time travel limitations on computations

When this program is run in time-travel mode, it can have undefined behavior
if any of the following limitations are violated:
* Simple commands read only from standard input or from files whose names are
one of the words of the command. For example, the simple command "_./doit -f x
nobody@gmail.com_" reads at most from standard input and from the files
_./doit_, _-f_, _x_, and _nobody@gmail.com_, if these files exist.
* The computation never accesses the same file via two different names. For
example, if the computation uses the file name _./doit_ and the file name
_doit_ without the leading "_./_", the behavior is undefined.
* Simple commands write only to standard output, to standard error, or to files
that are never otherwise accessed by the computation. For example, the _mv_ and
_rm_ commands cannot be used to mess up dependency checking, because they can
cause dependency problems only by modifying directories that are later
searched.

#### Don't care behaviors

Sometimes, when this program is run in time-travel mode, it will produce
results different from expected ones. Some of these cases are listed here:
* In this implementation, commands behave as if the time of day jumps around at
random. For example, "_(date >A; date >B)_" can put an earlier time stamp into
B than into A. It is this property of the program that prompts the nickname
"time travel shell".
* If a set of commands all read from standard input, or write to standard
output or standard error, and have no other dependencies that interfere with
each other, program can run them in parallel and interleave their reads and
writes arbitrarily. For example, "_tr A B; tr A C_" can run two instances of
[tr](http://pubs.opengroup.org/onlinepubs/9699919799/utilities/tr.html) in
parallel, both reading from standard input and writing to standard output; the
combination somewhat-randomly transforms some _A_s to _B_s and other _A_s to
_C_s as it copies input to output and it does not necessarily output blocks in
the same order that they were input.
* This shell attempts to execute the following commands as regular commands,
finding them via the _PATH_ environment variable and running them as
executables in a separate child process, even if the commands do not exist in
the _PATH_, and even though POSIX does not allow this behavior: 
[false](http://pubs.opengroup.org/onlinepubs/9699919799/utilities/false.html),
[fc](http://pubs.opengroup.org/onlinepubs/9699919799/utilities/fc.html),
[fg](http://pubs.opengroup.org/onlinepubs/9699919799/utilities/fg.html),
[getopts](http://pubs.opengroup.org/onlinepubs/9699919799/utilities/getopts.html),
[jobs](http://pubs.opengroup.org/onlinepubs/9699919799/utilities/jobs.html),
[kill](http://pubs.opengroup.org/onlinepubs/9699919799/utilities/kill.html),
[newgrp](http://pubs.opengroup.org/onlinepubs/9699919799/utilities/newgrp.html),
[pwd](http://pubs.opengroup.org/onlinepubs/9699919799/utilities/pwd.html),
[read](http://pubs.opengroup.org/onlinepubs/9699919799/utilities/read.html),
[true](http://pubs.opengroup.org/onlinepubs/9699919799/utilities/true.html),
[umask](http://pubs.opengroup.org/onlinepubs/9699919799/utilities/umask.html),
[unalias](http://pubs.opengroup.org/onlinepubs/9699919799/utilities/unalias.html),
[wait](http://pubs.opengroup.org/onlinepubs/9699919799/utilities/wait.html).

Usage
-----

This program comes with a _Makefile_ that supports the following actions:
* "_make_" builds the _timetrash_ program.
* "_make clean_" removes the program and all other temporary files and object
files that can be regenerated with "_make_".
* "_make check_" tests the _timetrash_ program on the available test cases.
* "_make dist_" makes a software distribution tarball 
lab1-_YOUR-USERNAME_.tar.gz and does some simple testing on it.

After building the program with **_make_**, it can be run directly by
invoking, for example, _./timetrash -p foo.sh_, where _foo.sh_ contains shell
commands to be executed.

An example shell script, _example.sh_, is provided for your convenience.

Limitations:
------------
  * 
  * 
  * 
  * 

Summary:
--------

SUMMARY
