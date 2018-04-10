/*-
 * msh.c
 *
 * Minishell C source
 * Show how to use "obtain_order" input interface function
 *
 * THIS FILE IS TO BE MODIFIED
 */

#include <stddef.h>            /* NULL */
#include <stdio.h>            /* setbuf, printf */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_STORED_COMMANDS 20
#define MAX_PIPED_COMMANDS 3

extern int obtain_order();        /* See parser.y for description */

struct command {
    // Store the number of commands in argvv
    int num_commands;
    // Store the number of arguments of each command
    int *args;
    // Store the commands
    char ***argvv;
    // Store the I/O redirection
    char *filev[3];
    // Store if the command is executed in background or foreground
    int bg;
};

void free_command(struct command *cmd) {
    if ((*cmd).argvv != NULL) {
        char **argv;
        for (; (*cmd).argvv && *(*cmd).argvv; (*cmd).argvv++) {
            for (argv = *(*cmd).argvv; argv && *argv; argv++) {
                if (*argv) {
                    free(*argv);
                    *argv = NULL;
                }
            }
        }
    }
    free((*cmd).args);
    int f;
    for (f = 0; f < 3; f++) {
        free((*cmd).filev[f]);
        (*cmd).filev[f] = NULL;
    }
}

void store_command(char ***argvv, char *filev[3], int bg, struct command *cmd) {
    int num_commands = 0;
    while (argvv[num_commands] != NULL) {
        num_commands++;
    }

    int f;
    for (f = 0; f < 3; f++) {
        if (filev[f] != NULL) {
            (*cmd).filev[f] = (char *) calloc(strlen(filev[f]), sizeof(char));
            strcpy((*cmd).filev[f], filev[f]);
        }
    }

    (*cmd).bg = bg;
    (*cmd).num_commands = num_commands;
    (*cmd).argvv = (char ***) calloc((num_commands + 1), sizeof(char **));
    (*cmd).args = (int *) calloc(num_commands, sizeof(int));
    int i;
    for (i = 0; i < num_commands; i++) {
        int args = 0;
        while (argvv[i][args] != NULL) {
            args++;
        }
        (*cmd).args[i] = args;
        (*cmd).argvv[i] = (char **) calloc((args + 1), sizeof(char *));
        int j;
        for (j = 0; j < args; j++) {
            (*cmd).argvv[i][j] = (char *) calloc(strlen(argvv[i][j]), sizeof(char));
            strcpy((*cmd).argvv[i][j], argvv[i][j]);
        }
    }
}

int command_executor(char ***argvv, char **filev, int num_commands, int bg) {
    /* Check requirement of maximum number of commands */
    if (num_commands > MAX_PIPED_COMMANDS) {
        printf("Error: number of commands exceeded\n");
        return -1;
    }

    int p[num_commands - 1][2]; /* Need N-1 pipes for N commands */
    pid_t pid, child_pid;     /* Parent and children pids */
    int i;                    /* Loop iterator */
    int status;               /* Exit code of child */

    /* Loop for each command */
    for (i = 0; i < num_commands; i++) {

        /* Initialization of pipes */
        if (i < num_commands - 1) {
            if (pipe(p[i]) < 0) {
                perror("Error creating pipe");
                return -1;
            }
        }

        /* Child creation */
        pid = fork();

        switch (pid) {
            case -1:
                perror("Error creating the child");
                return -1;
            case 0:
                /* CHILD */
                printf("Child %d\n", getpid());

                /* Redirection of standard output to pipe, except last command */
                if (i != num_commands - 1) {
                    if (close(STDOUT_FILENO) < 0) {
                        perror("Error closing standard output");
                        return -1;
                    }
                    if (dup(p[i][STDOUT_FILENO]) < 0) {
                        perror("Error redirecting output to pipe");
                        return -1;
                    }
                    if (close(p[i][STDOUT_FILENO]) < 0) {
                        perror("Error closing output of pipe");
                        return -1;
                    }
                    if (close(p[i][STDIN_FILENO]) < 0) {
                        perror("Error closing input of pipe");
                        return -1;
                    }
                }

                /* Redirection of standard input to pipe, except first command */
                if (i != 0) {
                    if (close(STDIN_FILENO) < 0) {
                        perror("Error closing standard input");
                        return -1;
                    }
                    if (dup(p[i - 1][STDIN_FILENO]) < 0) {
                        perror("Error redirecting input to pipe");
                        return -1;
                    }
                    if (close(p[i - 1][STDIN_FILENO]) < 0) {
                        perror("Error closing input of pipe");
                        return -1;
                    }
                    if (close(p[i - 1][STDOUT_FILENO]) < 0) {
                        perror("Error closing output of pipe");
                        return -1;
                    }
                }

                /* Redirection of standard input of first command to file (if required) */
                if ((i == 0) && (filev[0] != NULL)) {
                    if (close(STDIN_FILENO) < 0) {
                        perror("Error closing standard input");
                        return -1;
                    }
                    if (open(filev[0], O_RDONLY) < 0) {
                        perror("Error opening file for input redirection");
                        return -1;
                    }
                }

                /* Redirection of standard output of last command to file (if required) */
                if ((i == (num_commands - 1)) && (filev[1] != NULL)) {
                    if (close(STDOUT_FILENO) < 0) {
                        perror("Error closing standard output");
                        return -1;
                    }
                    if (open(filev[1], O_CREAT | O_TRUNC | O_WRONLY, 0666) < 0) {
                        perror("Error opening file for output redirection");
                        return -1;
                    }
                }

                /* Redirection of standard error output to file (if required) */
                if (filev[2] != NULL) {
                    if (close(STDERR_FILENO) < 0) {
                        perror("Error closing standard error output");
                        return -1;
                    }
                    if (open(filev[2], O_CREAT | O_TRUNC | O_WRONLY, 0666) < 0) {
                        perror("Error opening file for error output redirection");
                        return -1;
                    }
                }

                /* Execution of the command */
                if (execvp(argvv[i][0], argvv[i]) < 0) {
                    perror("Error while executing a command");
                    exit(-1);
                }

                break; /* End of child block */

            default:
                /* PARENT */
                /* Clean used pipes */
                if (i != 0) {
                    if (close(p[i - 1][STDOUT_FILENO]) < 0) {
                        perror("Error closing output of pipe");
                        return -1;
                    }
                    if (close(p[i - 1][STDIN_FILENO]) < 0) {
                        perror("Error closing input of pipe");
                        return -1;
                    }
                }

                /* If not executed in background, wait for children */
                if (bg == 0) {
                    child_pid = waitpid(pid, &status, 0);
                    if (pid != child_pid) {
                        perror("Error while waiting for the child");
                        return -1;
                    }
                    printf("Wait child %d\n", child_pid);
                } else {
                    printf("[%d]\n", pid);
                }
        }
    }
    return 0;
}

int show_saved_commands(struct command *saved_commands, int number_executed_commands) {
    int i;
    int command_counter;
    int args_counter;
    // Iterate through the array of saved_commands to print all arguments and commands, as in the initial code given
    for (i = 0; i < number_executed_commands; ++i) {
        printf("%d ", i);
        for (command_counter = 0; command_counter < saved_commands[i].num_commands; command_counter++) {
            for (args_counter = 0; (saved_commands[i].argvv[command_counter][args_counter] != NULL); args_counter++) {
                printf("%s ", saved_commands[i].argvv[command_counter][args_counter]);
            }
            if (command_counter != saved_commands[i].num_commands - 1) {
                printf("| ");
            }
        }

        if (saved_commands[i].filev[0] != NULL) printf("< %s ", saved_commands[i].filev[0]);/* IN */

        if (saved_commands[i].filev[1] != NULL) printf("> %s ", saved_commands[i].filev[1]);/* OUT */

        if (saved_commands[i].filev[2] != NULL) printf(">& %s ", saved_commands[i].filev[2]);/* ERR */

        if (saved_commands[i].bg) printf("& ");

        printf("\n");
    }
    return 0;
}

void reorder_stored_commands(struct command *saved_commands) {
    // Free the first command stored
    free_command(&saved_commands[0]);
    int i;
    // Move the others one place to make room for the new command to be stored
    for (i = 0; i < MAX_STORED_COMMANDS - 1; ++i) {
        memcpy(&saved_commands[i], &saved_commands[i + 1], sizeof(struct command));
    }
}

int myhistory(char ***argvv, struct command *saved_commands, const int number_executed_commands, int num_commands) {
    // If the command is executed with no arguments, show the stored ones
    if (argvv[0][1] == NULL) {
        show_saved_commands(saved_commands, number_executed_commands);
    } else if (argvv[0][2] == NULL) {
        // Get the number set as argument of the command
        int selected_command = (int) strtol(argvv[0][1], NULL, 10);
        // If the number is between 0 and the number of commands executed, run that command
        // NOTE: the number_executed_commands variable stops increasing after reaching MAX_STORED_COMMANDS value
        if (selected_command >= 0 && selected_command < number_executed_commands) {
            printf("Running command %s\n", argvv[0][1]);
            command_executor(saved_commands[selected_command].argvv,
                             saved_commands[selected_command].filev,
                             num_commands,
                             saved_commands[selected_command].bg);

        } else {
            printf("ERROR: Command not found\n");
            return -1;
        }
    } else {
        printf("Error: number of arguments exceeded\n");
        return -2;
    }
    return 0;
}

int mycd(char *path) {
    /* If no path is provided, get the HOME path */
    if (path == NULL) {
        if (chdir(getenv("HOME")) < 0) {
            perror("mycd error");
            return -1;
        }
    } else { /* Path provided */
        if (chdir(path) < 0) {
            perror("mycd error");
            return -1;
        }
    }

    /* Get the absolute path of the changed directory */
    char *final_dir = getcwd(NULL, 0);
    if (final_dir == NULL) {
        perror("mycd error");
        return -1;
    }

    printf("%s\n", final_dir);
    return 0;
}

int mytime(time_t start) {
    time_t now;
    double diff_t;
    int hours, mins, secs, remainder;

    if (time(&now) < 0) {
        perror("mytime error");
        return -1;
    }

    diff_t = difftime(now, start);

    if (diff_t < 0) {
        printf("mytime error: error calculating difference of time\n");
        return -1;
    }

    hours = (int) diff_t / 3600;
    remainder = (int) diff_t % 3600;
    mins = remainder / 60;
    secs = remainder % 60;

    printf("Uptime: %d h. %d min. %d s.\n", hours, mins, secs);

    return 0;
}

int myexit(char ****argvv, struct command *saved_commands, char **filev, int number_executed_commands) {
    printf("Goodbye!\n");
    // Following the structure in free_command(), check if there's been a command stored
    if (saved_commands != NULL) {
        int i;
        // Iterate through the saved commands and free each one of them
        for (i = 0; i < number_executed_commands; i++) {
            free_command(&saved_commands[i]);
        }
        // Free the array storing structs
        free(saved_commands);
    }
    // If the argvv array of the current command being executed exists, free it
    if (argvv != NULL) {
        char **argv;
        // Iterate through the commands
        for (; *argvv && **argvv; *argvv = *argvv + 1) {
            // Iterate through the arguments of the command
            for (argv = **argvv; argv && *argv; argv++) {
                // If it exists, free it and set to NULL the reference to the argument
                if (*argv) {
                    free(*argv);
                    *argv = NULL;
                }
            }
        }
        // Delete the reference to the command
        *argvv = NULL;
    }
    int f;
    for (f = 0; f < 3; f++) {
        // Free the array of redirections and set the reference to null
        free(filev[f]);
        filev[f] = NULL;
    }
    return 0;
}

int main(void) {
    char ***argvv;
    int num_commands;
    char *filev[3];
    int bg;
    int ret;

    setbuf(stdout, NULL);            /* Unbuffered */
    setbuf(stdin, NULL);

    time_t start_t;                  /* Initial time */
    time(&start_t);                  /* Store initial time */

    int number_executed_commands = 0;
    struct command *saved_commands;
    saved_commands = calloc(MAX_STORED_COMMANDS, sizeof(struct command));

    while (1) {
        fprintf(stderr, "%s", "msh> ");    /* Prompt */
        ret = obtain_order(&argvv, filev, &bg);
        if (ret == 0) break;        /* EOF */
        if (ret == -1) continue;    /* Syntax error */
        num_commands = ret - 1;        /* Line */
        if (num_commands == 0) continue;    /* Empty line */

/*
 * CODE ADDED BEYOND THIS POINT
 */

        if (strcmp(argvv[0][0], "mytime") == 0) {
            mytime(start_t);
        } else if (strcmp(argvv[0][0], "mycd") == 0) {
            mycd(argvv[0][1]);
        } else if (strcmp(argvv[0][0], "myhistory") == 0) {
            myhistory(argvv, saved_commands, number_executed_commands, num_commands);
        } else if (strcmp(argvv[0][0], "exit") == 0) {
            if (myexit(&argvv, saved_commands, filev, number_executed_commands) == 0) {
                exit(EXIT_SUCCESS);
            } else {
                perror("failed while freeing resources of msh");
                exit(EXIT_FAILURE);
            }
        } else {
            if (number_executed_commands < MAX_STORED_COMMANDS) {
                // If the number of executed commands is below the maximum established,
                // the array is not full and doesn't need reordering
                store_command(argvv, filev, bg, &saved_commands[number_executed_commands]);
                number_executed_commands = number_executed_commands + 1;
            } else {
                // If the number of executed commands has been reached, the array must be reordered
                reorder_stored_commands(saved_commands);
                // Store the command into the last position
                store_command(argvv, filev, bg, &saved_commands[MAX_STORED_COMMANDS - 1]);
            }
            // Execute the command
            command_executor(argvv, filev, num_commands, bg);
        }

        /* Wait for background processes that have ended (in zombie state) without blocking the execution */
        while (waitpid(-1, NULL, WNOHANG) > 0) {}

    } //fin while

    return 0;

} //end main
