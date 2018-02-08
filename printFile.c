#include "person.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int readBinary(char* inputFile);

int main(int argc, char *argv[]){
    if(argc != 2) {
        return 1;
    }
    
    //readBinary(argv[1]);
	
    /*int inputFileDesc = open(argv[1], O_RDONLY);
    
    if(inputFileDesc < 0)
        return 1;

    if(close(inputFileDesc) < 0)
        return 1;
    */
	return readBinary(argv[1]);
}

int readBinary(char* inputFile) {
    struct person input;

    int inputFileDesc = open(inputFile, O_RDONLY);
    if(inputFileDesc < 0)
        return 1;

    struct stat st;
    stat(inputFile, &st);
    long binSize = st.st_size;

    long fileRows = binSize/(sizeof(input.name) + sizeof(input.age) + 
            sizeof(input.id) + sizeof(input.control) + sizeof(input.salary));

    lseek(inputFileDesc, 0, SEEK_SET);

    int i;

    for(i = 0; i < fileRows; i++) {
        read(inputFileDesc, &input.name, sizeof(input.name));
        read(inputFileDesc, &input.age, sizeof(input.age));
        read(inputFileDesc, &input.id, sizeof(input.id));
        read(inputFileDesc, &input.control, sizeof(input.control));
        read(inputFileDesc, &input.salary, sizeof(input.salary));

        printf("%s\t%d\t%d\t%c\t%f\n", input.name, input.age, input.id, input.control, input.salary);
    }

    close(inputFileDesc);

    return 0;

}

