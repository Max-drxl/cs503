1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  `fgets()` is a good choice for this application because it will force a newline character at the end of the input stream as well as sanitizing input. If it encounters an error it will also simply return null ensuring we do not hit any undefined behavior. It also limits input length to protet the buffer.

2. You needed to use `malloc()` to allocate memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  Using `malloc()` allows us to do pointer arithmetic on the `cmd_buff` input whereas if we had allocated a fixed-size array we would not have been able to traverse it and manipulate it in that manner.


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  Shells will try to split and group commands using various characters including spaces, if we didn't trim our commands then the shell may start interpreting arguments as additional commands or other special input that should not be processed as a command.

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  1) We should implement the `>` character redirection to save output to a file. 2) We should implement the `<` character redirection to read input from a file to the terminal (Dragon command would be good to store the art in a file). 3) We should also implement the `>>` redirection command to append to files without overwriting all contents within them. They also have error counterparts that will be useful as well, particularly `2>>` to ensure we don't overwrite file contents if there's an error. There are a lot of opportunities for problems between accidentally overwriting files using the wrong command, forgetting to redirect errors, and then debugging when the complexity of commands is high due to multiple redirects.

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  Normal redirection is to send output or errors to a location like a file or terminal for viewing or for storage whereas piping is the redirect the result, or output, of the first command to the next command.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  STDOUT by design is for data in a pipe which may still be useful for humans but is definitely useful for further machine processing, while STDERR is an alerting process for humans to indicate something is wrong. You also may not want STDERR in your output, an example given that I enjoyed was accidentally sending pages to a printer and wasting ink on printing STDERR output.

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  The command to accomplish redirecting both STDERR and STDOUT together is `2>&1` but it must come after you've already redirected stdout to the file/terminal location using `>`. This would be beneficial particularly for debugging such that your test data, the input, is saved with your error, the output. Our shell should display error messages to the user via terminal always and then additionally allow for errors to be redirected to files - this allows us to be immediately notified of issues but also allows a record to be kept in something like an error log file. The shell itself should not exit on an error, just alert the user.