#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SZ 50
#define SPACE_CHAR ' '
#define NULL_TERM '\0'

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int);
//add additional prototypes here
//helpers
void space_strip(char *);
int  string_length(char *, int);
//prototypes
void reverse_string(char *, int);
void word_print(char *, int);
void string_replace(char *, int, int, char *, char *);

int setup_buff(char *buff, char *user_str, int len){
    //TODO: #4:  Implement the setup buff as per the directions
    char filler = '.';
    int length = 0;

    //set all chars in buff to periods first
    memset(buff, filler, len);
    //call first helper function, space_strip
    space_strip(user_str);
    //call second helper function, string_length
    length = string_length(user_str, len);
    //ensuring we don't memcpy with length as an error
    if (length < 1) {
        return length;
    }
    //update first x (length) number of chars from string
    memcpy(buff, user_str, length);
    return length;
}

void print_buff(char *buff, int len){
    printf("Buffer: ");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);
}

int count_words(char *buff, int str_len){
    //YOU MUST IMPLEMENT
    int word_start = 0;
    int wc = 0;
    char *buff_chars = buff;
    char *end_str = buff + str_len;

    //iterate over the pointers in buff until reaching length of str
    //basically the same code as before just without array notation
    //and don't need to check for doublespaces
    while (buff_chars < end_str) {
        if (word_start < 1) {
            if (*buff_chars != SPACE_CHAR) {
                wc++;
                word_start = 1;
            }
        } else {
            if (*buff_chars == SPACE_CHAR) {
                word_start = 0;
            }
        }
        buff_chars++;
    }
    return wc;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS
//best example of pointer manipulation, overloading comments for me
void space_strip(char *user_str){
    //create 2 new pointer variables that are loaded with the string
    char *new_str = user_str;
    char *old_str = user_str;
    int spaceCounter = 0;
    char tab_char = '\t';

    //loop until we hit the null termination char in old
    while (*old_str != NULL_TERM) {
        //set tabs equal to spaces
        if (*old_str == tab_char) {
            *old_str = SPACE_CHAR;
        }
        //default case, if not a space set *new pointer equal to
        //current *old pointer, increment *new pointer to next char
        if (*old_str != SPACE_CHAR) {
            *new_str = *old_str;
            new_str++;
            spaceCounter = 0;
        } else {
            //we hit a space char, set current *new pointer equal to
            //a space, increment *new pointer to next char, space = 1
            if (spaceCounter < 1) {
                *new_str = SPACE_CHAR;
                new_str++;
                spaceCounter = 1;
            }
        }
        //increment *old pointer to next char. when space = 1 we do not
        //increment the *new pointer, meaning old will become x char
        //ahead of new based on number of consecutive spaces found.
        //when we next set *new=*old new will have skipped the 
        //consecutive spaces
        old_str++;
    }
    //ensure our last pointer is null-terminated
    *new_str = NULL_TERM;
}

int string_length(char *user_str, int len){
    int length = 0;

    //loop until we hit the null termination char
    while (*user_str != NULL_TERM) {
        //ensure we don't allocate over our limit
        if (length > len) {
            printf("The user-supplied string is too large.\n");
            return -1;
        }
        length++;
        user_str++;
    }
    return length;
}

void reverse_string(char *buff, int str_len){
    //implement
    char temp;
    //could also do this by traversing like a linked list, but
    //we have the length so should take the shortcut
    char *end = buff + str_len - 1;
    
    while (buff < end) {
        //set temp to first pointer char, set first pointer char to 
        //end pointer char, then set end pointer char to temp char
        temp = *buff;
        *buff = *end;
        *end = temp;
        buff++;
        end--;
    }
}

void word_print(char *buff, int str_len){
    int word_start = 0;
    int wc = 0;
    char *buff_chars = buff;
    int wlen = 0;
    char *end_str = buff_chars + str_len;

    //same start as count_words -c however need to also 
    //keep track of length of words and print things out
    //i could re-use string_length here but that's more complicated
    while (buff_chars < end_str) {
        if (word_start < 1) {
            if (*buff_chars != SPACE_CHAR) {
                wc++;
                word_start = 1;
                printf("\n%d. ", wc);
            }
        } else {
            if (*buff_chars == SPACE_CHAR) {
                printf(" (%d)", wlen);
                word_start = 0;
                wlen = -1;
            }
        }
        //incrementing until we hit length of users' string
        printf("%c", *buff_chars);
        wlen++;
        buff_chars++;
    }
    //loop ends before last length so added here
    printf(" (%d)\n", wlen);
}

void string_replace(char *buff, int len, int str_len, char *sub_str, char *replacer){
    //implement
    char *buff_find_chars = buff;
    char *buff_rep_chars;
    char *match_test = NULL;
    char *sub_str_chars = sub_str;
    char *sub_match_test = NULL;
    char *rep = replacer;
    int matchFound;
    char *end_str = buff_find_chars + str_len;
    char *end_ptr = buff_find_chars + len;

    //couldn't find another way to do this without an additional malloc
    buff_rep_chars = malloc(len);
    if (buff_rep_chars == NULL) {
        printf("Error allocating memory for buffer, buff_rep_chars == NULL\n");
        //readme says to use exit(2) to indicate memory allocation fail
        exit(2);
    }
    memcpy(buff_rep_chars,buff,len);

    //re-use string_length function here to find length of sub_str
    int sub_str_len = string_length(sub_str,str_len);
    int replacer_len = string_length(replacer,str_len);

    //iterate through the users' string to find the match pointer position
    //and get all my pointers in the right places for replacement
    while (buff_find_chars < end_str) {
        //potential match
        if (*buff_find_chars == *sub_str_chars) {
            //test pointer to confirm all characters match
            match_test = buff_rep_chars;
            sub_match_test = sub_str_chars;
            matchFound = 1;
            //go until we hit end of string length
            while (sub_match_test < sub_str + sub_str_len) {
                //if test matches substr keep going
                //using test pointers so i don't need to rewind anything
                if (*match_test != *sub_match_test) {
                    matchFound = 0;
                    break;
                }
                match_test++;
                sub_match_test++;
            }
            //*match_test now has the rest of the users' string + filler 
            //after the sub_str
            //*buff_chars now is starting at the beginning of the sub_str
            if (matchFound == 1) {
                sub_match_test = NULL;

                //now i basically want to do:
                //buff_find_chars = replacer + match_test until len=50
                //which will truncate to 50 and prevent errors
                while (rep < replacer + replacer_len) {
                    if (buff_find_chars < end_ptr) {
                        *buff_find_chars = *rep;
                        buff_find_chars++;
                        rep++;
                    } else {
                        break;
                    }

                }
                while (buff_find_chars < end_ptr) {
                    *buff_find_chars = *match_test;
                    match_test++;
                    buff_find_chars++;
                }
                break;
            }
        }
        buff_find_chars++;
        buff_rep_chars++;
    }
    //exit now if no matching string found from provided input
    if (matchFound < 1) {
        printf("Error: no matching substring found.\n");
        exit(3);
    }
    buff_rep_chars = NULL;
    free(buff_rep_chars);
    printf("Modified String: %s\n", buff);
    return;
}

int main(int argc, char *argv[]){

    char *buff;             //placeholder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    //This is safe because of the logical OR || short-circuit of C and Java so 
    //if no commands were passed it'll exit(1) regardless due to argc<2 
    //indicating a command line error e.g. lack of commands. If a command is 
    //passed then it's now safe to check for the first character to be a hyphen, 
    //which if it is not a hyphen it'll error out for command line formatting.
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        printf("Error: No argument or missing hyphen, argv[1] = %s\n", argv[1]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    //If we receive <3 items in argc then we must be missing either 
    //the command arguments or the string to operate on. We've already
    //checked above for issues with the command arguments re: hyphens
    //so now if we're missing the string to perform functions it'll
    //exit(1) again due to command line error e.g. no string provided.
    if (argc < 3){
        usage(argv[0]);
        printf("Error: No string provided for argument, argv[2] = %s\n", argv[2]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    buff = malloc(BUFFER_SZ);
    if (buff == NULL) {
        printf("Error allocating memory for buffer, buff == NULL\n");
        //readme says to use exit(2) to indicate memory allocation fail but 
        //this TODO says to use return code 99 instead, following TODO here
        exit(99);
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d\n", user_str_len);
        if (user_str_len == -1){
            printf("The user-supplied string is too large.\n");
        }
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d\n", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;
        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
        case 'r':
            reverse_string(buff, user_str_len);
            //added length to printf here to stop the filler from printing
            printf("Reversed string: %.*s\n", user_str_len, buff);
            break;
        case 'w':
            printf("Word Print\n----------");
            word_print(buff, user_str_len);
            break;
        case 'x':
            if (argc<5) {
                printf("Missing required arguments!");
                exit(1);
            }
            char *sub_str = argv[3];
            char *replacer = argv[4];
            string_replace(buff, BUFFER_SZ, user_str_len, sub_str, replacer);
            //printf("Not Implemented!\n");
            break;
        default:
            usage(argv[0]);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff,BUFFER_SZ);
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//      PLACE A COMMENT BLOCK HERE EXPLAINING
//It's good practice to provide both the buffer and the length 
//because we can ensure that we do not overrun the buffer size more 
//easily. On top of that having the buffer, the buffer size, and 
//the length of the str inside of the buffer ensures we do not 
//spend additional processing on the internal buffer dot padding
//because we can stop at the known length of the str. In this 
//assignment I didn't use the buffer size inside most of the 
//functions until we reached string_replace. Our setup_buff and
//string_length helpers use the buffer size to ensure the string
//from the user is 50 chars or less so we don't need to worry about 
//it until we're replacing things in the string, then we run the 
//risk of going over 50 but I also truncate to 50 chars instead of 
//erroring.