#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h> 
#include <sys/stat.h>
#include "print_colours_head.h"
#define MAX_COMMAND_LENGTH 512
#define MAX_TOK_NO 50
#define MAX_HIST_NUM 20
#define MAX_ALIAS_NUM 10
#define TOK_DELIM " \t\n;&><|"
#define MEM_ALLOC_ERROR "Error: Memory unable to be allocated\n"
#define DEBUG
#define HistoryFile "./.HistoryFile"

typedef struct History {
    int hist_numb;
    char hist_command[MAX_COMMAND_LENGTH];
} History;

typedef struct Alias {
    char alias_name[20];
    char alias_command[MAX_COMMAND_LENGTH];
} Alias;

typedef struct Env_vars {
    int hist_count;
    int alias_no;
    char *cwd;
    char *user;
    char history[MAX_HIST_NUM][MAX_COMMAND_LENGTH];
    Alias aliases[MAX_ALIAS_NUM];
} Env_vars;

void init_shell(Env_vars *env_vars);
void loop_shell(Env_vars *env_vars);
char *read_input();
void add_history(char *input, Env_vars *env_vars);
char **tokenise_input(char *input);
void execute_command(char **args, Env_vars *env_vars);
void get_path(char **args);
void set_path(char **args);
void change_dir(char **args, Env_vars *env_vars);
void history(Env_vars *env_vars);
int exec_external(char **args);
int exec_history(char *input, Env_vars *env_vars);
int write_history_tofile();
int LoadHistory();
void addAlias(char **arg, Env_vars *env_vars);
void printAliases(Env_vars *env_vars);
void execute_alias(char **arg, Env_vars *env_vars);
void removeAlias(char **arg, Env_vars *env_vars);
void exit_shell(Env_vars *env_vars);