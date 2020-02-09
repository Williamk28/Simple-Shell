#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include<sys/wait.h> 
#define STR_LIMIT 512
#define DEBUG

void initShell();
void getInput(char *str);
char **tokenise(char *str);
void commandHandler(char **args);
void setPath(char* str);
void getPath();
void changeDir(char* path);
void execArgs(char **args);