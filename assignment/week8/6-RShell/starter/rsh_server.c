
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

//INCLUDES for extra credit
//#include <signal.h>
//#include <pthread.h>
//-------------------------

#include "dshlib.h"
#include "rshlib.h"


/*
 * start_server(ifaces, port, is_threaded)
 *      ifaces:  a string in ip address format, indicating the interface
 *              where the server will bind.  In almost all cases it will
 *              be the default "0.0.0.0" which binds to all interfaces.
 *              note the constant RDSH_DEF_SVR_INTFACE in rshlib.h
 * 
 *      port:   The port the server will use.  Note the constant 
 *              RDSH_DEF_PORT which is 1234 in rshlib.h.  If you are using
 *              tux you may need to change this to your own default, or even
 *              better use the command line override -s implemented in dsh_cli.c
 *              For example ./dsh -s 0.0.0.0:5678 where 5678 is the new port  
 * 
 *      is_threded:  Used for extra credit to indicate the server should implement
 *                   per thread connections for clients  
 * 
 *      This function basically runs the server by: 
 *          1. Booting up the server
 *          2. Processing client requests until the client requests the
 *             server to stop by running the `stop-server` command
 *          3. Stopping the server. 
 * 
 *      This function is fully implemented for you and should not require
 *      any changes for basic functionality.  
 * 
 *      IF YOU IMPLEMENT THE MULTI-THREADED SERVER FOR EXTRA CREDIT YOU NEED
 *      TO DO SOMETHING WITH THE is_threaded ARGUMENT HOWEVER.  
 */
int start_server(char *ifaces, int port, int is_threaded){
    int svr_socket;
    int rc;

    //
    //TODO:  If you are implementing the extra credit, please add logic
    //       to keep track of is_threaded to handle this feature
    //

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0){
        int err_code = svr_socket;  //server socket will carry error code
        return err_code;
    }

    rc = process_cli_requests(svr_socket);

    stop_server(svr_socket);


    return rc;
}

/*
 * stop_server(svr_socket)
 *      svr_socket: The socket that was created in the boot_server()
 *                  function. 
 * 
 *      This function simply returns the value of close() when closing
 *      the socket.  
 */
int stop_server(int svr_socket){
    return close(svr_socket);
}

/*
 * boot_server(ifaces, port)
 *      ifaces & port:  see start_server for description.  They are passed
 *                      as is to this function.   
 * 
 *      This function "boots" the rsh server.  It is responsible for all
 *      socket operations prior to accepting client connections.  Specifically: 
 * 
 *      1. Create the server socket using the socket() function. 
 *      2. Calling bind to "bind" the server to the interface and port
 *      3. Calling listen to get the server ready to listen for connections.
 * 
 *      after creating the socket and prior to calling bind you might want to 
 *      include the following code:
 * 
 *      int enable=1;
 *      setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
 * 
 *      when doing development you often run into issues where you hold onto
 *      the port and then need to wait for linux to detect this issue and free
 *      the port up.  The code above tells linux to force allowing this process
 *      to use the specified port making your life a lot easier.
 * 
 *  Returns:
 * 
 *      server_socket:  Sockets are just file descriptors, if this function is
 *                      successful, it returns the server socket descriptor, 
 *                      which is just an integer.
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code is returned if the socket(),
 *                               bind(), or listen() call fails. 
 * 
 */
int boot_server(char *ifaces, int port){
    int svr_socket;
    int rc;

    struct sockaddr_in addr;

    //AF_INET = IPv4 Internet protocols, 
    //SOCK_STREAM = sequenced, reliable, two-way, connection-based byte streams
    //0 = single protocol default
    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket < 0) {
        perror("Socket in Server failed");
        return ERR_RDSH_COMMUNICATION;
    }

    int enable=1;
    setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    addr.sin_family = AF_INET;
    //using ifaces did not work so using INADDR_ANY
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    //bind expects this struct so using it
    rc = bind(svr_socket, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in));
    if (rc < 0) {
        perror("Bind in Server failed");
        return ERR_RDSH_COMMUNICATION;
    }

    rc = listen(svr_socket, 20);
    if (rc < 0) {
        perror("Listen in Server failed");
        return ERR_RDSH_COMMUNICATION;
    }

    return svr_socket;
}

/*
 * process_cli_requests(svr_socket)
 *      svr_socket:  The server socket that was obtained from boot_server()
 *   
 *  This function handles managing client connections.  It does this using
 *  the following logic
 * 
 *      1.  Starts a while(1) loop:
 *  
 *          a. Calls accept() to wait for a client connection. Recall that 
 *             the accept() function returns another socket specifically
 *             bound to a client connection. 
 *          b. Calls exec_client_requests() to handle executing commands
 *             sent by the client. It will use the socket returned from
 *             accept().
 *          c. Loops back to the top (step 2) to accept connecting another
 *             client.  
 * 
 *          note that the exec_client_requests() return code should be
 *          negative if the client requested the server to stop by sending
 *          the `stop-server` command.  If this is the case step 2b breaks
 *          out of the while(1) loop. 
 * 
 *      2.  After we exit the loop, we need to cleanup.  Dont forget to 
 *          free the buffer you allocated in step #1.  Then call stop_server()
 *          to close the server socket. 
 * 
 *  Returns:
 * 
 *      OK_EXIT:  When the client sends the `stop-server` command this function
 *                should return OK_EXIT. 
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code terminates the loop and is
 *                returned from this function in the case of the accept() 
 *                function failing. 
 * 
 *      OTHERS:   See exec_client_requests() for return codes.  Note that positive
 *                values will keep the loop running to accept additional client
 *                connections, and negative values terminate the server. 
 * 
 */
int process_cli_requests(int svr_socket){
    int rc;
    int cli_socket;
    while(1) {
        cli_socket = accept(svr_socket, NULL, NULL);
        if (cli_socket < 0) {
            rc = ERR_RDSH_COMMUNICATION;
            break;
        }
        rc = exec_client_requests(cli_socket);
        if (rc < 0) {
            break;
        }
        //this on rc = OK, switch on everything else
        stop_server(cli_socket);
        printf(RCMD_MSG_CLIENT_EXITED);
    }
    stop_server(cli_socket);
    //processing switch for easier error control
    switch (rc) {
        case ERR_RDSH_COMMUNICATION:
            printf(CMD_ERR_RDSH_COMM);
            return ERR_RDSH_COMMUNICATION;
        case ERR_RDSH_SERVER:
            printf(CMD_ERR_RDSH_ITRNL, ERR_RDSH_SERVER);
            return ERR_RDSH_SERVER;
        case ERR_RDSH_CMD_EXEC:
            printf(CMD_ERR_RDSH_EXEC);
            return ERR_RDSH_CMD_EXEC;
        case OK_EXIT:
            printf(RCMD_MSG_SVR_STOP_REQ);
        default:
            break;
    }
    return OK_EXIT;
}

/*
 * exec_client_requests(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client
 *   
 *  This function handles accepting remote client commands. The function will
 *  loop and continue to accept and execute client commands.  There are 2 ways
 *  that this ongoing loop accepting client commands ends:
 * 
 *      1.  When the client executes the `exit` command, this function returns
 *          to process_cli_requests() so that we can accept another client
 *          connection. 
 *      2.  When the client executes the `stop-server` command this function
 *          returns to process_cli_requests() with a return code of OK_EXIT
 *          indicating that the server should stop. 
 * 
 *  Note that this function largely follows the implementation of the
 *  exec_local_cmd_loop() function that you implemented in the last 
 *  shell program deliverable. The main difference is that the command will
 *  arrive over the recv() socket call rather than reading a string from the
 *  keyboard. 
 * 
 *  This function also must send the EOF character after a command is
 *  successfully executed to let the client know that the output from the
 *  command it sent is finished.  Use the send_message_eof() to accomplish 
 *  this. 
 * 
 *  Of final note, this function must allocate a buffer for storage to 
 *  store the data received by the client. For example:
 *     io_buff = malloc(RDSH_COMM_BUFF_SZ);
 *  And since it is allocating storage, it must also properly clean it up
 *  prior to exiting.
 * 
 *  Returns:
 * 
 *      OK:       The client sent the `exit` command.  Get ready to connect
 *                another client. 
 *      OK_EXIT:  The client sent `stop-server` command to terminate the server
 * 
 *      ERR_RDSH_COMMUNICATION:  A catch all for any socket() related send
 *                or receive errors. 
 */
int exec_client_requests(int cli_socket) {
    command_list_t cmd_list = {0};
    int rc = 0;
    int localFun;
    int exit = 0;
    int bytes_rcvd = 0;
    int is_last_chunk;
    char *recv_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (recv_buff == NULL) {
        perror("Malloc in Server failed - recv_buff");
        return(ERR_RDSH_SERVER);
    }

    while (1) {
        //sanitize server recv_buff
        memset(recv_buff, SPACE_CHAR, sizeof(RDSH_COMM_BUFF_SZ));
        //resolves to bytes_rcvd += bytes from client
        while((bytes_rcvd += recv(cli_socket, recv_buff + bytes_rcvd, sizeof(recv_buff),0)) > 0) {
            if (bytes_rcvd < 0){
                perror("RECV in Server failed");
                rc = ERR_RDSH_COMMUNICATION;
                break;
            }
            if (bytes_rcvd == 0){
                rc = OK;
                break;
            }
            //will need to confirm this is working as expected
            is_last_chunk = ((char)recv_buff[bytes_rcvd-1] == '\0') ? 1 : 0;
            if (is_last_chunk) {
                break;
            }
        }
        //we use the recv buffer here so it's no longer reliable to send back
        rc = build_cmd_list(recv_buff, &cmd_list);
        if (rc != OK) {
            rc = ERR_RDSH_SERVER;
            break;
        }
        if (cmd_list.num > 1) {
            rc = rsh_execute_pipeline(cli_socket, &cmd_list);
            printf(RCMD_MSG_SVR_EXEC_REQ, cmd_list.commands[0].argv[0]);
        } else {
            if (cmd_list.num == 1) {
                rc = rsh_built_in_cmd(&cmd_list.commands[0]);
                if ((rc == OK) || (rc == OK_EXIT)) {
                    //0 is OK and normal EXIT but client can have
                    //multiple commands to send within one connection
                    //so added this check
                    exit = -1;
                }
            }
        }
        //send and cleanup before checking rc
        bytes_rcvd = 0;
        if (cmd_list.num == 1) {
            localFun = send_message_eof(cli_socket);
            localFun = clear_cmd_buff(&cmd_list.commands[0]);
        } else {
            for (int i = 0; i < cmd_list.num; i++) {
                if (i == cmd_list.num - 1) {
                    localFun = send_message_string(cli_socket, cmd_list.commands[i]._cmd_buffer);
                }
                localFun = clear_cmd_buff(&cmd_list.commands[i]);
            }
        }
        cmd_list.num = 0;
        //break only on error or exit
        if ((rc < 0) || (exit < 0)) {
            break;
        }
    }
    localFun = free_cmd_list(&cmd_list);
    if (localFun) {
        localFun = 0;
    }
    //additional protection if needed
    if (recv_buff) {
        free(recv_buff);
    }
    //processing switch for easier error control
    switch (rc) {
        case OK:
            return OK;
        case ERR_RDSH_COMMUNICATION:
            return ERR_RDSH_COMMUNICATION;
        case ERR_RDSH_SERVER:
            return ERR_RDSH_SERVER;
        case ERR_RDSH_CMD_EXEC:
            return ERR_RDSH_CMD_EXEC;
        case OK_EXIT:
        default:
            break;
    }
    return OK_EXIT;
}

/*
 * send_message_eof(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client

 *  Sends the EOF character to the client to indicate that the server is
 *  finished executing the command that it sent. 
 * 
 *  Returns:
 * 
 *      OK:  The EOF character was sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the EOF character. 
 */
int send_message_eof(int cli_socket){
    int rc;
    rc = send(cli_socket, &RDSH_EOF_CHAR, 1, 0);
    if (rc < 1) {
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}

/*
 * send_message_string(cli_socket, char *buff)
 *      cli_socket:  The server-side socket that is connected to the client
 *      buff:        A C string (aka null terminated) of a message we want
 *                   to send to the client. 
 *   
 *  Sends a message to the client.  Note this command executes both a send()
 *  to send the message and a send_message_eof() to send the EOF character to
 *  the client to indicate command execution terminated. 
 * 
 *  Returns:
 * 
 *      OK:  The message in buff followed by the EOF character was 
 *           sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the message followed by the EOF character. 
 */
int send_message_string(int cli_socket, char *buff){
    int rc;
    char *msg = buff;
    int msgLen = strlen(msg);
    rc = send(cli_socket,msg,msgLen,0);
    if (rc < 0) {
        return ERR_RDSH_COMMUNICATION;
    }
    send_message_eof(cli_socket);
    return OK;
}


/*
 * rsh_execute_pipeline(int cli_sock, command_list_t *clist)
 *      cli_sock:    The server-side socket that is connected to the client
 *      clist:       The command_list_t structure that we implemented in
 *                   the last shell. 
 *   
 *  This function executes the command pipeline.  It should basically be a
 *  replica of the execute_pipeline() function from the last deliverable. 
 *  The only thing different is that you will be using the cli_sock as the
 *  main file descriptor on the first executable in the pipeline for STDIN,
 *  and the cli_sock for the file descriptor for STDOUT, and STDERR for the
 *  last executable in the pipeline.  See picture below:  
 * 
 *      
 *┌───────────┐                                                    ┌───────────┐
 *│ cli_sock  │                                                    │ cli_sock  │
 *└─────┬─────┘                                                    └────▲──▲───┘
 *      │   ┌──────────────┐     ┌──────────────┐     ┌──────────────┐  │  │    
 *      │   │   Process 1  │     │   Process 2  │     │   Process N  │  │  │    
 *      │   │              │     │              │     │              │  │  │    
 *      └───▶stdin   stdout├─┬──▶│stdin   stdout├─┬──▶│stdin   stdout├──┘  │    
 *          │              │ │   │              │ │   │              │     │    
 *          │        stderr├─┘   │        stderr├─┘   │        stderr├─────┘    
 *          └──────────────┘     └──────────────┘     └──────────────┘   
 *                                                      WEXITSTATUS()
 *                                                      of this last
 *                                                      process to get
 *                                                      the return code
 *                                                      for this function       
 * 
 *  Returns:
 * 
 *      EXIT_CODE:  This function returns the exit code of the last command
 *                  executed in the pipeline.  If only one command is executed
 *                  that value is returned.  Remember, use the WEXITSTATUS()
 *                  macro that we discussed during our fork/exec lecture to
 *                  get this value. 
 */
int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    int rc = 0;
    int numComs = clist->num;
    int pipes[numComs - 1][2];  // Array of pipes
    int pids;        // Array to store process IDs

    // Create pipes
    for (int i = 0; i < (numComs - 1); i++) {
        if (pipe(pipes[i]) == -1) {
            printf("Error with pipe\n");
            return ERR_RDSH_SERVER;
        }
    }
    // Create processes for each command
    for (int i = 0; i < numComs; i++) {
        pids = fork();
        if (pids == -1) {
            perror("Error with fork\n");
            return ERR_RDSH_SERVER;
        }
        //child process executions
        if (pids == 0) { 
            // Set up input pipe for all except first process
            if (i > 0) {
                rc = dup2(pipes[i-1][0], STDIN_FILENO);
                if (rc < 0) {
                    perror("Error with dup2\n");
                    exit(ERR_MEMORY);
                }
            }
            // Set up output pipe for all except last process
            if (i != numComs - 1) {
                rc = dup2(pipes[i][1], STDOUT_FILENO);
                if (rc < 0) {
                    perror("Error with dup2\n");
                    exit(ERR_MEMORY);
                }
            } else {
                rc = dup2(cli_sock, STDOUT_FILENO);
                if (rc < 0) {
                    perror("Error with dup2\n");
                    exit(ERR_MEMORY);
                }
                close(cli_sock);
            }
            // Close all pipe ends in child
            for (int j = 0; j < (numComs - 1); j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            //can't re-use exec_cmd here due to fork logic
            rc = execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            if (rc < 0) {
                perror("Error with execvp\n");
                exit(ERR_MEMORY);
            }
            //exit all child processes after Executing + Closing
            exit(rc);
        }
    }
    // Parent process: close all pipe ends
    for (int i = 0; i < (numComs - 1); i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    //close(cli_sock);
    // Wait for all children
    for (int i = 0; i < numComs; i++) {
        waitpid(pids, &pids, 0);
        rc = WEXITSTATUS(pids);
    }

    return rc;
}

/**************   OPTIONAL STUFF  ***************/
/****
 **** NOTE THAT THE FUNCTIONS BELOW ALIGN TO HOW WE CRAFTED THE SOLUTION
 **** TO SEE IF A COMMAND WAS BUILT IN OR NOT.  YOU CAN USE A DIFFERENT
 **** STRATEGY IF YOU WANT.  IF YOU CHOOSE TO DO SO PLEASE REMOVE THESE
 **** FUNCTIONS AND THE PROTOTYPES FROM rshlib.h
 **** 
 */

/*
 * rsh_match_command(const char *input)
 *      cli_socket:  The string command for a built-in command, e.g., dragon,
 *                   cd, exit-server
 *   
 *  This optional function accepts a command string as input and returns
 *  one of the enumerated values from the BuiltInCmds enum as output. For
 *  example:
 * 
 *      Input             Output
 *      exit              BI_CMD_EXIT
 *      dragon            BI_CMD_DRAGON
 * 
 *  This function is entirely optional to implement if you want to handle
 *  processing built-in commands differently in your implementation. 
 * 
 *  Returns:
 * 
 *      BI_CMD_*:   If the command is built-in returns one of the enumeration
 *                  options, for example "cd" returns BI_CMD_CD
 * 
 *      BI_NOT_BI:  If the command is not "built-in" the BI_NOT_BI value is
 *                  returned. 
 */
Built_In_Cmds rsh_match_command(const char *input)
{
    if (strcmp(input,EXIT_CMD) == 0) {
        return BI_CMD_EXIT;
    } else if (strcmp(input,"dragon") == 0) {
        return BI_CMD_DRAGON;
    } else if (strcmp(input,"cd") == 0) {
        return BI_CMD_CD;
    } else if (strcmp(input,"stop-server") == 0) {
        return BI_CMD_STOP_SVR;
    } else {
        return BI_NOT_BI;
    };
}

/*
 * rsh_built_in_cmd(cmd_buff_t *cmd)
 *      cmd:  The cmd_buff_t of the command, remember, this is the 
 *            parsed version fo the command
 *   
 *  This optional function accepts a parsed cmd and then checks to see if
 *  the cmd is built in or not.  It calls rsh_match_command to see if the 
 *  cmd is built in or not.  Note that rsh_match_command returns BI_NOT_BI
 *  if the command is not built in. If the command is built in this function
 *  uses a switch statement to handle execution if appropriate.   
 * 
 *  Again, using this function is entirely optional if you are using a different
 *  strategy to handle built-in commands.  
 * 
 *  Returns:
 * 
 *      BI_NOT_BI:   Indicates that the cmd provided as input is not built
 *                   in so it should be sent to your fork/exec logic
 *      BI_EXECUTED: Indicates that this function handled the direct execution
 *                   of the command and there is nothing else to do, consider
 *                   it executed.  For example the cmd of "cd" gets the value of
 *                   BI_CMD_CD from rsh_match_command().  It then makes the libc
 *                   call to chdir(cmd->argv[1]); and finally returns BI_EXECUTED
 *      BI_CMD_*     Indicates that a built-in command was matched and the caller
 *                   is responsible for executing it.  For example if this function
 *                   returns BI_CMD_STOP_SVR the caller of this function is
 *                   responsible for stopping the server.  If BI_CMD_EXIT is returned
 *                   the caller is responsible for closing the client connection.
 * 
 *   AGAIN - THIS IS TOTALLY OPTIONAL IF YOU HAVE OR WANT TO HANDLE BUILT-IN
 *   COMMANDS DIFFERENTLY. 
 */
Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd)
{
    int rc = 1;
    Built_In_Cmds exe;
    exe = rsh_match_command(cmd->argv[0]);
    switch (exe) {
        case BI_CMD_CD:
            if (cmd->argc == 1) {
            } else if (cmd->argc > 2) {
                printf("Error - Multiple arguments to 'cd'\n");
                rc = ERR_RDSH_CMD_EXEC;
                break;
            } else {
                rc = chdir(cmd->argv[1]);
                if (rc < 0) {
                    printf("Error with built-in command\n");
                }
            }
            return BI_EXECUTED;
        case BI_CMD_DRAGON:
            print_dragon();
            return BI_EXECUTED;
        case BI_CMD_EXIT:
            return OK;
        case BI_CMD_STOP_SVR:
            return OK_EXIT;
        //if not built-in command, fork/exec as an external command
        case BI_NOT_BI:
            rc = exec_cmd(cmd);
            if (rc != OK) {
                printf("Single command execution failed.\n");
                rc = ERR_RDSH_CMD_EXEC;
                break;
            }
            return BI_NOT_BI;
        default:
            break;
    }
    return rc;
}
