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


int singleCommandExecutor(char ** argvv) {
    printf("Single command bru\n");
    return 0;
}

int pipedCommandExecutor() {
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

        if (num_commands == 1) {
            singleCommandExecutor(argvv[0]);
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


