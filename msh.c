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

#define INPUT_REDIRECTION 0
#define OUTPUT_REDIRECTION 1
#define ERROR_REDIRECTION 2
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
            (*cmd).filev[f] = (char *) malloc(strlen(filev[f]) * sizeof(char));
            strcpy((*cmd).filev[f], filev[f]);
        }
    }

    (*cmd).bg = bg;
    (*cmd).num_commands = num_commands;
    (*cmd).argvv = (char ***) malloc((num_commands + 1) * sizeof(char **));
    (*cmd).args = (int *) malloc(num_commands * sizeof(int));
    int i;
    for (i = 0; i < num_commands; i++) {
        int args = 0;
        while (argvv[i][args] != NULL) {
            args++;
        }
        (*cmd).args[i] = args;
        (*cmd).argvv[i] = (char **) malloc((args + 1) * sizeof(char *));
        int j;
        for (j = 0; j < args; j++) {
            (*cmd).argvv[i][j] = (char *) malloc(strlen(argvv[i][j]) * sizeof(char));
            strcpy((*cmd).argvv[i][j], argvv[i][j]);
        }
    }
}

int is_redirected(char **filev) {
    if (filev[0] != NULL || filev[1] != NULL || filev[2] != NULL) {
        return 1;
    } else {
        return 0;
    }
}

int get_redirection_type(char **filev) {
    if (filev[0] != NULL && filev[1] == NULL && filev[2] == NULL) {
        return INPUT_REDIRECTION;
    } else if (filev[1] != NULL && filev[0] == NULL && filev[2] == NULL) {
        return OUTPUT_REDIRECTION;
    } else if (filev[2] != NULL && filev[0] == NULL && filev[1] == NULL) {
        return ERROR_REDIRECTION;
    }
    // Else, there is more than one redirection
    perror("Invalid redirection type");
    return -1;
}

int show_saved_commands(struct command **saved_commands, int number_executed_commands) {
    int i;
    for (i = 0; i < number_executed_commands; ++i) {
        printf("%d ", i);
        int j;
        for (j = 0; j < saved_commands[i]->num_commands; ++j) {
            int k;
            for (k = 0; k < saved_commands[i]->args[j]; ++k) {
                printf("%s ", saved_commands[i]->argvv[j][k]);
            }
            if (is_redirected(saved_commands[i]->filev)) {
                switch (get_redirection_type((saved_commands[i]->filev))) {
                    case INPUT_REDIRECTION:
                        printf("< ");
                        printf("%s", saved_commands[i]->filev[INPUT_REDIRECTION]);
                        break;
                    case OUTPUT_REDIRECTION:
                        printf("> ");
                        printf("%s", saved_commands[i]->filev[OUTPUT_REDIRECTION]);
                        break;
                    case ERROR_REDIRECTION:
                        printf(">& ");
                        printf("%s", saved_commands[i]->filev[ERROR_REDIRECTION]);
                        break;
                    default:
                        perror("Error while reading the redirection attribute");
                        return -1;
                }
            }

            if (saved_commands[i]->num_commands != 1 && saved_commands[i]->num_commands != j + 1) {
                printf("| ");
            }
        }
        if (saved_commands[i]->bg) {
            printf("& ");
        }
        printf("\n");
        //FIXME: cuando tengo redireccion se va a la puta
    }
    return 0;
}

void reorder_stored_commands(struct command **saved_commands) {
    free_command(saved_commands[0]);
    int i;
    for (i = 0; i < MAX_STORED_COMMANDS - 1; ++i) {
        memcpy(saved_commands[i], saved_commands[i + 1], sizeof(struct command));
    }
}

void
store_struct_command(struct command **saved_commands, int *number_executed_commands, struct command current_command) {
    if (*number_executed_commands < MAX_STORED_COMMANDS) {
        memcpy(saved_commands[*number_executed_commands], &current_command, sizeof(struct command));
        *number_executed_commands = *number_executed_commands + 1;

    } else {
        reorder_stored_commands(saved_commands);
        memcpy(saved_commands[MAX_STORED_COMMANDS - 1], &current_command, sizeof(struct command));
    }

}

void saved_command_executor(struct command **saved_commands, int selected_command, int num_commands) {
    // if (saved_commands[selected_command]->num_commands == 1) {
    //     single_command_executor(saved_commands[selected_command]->argvv, saved_commands[selected_command]->filev,
    //                             saved_commands[selected_command]->bg);
    // } else {
        command_executor(saved_commands[selected_command]->argvv, saved_commands[selected_command]->filev,
                               num_commands,
                               saved_commands[selected_command]->bg);
    // }
}

int myhistory(char ***argvv, struct command **saved_commands, int *number_executed_commands, int num_commands) {
    if (argvv[0][1] == NULL) {
        show_saved_commands(saved_commands, *number_executed_commands);
    } else {
        //FIXME: si aun no esta lleno esto no da error
        if (atoi(argvv[0][1]) >= 0 && atoi(argvv[0][1]) < MAX_STORED_COMMANDS) {
            printf("Running command %s\n", argvv[0][1]);
            saved_command_executor(saved_commands, atoi(argvv[0][1]), num_commands);
        } else {
            printf("Error: command not found\n");
            return -1;
        }
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

    if(time(&now) < 0) {
      perror("mytime error");
      return -1;
    }

    diff_t = difftime(now, start);

    hours = diff_t / 3600;
    remainder = (long) diff_t % 3600;
    mins = remainder / 60;
    secs = remainder % 60;

    printf("Uptime: %d h. %d min. %d s.\n", hours, mins, secs);

    return 0;
}

int myexit() {
    //TODO:
    return 0;
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
                // if (strcmp(argvv[0][0], "mytime") == 0) {
                //     if(mytime(start_t) < 0) {
                //       exit(-1);
                //     }
                // } else

                if (strcmp(argvv[0][0], "mycd") == 0) {
                    if(mycd(argvv[0][1]) < 0) {
                      exit(-1);
                    }
                } else

                // if (strcmp(argvv[0][0], "myhistory") == 0) {
                //     if(myhistory(argvv, saved_commands, &number_executed_commands, num_commands) < 0) {
                //       exit(-1);
                //     }
                // } else

                if (strcmp(argvv[0][0], "exit") == 0) {
                    if(myexit() < 0) {
                      exit(-1);
                    }
                } else

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
                    child_pid = wait(&status);
                    printf("Wait child %d\n", child_pid);
                } else {
                    printf("[%d]\n", pid);
                }
        }
    }
    return 0;
}

int main(void) {
    char ***argvv;
    // int command_counter;
    int num_commands;
    // int args_counter;
    char *filev[3];
    int bg;
    int ret;

    setbuf(stdout, NULL);            /* Unbuffered */
    setbuf(stdin, NULL);

    time_t start_t;                  /* Initial time */
    time(&start_t);                  /* Store initial time */

    int number_executed_commands = 0;
    struct command **saved_commands;
    saved_commands = malloc(sizeof(struct command) * MAX_STORED_COMMANDS);
    int i;
    for (i = 0; i < MAX_STORED_COMMANDS; ++i) {
        saved_commands[i] = malloc(sizeof(struct command));
        saved_commands[i]->args = malloc(sizeof(char));
        saved_commands[i]->argvv = malloc(sizeof(char));
        *saved_commands[i]->argvv = malloc(sizeof(char *));
        **saved_commands[i]->argvv = malloc(sizeof(char **));
        saved_commands[i]->filev[0] = NULL;
        saved_commands[i]->filev[1] = NULL;
        saved_commands[i]->filev[2] = NULL;
    }

    while (1) {
        fprintf(stderr, "%s", "msh> ");    /* Prompt */
        ret = obtain_order(&argvv, filev, &bg);
        if (ret == 0) break;        /* EOF */
        if (ret == -1) continue;    /* Syntax error */
        num_commands = ret - 1;        /* Line */
        if (num_commands == 0) continue;    /* Empty line */

/*
 * THE PART THAT MUST BE REMOVED STARTS HERE
 * THE FOLLOWING LINES ONLY GIVE AN IDEA OF HOW TO USE THE STRUCTURES
 * argvv AND filev. THESE LINES MUST BE REMOVED.
 */
        struct command *current_command;
        current_command = malloc(sizeof(struct command));
        current_command->args = malloc(sizeof(char));
        current_command->argvv = malloc(sizeof(char));
        *current_command->argvv = malloc(sizeof(char *));
        **current_command->argvv = malloc(sizeof(char **));
        current_command->filev[0] = NULL;
        current_command->filev[1] = NULL;
        current_command->filev[2] = NULL;

        store_command(argvv, filev, bg, current_command);
        store_struct_command(saved_commands, &number_executed_commands, *current_command);

        command_executor(argvv, filev, num_commands, bg);

    } //fin while

    return 0;

} //end main
