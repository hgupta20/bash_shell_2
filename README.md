# bash_shell_2
A shell with redirection and signal handling

* SIGINT
  This signal is usually sent when the user enters Ctrl-c. When your shell receives SIGINT it should print the following:

* SIGCHLD
  This signal is sent to the parent when one of its child processes terminates or stops. Move your implementation of waitpid() into the SIGCHLD handler to print:

* SIGTERM
  SIGTERM is generated when kill pid is sent from the command line.
  Send SIGTERM to all children and exit this process normally.

* SIGTSTP
  This is the signal sent by pressing Ctrl-z on the keyboard.
  Pause program execution.

*  SIGCONT
   Resume process execution if stopped, otherwise ignore.
   Your shell should resume with the prompt printed to screen

*  IO Redirection

A pipe is a special kind of file type where the first data written is the first data read. Pipes are great because they allow us to set up pipelines like this:

CS361 > command1 | command2 | command3

Where the output of command1 is the input to command2 and the output of command2 is the input to command3 and command3 outputs to stdout. You may have even used pipes before. For instance, you can sort all the output of ls using sort

CS361 > ls -al | sort

Which takes the output of ps and writes it to a pipe. Grep reads from the pipe and then prints its output to stdout. We don't always want to print to stdout or read from stdin. In this case we can Redirect Input and Output. This is done with the >, >>, and < commands.

* Redirect output - output that would have printed to stdout is printed to the file instead.
* command > output.txt saves the output of command into a file called output.txt
* command >> output.txt appends the output of command onto the file called output.txt
* Redirect input - Input that would have read from stdin is read from a file
* command < input.txt command gets its input from input.txt.
* Your job is to implement |, <, >, and >>.

* Background Jobs

Another shell feature is the ability to run jobs in the background. A job is initialized in the background by appending an ampersand, &, to the end of a command. For example:

CS361 > emacs hw3.c &

In this command the text editor emacs is started with hw3.c ready to edit, but its in the background. To bring it to the foreground type:

CS361 > fg

To send the editor back to the background type Ctrl-z. A background job generating output to stdout will typically interleave output with the foreground program. A background job attempting to read from the terminal will be sent SIGTTIN. The job should wait until it is brought into the foreground with SIGCONT before attempting the read.

To implement &, fg, and properly handle signals SIGTTOUT, SIGTTIN
