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
#define HISTORY_FILE ".hist_list"
#define ALIAS_FILE ".aliases"
#define DEBUG

typedef struct Alias {
    char alias_name[20];
    char alias_command[MAX_COMMAND_LENGTH];
} Alias;

typedef struct Env_vars {
    int alias_no;
    char *cwd;
    char *user;
    char *path;
    Alias aliases[10];
} Env_vars;

void init_shell(Env_vars *env_vars);
int load_history();
int load_aliases(Env_vars *env_vars);

void loop_shell(Env_vars *env_vars);
char *read_input();
void add_history(char *input);
char **tokenise_input(char *input, Env_vars *env_vars);
void execute_alias(char **arg, Env_vars *env_vars);

void execute_command(char **args, Env_vars *env_vars);
void get_path(char **args);
void set_path(char **args);
void change_dir(char **args, Env_vars *env_vars);

void history(char **args);
void exec_history(char **args, Env_vars *env_vars);
void history_error_decrement();
void history_invokation_check_normal_case(int value, Env_vars *env_vars);
void history_invokation_check_edge_case(int value, Env_vars *env_vars);

void add_alias(char **arg, Env_vars *env_vars);
void print_aliases(Env_vars *env_vars);
void remove_alias(char **arg, Env_vars *env_vars);
int exec_external(char **args);

int save_history();
int save_aliases(Env_vars *env_vars);
void exit_shell(int exit_code, Env_vars *env_vars);