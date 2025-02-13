1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  `Fork()` allows us to continue our current program while also executing commands in the child program that `execvp()` establishes and also allows us to confirm that the child program worked as expected. If we just did `execvp()` directly then we would exit our loops and other behaviors within our shell to perform whatever the command is like perhaps a `cd` to a different directory, and then all processing would be complete after moving to the different directory.

2. What happens if the `fork()` system call fails? How does your implementation handle this scenario?

    > **Answer**:  If the `fork()` system call fails then it will return -1 without creating a child process and `errno` will be set to indicate the error. My implementation will check the return code of `fork()` and if it's less than 0 will `printf()` an error indicating `fork()` failed and then exit the program immediately to prevent undefined behavior.

3. How does `execvp()` find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  `Execvp()` finds the command to execute by basically using grep on our system's `$PATH` variable to find any program with the same exact matching name within one of the `$PATH` directories as was provided in the first argument to the `execvp()` call.

4. What is the purpose of calling `wait()` in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  The purpose of calling `wait()` in the parent process after forking is to ensure that the child process finishes before the parent continues processing. If we didn't call `wait()` then the child process could still be processing while the parent process moves on, thus making the child a zombie because it still has its process ID contained in the process table since the parent process did not wait to read its exit status.

5. In the referenced demo code we used `WEXITSTATUS()`. What information does this provide, and why is it important?

    > **Answer**:  `WEXITSTATUS()` is a macro that evaluates the termination status of the child process by extracting the least significant 8 bits returned from its state changing. This is important to verify whether the child process performed as expected.

6. Describe how your implementation of `build_cmd_buff()` handles quoted arguments. Why is this necessary?

    > **Answer**:  While processing the string from the `cmd_line`, I handle quoted arguments by looking for them first and checking if I have reached the end of the `cmd_line` input immediately afterward before I look for arguments separated by spaces. If I had reached the end of the `cmd_line` without checking and immediately began processing spaces arguments then my logic adds an additional space value into the next `argv` which causes tests to fail as that space will be sent into the execute functions. Otherwise the logic for handling quotes is basically the same as handling spaces, just that I start and end on a quote instead of starting and ending on a space. It's necessary to handle quotes separately because if any part of the argument can be empty, contain spaces that need to be preserved, or contain special characters or variable names then the shell may incorrectly interpret the argument unless there are quotes to preserve these characters and enforce variable interpolation. For these reasons, when searching using filenames or URLs especially, quotations are the standard.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  There were a lot of unexpected challenges and changes in my parsing logic compared to the previous assignment. I used `malloc()` in the previous assignment to separate the 'exe' portion from the 'arg' portion because each command was broken into two elements: execution and arguments. This meant I could simply `strcpy()` my execution into the `command` struct while keeping the arguments separate, then trimming the arguments to remove additional whitespace and also simply `strcpy()` the arguments into the `command` struct as well. That approach is a little wasteful though and I am not really separating the arguments into different arguments, just processing all of them as one large chunk. I also had a shortcut if given one command to just `strcpy()` the command without any further processing. This assignment forces processing of all arguments so none of my old parsing logic was really useable anymore and it took me a while to realize that I only needed to use `malloc()` once and could instead process everything into the `cmd_buff` struct `argv[]` values from there. Since we're only processing one command at a time there's no need to try a shortcut as we're always given only one command, so I always enter the parsing logic and treat the command the same way I treat arguments. I think my parsing logic now though is pretty strong and should be re-usable with minimal changes to support pipes in the future.

8. For this question, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  Signals are a form of asynchronous event notification and control in a Linux system that are not processed one-after-the-other or used for data exchange like other forms of IPCs. 

- Find and describe three commonly used signals (e.g., `SIGKILL`, `SIGTERM`, `SIGINT`). What are their typical use cases?

    > **Answer**:  `SIGKILL` is a kernel-executed signal typically used to terminate a process that is unresponsive - because the process is unresponsive it cannot handle the signal itself which is why the kernel kicks in to kill the process. `SIGTERM` is like a more polite `SIGKILL` which can be sent from the Linux process handler and other processes that the to-be-terminated process will try to handle itself to ensure proper cleanup and graceful exit. `SIGINT` is the signal sent from a keyboard when the user performs the `CTRL+C` command from their terminal while a process is running to end the process gracefully with proper cleanup.

- What happens when a process receives `SIGSTOP`? Can it be caught or ignored like `SIGINT`? Why or why not?

    > **Answer**:  `SIGSTOP` cannot be caught, blocked, or ignored because when a process receives `SIGSTOP` the kernel takes control from the process to stop that process. This can be used when a process cannot handle stopping itself for whatever reason, or during a debugger to temporarily pause the process so its state can be inspected further.
