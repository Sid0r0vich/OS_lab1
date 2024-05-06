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
volatile int sigalrm_atomic_t = 1;

void catch_int() {
	sigint_atomic_t = 0;
}

void catch_alrm() {
	sigalrm_atomic_t = 0;
}
 
int main()
{	
	struct sigaction my_sigint_handler = {.sa_handler = catch_int}; 
	if ( sigaction(SIGINT, &my_sigint_handler, 0) < 0 ) {
		perror("sigaction error!");
		exit(1);
	};
	
	struct sigaction my_sigalrm_handler = {.sa_handler = catch_alrm}; 
	if ( sigaction(SIGALRM, &my_sigalrm_handler, 0) < 0 ) {
		perror("sigaction error!");
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
    alarm(5);
    while (sigint_atomic_t) {
    	op: fd = open(fifo, O_RDONLY);
        if (fd < 0) {
        	if (errno == EINTR) {
        		if (!sigint_atomic_t) {
        			printf("\ncatch sigint!\nexit...\n");
					exit(0);
				}

				if (!sigalrm_atomic_t) {
					sigalrm_atomic_t = 1;
        			printf("waiting for a message!\n");
        			alarm(5);
					goto op;
				}
        	}

        	perror("could not open fd!");
        	exit(1);
		}
		
        rd: while ( ( res = read(fd, c, 1) ) > 0 ) {
        	printf("%s", c);
        }

		if (res < 0) {
        	if (errno == EINTR) {
        		if (!sigint_atomic_t) {
        			printf("\ncatch sigint!\nexit...\n");
					exit(0);
				}

				if (!sigalrm_atomic_t) {
				    sigalrm_atomic_t = 1;
        			printf("waiting for a message!\n");
        			alarm(5);
					goto rd;
				}
        	}
			
			perror("read error!");
			exit(1);
		}
        
        close(fd);

        if (!sigalrm_atomic_t) {
        	sigalrm_atomic_t = 1;
        	printf("waiting for a message!\n");
        	alarm(5);
        }
    }
    return 0;
}
