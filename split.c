#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include "person.h"

int splitBinary(char *inputFile, char *youngerOutputFile, char *olderOutputFile, int thresholdAge);


int main(int argc, char *argv[]) {
    //TODO: handle errors
    return splitBinary(argv[2], argv[3], argv[4], atoi(argv[1]));
}

int splitBinary(char *inputFile, char *youngerOutputFile, char *olderOutputFile, int thresholdAge) {
    Person auxPerson;

    /* We get the file descriptor of the input file */
    int fdInput = openat(AT_FDCWD, inputFile, O_RDONLY);
    int fdYoungerOutput = openat(AT_FDCWD, youngerOutputFile, O_CREAT | O_APPEND | O_WRONLY,
                                 S_IRWXU + S_IRWXG + S_IRWXO);
    int fdOlderOutput = openat(AT_FDCWD, olderOutputFile, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU + S_IRWXG + S_IRWXO);

    /* Check if an error ocurred when opening file */
    if (fdInput < 0 || fdYoungerOutput < 0 || fdOlderOutput < 0) {
        return -1;
    }


    /* We use stat system call to get the size of the binary file */
    struct stat st;
    stat(inputFile, &st);
    long binSize = st.st_size;

    /* We calculate the number of entries that the binary file has */
    long fileRows = binSize / sizeof(Person);

    /* Looks for the first location of the input file in memory */
    lseek(fdInput, 0, SEEK_SET);

    /* This loop will repeat the number of structs contained in the binary
     * file */
    for (int i = 0; i < fileRows; i++) {
        /* This line read each struct in the file and save the value in the
         * auxiliary struct input */

        read(fdInput, &auxPerson, sizeof(auxPerson));

        if (auxPerson.age < thresholdAge) {
            //fill the younger file
            write(fdYoungerOutput, &auxPerson, sizeof(auxPerson));
        } else {
            //fill the older file
            write(fdOlderOutput, &auxPerson, sizeof(auxPerson));
        }
    }

    /* Close input file */
    close(fdInput);
    close(fdOlderOutput);
    close(fdYoungerOutput);

    return 0;

}