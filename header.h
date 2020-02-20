#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "print_colours_head.h"
#define MAX_COMMAND_LENGTH 512
#define MAX_TOK_NO 50
#define TOK_DELIM " \t|><&;\n"
#define MEM_ALLOC_ERROR "Error: Memory unable to be allocated\n"
#define DEBUG

typedef struct Env_vars {
    char *cwd;
    char *user;
} Env_vars;

typedef struct {
    char *alias;
    char *command;
} Alias_Struct;

void init_shell(Env_vars *env_vars);
void loop_shell(Env_vars *env_vars);
char *read_input();
void add_history(char *input);
char **tokenise_input(char *input);
void execute_command(char **args, Env_vars *env_vars);
int getPath();
void set_path(char* str);
void change_dir(char* path, Env_vars *env_vars);
void history();
int exec_external(char **args);
void exec_history(char **args, Env_vars *env_vars);
void addAlias(char **arg);
void printAliases();
//Defining history structure
   int Hist_numb;
   char hist[20][512];

int NumOfAliases = 0;
Alias_Struct Aliases[10];