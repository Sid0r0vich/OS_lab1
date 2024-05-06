#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

volatile int sigint_atomic_t = 1;

void catch_int() {
	sigint_atomic_t = 0;
}
 
int main()
{	
	struct sigaction my_sigint_handler = {.sa_handler = catch_int}; 
	if ( sigaction(SIGINT, &my_sigint_handler, 0) < 0 ) {
		fprintf(stderr, "sigaction error!\n");
		exit(1);
	};
	
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	
    int fd;
    char* fifo = "/tmp/fifo";

 	unlink(fifo);
    if (mkfifo(fifo, 0666) == -1) {
    	perror("mkfifo error!");
    	exit(1);
    };
 
    char c[2];
    c[1] = 0;
    int res;
    printf("start!\n");
    while (sigint_atomic_t) {
        fd = open(fifo, O_RDONLY);
        if (fd < 0) {
        	if (errno == EINTR && !sigint_atomic_t) {
        		printf("\ncatch sigint!\nexit...\n");
				exit(0);
        	}

        	perror("could not open fd!");
        	exit(1);
		}
		
        while ( ( res = read(fd, c, 1) ) > 0 ) {
        	printf("%s", c);
        }

		if (res < 0) {
			if (errno == EINTR && !sigint_atomic_t) {
				printf("\ncatch sigint!\nexit...\n");
				exit(0);
			}
			
			perror("read error!");
			exit(1);
		}
        
        close(fd);
    }
    return 0;
}
