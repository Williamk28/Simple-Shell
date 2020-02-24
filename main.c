#include "header.h"

void main() {
    
    Env_vars env_vars;

    init_shell(&env_vars);

    loop_shell(&env_vars);
}

void init_shell(Env_vars *env_vars) {
    env_vars->hist_no = 0;
    env_vars->alias_no = 0;
    env_vars->cwd = getenv("HOME");
    env_vars->user = getenv("USER");

    if (0 != chdir(env_vars->cwd)) {
        bred();
        perror("Shell");
        reset_colour();
    }

    #ifdef DEBUG
        printf("Home Directory: %s\n",getcwd(NULL,0));
    #endif
}

void loop_shell(Env_vars *env_vars) {
    int status = 1;
    char *input = malloc(sizeof(char) * MAX_COMMAND_LENGTH);
    char **args;    
    
    while (1) {
        bgreen();
        printf("%s: ", env_vars->user);
        bblue();
        printf("%s> ", env_vars->cwd);
        reset_colour();
        
        input = read_input();
        if (input[0] == '\n') { continue; }
        else {
            if (exec_history(input, env_vars)) {
                args = tokenise_input(input);
                execute_alias(args, env_vars);
            } else { continue; }
        }        

        free(input);
        free(args);
    }
}

char *read_input() {
    char *input = malloc(sizeof(char) * MAX_COMMAND_LENGTH);

    if (!input) {
        bred();
        fprintf(stderr, MEM_ALLOC_ERROR);
        reset_colour();
        exit(0);
    } else if (NULL == fgets(input, MAX_COMMAND_LENGTH, stdin)) {
        printf("\n");
        exit(0);
    } else {
        return input;
    }
}

int exec_history(char *input, Env_vars *env_vars) {
    /*if a history command*/
    if (input[0] == '!') {
        int index;
        
        /*if second character is a number*/
        if (isdigit(input[1])) {
            index = input[1] - '0';

            /*if number has 2 digits*/
            if (isdigit(input[2])) {
                index = (input[2] - '0')*10 + input[1];

                /*if index is out of range of history*/
                if (index > env_vars->hist_no) {
                    bred();
                    printf("history: Index out of range\n");
                    reset_colour();
                    return 0;
                } 
                /*invoke command from history*/
                else {
                    strcpy(input, env_vars->history[index]);
                    return 1;
                }
            } else if (input[2] == '\n') {
                if (index > env_vars->hist_no) {
                    bred();
                    printf("history: Index out of range\n");
                    reset_colour();
                    return 0;
                } else {
                    strcpy(input, env_vars->history[index-1]);
                    return 1;
                }
            }
            /*if third character is an invalid argument*/
            else { 
                bred();
                printf("%s: Not a valid history argument\n", input);
                reset_colour(); 
                return 0;
            } 
        } 
        /*if second character is '-'*/
        else if (input[1] == '-') {
            /*if third character is a number*/
            if (isdigit(input[2])) {
                index = input[2] - '0';

                /*if number has 2 digits*/
                if (isdigit(input[3])) {
                    index = (input[3] - '0')*10 + input[3];
                } 
            } else {
                bred(); 
                printf("%s: Not a valid history argument\n", input); 
                reset_colour();
                return 0;
            }
        }
        /*if second character is '!'*/
        else if (input[1] == '!') {
            /*if third character is an invalid argument*/
            if (input[2] != '\n') { 
                bred();
                printf("%s: Not a valid history argument\n", input);
                reset_colour();
                return 0;
            } 
            /*invoke last command from history*/
            else {
                index = env_vars->hist_no;
                strcpy(input, env_vars->history[index-1]);
                return 1;
            }
        }
        /*if second character is an invalid argument*/
        else { 
            bred();
            printf("%s: Not a valid history argument\n", input); 
            reset_colour();
            return 0;
        }
    }
    /*if not a history command*/
    else {
        add_history(input, env_vars);
        return 1;
    }
}

void add_history(char *input, Env_vars *env_vars) {
    char input2[MAX_COMMAND_LENGTH];
    
    strcpy(input2, input);
    if (env_vars->hist_no < MAX_HIST_NUM) {
        strcpy(env_vars->history[env_vars->hist_no], input); 
        env_vars->hist_no++;
    } else {
        for (int i = 0; i < MAX_HIST_NUM-1; i++) {
            strcpy(env_vars->history[i], env_vars->history[i+1]);
        }
        strcpy(env_vars->history[env_vars->hist_no-1], input);
    }
}

char **tokenise_input(char *input) {
    int position = 0;
    char **tokens = malloc(sizeof(char*) * MAX_TOK_NO);
    char *token;

    if (!tokens) {
        bred();
        fprintf(stderr, MEM_ALLOC_ERROR);
        reset_colour();
        exit(EXIT_FAILURE);
    }

    token = strtok(input, TOK_DELIM);
    while (NULL != token) {
        if (0 == strcmp("exit", token)) exit(EXIT_SUCCESS);
        tokens[position] = token;
        position++;

        #ifdef DEBUG
        printf("\"");
        printf("%s", token);
        printf("\"\n");
        #endif

        token = strtok(NULL, TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}


void execute_command(char **args, Env_vars *env_vars) {
    if (strcmp(args[0], "getpath") == 0) {
        get_path(args);
    } else if (strcmp(args[0], "setpath") == 0) {
        set_path(args);
    } else if (strcmp(args[0], "cd") == 0) {
        change_dir(args, env_vars);
    } else if (strcmp(args[0], "history") == 0) { 
        history(env_vars);        
    } else if (strcmp(args[0], "alias") == 0){
        if (args[1] != NULL){
            add_alias(args, env_vars);
        }
        else{
            print_aliases(env_vars);
        }
    } else if (strcmp(args[0], "unalias") == 0) {
        remove_alias(args, env_vars);
    }
    else {
        exec_external(args);
    } 
}

void get_path(char **args) {
    if (args[1] != NULL) {
        bred();
        printf("getpath: Too many arguemnts. Usage: 'getpath'\n");
        reset_colour();
    } else {
        printf("%s\n", getenv("PATH"));
    }
}

void set_path(char **args) {
    bred();
    if (NULL == args[1]) {
        printf("setpath: Missing path argument. Usage: 'setpath <path variables>' variables should be separated by ','\n");
    } else if (NULL != args[2]) {
        printf("setpath: Too many arguments. Usage: 'setpath <path variables>' variables should be separated by ','\n");
    } else {
        if (0 != setenv("PATH", args[1], 1)) {
            perror(args[0]);
        }
    }
    reset_colour();
}

void change_dir(char **args, Env_vars *env_vars) {
    bred();
    if(NULL == args[1]) {
        if (0 != chdir(getenv("HOME"))) {
            perror("cd");
        } else {
            env_vars->cwd = getenv("HOME");
        }        
    } else if (NULL != args[2]) {
        printf("cd: Too many arguemnts. Usage: 'cd <path>'\n");
    } else {
        if (0 != chdir(args[1])) {
            perror(args[1]);
        } else {
            env_vars->cwd = getcwd(NULL, 0);
        }
    }
    reset_colour();
}

void history(Env_vars *env_vars) {
    for (int i = 0; i <= env_vars->hist_no-1; i++) {
        printf("%i: %s",i+1, env_vars->history[i]);
    }
}

void add_alias(char **arg, Env_vars *env_vars){
    int replace = 0;
    char aliasCommand[MAX_COMMAND_LENGTH] = "";

    //Checks if the alias command is not empty
    if (arg[2] == NULL){
        bred();
        printf("alias: Missing arguments. Usage: 'alias', 'alias <alias name> <command>'\n");
    }
    else {
        //Concatenates the rest of the commnad line as one command after the 3rd argument
        strcpy(aliasCommand, arg[2]);
        int i = 3;
        while (arg[i] != NULL){
            strcat(aliasCommand, " ");
            strcat(aliasCommand, arg[i]);
            i++;
        }
        //Checks if an alias with the same name exists
        for (int i = 0; i < env_vars->alias_no; i++) {
            if (strcmp(arg[1], env_vars->aliases[i].alias_name) == 0){
                //Replace the first command with the second 
                strcpy(env_vars->aliases[i].alias_command, aliasCommand);
                replace = 1;
                yellow();
                printf("alias: '%s' replaced\n", arg[1]);
            }
        }
        //Adds the alias unless it reached the limit
        if (env_vars->alias_no < 10 && replace == 0){
            strcpy(env_vars->aliases[env_vars->alias_no].alias_name, arg[1]);
            strcpy(env_vars->aliases[env_vars->alias_no].alias_command, aliasCommand);
            env_vars->alias_no++;
        }
        //If alias has been replaced then do nothing
        else if (replace == 1){
            //Do nothing
        }
        else{
            bred();
            printf("alias: Limit of 10 aliases reached\n");
        }
    }
    reset_colour();
}

  //Prints the ALias arrays
void print_aliases(Env_vars *env_vars) {
    if (env_vars->alias_no == 0){
        bred();
        printf("alias: There are currently no aliases set\n");
    }
    else{
        for(int i = 0; i < env_vars->alias_no; i++){
            printf("%i: %s = '%s'\n",i, env_vars->aliases[i].alias_name, env_vars->aliases[i].alias_command);
        } 
    }
    reset_colour();
}

void remove_alias(char **args, Env_vars *env_vars) {
    int alias = 0;
    
    // Checking if an argument is entered.
    bred();
    if (args[1] == NULL) {
        printf("unalias: Missing arguments. Usage: 'unalias <alias name>'\n");
    }
    // Checking if there are any existing aliases.
    else if (env_vars->alias_no == 0) {
        printf("unalias: No aliases are set\n");
    } else if (args[2] != NULL) {
        printf("unalias: Too many arguments. Usage: 'unalias <alias name>'\n");
    }
    // Checking if the alias exists.
    else {
        for (int i = 0; i < env_vars->alias_no; i++) {
            if (strcmp (args[1], env_vars->aliases[i].alias_name) == 0) {
                // Moving the specified alias to the end of list.
                yellow();
                printf ("unalias: '%s' deleted\n", env_vars->aliases[i].alias_name);
                reset_colour();
                for (int j = i; j < env_vars->alias_no; j++) {
                    env_vars->aliases[j] = env_vars->aliases[j + 1];
                    i--;
                }  
                env_vars->alias_no--;
                alias = 1;  
            }
        }
        if(alias == 0) {
            bred();
            printf("unalias: No such alias: '%s'\n", args[1]);
        }
    }
    reset_colour();
}
   


/*Executes the alias command if theres an alias otherwise execute the command*/
void execute_alias(char **arg, Env_vars *env_vars){
    int alias = 0;
    for (int i = 0; i < 10; i++) {
        if (strcmp(arg[0], env_vars->aliases[i].alias_name) == 0){
            char **command = tokenise_input(env_vars->aliases[i].alias_command);
            execute_command(command, env_vars);
            alias = 1;
        }  
    }
    if (alias == 0){
        execute_command(arg, env_vars);
    }
}

/*Executes external commands*/
int exec_external(char **args){ 
    pid_t pid, wpid;
    int status;
    
    pid = fork();
    if (pid == 0) {
        // Child Process
        if (-1 == execvp(args[0], args)) {
            bred();
            perror(args[0]);
            reset_colour();
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error Forking
        bred();
        perror(args[0]);
        reset_colour();
    } else {
        //Parent Process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
} 

//   int write_history_tofile() {
//       FILE *fp;

//       fp = fopen(HistoryFile, "w"); 

//       if(fp != NULL) {
//           if(count < 20) {
//                for(int i=0; i < count-1; i++) {
//                fprintf(fp, "%s",hist[i]); 
//               }
//             fclose(fp);
//             return 1;
//               } else {
//               for(int i=0; i < 19; i++) {
//               fprintf(fp, "%s",hist[Hist_numb]); 
//               Hist_numb = (Hist_numb + 1) % 20;
//               }
//               fclose(fp); 
//               return 1;
//           }
//       } else {
//           printf("Error, Could not find file! \n");
//           return 0; 
//       }
//      fclose(fp);
//   }

//  int LoadHistory () {
//       FILE *fp;
//       char line[512];
//       Hist_numb = 0;
//       count = 0;

//       fp = fopen(HistoryFile, "r");

//       if(fp == NULL) {
//           return 0;
//       }
      
//       while(1) { 
//          if(fgets(line, 512, fp) == NULL) {
//          break;
//       }
//       //setting last char to a new line!
//         line[strlen(line)-1] = '\n';
//         add_history(line);
//   }
//          fclose(fp);
//          return 1;
//  }

//  void exit_shell(Env_vars *env_vars) {

//  }
 