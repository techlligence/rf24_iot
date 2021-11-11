#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

using namespace std;

int main(int argc, char **argv)
{
	char message[1024];
	int fd = open("toggleFIFO", O_RDONLY);
	if(fd == -1){
		return 1;
	}

	while(1){
		printf("Reading message\n");
		read(fd, message, 1024); 
		printf("%s\n", message);
	}
	close(fd);

	return 0;

}

