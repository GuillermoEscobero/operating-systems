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
    }
    return 0;
}

int redirected_command_executor(char ***argvv, char **filev) {
    int syscall_status;
    int executed_command_status;
    pid_t child_pid;

    int pipe_fd[2];
    pipe(pipe_fd);

    pid_t pid = fork();
    switch (pid) {
        case -1:
            perror("Error creating the child");
            return -1;
        case 0:
            printf("Child <%d>\n", getpid());
            close(STDOUT_FILENO);
            dup(pipe_fd[STDOUT_FILENO]);
            close(pipe_fd[STDOUT_FILENO]);
            close(pipe_fd[STDIN_FILENO]);

            //ERROR: que pasa si no es single command?
            syscall_status = execvp(argvv[0][0], argvv[0]);
            if (syscall_status < 0) {
                // The syscall exec() did not find the command required to execute
                perror("Error in the execution of the command");
                exit(-1);
            }
        default:
            close(STDIN_FILENO);
            dup(pipe_fd[STDIN_FILENO]);
            close(pipe_fd[STDIN_FILENO]);
            close(pipe_fd[STDOUT_FILENO]);

            int file_descriptor;
            void *buffer;

            if (filev[0] != NULL) {
                file_descriptor = open(filev[0], O_RDONLY);

            } else if (filev[1] != NULL) {
                file_descriptor = open(filev[1], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU + S_IRWXG + S_IRWXO);

            } else {
                file_descriptor = open(filev[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU + S_IRWXG + S_IRWXO);

            }
            //TODO: hacerlo para toods los casos
            struct stat st;
            stat(filev[1], &st);
            size_t binary_size = st.st_size;

            buffer = malloc(binary_size);

            wait(&executed_command_status);
            if (executed_command_status != 0) {
                // The command exited with a number diferent from 0
                perror("Error while executing the command");
                return -1;
            }

            read(STDIN_FILENO, &buffer, binary_size);
            write(file_descriptor, &buffer, binary_size);

            close(STDIN_FILENO);
            close(file_descriptor);

            //ERROR: no funca pero si hace el archivo, algun wait o algo hay que hacer
            return 0;

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

                if (executed_command_status != 0) {
                    // The command exited with a number diferent from 0
                    perror("Error while executing the command");
                    return -1;
                }
                printf("Wait child <%d>\n", child_pid);
            }
            return 0;
    }
}

int piped_command_executor(char ***argvv, int num_commands) {
    printf("Piped command bru\n");
    for (int i = 0; i < num_commands; ++i) {

    }
    return 0;
}

int mycd(char *path) {
    /* Change path to new one */
    if(chdir(path) < 0) {
        perror("mycd error");
        return -1;
    }

    /* Get the absolute path of the changed directory */
    char* final_dir = getcwd(NULL, 0);

    if(final_dir == NULL) {
        perror("getcwd error");
        return -1;
    }

    printf("%s\n", final_dir);
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

    setbuf(stdout, NULL);            /* Unbuffered */
    setbuf(stdin, NULL);

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

        if(strcmp(argvv[0][0], "mycd") == 0) {
            mycd(argvv[0][1]);
            continue;
        }

        if (is_redirected(filev)) {
            redirected_command_executor(argvv, filev);
        }
        if (num_commands == 1) {
            single_command_executor(argvv, bg);
        } else {
            piped_command_executor(argvv, num_commands);
        }






















        /*
               for (command_counter = 0; command_counter < num_commands; command_counter++)
               {
                   *//* argvv: complete entry form the terminal ([0][0] = ls e.g.)*//*
			for (args_counter = 0; (argvv[command_counter][args_counter] != NULL); args_counter++)
			{
				printf("%s ", argvv[command_counter][args_counter]);
			}
			printf("\n");
		}

		if (filev[0] != NULL) printf("< %s\n", filev[0]);*//* IN *//*

		if (filev[1] != NULL) printf("> %s\n", filev[1]);*//* OUT *//*

		if (filev[2] != NULL) printf(">& %s\n", filev[2]);*//* ERR *//*

		if (bg) printf("&\n");
*/

/*
 * END OF THE PART TO BE REMOVED
 */

    } //fin while

    return 0;

} //end main
