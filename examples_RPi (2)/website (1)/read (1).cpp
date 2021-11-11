#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <thread>

using namespace std;

int main(int argc, char **argv)
{
	bool testBool = false;
	char message[1024];
	int fd = open("../toggleFIFO", O_RDONLY | O_NONBLOCK);
	if(fd == -1){
		printf("EXITED");
		return 1;
	}
	while(1){	
		while(read(fd,message,1024) != -1 && read(fd,message,104) != 0 ){
			cout << message << endl;
		}
	}

	cout << "Some data present" << endl;

	close(fd);


	return 0;

}

