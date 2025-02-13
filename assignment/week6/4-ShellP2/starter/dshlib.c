#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"
#include <errno.h>

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */

//my dshlib.h is edited to include prototypes

int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    //only malloc this so we don't run into dupe free errors
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (cmd_buff->_cmd_buffer == NULL) {
        printf("Error allocating memory for buffer, cmd_buff == NULL\n");
        return(ERR_MEMORY);
    }
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    //single free for single malloc
    free(cmd_buff->_cmd_buffer);
    return OK;
}

int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    //set argv to null term first using argc
    for (int i = 0; i < cmd_buff->argc; i++) {
        *cmd_buff->argv[i] = '\0';
    }
    //set argc to 0 now and _cmd_buffer to null term
    cmd_buff->argc = 0;
    *cmd_buff->_cmd_buffer = '\0';
    return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    cmd_buff_t *cmd = cmd_buff;
    char *cmd_input = cmd_line;
    int argCounter = 0;

    //pipes not supported this week
    if (strchr(cmd_input, PIPE_CHAR) != NULL) {
        printf(CMD_ERR_PIPE_LIMIT, 1);
        return ERR_TOO_MANY_COMMANDS;
    }
    fullTrim(cmd_input);
    //command + arg(s)
    while (*cmd_input != '\0') {
        //exit loop if too many commands
        if (argCounter == CMD_MAX) {
            printf("Too many arguments!\n");
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }
        //found quote
        if (*cmd_input == '\"') {
            cmd_input++;
            if (strlen(cmd_input) > ARG_MAX) {
                printf("Too many characters in argument!\n");
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            cmd->argv[argCounter] = cmd_input;
            while (*cmd_input != '\0' && *cmd_input != '\"') {
                cmd_input++;
            }
            if (*cmd_input == '\"') {
                *cmd_input = '\0';
                cmd_input++;
            }
            argCounter++;
        }
        //break on null term
        if (*cmd_input == '\0') {
            break;
        }
        //found normal arg
        if (*cmd_input != SPACE_CHAR) {
            if (strlen(cmd_input) > ARG_MAX) {
                printf("Too many characters in argument!\n");
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            cmd->argv[argCounter] = cmd_input;
            while (*cmd_input != '\0' && *cmd_input != SPACE_CHAR) {
                cmd_input++;
            }
            if (*cmd_input != '\0') {
                *cmd_input = '\0';
                cmd_input++;
            }
            argCounter++;
        //skip leading spaces in normal args, don't increment argCounter
        } else {
            while (*cmd_input == SPACE_CHAR) {
                cmd_input++;
            }
            cmd_input--;
            *cmd_input = '\0';
            cmd_input++;
        }
        //increment argc
        cmd->argc++;
    }
    //array ends in null for execvp, use CMD_MAX above to ensure there is 
    //always a spot available in argv for NULL due to CMD_ARGV_MAX
    cmd->argv[argCounter] = NULL;
    return OK;
}

int exec_local_cmd_loop()
{
    char *cmd_buff;
    int rc = 0;
    cmd_buff_t cmd = {0};
    Built_In_Cmds exe;

    //MAIN LOOP
    rc = alloc_cmd_buff(&cmd);
    if (rc != OK) {
        exit(ERR_MEMORY);
    }
    while (1)
    {
        rc = 0;
        printf("%s", SH_PROMPT);
        cmd_buff = cmd._cmd_buffer;
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
        //parsing input to cmd_buff_t *cmd_buff
        if (strlen(cmd_buff) < 1) {
            printf(CMD_WARN_NO_CMD);
            rc = WARN_NO_CMDS;
            continue;
        } else {
            rc = build_cmd_buff(cmd_buff, &cmd);
            if (rc != OK) {
                break;
            }
        }
        //if built-in command, execute builtin logic for exit, cd, dragon
        exe = match_command(cmd.argv[0]);
        switch (exe) {
            case BI_CMD_CD:
                if (cmd.argc == 1) {
                    rc = OK;
                } else if (cmd.argc > 2) {
                    rc = ERR_TOO_MANY_COMMANDS;
                } else {
                    rc = exec_built_in_cmd(&cmd);
                }
                break;
            case BI_CMD_DRAGON:
                print_dragon();
                break;
            case BI_CMD_EXIT:
                exit(OK_EXIT);
                break;
            //if not built-in command, fork/exec as an external command
            case BI_NOT_BI:
                rc = exec_cmd(&cmd);
                break;
            default:
                break;
        }
        //clear after commands executed
        if (cmd.argc > 0) {
            rc = clear_cmd_buff(&cmd);
        }
    }
    //free after loop is completed
    rc = free_cmd_buff(&cmd);
    return OK;
}

Built_In_Cmds match_command(const char *input) {
    if (strcmp(input,EXIT_CMD) == 0) {
        return BI_CMD_EXIT;
    } else if (strcmp(input,"dragon") == 0) {
        return BI_CMD_DRAGON;
    } else if (strcmp(input,"cd") == 0) {
        return BI_CMD_CD;
    } else {
        return BI_NOT_BI;
    }
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    int rc = 0;
    //feels wasteful to move out of the switch unless it's 'cd'
    rc = chdir(cmd->argv[1]);
    if (rc < 0) {
        printf("Error with built-in command");
    }
    return rc;
}

int exec_cmd(cmd_buff_t *cmd) {
    int f_result;
    int c_result;
    int rc = 0;
    //this is basically the same as the fork-exec.c demo file
    f_result = fork();
    if (f_result < 0) {
        printf("Error with fork\n");
        exit(1);
    }

    if (f_result == 0) {
        rc = execvp(cmd->argv[0],cmd->argv);
        if (rc < 0) {
            printf("Error with execvp\n");
            exit(1);
        }
    } else {
        wait(&c_result);
    }
    return OK;
}

void fullTrim(char *user_str){
    int strLen = strlen(user_str);
    //move to end of trim -1 because null term
    char* trim = user_str + strLen -1;
    //rtrim functionality to replace any space with null term
    while(*trim == SPACE_CHAR) {
        *trim = '\0';
        trim--;
    }
    //move to start of trim
    while(*trim) {
        trim--;
    }
    trim++;
    leftTrim(trim);
}

void leftTrim(char *user_str){
    int spaceCount = 0;
    char* trim = user_str;

    //ltrim functionality only if needed
    while (*trim == SPACE_CHAR) {
        spaceCount++;
        trim++;
    }
    if (spaceCount > 0) {
        strcpy(user_str, trim);
    }
}