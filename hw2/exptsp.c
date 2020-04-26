#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int pipes[2] ;

void handler (int sig)
{
    if(sig == SIGINT) {
	// Result message
	exit(0) ;
    }
}

int
main(int argc, char* argv[])
{	
    signal(SIGINT, handler) ;
	
    // Create Pipe
    if(pipe(pipes) != 0) {
	perror("Error") ;
	exit(1) ;
    }
    printf("%d %d\n", pipes[0], pipes[1] );
	
	
    for(int i=0;i<argv[2];i++) {
	// child process
	if(fork()==0) {
		printf("child %d from %d\n", getpid(), getppid()) ;

	}else {

	}

    }
}
