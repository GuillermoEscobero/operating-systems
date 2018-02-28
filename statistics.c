#include "person.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int calculateStatistics(char * inputFile);

int main(int argc, char *argv[]){
    /* Check that the arguments number is correct */
    if(argc != 2) {
        return 1;
    }

	return calculateStatistics(argv[1]);
}

int calculateStatistics(char* inputFile) {

    /* Declaration of auxiliary person struct */
    Person input;

    /* We get the file descriptor of the input file */
    int inputFileDesc = open(inputFile, O_RDONLY);

    /* Check if an error ocurred when opening file */
    if(inputFileDesc < 0)
        return 1;

    /* We use stat system call to get the size of the binary file */
    struct stat st;
    stat(inputFile, &st);
    long binSize = st.st_size;

    /* We calculate the number of entries that the binary file has */
    long fileRows = binSize/sizeof(Person);

    int i;

    /* Declaration of variables that will store the raw data for calculating
     * the stats */
    double totalSalary = 0.0;
    unsigned int totalAge = 0;

    /* We create arrays of 128 (printable ascii code) to store the frequency of
     * ID char and the total age of the people with that ID char */
    unsigned int arrayChar[128] = {0};
    unsigned int arrayAge[128] = {0};

    /* This loop will repeat the number of structs contained in the binary
     * file */
    for(i = 0; i < fileRows; i++) {
        /* This line reads each struct in the file and save the value in the
         * auxiliary struct input */
        read(inputFileDesc, &input, sizeof(Person));

        totalSalary += input.salary;
        totalAge += input.age;

        /* We increment the frequency of the ID control char */
        arrayChar[(int)input.id_ctrl] += 1;
        /* We add the current age to the total age of this ID control char */
        arrayAge[(int)input.id_ctrl] += input.age;
    }

    /* We calculate the most frequent ID control character, saving the ascii
     * code in index variable */
    int max = arrayChar[0];
    int index = 0;

    for(i = 0; i < 255; i++) {
        if(arrayChar[i] > max) {
            max = arrayChar[i];
            index = i;
        }
    }

    /* We print the output required */
    printf("Average salary: %.0f\n", totalSalary/fileRows);
    printf("Average age: %ld\n", totalAge/fileRows);
    printf("Most frequent ID control character: %c\n", (char)index);
    /* To calculate the avg age for the most frequent ID control char, we get
     * the total age of people with that ID control, and divide by the
     * frequency of that ID control char */
    printf("Average age for the Most frequent ID control character: %d\n",
            arrayAge[index]/arrayChar[index]);

    /* Close input file */
    close(inputFileDesc);

    return 0;
}
