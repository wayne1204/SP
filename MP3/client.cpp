// C program to implement one side of FIFO 
// This side reads first, then reads 
#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <limits.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <sys/signal.h> 
#include <unistd.h> 
#include <iostream>
#include "csie_box.h"
using namespace std;

void sig_handler(int sig);
static csie_box* client;  

int main(int argc, char** argv) 
{ 
	int fd1, fd2; 
	char arr1[80], arr2[80]; 
	char server_fifo[PATH_MAX], client_fifo[PATH_MAX];

	// create FIFO
	client = new csie_box();
	client->parse(argv[1]);
	sprintf(server_fifo, "%s/server_to_client.fifo", client->getFifoPath().c_str());
	sprintf(client_fifo, "%s/client_to_server.fifo", client->getFifoPath().c_str());
 	mkfifo(server_fifo, 0666); 
	mkfifo(client_fifo, 0666);
	signal(SIGINT, sig_handler);
	rmdir(client->getDirectory().c_str());
	mkdir(client->getDirectory().c_str(), 0);

	// communicate
	fd1 = open(server_fifo, O_RDONLY);
	read(fd1, arr1, sizeof(arr1)); 
	// printf("Server process ID: %s\n", arr1);
	close(fd1);

	fd2 = open(client_fifo, O_WRONLY);
	sprintf(arr2, "%d", getpid());
	write(fd2, arr2, sizeof(arr2)); 
	close(fd2);
	chmod(client->getDirectory().c_str(), 0700);

	fd1 = open(server_fifo, O_RDONLY);
	client->readFiles(fd1);
	close(fd1);

	while (1) 
	{ 

	} 
	return 0; 
} 

void sig_handler(int sig){
	signal(SIGINT, sig_handler);
	if(sig == SIGINT){
		string path = client->getDirectory();
		client->removeDir(path);
		rmdir(path.c_str());
		exit(0);
	}
}

