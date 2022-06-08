# Unix Miniprojects
Tiny projects built as part of college assignments to get some familiarity with the Unix-family operating systems.

Contents:
* `unix-tools` contains solutions to various tasks requiring the use of grep, sed and awk.
* `shell-scripting` has two shell scripts: one calculates GCD of two numbers, the other is a primitive CLI file manager.
* In `c-programming` there are two programs written in C.
  * One multiplies two matrices, but uses file IO and parallelizes the work across multiple process which communicate with their parent over Unix pipes.
  * The second program is a simple chat application that allows multiple users to exchange text messages in a terminal.
    The communication happens over Unix-sockets, so it's local and has little practical use. 
