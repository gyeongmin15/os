#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

    int 
pthread_mutex_lock (pthread_mutex_t * mutex)
{
    static __thread int n_lock = 0 ;
    n_lock += 1 ;

    int (* pthread_mutex_lock_p)(pthread_mutex_t * mutex) = 0 ;	
    char * error ;
    pthread_t tid ;

    pthread_mutex_lock_p = dlsym(RTLD_NEXT, "pthread_mutex_lock") ;
    if((error = dlerror()) != 0x0)
	exit(1) ;
    
    if (mkfifo(".ddtrace",0666)) {
	if(errno != EEXIST) {
	    perror("fail to open fifo: ") ;
	    exit(1) ;
	}
    }

    int fd = open(".ddtrace", O_WRONLY | O_SYNC) ;
    char buf[256] ;

    tid = pthread_self() ;
    snprintf(buf, 256, "lock %ld %d %p", tid, mutex->__data.__owner, mutex) ;
    //fputs(buf, stderr) ;

    if (n_lock == 1) {
    	write(fd, buf, 256) ;
	// backtrace
    }
    close(fd) ;
   	
    n_lock -= 1 ;
    return pthread_mutex_lock_p(mutex) ;
}

    int 
pthread_mutex_unlock (pthread_mutex_t * mutex)
{
    static __thread int n_unlock = 0 ;
    n_unlock += 1 ;

    int (* pthread_mutex_unlock_p)(pthread_mutex_t * mutex) = 0 ;
    char * error ;
    pthread_t tid ;

    pthread_mutex_unlock_p = dlsym(RTLD_NEXT, "pthread_mutex_unlock") ;
    if ((error = dlerror()) != 0x0)
	exit(1) ;

    if (mkfifo(".ddtrace",0666)) {
	if(errno != EEXIST) {
	    perror("fail to open fifo: ") ;
	    exit(1) ;
	}
    }

    int fd = open(".ddtrace", O_WRONLY | O_SYNC) ;
    char buf[256] ;

    tid = pthread_self() ;
    snprintf(buf, 256, "unlock %ld %d %p", tid , mutex->__data.__owner, mutex) ;
    //fputs(buf, stderr) ;
    
    if(n_unlock == 1) {
    	write(fd, buf, 256) ;
    	
	// backtrace
    }
    close(fd) ;
    
    n_unlock -= 1 ;
    return pthread_mutex_unlock_p(mutex) ;
}

