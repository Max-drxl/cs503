#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

void fullTrim(char *user_str);
void leftTrim(char *user_str);
/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    char *ptr = NULL;
    char *token = NULL;
    int comCounter = 0;
    int comLength = 0;
    command_list_t *comList = clist;
    char *cmd_input = cmd_line;
    
    //need to populate comList with commands up to CMD_MAX = 8
    //each pipe represents a new command
    //strtok with pipe delimiter to find all commands in string
    if (strchr(cmd_input, PIPE_CHAR) != NULL) {
        token = strtok(cmd_input, PIPE_STRING);
        while (token != NULL) {
            comList->num = comCounter;
            if (comCounter >= CMD_MAX) {
                return(ERR_TOO_MANY_COMMANDS);
            }
            fullTrim(token);
            //need to process token first for exe
            //strchr finds a char in str and returns pointer
            if (strchr(token, SPACE_CHAR) == NULL) {
                //strlen to check if it's greater than allowable size
                if (strlen(token) > EXE_MAX) {
                    return(ERR_CMD_OR_ARGS_TOO_BIG);
                }
                //strcpy to copy execute to exe and arguments to arg
                strcpy(comList->commands[comCounter].exe, token);
            } else {
                //space after command represents args
                if (strchr(token, SPACE_CHAR) != NULL) {
                    //create an open string for pointer arithmetic
                    char *exes = malloc(EXE_MAX);
                    if (exes == NULL) {
                        printf("Error allocating memory for buffer, exes == NULL\n");
                        exit(2);
                    }
                    memset(exes, SPACE_CHAR, EXE_MAX);
                    //space exists so we can make ptr
                    ptr = strchr(token, SPACE_CHAR);
                    //load token into exes to store command
                    while (*token != *ptr) {
                        *exes = *token;
                        exes++;
                        token++;
                        comLength++;
                    }
                    exes = exes - comLength;
                    fullTrim(exes);
                    //check if it's too big
                    if (strlen(exes) > EXE_MAX) {
                        return(ERR_CMD_OR_ARGS_TOO_BIG);
                    }
                    //strcpy to copy exes to exe
                    strcpy(comList->commands[comCounter].exe, exes);
                    comLength = 0;
                    //if we malloc we free
                    exes = NULL;
                    free(exes);
                    //args only need left trim
                    leftTrim(ptr);
                    //check if it's too big
                    if (strlen(ptr) > ARG_MAX) {
                        return(ERR_CMD_OR_ARGS_TOO_BIG);
                    }
                    //strcpy to copy ptr to arg
                    strcpy(comList->commands[comCounter].args, ptr);
                }
            }
            //keep processing tokens
            token = strtok(NULL, PIPE_STRING);
            comCounter++;
        }
    } else {
        //basically the same code here as the else statement without token
        if (strchr(cmd_input, SPACE_CHAR) != NULL) {
            //create an open string for pointer arithmetic
            char *exes = malloc(EXE_MAX);
            if (exes == NULL) {
                printf("Error allocating memory for buffer, exes == NULL\n");
                exit(2);
            }
            memset(exes, SPACE_CHAR, EXE_MAX);

            ptr = strchr(cmd_input, SPACE_CHAR);
            while (*cmd_input != *ptr) {
                *exes = *cmd_input;
                exes++;
                cmd_input++;
                comLength++;
            }
            exes = exes - comLength;
            fullTrim(exes);
            //check if it's too big
            if (strlen(exes) > EXE_MAX) {
                return(ERR_CMD_OR_ARGS_TOO_BIG);
            }
            strcpy(comList->commands[comCounter].exe, exes);
            comLength = 0;
            //if we malloc we free
            exes = NULL;
            free(exes);
            //args only need left trim
            leftTrim(ptr);
            //check if it's too big
            if (strlen(ptr) > ARG_MAX) {
                return(ERR_CMD_OR_ARGS_TOO_BIG);
            }
            //strcpy to copy ptr to arg
            strcpy(comList->commands[comCounter].args, ptr);
        } else {
            //check if it's too big
            if (strlen(cmd_input) > EXE_MAX) {
                return(ERR_CMD_OR_ARGS_TOO_BIG);
            }
            //strcpy to copy cmd_input to exe
            strcpy(comList->commands[comCounter].exe, cmd_input);
        }
    }
    return(OK);
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