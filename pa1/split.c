#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "person.h"

int splitBinary(char *inputFile, char *youngerOutputFile, char *olderOutputFile,
  int thresholdAge);


int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Error: Wrong arguments number");
        return -1;
    }

    return splitBinary(argv[2], argv[3], argv[4],
      (int) strtol(argv[1], NULL, 10));

}

int splitBinary(char *inputFile, char *youngerOutputFile, char *olderOutputFile,
  int thresholdAge) {
    Person auxPerson;

    /* We get the input file descriptor of the input file with a read only
     * file permission */
    int fdInput = open(inputFile, O_RDONLY);

    /* We open both output files in order to store the information, the flags
     * allow the creation of the file or addition at the end to a pre existing
     * one */
    int fdYoungerOutput = open(youngerOutputFile, O_CREAT | O_APPEND | O_WRONLY,
                               S_IRWXU + S_IRWXG + S_IRWXO);
    int fdOlderOutput = open(olderOutputFile, O_CREAT | O_APPEND | O_WRONLY,
                              S_IRWXU + S_IRWXG + S_IRWXO);

    /* Check if an error ocurred when opening file */
    if (fdInput < 0) {
        printf("Error: No such file or directory");
        return -1;
    }

    if (fdYoungerOutput < 0 || fdOlderOutput < 0) {
        printf("Error: Output files creation failed");
        return -1;
    }


    /* We use stat system call to get the size of the binary file */
    struct stat st;
    stat(inputFile, &st);
    long binSize = st.st_size;

    /* We calculate the number of entries that the binary file has */
    long fileRows = binSize / sizeof(Person);

    if (fileRows == 0) {
        printf("Error: Input file is empty");
        return -1;
    }

    /* Looks for the first location of the input file in memory */
    if(lseek(fdInput, 0, SEEK_SET) == -1) {
      perror("Error");
      return -1;
    }

    /* This loop will repeat the number of structs contained in the binary
     * file */
    int i;
    for (i = 0; i < fileRows; i++) {
        /* We read each struct in the file and save the value in the
         * auxiliary struct input to be compared later */
        if(read(fdInput, &auxPerson, sizeof(auxPerson)) == -1) {
          perror("Error");
          return -1;
        }

        if (auxPerson.age < thresholdAge) {
            /* If the age of the read person is below the threshold we write
             * it in the corresponding file */
            if(write(fdYoungerOutput, &auxPerson, sizeof(auxPerson)) == -1) {
              perror("Error");
              return -1;
            }
        } else {
            /* Fill the other file if the condition was not fulfilled */
            if(write(fdOlderOutput, &auxPerson, sizeof(auxPerson)) == -1) {
              perror("Error");
              return -1;
            }
        }
    }

    /* Close input and output files */
    close(fdInput);
    close(fdOlderOutput);
    close(fdYoungerOutput);

    return 0;

}
