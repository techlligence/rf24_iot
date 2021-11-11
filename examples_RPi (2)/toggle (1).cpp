#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

int main(int argc, char **argv)
{
	unsigned long message = 6644;
	int fd = open("toggleFIFO", O_WRONLY);
	if(fd == -1){
		return 1;
	}

	while(1){
		printf("Writing message\n");
		write(fd, &message, sizeof(message)); 
	}
	close(fd);

	return 0;

}

