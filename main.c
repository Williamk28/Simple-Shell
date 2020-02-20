#include "header.h"

int Hist_numb = 0;
int count = 0;

int main() {
    
    Env_vars env_vars;

    init_shell(&env_vars);

    loop_shell(&env_vars);

    //exitShell();

    return EXIT_SUCCESS;
}

void init_shell(Env_vars *env_vars) {
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
    char *input;
    char **args;
    
    do {
        bgreen();
        printf("%s: ", env_vars->user);
        bblue();
        printf("%s> ", env_vars->cwd);
        reset_colour();
        
        input = read_input();
        add_history(input);
        args = tokenise_input(input);
        execute_command(args, env_vars);

        free(input);
        free(args);
    } while (1);
}

char *read_input() {
    char *str = malloc(sizeof(char) * MAX_COMMAND_LENGTH);

    if (!str) {
        bred();
        fprintf(stderr, MEM_ALLOC_ERROR);
        reset_colour();
        exit(EXIT_FAILURE);
    } else if (NULL == fgets(str, MAX_COMMAND_LENGTH, stdin)) {
        printf("\n");
        exit(EXIT_SUCCESS);
    } else {
        return str;
    }
}

void add_history(char *input) {
    strcpy(hist[Hist_numb], input); 
    Hist_numb  = (Hist_numb + 1) % 20;
    count++;
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

    if(args[0] == NULL) {
        return;
    } else if(strcmp(args[0], "getpath") == 0) {
        printf("%s\n", getenv("PATH"));
    } else if(strcmp(args[0], "setpath") == 0) {
        set_path(args[1]);
    } else if(strcmp(args[0], "cd") == 0) {
        change_dir(args[1], env_vars);
    } else if (strcspn(args[0], "!") == 0) {
        exec_history(args, env_vars);
    } else if(strcmp(args[0], "history") == 0) { 
        history();        
    } else {
        exec_external(args);
    }   
}

void set_path(char *arg) {
    if (NULL == arg) printf("Error: Missing path argument\n"); 
    if (0 != setenv("PATH", arg, 1)) perror("Shell");
}

void change_dir(char *arg, Env_vars *env_vars) {
    if(NULL == arg) {
        if (0 != chdir(getenv("HOME"))) {
            perror("Shell");
        } else {
            env_vars->cwd = getenv("HOME");
        }        
    } else {

        if (0 != chdir(arg)) {
            perror(arg);
        } else {
            env_vars->cwd = getcwd(NULL, 0);
        }
    }
}

void history() {
    if(count < 20) {
        for(int i=0; i <= count-1; i++) {
            printf("Command %d: %s",i+1,hist[i]);
        }
    } else { 
        for(int i=0; i < 20; i++) {
            printf("Command %d: %s",i+1,hist[Hist_numb]);
            Hist_numb = (Hist_numb + 1) % 20;
        }
    }
}


int exec_external(char **args){ 
    pid_t pid, wpid;
    int status;
    
    pid = fork();
    if (pid == 0) {
        // Child Process
        if (-1 == execvp(args[0], args)) {
            perror("Shell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error Forking
        perror("Shell");
    } else {
        //Parent Process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
} 

//Tokenise Method breaking history.
void exec_history(char **args, Env_vars *env_vars) {
    //Holds the return from tokenise
   char  **temp;
   //Holds the value  of input from ! to compare when using array 
    int value;
    //Creates a copy to allows to trash the array without effecting main array
    char TempValue[1][512];
   //This is for !!
     if(strcmp(args[0], "!!") == 0){
         printf("Executing last command\n");
         if(Hist_numb == 0) { 
              strcpy(hist[19], hist[18]); 
         strcpy(TempValue[0], hist[19]); 
        temp = tokenise_input(TempValue[0]);
         } else if(Hist_numb == 1) { 
              strcpy(hist[0], hist[19]); 
            strcpy(TempValue[0], hist[0]); 
        temp = tokenise_input(TempValue[0]);
         } else {
         strcpy(hist[Hist_numb-1], hist[Hist_numb-2]); 
                  strcpy(TempValue[0], hist[Hist_numb-1]); 
        temp = tokenise_input(TempValue[0]);
         }

        execute_command(temp, env_vars);
        return;
           //This is for !-<no>
         } else if(args[0][1] == 45 ) {
         if(args[0][2] >= 48 && args[0][2] <= 57) {
             if(args[0][3] == 0) {
                value =  (Hist_numb - 1) - ((args[0][2])-48);
                if(value < 0) {
                    value = value + 20;
                }
             } else {
                 value = (((args[0][2]-48)*10) + args[0][3])-48;
                 value = (Hist_numb - 1) - value;
                 if(value < 0) {
                     value = value + 20;
                 }
             } 
         if(value <= count-1 && value >= 0 && value < 20) { 
              if(Hist_numb == 0) { 
             strcpy(hist[19], hist[value]);
             } else { 
             strcpy(hist[Hist_numb-1], hist[value]); 
             }
        strcpy(TempValue[0], hist[value]); 
        temp = tokenise_input(TempValue[0]);
        execute_command(temp, env_vars);
        return;
         } else {
           printf("You cannot select a value out of range of the history!\n");
           if(Hist_numb == 0) {
              count = count - 1;
              Hist_numb = 19;
           } else {
            count = count - 1;
            Hist_numb = Hist_numb - 1;
           }
           return;
       }
     }  else {
        printf("Please enter a integer command! e.g. !2 , !-2, !! \n");
           if(Hist_numb == 0) {
              count = count - 1;
              Hist_numb = 19;
           } else {
            count = count - 1;
            Hist_numb = Hist_numb - 1;
           }
           return;
       }
     
        //This is for if just !<no>
     }  else if(args[0][1] >= 48 && args[0][1] <= 57) {
         if(args[0][2] == 0) {
           value = (args[0][1])-49;
         } else {
             value = (((args[0][1]-48)*10) + args[0][2])-49;
         }
         if(value < count-1 && value >= 0 && value <= 20) { 
             if(Hist_numb == 0) { 
             strcpy(hist[19], hist[value]);
             } else {
         strcpy(hist[Hist_numb-1], hist[value]); 
             }
        strcpy(TempValue[0], hist[value]); 
        temp = tokenise_input(TempValue[0]);
        execute_command(temp, env_vars);
        return;
     } else {
         printf("You cannot select a value out of range of the history! \n");
           if(Hist_numb == 0) {
              count = count - 1;
              Hist_numb = 19;
           } else {
            count = count - 1;
            Hist_numb = Hist_numb - 1;
           }
           return;
       } 
       } else {
               if(args[0][1] <= 48 || args[0][1] >= 57 ) {
        printf("Please enter a integer command! e.g. !2 , !-2, !! \n");
           if(Hist_numb == 0) {
              count = count - 1;
              Hist_numb = 19;
           } else {
            count = count - 1;
            Hist_numb = Hist_numb - 1;
           }
           return;
        }
    }
  }  

void removeAlias(char *arg) {
        if (NumOfAliases == NULL) {
            printf("There are no aliases.");
            // ^^Checking if there are any existing aliases^^
        }
        else if (arg != NULL) {
            for (int i = 0; i < NumOfAliases; i++) {
                if (strcmp(arg[1], Aliases[i].alias)) {
                    // ^^Checking if the alias exists^^
                    Aliases[i].command = NULL;
                    NumOfAliases = NumOfAliases - 1;
                    // ^^Deleting the alias^^
                    for (int j = Aliases[i + 1]; j < Aliases; j++) {
                    Aliases[j - 1] = Aliases[j];
                    // ^^ Moving all elements after NULL gap to the left by one^^
                    }
                }
            }
        } else {
            printf("This alias doesn't exist.")
            // ^^If the argument doesn't match any existing alias^^
        }
    }

    void printAliases() { 
        if (NumOfAliases == NULL) {
            printf("There are no aliases.")
            // ^^Checking if there are any existing aliases^^
        }
        else {
            printf("There are %d aliases: \n", NumOfAliases)
            printf("%s \n", Aliases[10])
            // ^^Printing out the list of aliases^^
        }
    }
