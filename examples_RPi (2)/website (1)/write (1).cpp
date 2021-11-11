#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char* argv[]){
	char message[] = "1";
	int fd = open("testFIFO", O_WRONLY);
	if(fd == -1){
		return 1;
	}
	
	if(write(fd, &message, sizeof(message)) == -1){
			return 2;
	}
	printf("%s\n", message);

	close(fd);

	return 0;
}
