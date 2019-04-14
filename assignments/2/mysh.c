/*****************************************
 * RTOS Assignment 2 - Shell
 * Christian Legge 201422748
 * Implemented features: Cancellation, History, Suspension
 ****************************************/

#include <signal.h>
#include <wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int numUtilities = 2;
char* utilities[] = {
    "exit",
    "history"
};

int child_pid = 0;
int kill_child = 0;
int suspend_child = 0;

char** history_commands = NULL;
int numHistory = 0;

void handleUtility(char** args) 
{
    if (strcmp(args[0], "exit") == 0)
    {
        printf("Exiting.");
        exit(0);
    }
    else if (strcmp(args[0], "history") == 0)
    {
        for (int i = 0; i < numHistory; i++)
        {
            printf("%s", history_commands[i]);
        }
    }
}

char** parse_commandline(char* cmd)
{
    char** cmdwithargs = NULL;
    char* sub;
    int pos = 0;
    while((sub = strsep(&cmd, " ")) != NULL)
    {
        if (strcmp(sub, "") == 0) 
        {
            continue;
        }
        cmdwithargs = realloc(cmdwithargs, sizeof(char*) * (pos+1));
        cmdwithargs[pos++] = sub;
    }
    return cmdwithargs;

}

void ctrlCfunc(int sig)
{
    signal(SIGINT, ctrlCfunc);
    if (child_pid > 0)
    {
        kill(child_pid, SIGINT);
    }
}

void ctrlZfunc(int sig)
{
    signal(SIGTSTP, ctrlZfunc);
    if (child_pid > 0)
    {
        kill(child_pid, SIGTSTP);
    }
}

int main() 
{
    signal(SIGINT, ctrlCfunc);
    signal(SIGTSTP, ctrlZfunc);

    int skip = 0;
    while(1)
    {
        printf("@>");
        char* cmd_line;
        size_t buffer = 0;
        getline(&cmd_line, &buffer, stdin);
        char* full_line = strdup(cmd_line);
        cmd_line = strsep(&cmd_line, "\n");
        char** args = parse_commandline(cmd_line);
        if ((int)args[0] == EOF)
        {
            exit(0);
        }
        if (args == NULL) 
        {
            continue;
        }
        history_commands = realloc(history_commands, sizeof(char*) * (numHistory + 1));
        history_commands[numHistory++] = strdup(full_line);
        for (int i = 0; i < numUtilities; i++)
        {
            if (strcmp(args[0], utilities[i]) == 0)
            {
                handleUtility(args);
                skip = 1;
                break;
            }
        }
        if (skip) 
        {
            skip = 0;
            continue;
        }
        child_pid = fork();
        if (child_pid == 0) 
        {
            int retval = execvp(args[0], args);
            if (retval == -1) 
            {
                printf("Error: command '%s' not found\n", args[0]);
                exit(-1);
            }
        }
        else if (child_pid > 0) 
        {
            int status, checkStatus = 0;
            do 
            {

                waitpid(child_pid, &status, WUNTRACED);
            }
            while (!WIFSIGNALED(status) && !WIFEXITED(status) && !WIFSTOPPED(status));
        }
        free(args);
    }

    return 0;
}
