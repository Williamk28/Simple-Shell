#include "header.h"

int Hist_numb = 0;
int count = 0;
char *hist;

int main() {
    
    EnvVars envVars;

    initShell(&envVars);

    loopShell(&envVars);

    //exitShell();

    return EXIT_SUCCESS;
}

void initShell(EnvVars *envVars) {

    envVars->cwd = getenv("HOME");
    envVars->user = getenv("USER");

    chdir(envVars->cwd);

    if(strcmp(getcwd(NULL, 0), envVars->cwd) != 0) {
        printf("Error setting HOME directory.");
    }

    #ifdef DEBUG
    printf("Home Directory: %s\n",getcwd(NULL,0));
    #endif
}

void loopShell(EnvVars *envVars) {
    char *input;
    char **args;
    int status;
    

    do {
        printf("%s: %s> ", envVars->user, envVars->cwd);
        input = readInput();
        strcpy(hist[Hist_numb], input); 
        Hist_numb  = (Hist_numb + 1) % 20;
        count++;
        args = tokeniseInput(input);
        status = executeCommand(args, envVars);

        free(input);
        free(args);
    } while (1);
}

char *readInput() {
    char *str = malloc(sizeof(char) * MAX_COMMAND_LENGTH);

    if (!str) {
        fprintf(stderr, "Error: allocation error\n");
        exit(EXIT_FAILURE);
    }

    if (fgets(str, MAX_COMMAND_LENGTH, stdin) == NULL) {
        printf("\n");
        exit(EXIT_SUCCESS);
    } else {
        return str;
    }
}

char **tokeniseInput(char *input)
{
    int position = 0;
    char **tokens = malloc(sizeof(char*) * MAX_TOK_NO);
    char *token;

    if (!tokens) {
        fprintf(stderr, "Error: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(input, TOK_DELIM);
    while (token != NULL) {
        if (strcmp("exit", token) == 0)
        {
            exit(EXIT_SUCCESS);
        }
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

int executeCommand(char **args, EnvVars *envVars) {

    if(args[0] == NULL) {
        return 1;
    }

    if(strcmp(args[0], "getpath") == 0) {
        return getPath();
    } else if(strcmp(args[0], "setpath") == 0) {

        if(args[1] == NULL) {
            printf("Error: Missing path argument\n");
            return 1;
        }
        return setPath(args[1]);

    } else if(strcmp(args[0], "cd") == 0) {
        changeDir(args[1], envVars);
    } else if (strcmp(args[0], "!") == 0) {
        execHistory(args);
    }
    else {
        return execExternal(args);
    }   
}

int getPath() {
    printf("%s\n", getenv("PATH"));
    return 1;
}

int setPath(char *arg) {
    setenv("PATH", arg, 1);
    return 1;
}

void changeDir(char *arg, EnvVars *envVars) {
    if(arg == NULL) {
        chdir(getenv("HOME"));
        envVars->cwd = getenv("HOME");
    } else {

    if (chdir(arg)!= 0){
        perror(arg);
    }
    envVars->cwd = getcwd(NULL, 0);
    }
}

int execExternal(char **args)
{ 
    pid_t pid, wpid;
    int status;
    
    pid = fork();
    if(pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("Shell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("Shell");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
} 

void execHistory(char **args) {
   char  **temp;
     if(strcmp(args[0], "!!") == 0){
         Printf("Executing last command");
         temp = tokenise(hist[count-2]);
         commandHandler(temp);
     }
}
