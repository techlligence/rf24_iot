#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <cstring>

using namespace std;

int main(int argc, char **argv)
{
	string s = "Hello World!";
	char message[s.length() + 1];

	strcpy(message, s.c_str());
	
	int fd = open("toggleFIFO", O_WRONLY);
	if(fd == -1){
		return 1;
	}

	while(1){
		if(write(fd, &message, sizeof(message)) == -1){
			return 2;
		}
	}
	close(fd);

	return 0;

}

