#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

void fullTrim(char *user_str);
void leftTrim(char *user_str);
void space_strip(char *user_str);
int build_cmd_w_args(char *comString, command_list_t *comList, int comCounter);
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
    char *comString = NULL;
    int comCounter = 0;
    command_list_t *comList = clist;
    char *cmd_input = cmd_line;
    int rc = 0;
    
    if (strchr(cmd_input, PIPE_CHAR) != NULL) {
        comString = strtok(cmd_input, PIPE_STRING);
        fullTrim(comString);
    } else if (comString == NULL && strchr(cmd_input, SPACE_CHAR) != NULL) {
        //comString = full command, no pipes, yes spaces
        comString = cmd_input;
        fullTrim(comString);
        rc = build_cmd_w_args(comString, comList, comCounter);
        if (rc < 0) {
            return(rc);
        }
        return(OK);
    } else {
        //comString = only exes, no pipes, no spaces
        comString = cmd_input;
        fullTrim(comString);
        if (strlen(comString) > EXE_MAX) {
            return(ERR_CMD_OR_ARGS_TOO_BIG);
        }
        //strcpy to copy execute to exe
        strcpy(comList->commands[comCounter].exe, comString);
        return(OK);
    }
    while (comString != NULL) {
        comList->num = comCounter;
        if (comCounter >= CMD_MAX) {
            return(ERR_TOO_MANY_COMMANDS);
        }
        fullTrim(comString);
        if (strchr(comString, SPACE_CHAR) == NULL) {
            //strlen to check if it's greater than allowable size
            if (strlen(comString) > EXE_MAX) {
                return(ERR_CMD_OR_ARGS_TOO_BIG);
            }
            //strcpy to copy execute to exe
            strcpy(comList->commands[comCounter].exe, comString);
        } else {
            rc = build_cmd_w_args(comString, comList, comCounter);
            if (rc < 0) {
                return(rc);
            }
        }
        comString = strtok(NULL, PIPE_STRING);
        comCounter++;
    }
    return(OK);
}

int build_cmd_w_args(char *comString, command_list_t *comList, int comCounter) {
    char *ptr = NULL;
    int comLength = 0;

    char *exes = malloc(EXE_MAX);
    if (exes == NULL) {
        printf("Error allocating memory for buffer, exes == NULL\n");
        exit(2);
    }
    memset(exes, SPACE_CHAR, EXE_MAX);
    ptr = strchr(comString, SPACE_CHAR);
    while (*comString != *ptr) {
        *exes = *comString;
        exes++;
        comString++;
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
    //always left trim args
    leftTrim(ptr);
    //remove any double inner spaces from args as needed
    if (strchr(ptr, SPACE_CHAR) != NULL) {
        space_strip(ptr);
    }
    //check if it's too big
    if (strlen(ptr) > ARG_MAX) {
        return(ERR_CMD_OR_ARGS_TOO_BIG);
    }
    //strcpy to copy ptr to arg
    strcpy(comList->commands[comCounter].args, ptr);
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

void space_strip(char *user_str){
    //space_strip function from stringfun assignment
    char *new_str = user_str;
    char *old_str = user_str;
    int spaceCounter = 0;
    char tab_char = '\t';

    while (*old_str != '\0') {
        if (*old_str == tab_char) {
            *old_str = SPACE_CHAR;
        }
        if (*old_str != SPACE_CHAR) {
            *new_str = *old_str;
            new_str++;
            spaceCounter = 0;
        } else {
            if (spaceCounter < 1) {
                *new_str = SPACE_CHAR;
                new_str++;
                spaceCounter = 1;
            }
        }
        old_str++;
    }
    *new_str = '\0';
}