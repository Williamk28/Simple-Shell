#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include<sys/wait.h> 
#define MAX_COMMAND_LENGTH 512
#define MAX_TOK_NO 50
#define TOK_DELIM " \t|><&;\n"
#define DEBUG

typedef struct {
    char *cwd;
    char *user;
} EnvVars;

void initShell(EnvVars *envVars);
void loopShell(EnvVars *envVars);
char *readInput();
char **tokeniseInput(char *input);
int executeCommand(char **args, EnvVars *envVars);
int getPath();
int setPath(char* str);
int execExternal(char **args);
