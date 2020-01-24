#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#define STR_LIMIT 512
#define PROMPT "> "
#define DELIMITER " \t|><&;\n"
#define HOME getenv("HOME")
#define DEBUG

char *cwd;
int getCommand()
{
    //Variable to store users input. Set to hold a max of 512 characters
    char str[STR_LIMIT];
    //Print out "> " and wait for user input
    printf(PROMPT);
    char *input = fgets(str, STR_LIMIT, stdin);
    //Check for ctrl + D input
    if (input == NULL)
    {
        printf("\n");
        exit(0);
    }
    //Tokenise user input using Delimeters
    char *token = strtok(str, DELIMITER);
    while (token != NULL)
    {
        //Check if user inputs exit command
        if (strcmp("exit", token) == 0)
        {
            exit(0);
        }
        //Check that tokens are correct
        #ifdef DEBUG
        printf("\"");
        printf("%s", token);
        printf("\"\n");
        #endif
        token = strtok(NULL, DELIMITER);
    }
    return 1;
}

int main()
{
    //Set working directory to users home directory
    cwd = HOME;
    //Loop until the shell is terminated
    do
    {
        //Print current directory
        printf("Dir: %s", cwd);
        //Get user input
        getCommand();
    } while (1);
}
