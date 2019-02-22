// C program to implement one side of FIFO 
// This side writes first, then reads 
#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <limits.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <sys/inotify.h>
#include "csie_box.h"
#include "dirent.h"
using namespace std;

int main(int argc, char** argv) 
{ 
	int fd1, fd2; 
	int ntfy, wd;

	char arr1[BUFSIZE], arr2[BUFSIZE]; 
	char server_fifo[PATH_MAX], client_fifo[PATH_MAX];
	csie_box server;

	// create FIFO
	server.parse(argv[1]);
	sprintf(server_fifo, "%s/server_to_client.fifo", server.getFifoPath().c_str());
	sprintf(client_fifo, "%s/client_to_server.fifo", server.getFifoPath().c_str());
 	mkfifo(server_fifo, 0666); 
	mkfifo(client_fifo, 0666); 

	// communicate
	fd1 = open(server_fifo, O_WRONLY);
	sprintf(arr1, "%d", getpid());
	write(fd1, arr1, sizeof(arr1)); 
	close(fd1);

	fd2 = open(client_fifo, O_RDONLY);
	read(fd2, arr2, sizeof(arr2)); 
	// printf("Client process ID: %s\n", arr2);
	close(fd2);

	fd1 = open(server_fifo, O_WRONLY);
	server.sendFiles(fd1, server.getDirectory());
	close(fd1);

	ntfy = inotify_init();
	
	while (1) 
	{ 
		wd = inotify_add_watch( ntfy, server.getDirectory().c_str(),
						   IN_CREATE | IN_MODIFY | IN_DELETE );
		int length = read(ntfy, arr1, BUFSIZE );

		struct inotify_event *event = (struct inotify_event*) &arr1[0];     
		if ( event->mask & IN_CREATE ) {
			if ( event->mask & IN_ISDIR ) {
				// printf( "New directory %s created.\n", event->name );
			}
			else {
				// printf( "New file %s created.\n", event->name );
			}
		}
		// // Open FIFO for write only 
		// fd = open(myfifo, O_WRONLY); 
		// // Take an input arr2ing from user. 
		// // 80 is maximum length 
		// fgets(arr2, 80, stdin); 

		// // Write the input arr2ing on FIFO 
		// // and close it 
		// write(fd, arr2, strlen(arr2)+1); 
		// close(fd); 

		// // Open FIFO for Read only 
		// fd = open(myfifo, O_RDONLY); 

		// // Read from FIFO 
		// read(fd, arr1, sizeof(arr1)); 

		// // Print the read message 
		// printf("User2: %s\n", arr1); 
		// close(fd); 
	} 
	return 0; 
} 
