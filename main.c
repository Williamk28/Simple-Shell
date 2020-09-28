#include "header.h"

/* GROUP 4 */
/* Elom Boccorh, William Kong, Daniel Lowry, Kieran Malloy and Joe Stewart */
char hist[20][512];
int Hist_numb = 0;
int count = 0;
char usedAliases[MAX_ALIASES][MAX_COMMAND_LENGTH];
int noUsedAliases = 0;

/* creates a struct, then calls to initiate and run the shell */
int main()
{

    Env_vars env_vars;

    init_shell(&env_vars);

    loop_shell(&env_vars);
}

/* Initiates the shell with all the necessary parameters of env_vars */
void init_shell(Env_vars *env_vars)
{
    env_vars->cwd = getenv("HOME");
    env_vars->user = getenv("USER");
    env_vars->path = getenv("PATH");
    env_vars->alias_no = 0;

    if (0 != chdir(env_vars->cwd))
    {
        bred();
        perror("Shell");
        reset_colour();
    }

    if (load_history() == 0)
    {
        bred();
        printf("Error, File does not exist! \n");
        reset_colour();
    }
    else
    {
        bgreen();
        printf("File has been successfully loaded. \n");
        reset_colour();
    }

    load_aliases(env_vars);

#ifdef DEBUG
    printf("Home Directory: %s\n", getcwd(NULL, 0));
#endif
}

/* Loads history into array from file */
int load_history()
{
    FILE *fp;
    char line[MAX_COMMAND_LENGTH];

    fp = fopen(HISTORY_FILE, "r");

    if (fp == NULL)
    {
        return 0;
    }

    while (NULL != fgets(line, 512, fp))
    {
        //setting last char to a new line!
        line[strlen(line) - 1] = '\n';
        add_history(line);
    }
    fclose(fp);
    return 1;
}

/* Loads aliases into array from file */
int load_aliases(Env_vars *env_vars)
{
    FILE *fp;
    char line[512];
    env_vars->alias_no = 0;
    char **args;

    fp = fopen(ALIAS_FILE, "r");

    if (fp == NULL)
    {
        return 0;
    }

    while (1)
    {
        if (fgets(line, 512, fp) == NULL)
        {
            break;
        }
        //setting last char to a new line!
        line[strlen(line) - 1] = '\n';
        args = tokenise_input(line, env_vars);
        add_alias(args, env_vars);
    }
    fclose(fp);
    return 1;
}

/* Loops the input to keep the shell running */
void loop_shell(Env_vars *env_vars)
{
    char *input;
    char **args;

    do
    {
        bgreen();
        printf("%s: ", env_vars->user);
        bblue();
        printf("%s> ", env_vars->cwd);
        reset_colour();

        input = read_input(env_vars);
        if (input[0] == '\n')
        {
            continue;
        }
        else
        {
            add_history(input);
            args = tokenise_input(input, env_vars);
            if (args[0] != NULL)
                execute_command(args, env_vars);
        }
        memset(usedAliases, 0, sizeof(usedAliases));
        noUsedAliases = 0;
        free(input);
        free(args);
    } while (1);
}

/* Reads in thye users input */
char *read_input(Env_vars *env_vars)
{
    char *str = malloc(sizeof(char) * MAX_COMMAND_LENGTH);

    if (!str)
    {
        bred();
        fprintf(stderr, MEM_ALLOC_ERROR);
        reset_colour();
        exit_shell(EXIT_FAILURE, env_vars);
    }
    else if (NULL == fgets(str, MAX_COMMAND_LENGTH, stdin))
    {
        printf("\n");
        exit_shell(EXIT_SUCCESS, env_vars);
    }
    else
    {
        return str;
    }
}

/* Adds the input to the history */
void add_history(char *input)
{
    strcpy(hist[Hist_numb], input);
    Hist_numb = (Hist_numb + 1) % 20;
    count++;
}

/* Tokenises the input for parsing */
char **tokenise_input(char *input, Env_vars *env_vars)
{
    int position = 0;
    char **tokens = malloc(sizeof(char *) * MAX_TOK_NO);
    char *token;

    if (!tokens)
    {
        bred();
        fprintf(stderr, MEM_ALLOC_ERROR);
        reset_colour();
        exit_shell(EXIT_FAILURE, env_vars);
    }

    token = strtok(input, TOK_DELIM);
    while (NULL != token)
    {
        if (0 == strcmp("exit", token))
            exit_shell(EXIT_SUCCESS, env_vars);
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

/* Executes the alias command if theres an alias otherwise execute the command */
void execute_alias(char **args, Env_vars *env_vars, int index)
{

    char newCmd[MAX_COMMAND_LENGTH];

    for (int i = 0; i < noUsedAliases; i++)
    {
        if (strcmp(usedAliases[i], args[0]) == 0)
        {
            fprintf(stderr, "Error: Alias loop detected\n");
            return;
        }
    }
    strcpy(usedAliases[noUsedAliases], args[0]);
    noUsedAliases++;

    strcpy(newCmd, env_vars->aliases[index].alias_command);

    int j = 1;
    while (args[j] != NULL)
    {
        strcat(newCmd, " ");
        strcat(newCmd, args[j]);
        j++;
    }

    char **command = tokenise_input(newCmd, env_vars);
    execute_command(command, env_vars);
}
/* Calls to the necessary internal command or to the execute external function */
void execute_command(char **args, Env_vars *env_vars)
{
    if (strcmp(args[0], "getpath") == 0)
    {
        get_path(args);
    }
    else if (strcmp(args[0], "setpath") == 0)
    {
        set_path(args);
    }
    else if (strcmp(args[0], "cd") == 0)
    {
        change_dir(args, env_vars);
    }
    else if (strcmp(args[0], "history") == 0)
    {
        history(args);
    }
    else if (strcspn(args[0], "!") == 0)
    {
        exec_history(args, env_vars);
    }
    else if (strcmp(args[0], "alias") == 0)
    {
        if (args[1] != NULL)
        {
            add_alias(args, env_vars);
        }
        else
        {
            print_aliases(env_vars);
        }
    }
    else if (strcmp(args[0], "unalias") == 0)
    {
        remove_alias(args, env_vars);
    }
    else
    {
        for (int i = 0; i < MAX_ALIASES; i++)
        {
            if (strcmp(env_vars->aliases[i].alias_name, args[0]) == 0)
            {
                execute_alias(args, env_vars, i);
                return;
            }
        }
        exec_external(args);
    }
}

/* Gets the current path variable */
void get_path(char **args)
{
    if (args[1] != NULL)
    {
        bred();
        printf("getpath: Too many arguemnts. Usage: 'getpath'\n");
        reset_colour();
    }
    else
    {
        printf("%s\n", getenv("PATH"));
    }
}

/* Sets the path variable */
void set_path(char **args)
{
    bred();
    if (NULL == args[1])
    {
        printf("setpath: Missing path argument. Usage: 'setpath <path variables>' variables should be separated by ','\n");
    }
    else if (NULL != args[2])
    {
        printf("setpath: Too many arguments. Usage: 'setpath <path variables>' variables should be separated by ','\n");
    }
    else
    {
        if (0 != setenv("PATH", args[1], 1))
        {
            perror(args[0]);
        }
    }
    reset_colour();
}

/* Changes current working directory */
void change_dir(char **args, Env_vars *env_vars)
{
    bred();
    if (NULL == args[1])
    {
        if (0 != chdir(getenv("HOME")))
        {
            perror("cd");
        }
        else
        {
            env_vars->cwd = getenv("HOME");
        }
    }
    else if (NULL != args[2])
    {
        printf("cd: Too many arguemnts. Usage: 'cd <path>'\n");
    }
    else
    {
        if (0 != chdir(args[1]))
        {
            perror(args[1]);
        }
        else
        {
            env_vars->cwd = getcwd(NULL, 0);
        }
    }
    reset_colour();
}

/* Prints out the history */
void history(char **args)
{
    if (NULL != args[1])
    {
        printf("history: Too many arguments. Usage: 'history'\n");
    }
    else if (count < 20)
    {
        for (int i = 0; i <= count - 1; i++)
        {
            printf("Command %d: %s", i + 1, hist[i]);
        }
    }
    else
    {
        for (int i = 0; i < 20; i++)
        {
            printf("Command %d: %s", i + 1, hist[Hist_numb]);
            Hist_numb = (Hist_numb + 1) % 20;
        }
    }
}

/* Executes history invocation commands */
void exec_history(char **args, Env_vars *env_vars)
{
    //Holds the return from tokenise
    char **temp;
    char **temp2;
    //Holds the value  of input from ! to compare when using array
    int value;
    //Creates a copy to allows to trash the array without effecting main array
    char TempValue[1][512];

    //This is for !!
    if (strcmp(args[0], "!!") == 0)
    {
        if (Hist_numb == 1 && count == 1)
        {
            bred();
            printf("There is no previous command to call! \n");
            reset_colour();
            Hist_numb--;
            count--;
            return;
        }

        if (args[1] != 0)
        {
            bred();
            printf("Too many arguments, please follow the format (!!) \n");
            reset_colour();
            history_error_decrement();
            return;
        }

        printf("Executing last command\n");
        if (Hist_numb == 0 && count >= 20)
        {
            if (strcmp(hist[18], "history\n") == 0)
            {
                Hist_numb = 19;
                count--;
                strcpy(TempValue[0], hist[18]);
                temp = tokenise_input(TempValue[0], env_vars);
                execute_command(temp, env_vars);
                return;
            }
            strcpy(hist[19], hist[18]);
            strcpy(TempValue[0], hist[19]);
            temp = tokenise_input(TempValue[0], env_vars);
        }
        else if (Hist_numb == 1 && count >= 20)
        {
            if (strcmp(hist[19], "history\n") == 0)
            {
                Hist_numb = 0;
                count--;
                strcpy(TempValue[0], hist[19]);
                temp = tokenise_input(TempValue[0], env_vars);
                execute_command(temp, env_vars);
            }
            strcpy(hist[0], hist[19]);
            strcpy(TempValue[0], hist[0]);
            temp = tokenise_input(TempValue[0], env_vars);
        }
        else
        {
            strcpy(hist[Hist_numb - 1], hist[Hist_numb - 2]);
            strcpy(TempValue[0], hist[Hist_numb - 1]);
            if (strcmp(TempValue[0], "history\n") == 0)
            {
                Hist_numb--;
                count--;
            }
            temp = tokenise_input(TempValue[0], env_vars);
            execute_command(temp, env_vars);
            return;
        }
        //This is for !-<no>
    }
    else if (args[0][1] == 45)
    {
        if (args[1] != 0)
        {
            bred();
            printf("Too many arguments, please follow the format (!-<no>) \n");
            reset_colour();
            history_error_decrement();
            return;
        }
        if (args[0][2] >= 48 && args[0][2] <= 57)
        {
            if (args[0][3] == 0)
            {
                value = (Hist_numb - 1) - ((args[0][2]) - 48);
                if (value < 0)
                {
                    value = value + 20;
                }
            }
            else
            {
                value = (((args[0][2] - 48) * 10) + args[0][3]) - 48;
                value = (Hist_numb - 1) - value;
                if (value < 0)
                {
                    value = value + 20;
                }
            }
            if (value <= count - 1 && value >= 0 && value < 20)
            {
                if (Hist_numb == 0)
                {
                    if (strcmp(hist[value], "history\n") == 0)
                    {
                        history_invokation_check_edge_case(value, env_vars);
                        return;
                    }
                    strcpy(hist[19], hist[value]);
                }
                else
                {
                    if (strcmp(hist[value], "history\n") == 0)
                    {
                        history_invokation_check_normal_case(value, env_vars);
                        return;
                    }
                }
                strcpy(hist[Hist_numb - 1], hist[value]);
                strcpy(TempValue[0], hist[value]);
                temp = tokenise_input(TempValue[0], env_vars);
                execute_command(temp, env_vars);
                return;
            }
            else
            {
                bred();
                printf("You cannot select a value out of range of the history!\n");
                reset_colour();
                history_error_decrement();
                return;
            }
        }
        else
        {
            bred();
            printf("Please enter a Integer command! e.g. !2 , !-2, !! \n");
            reset_colour();
            history_error_decrement();
            return;
        }
        // This is for !<no>
    }
    else if (args[0][1] >= 48 && args[0][1] <= 57)
    {
        if (args[1] != 0)
        {
            bred();
            printf("Too many arguments, please follow the format (!<no>) \n");
            reset_colour();
            history_error_decrement();
            return;
        }
        if (args[0][2] == 0)
        {
            value = (args[0][1]) - 49;
            if (Hist_numb == 1 || Hist_numb == 2)
            {
                value = (args[0][1]) - 50;
            }
        }
        else
        {
            if (args[0][2] >= 48 && args[0][2] <= 57)
            {
                value = (((args[0][1] - 48) * 10) + args[0][2]) - 49;
                if (Hist_numb == 1 || Hist_numb == 2)
                {
                    value = (((args[0][1] - 48) * 10) + args[0][2]) - 50;
                }
            }
            else
            {
                bred();
                printf("Error: Please use integer numbers. e.g.!<Number>\n");
                reset_colour();
                history_error_decrement();
                return;
            }
        }
        if (value < count - 1 && value >= 0 && value <= 20)
        {
            if (count > 20)
            {
                value = value + Hist_numb;
            }
            if (value > 20)
            {
                value = value - 21;
            }
            if (Hist_numb == 0)
            {
                if (strcmp(hist[value], "history\n") == 0)
                {
                    history_invokation_check_edge_case(value, env_vars);
                    return;
                }
                strcpy(hist[19], hist[value]);
            }
            else
            {
                if (strcmp(hist[value], "history\n") == 0)
                {
                    history_invokation_check_normal_case(value, env_vars);
                    return;
                }
                strcpy(hist[Hist_numb - 1], hist[value]);
            }
            strcpy(TempValue[0], hist[value]);
            temp = tokenise_input(TempValue[0], env_vars);
            execute_command(temp, env_vars);
            return;
        }
        else
        {
            bred();
            printf("You cannot select a value out of range of the history! \n");
            reset_colour();
            history_error_decrement();
            return;
        }
    }
    else
    {
        if (args[0][1] <= 48 || args[0][1] >= 57)
        {
            bred();
            printf("Please enter a correct command! e.g. !2 , !-2, !! \n");
            reset_colour();
            history_error_decrement();
            return;
        }
    }
}

void history_error_decrement()
{
    if (Hist_numb == 0)
    {
        count = count - 1;
        Hist_numb = 19;
    }
    else
    {
        count = count - 1;
        Hist_numb = Hist_numb - 1;
    }
}

void history_invokation_check_normal_case(int value, Env_vars *env_vars)
{
    char **temp;
    char TempValue[1][512];

    Hist_numb--;
    count--;
    strcpy(TempValue[0], hist[value]);
    temp = tokenise_input(TempValue[0], env_vars);
    execute_command(temp, env_vars);
}

void history_invokation_check_edge_case(int value, Env_vars *env_vars)
{
    char **temp;
    char TempValue[1][512];

    Hist_numb = 19;
    count--;
    strcpy(TempValue[0], hist[value]);
    temp = tokenise_input(TempValue[0], env_vars);
    execute_command(temp, env_vars);
}

/* Adds alias to the alias list */
void add_alias(char **arg, Env_vars *env_vars)
{
    int i = 3;
    int replace = 0;
    char aliasCommand[MAX_COMMAND_LENGTH] = "";

    //Checks if the alias command is not empty
    if (arg[2] == NULL)
    {
        bred();
        printf("alias: Missing arguments. Usage: 'alias', 'alias <alias name> <command>'\n");
    }
    else
    {
        //Concatenates the rest of the commnad line as one command after the 3rd argument
        strcpy(aliasCommand, arg[2]);
        int i = 3;
        while (arg[i] != NULL)
        {
            strcat(aliasCommand, " ");
            strcat(aliasCommand, arg[i]);
            i++;
        }
        //Checks if an alias with the same name exists
        for (int i = 0; i < env_vars->alias_no; i++)
        {
            if (strcmp(arg[1], env_vars->aliases[i].alias_name) == 0)
            {
                //Replace the first command with the second
                strcpy(env_vars->aliases[i].alias_command, aliasCommand);
                replace = 1;
                yellow();
                printf("alias: '%s' replaced\n", arg[1]);
            }
        }
        //Adds the alias unless it reached the limit
        if (env_vars->alias_no < 10 && replace == 0)
        {
            strcpy(env_vars->aliases[env_vars->alias_no].alias_name, arg[1]);
            strcpy(env_vars->aliases[env_vars->alias_no].alias_command, aliasCommand);
            env_vars->alias_no++;
        }
        //If alias has been replaced then do nothing
        else if (replace == 1)
        {
            //Do nothing
        }
        else
        {
            bred();
            printf("alias: Limit of 10 aliases reached\n");
        }
    }
    reset_colour();
}

//Prints the ALias arrays
void print_aliases(Env_vars *env_vars)
{
    if (env_vars->alias_no == 0)
    {
        bred();
        printf("alias: There are currently no aliases set\n");
    }
    else
    {
        for (int i = 0; i < env_vars->alias_no; i++)
        {
            printf("alias %s = '%s'\n", env_vars->aliases[i].alias_name, env_vars->aliases[i].alias_command);
        }
    }
    reset_colour();
}

/* Removes an alias */
void remove_alias(char **args, Env_vars *env_vars)
{
    int alias = 0;

    // Checking if an argument is entered.
    bred();
    if (args[1] == NULL)
    {
        printf("unalias: Missing arguments. Usage: 'unalias <alias name>'\n");
    }
    // Checking if there are any existing aliases.
    else if (env_vars->alias_no == 0)
    {
        printf("unalias: No aliases are set\n");
    }
    else if (args[2] != NULL)
    {
        printf("unalias: Too many arguments. Usage: 'unalias <alias name>'\n");
    }
    // Checking if the alias exists.
    else
    {
        for (int i = 0; i < env_vars->alias_no; i++)
        {
            if (strcmp(args[1], env_vars->aliases[i].alias_name) == 0)
            {
                // Moving the specified alias to the end of list.
                yellow();
                printf("unalias: '%s' deleted\n", env_vars->aliases[i].alias_name);
                reset_colour();
                for (int j = i; j < env_vars->alias_no; j++)
                {
                    env_vars->aliases[j] = env_vars->aliases[j + 1];
                    i--;
                }
                env_vars->alias_no--;
                alias = 1;
            }
        }
        if (alias == 0)
        {
            bred();
            printf("unalias: No such alias: '%s'\n", args[1]);
        }
    }
    reset_colour();
}

/*Executes external commands*/
int exec_external(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        // Child Process
        if (-1 == execvp(args[0], args))
        {
            bred();
            perror(args[0]);
            reset_colour();
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    else if (pid < 0)
    {
        // Error Forking
        bred();
        perror(args[0]);
        reset_colour();
    }
    else
    {
        //Parent Process
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

/* Saves the history array to file */
int save_history()
{
    FILE *fp;

    fp = fopen(HISTORY_FILE, "w");

    if (fp != NULL)
    {
        if (count < 20)
        {
            for (int i = 0; i < count - 1; i++)
            {
                fprintf(fp, "%s", hist[i]);
            }
            fclose(fp);
            return 1;
        }
        else
        {
            for (int i = 0; i < 19; i++)
            {
                fprintf(fp, "%s", hist[Hist_numb]);
                Hist_numb = (Hist_numb + 1) % 20;
            }
            fclose(fp);
            return 1;
        }
    }
    else
    {
        bred();
        printf("Error, Could not find file! \n");
        reset_colour();
        return 0;
    }
    fclose(fp);
}

/* Saves aliases to file */
int save_aliases(Env_vars *env_vars)
{
    FILE *fp;

    fp = fopen(ALIAS_FILE, "w");

    if (fp != NULL)
    {
        for (int i = 0; i < env_vars->alias_no; i++)
        {
            fprintf(fp, "alias %s %s\n", env_vars->aliases[i].alias_name, env_vars->aliases[i].alias_command);
        }
        fclose(fp);
        return 1;
    }
    else
    {
        printf("Error, could not find file!");
        return 0;
    }
    fclose(fp);
}

/* Tidys up things, calls to save history and alias and exits */
void exit_shell(int exit_code, Env_vars *env_vars)
{
    /*Reset Path*/
    if (0 != setenv("PATH", env_vars->path, 1))
    {
        bred();
        perror("shell");
        reset_colour();
    }

    if (0 != chdir(getenv("HOME")))
    {
        perror("cd");
    }
    else
    {
        env_vars->cwd = getenv("HOME");
    }

    printf("%s\n", getenv("PATH"));

    if (save_history() == 0)
    {
        bred();
        printf("Writing to history to file, failed! \n");
        reset_colour();
    }
    else
    {
        bgreen();
        printf("Writing to history to file is successful. \n");
        reset_colour();
    }
    if (save_aliases(env_vars) == 0)
    {
        bred();
        printf("Writing to alias to file, failed! \n");
        reset_colour();
    }
    else
    {
        bgreen();
        printf("Writing to alias to file is successful. \n");
        reset_colour();
    }

    exit(exit_code);
}
