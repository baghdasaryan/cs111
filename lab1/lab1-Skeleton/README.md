Lab 1: Time travel shell
========================

Contributors:
-------------
  Georgi   Baghdasaryan     UID: 603 875 284
  Sixiang  Gu               UID: 903 943 192

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


// TODO: CHANGE


Shell scripts executed by this program all follow these rules:
* They limit themselves to a small subset of the shell syntax, described in
"Shell syntax subset" below.
* Simple commands in scripts have limited behavior, described in "Time travel
limitations on computations" below.
* In some cases they behave in special ways, as described in "Don't care
behaviors" below. 


Simple commands in scripts have limited behavior, described in [Time travel
limitations on computations](#Time travel limitations on computations) below









### Some details on syntax and semantics of the shell subset described above

The "time travel" feature of this shell is feasible partly because of the
restricted subset of the Linux shell.

#### Shell syntax subset

Your implementation of the shell needs to support only the following small subset of the standard POSIX shell grammar:

    Words, consisting of a maximal sequence of one or more adjacent characters that are ASCII letters (either upper or lower case), digits, or any of: ! % + , - . / : @ ^ _
    The following eight special tokens: ; | && || ( ) < >
    Simple commands, which are sequences of one or more words. The first word is the file to be executed.
    Subshells, which are complete commands surrounded by ( ).
    Commands, which are simple commands or subshells followed by I/O redirections. An I/O redirection is possibly empty, or < followed by a word, or > followed by a word, or < followed by a word followed by > followed by a word.
    Pipelines, which are one or more commands separated by |.
    And-ors, which are one or more pipelines separated by && or ||. The && and || operators are left-associative and have the same operator precedence.
    Complete commands, which are one or more and-ors each separated by a semicolon or newline, and which are optionally followed by a semicolon. An entire shell script is a complete command.
    Comments, each consisting of a # that is not immediately preceded by an ordinary token, followed by characters up to (but not including) the next newline.
    White space consisting of space, tab, and newline. Newline is special: as described above, it can substitute for semicolon. Also, although white space can ordinarily appear before and after any token, the only tokens that newlines can appear before are (, ), and the first words of simple commands. Newlines may follow any special token other than < and >.

If your shell's input does not fall within the above subset, your implementation should output to stderr a syntax error message that starts with the line number and a colon, and should then exit.

Your implementation can have undefined behavior if any of the following features are used. In other words, our test cases won't use these features and your program need not diagnose an error if these features are used.

    Shell reserved words such as !, {, if, and function, when used as the first word of a command.
    Commands that invoke special built-in utilities such as break, ., and exit. Exception: your implementation should support the special-builtin utility exec with a command and optional arguments (you need not support exec without a command).
    A token consisting entirely of digits, immediately before < or > (for example, as in the command "cat 2>/dev/null").
    Two adjacent left parentheses (( – see Token Recognition for why. 









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

Similarly, in some cases, your company's scripts don't care how your implementation behaves, and it's OK for it to depart from established semantics when it is run in time-travel mode.

    It is OK if commands behave as if the time of day jumps around at random. For example, it is OK if "(date >A; date >B)" puts an earlier time stamp into B than into A. It is this property of your implementation that prompts the nickname "time travel shell".
    If a set of commands all read from standard input, or write to standard output or standard error, and have no other dependencies that interfere with each other, your implementation can run them in parallel and interleave their reads and writes arbitrarily. For example, "tr A B; tr A C" can run two instances of tr in parallel, both reading from standard input and writing to standard output; the combination somewhat-randomly transforms some As to Bs and other As to Cs as it copies input to output and it does not necessarily output blocks in the same order that they were input.

You can simplify your shell in one other way, regardless of whether it is run in time-travel mode:

    It is OK if your shell attempts to execute the following commands as regular commands, finding them via the PATH environment variable and running them as executables in a separate child process, even if the commands do not exist in the PATH, and even though POSIX does not allow this behavior: false fc fg getopts jobs kill newgrp pwd read true umask unalias wait






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

An example shell script, in _example.sh_, is provided for your convenience.

Features:
---------
  *
  *
  *
  *

Limitations:
------------
  *
  *
  *
  *

Summary:
--------


