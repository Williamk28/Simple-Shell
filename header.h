#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> 
#include <sys/stat.h>
#include "print_colours_head.h"
#define MAX_COMMAND_LENGTH 512
#define MAX_TOK_NO 50
#define TOK_DELIM " \t|><&;\n"
#define MEM_ALLOC_ERROR "Error: Memory unable to be allocated\n"
#define DEBUG
#define HistoryFile "./HistoryFile.txt"

typedef struct Alias {
    char alias_name[20];
    char alias_command[MAX_COMMAND_LENGTH];
} Alias;

typedef struct Env_vars {
    int alias_no;
    char *cwd;
    char *user;
    Alias aliases[10];
} Env_vars;

void init_shell(Env_vars *env_vars);
void loop_shell(Env_vars *env_vars);
char *read_input();
void add_history(char *input);
char **tokenise_input(char *input);
void execute_command(char **args, Env_vars *env_vars);
void get_path(char **args);
void set_path(char **args);
void change_dir(char **args, Env_vars *env_vars);
void history();
int exec_external(char **args);
void exec_history(char **args, Env_vars *env_vars);
int write_history_tofile();
int LoadHistory();
void AddHistory(char *line);
void add_alias(char **arg, Env_vars *env_vars);
void print_aliases(Env_vars *env_vars);
void execute_alias(char **arg, Env_vars *env_vars);
void remove_alias(char **arg, Env_vars *env_vars);
void exit_shell(Env_vars *env_vars);
//Defining history structure
   int Hist_numb;
   char hist[20][512];
