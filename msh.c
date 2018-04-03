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

int single_command_executor(char ***argvv, int bg) {
        int syscall_status;
        int executed_command_status;
        pid_t child_pid;
        pid_t pid = fork();

        switch (pid) {
        case -1:
                perror("Error creating the child");
                return -1;
        case 0:
                printf("Child <%d>\n", getpid());
                syscall_status = execvp(argvv[0][0], argvv[0]);
                if (syscall_status < 0) {
                        // The syscall exec() did not find the command required to execute
                        perror("Error in the execution of the command");
                        exit(syscall_status);
                }
                break;
        default:
                if (!bg) {
                        /* Wait for the children created in the fork, not children from previous forks  */
                        child_pid = waitpid(pid, &executed_command_status, 0);

                        if (child_pid != pid) {
                                perror("Error while waiting for the child");
                                return -1;
                        }

                        /*if (executed_command_status != 0) {
                            // The command exited with a number diferent from 0
                            perror("Error while executing the command");
                            return -1;
                           }*/
                        printf("Wait child <%d>\n", child_pid);
                }
                return 0;
        }
}

int piped_command_executor(char ***argvv, char **filev, int num_commands, int bg) {
        /* Check requirement of maximum number of commands */
        if(num_commands > MAX_PIPED_COMMANDS) {
                printf("Error: number of commands exceeded\n");
                return -1;
        }

        int p[num_commands-1][2]; /* Need N-1 pipes for N commands */
        pid_t pid, child_pid;     /* Parent and children pids */
        int i;                    /* Loop iterator */
        int status;               /* Exit code of child */

        /* Loop for each command */
        for(i = 0; i < num_commands; i++) {

                /* Initialization of pipes */
                if(i < num_commands-1) {
                        if(pipe(p[i]) < 0) {
                                perror("Error creating pipe");
                                return -1;
                        }
                }

                /* Child creation */
                pid = fork();

                switch(pid) {
                case -1:
                        perror("Error creating the child");
                        return -1;
                case 0:
                        /* CHILD */
                        printf("Child %d\n", getpid());

                        /* Redirection of standard output to pipe, except last command */
                        if(i != num_commands-1) {
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
                        if(i != 0) {
                                if (close(STDIN_FILENO) < 0) {
                                  perror("Error closing standard input");
                                  return -1;
                                }
                                if (dup(p[i-1][STDIN_FILENO]) < 0) {
                                  perror("Error redirecting input to pipe");
                                  return -1;
                                }
                                if (close(p[i-1][STDIN_FILENO]) < 0) {
                                  perror("Error closing input of pipe");
                                  return -1;
                                }
                                if (close(p[i-1][STDOUT_FILENO]) < 0) {
                                  perror("Error closing output of pipe");
                                  return -1;
                                }
                        }

                        /* Redirection of standard input of first command to file (if required) */
                        if((i == 0) && (filev[0] != NULL)) {
                                if(close(STDIN_FILENO) < 0) {
                                  perror("Error closing standard input");
                                  return -1;
                                }
                                if(open(filev[0], O_RDONLY) < 0) {
                                  perror("Error opening file for input redirection");
                                  return -1;
                                }
                        }

                        /* Redirection of standard output of last command to file (if required) */
                        if((i == (num_commands-1)) && (filev[1] != NULL)) {
                                if(close(STDOUT_FILENO) < 0) {
                                  perror("Error closing standard output");
                                  return -1;
                                }
                                if(open(filev[1], O_CREAT|O_TRUNC|O_WRONLY, 0666) < 0) {
                                  perror("Error opening file for output redirection");
                                  return -1;
                                }
                        }

                        /* Redirection of standard error output to file (if required) */
                        if(filev[2] != NULL) {
                                if(close(STDERR_FILENO) < 0) {
                                  perror("Error closing standard error output");
                                  return -1;
                                }
                                if(open(filev[2], O_CREAT|O_TRUNC|O_WRONLY, 0666) < 0) {
                                  perror("Error opening file for error output redirection");
                                  return -1;
                                }
                        }

                        /* Execution of the command */
                        if(execvp(argvv[i][0], argvv[i]) < 0) {
                                perror("Error while executing a command");
                                exit(-1);
                        }

                        break; /* End of child block */

                default:
                        /* PARENT */
                        /* Clean used pipes */
                        if(i != 0) {
                                if(close(p[i-1][STDOUT_FILENO]) < 0) {
                                  perror("Error closing output of pipe");
                                  return -1;
                                }
                                if(close(p[i-1][STDIN_FILENO]) < 0) {
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
    char* final_dir = getcwd(NULL, 0);
    if(final_dir == NULL) {
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

  time(&now);
  diff_t = difftime(now, start);

  hours = diff_t / 3600;
  remainder = (long)diff_t % 3600;
  mins = remainder / 60;
  secs = remainder % 60;

  printf("Uptime: %d h. %d min. %d s.\n", hours, mins, secs);

  return 0;
}

int main(void) {
        char ***argvv;
        int command_counter;
        int num_commands;
        int args_counter;
        char *filev[3];
        int bg;
        int ret;

        setbuf(stdout, NULL);        /* Unbuffered */
        setbuf(stdin, NULL);
        
        time_t start_t;
        time(&start_t);

        while (1) {
                fprintf(stderr, "%s", "msh> "); /* Prompt */
                ret = obtain_order(&argvv, filev, &bg);
                if (ret == 0) break; /* EOF */
                if (ret == -1) continue; /* Syntax error */
                num_commands = ret - 1; /* Line */
                if (num_commands == 0) continue; /* Empty line */

                if(strcmp(argvv[0][0], "mytime") == 0) {
                    mytime(start_t);
                    continue;
                }

                if(strcmp(argvv[0][0], "mycd") == 0) {
                    mycd(argvv[0][1]);
                    continue;
                }

                if (num_commands == 1) {
                    single_command_executor(argvv, bg);
                } else {
                    piped_command_executor(argvv, filev, num_commands, bg);
                }

        } //fin while

        return 0;

} //end main
