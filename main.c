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
        if (input[0] == '\n') { continue; }
        else {
            add_history(input);
            args = tokenise_input(input);
            execute_command(args, env_vars);
        }
        

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
        printf("Executing history! \n");
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
       // printf("Command 4: %s",hist[3]);
    } else if(strcmp(args[0], "writehistory") == 0) { 
       if(write_history_tofile() == 0) {
           printf("Writing to history to file, failed! \n");
       } else { 
            printf("Writing to history to file is successfull. \n"); 
       }

       } else if(strcmp(args[0], "loadhistory") == 0) {
           if(LoadHistory() == 0) { 
               printf("Error, File does not exist! \n");
           } else {
               printf("Success file has been loaded. \n");
           }
    } else {
        exec_external(args);
    }   
}

void set_path(char *arg) {
    if (NULL == arg) printf("Error: Missing path argument\n") ;
    else if (0 != setenv("PATH", arg, 1)) perror("Shell");
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

void exec_history(char **args, Env_vars *env_vars) {
    //Holds the return from tokenise
   char  **temp;
   char  **temp2;
   //Holds the value  of input from ! to compare when using array 
    int value;
    //Creates a copy to allows to trash the array without effecting main array
    char TempValue[1][512];
   //This is for !!
     if(strcmp(args[0], "!!") == 0){
         if(Hist_numb == 1 && count == 1) {
             printf("There is no previous command to call! \n");
             Hist_numb--;
             count--;
             return;
         }
        if(args[1] != 0) {
            printf("Too many arguments, please follow the format (!!) \n");
            HistoryErrorDecrement();
            return;
         }
         printf("Executing last command\n");
         if(Hist_numb == 0 && count >= 20) { 
           if(strcmp(hist[18],"history\n") == 0){
             Hist_numb = 19;
             count--;
            strcpy(TempValue[0], hist[18]);
             temp = tokenise_input(TempValue[0]);
            execute_command(temp, env_vars);
            return;
         }
         strcpy(hist[19], hist[18]); 
         strcpy(TempValue[0], hist[19]); 
        temp = tokenise_input(TempValue[0]);
         } else if(Hist_numb == 1 && count >= 20) { 
              if(strcmp(hist[19],"history\n") == 0){ 
                  Hist_numb = 0;
                  count--;
                  strcpy(TempValue[0], hist[19]);
                  temp = tokenise_input(TempValue[0]);
                  execute_command(temp, env_vars);
              }
            strcpy(hist[0], hist[19]); 
            strcpy(TempValue[0], hist[0]); 
            temp = tokenise_input(TempValue[0]);
         } else {
         strcpy(hist[Hist_numb-1], hist[Hist_numb-2]); 
         strcpy(TempValue[0], hist[Hist_numb-1]);
         if(strcmp(TempValue[0],"history\n") == 0){
             Hist_numb--;
             count--;
         }
        temp = tokenise_input(TempValue[0]);
         }

        execute_command(temp, env_vars);
        return;
           //This is for !-<no>
         } else if(args[0][1] == 45 ) {
            if(args[1] != 0) {
             printf("Too many arguments, please follow the format (!-<no>) \n");
             HistoryErrorDecrement();
             return;
            }
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
          if(strcmp(hist[value],"history\n") == 0){
             HistoryInvokationCheckEdgeCase(value, env_vars);
            return;
              }
             strcpy(hist[19], hist[value]);
                 } else {
            if(strcmp(hist[value],"history\n") == 0){
            HistoryInvokationCheckNormalCase(value, env_vars);
            return;
            }
           }
         strcpy(hist[Hist_numb-1], hist[value]); 
        strcpy(TempValue[0], hist[value]); 
        temp = tokenise_input(TempValue[0]);
        execute_command(temp, env_vars);
        return;
         } else {
           printf("You cannot select a value out of range of the history!\n");
         HistoryErrorDecrement();
           return;
       }
     }  else {
        printf("Please enter a Integer command! e.g. !2 , !-2, !! \n");
             HistoryErrorDecrement();
           return;
       }
       // This is for !<no>
          }  else if(args[0][1] >= 48 && args[0][1] <= 57) {
             if(args[1] != 0) {
             printf("Too many arguments, please follow the format (!<no>) \n");
             HistoryErrorDecrement();
             return;
            }
         if(args[0][2] == 0) {
           value = (args[0][1])-49;
         } else {
             if(args[0][2] >= 48 && args[0][2] <= 57) {
             value = (((args[0][1]-48)*10) + args[0][2])-49;
             } else {
                 printf("Error: Please use integer numbers. e.g.!<Number>\n");
                 HistoryErrorDecrement();
                 return;
             }

         }
         if(value < count-1 && value >= 0 && value <= 20) {
             if(count > 20) {
            value = value + Hist_numb;
             }
            if(value > 20) {
                value = value - 21;
            }
             if(Hist_numb == 0) { 
          if(strcmp(hist[value],"history\n") == 0){
             HistoryInvokationCheckEdgeCase(value, env_vars);
            return;
          }
             strcpy(hist[19], hist[value]);
             } else {
           if(strcmp(hist[value],"history\n") == 0){
             HistoryInvokationCheckNormalCase(value, env_vars);
            return;
           }
         strcpy(hist[Hist_numb-1], hist[value]); 
             }
        strcpy(TempValue[0], hist[value]); 
        temp = tokenise_input(TempValue[0]);
        execute_command(temp, env_vars);
        return;
     } else {
         printf("You cannot select a value out of range of the history! \n");
            HistoryErrorDecrement();
           return;
     }
       } else {
          if(args[0][1] <= 48 || args[0][1] >= 57 ) {
        printf("Please enter a correct command! e.g. !2 , !-2, !! \n");
         HistoryErrorDecrement();
           return;
    }
  }
}  

  void HistoryErrorDecrement() {
   if(Hist_numb == 0) {
     count = count - 1;
     Hist_numb = 19;
   } else {
     count = count - 1;
     Hist_numb = Hist_numb - 1;
   }
  }

  void HistoryInvokationCheckNormalCase(int value, Env_vars *env_vars) {
    char **temp;
    char TempValue[1][512];
        Hist_numb--;
        count--;
        strcpy(TempValue[0], hist[value]);
        temp = tokenise_input(TempValue[0]);
        execute_command(temp, env_vars);
  }

    void HistoryInvokationCheckEdgeCase(int value, Env_vars *env_vars) {
    char **temp;
    char TempValue[1][512];
        Hist_numb = 19;
        count--;
        strcpy(TempValue[0], hist[value]);
        temp = tokenise_input(TempValue[0]);
        execute_command(temp, env_vars);
    }

  int write_history_tofile() {
      FILE *fp;


      fp = fopen(HistoryFile, "w"); 

      if(fp != NULL) {
          if(count < 20) {
               for(int i=0; i < count-1; i++) {
               fprintf(fp, "%s",hist[i]); 
              }
            fclose(fp);
            return 1;
              } else {
              for(int i=0; i < 19; i++) {
              fprintf(fp, "%s",hist[Hist_numb]); 
              Hist_numb = (Hist_numb + 1) % 20;
              }
              fclose(fp); 
              return 1;
          }
      } else {
          printf("Error, Could not find file! \n");
          return 0; 
      }
     fclose(fp);
  }

 int LoadHistory () {
      FILE *fp;
      char line[512];
      Hist_numb = 0;
      count = 0;

      fp = fopen(HistoryFile, "r");

      if(fp == NULL) {
          return 0;
      }
      
      while(1) { 
         if(fgets(line, 512, fp) == NULL) {
         break;
      }
      //setting last char to a new line!
        line[strlen(line)-1] = '\n';
        AddHistory(line);
  }
         fclose(fp);
         return 1;
 }
   
    void AddHistory(char *line)  {
        strcpy(hist[Hist_numb], line);
        Hist_numb = (Hist_numb + 1) % 20;
        count ++;
        return;
    }