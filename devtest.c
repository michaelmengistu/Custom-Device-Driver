#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    unsigned int result;
    int fd;     // File descriptor 
    int i, j, read_i, read_j;   // variables
	char * wirteToDev= (char*) malloc(2*sizeof(char*)); //will hold the values of inputs i and j, and then wirtes to my multiplier device
    char* readFromDev = (char*) malloc(3*sizeof(char*));; //reads from my multiplier device
    char input = 0;
    
	
    fd = open("/dev/multiplier",O_RDWR); // Opens my  file for reading and writing 
   
   
	/*checks and returns error if the multiplier device not found*/
    if(fd == -1) {
        printf("Failed to open device file!\n");
        return -1;
    }
    
	/*multiplies i and j using my multiplier device */
    for(i = 0; i <= 16; i++) {
        for(j = 0; j <= 16; j++) {
			
            		/*Write value to registers using device*/
			wirteToDev[0] = i; //stores i in buffer
			wirteToDev[1] = j; // stores j in buffer
			write(fd, wirteToDev, 2); //wirtes  i and j into device
			
            		/*Reads i, j, and result from the device*/
			read(fd, readFromDev, 3); //reads from device and stores in a buffer
			read_i = (int) readFromDev[0];
			read_j = (int) readFromDev[1];
			result = (int) readFromDev[2];
            printf("%u * %u = %u\n\r", read_i, read_j, result); //prints i, j, and result
                
            /* Validates result*/ 
            if(result == (i*j))
                printf("Result Correct!");
            else
                printf("Result Incorrect!");
                
            
            input = getchar();// Reads from terminal 
            /*Continue unless user entered 'q' */
            if(input == 'q') {
                close(fd); //closes device
                return 0;
            }
        }
    }
    close(fd); //closes device
    return 0;
}
