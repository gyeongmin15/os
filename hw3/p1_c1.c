#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include <error.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <pthread.h>
#include <semaphore.h>

typedef struct {
    sem_t filled ;
    sem_t empty ;
    pthread_mutex_t lock ;
    char ** elem ;
    int capacity ;
    int num ; 
    int front ;
    int rear ;
} bounded_buffer ;

bounded_buffer * buf = 0x0 ;

int m[50][50];
bool visited[50];
int city[50];

int lowcost = INT_MAX;
int checked;

int ans = INT_MAX;
int sub_result;
////////////////
int path[50] ;
int used[50] ;
int length = 0 ;
int min = -1 ;

int prefix;
int N;
int checked_route ;

void 
bounded_buffer_init(bounded_buffer * buf, int capacity) {
    sem_init(&(buf->filled), 0, 0) ;
    sem_init(&(buf->empty), 0, capacity) ;
    pthread_mutex_init(&(buf->lock), 0x0) ;
    buf->capacity = capacity ;
    buf->elem = (char **) calloc(sizeof(char *), capacity) ;
    buf->num = 0 ;
    buf->front = 0 ;
    buf->rear = 0 ;
}

    void 
bounded_buffer_queue(bounded_buffer * buf, char * msg) 
{
    sem_wait(&(buf->empty)) ;
    pthread_mutex_lock(&(buf->lock)) ;
    buf->elem[buf->rear] = msg ;
    buf->rear = (buf->rear + 1) % buf->capacity ;
    buf->num += 1 ;
    pthread_mutex_unlock(&(buf->lock)) ;
    sem_post(&(buf->filled)) ;
}

    char * 
bounded_buffer_dequeue(bounded_buffer * buf) 
{
    char * r = 0x0 ;
    sem_wait(&(buf->filled)) ;
    pthread_mutex_lock(&(buf->lock)) ;
    r = buf->elem[buf->front] ;
    buf->front = (buf->front + 1) % buf->capacity ;
    buf->num -= 1 ;
    pthread_mutex_unlock(&(buf->lock)) ;
    sem_post(&(buf->empty)) ;
    return r ;
}

int
_prefix(int idx) {
    int i;

    if(idx==prefix) {
	if(min==-1 || min > length) {
	    min = length ;

	    char s1[10];
	    sprintf(s1, "%d", min);
	    bounded_buffer_queue(buf, strdup(s1)) ;
	    
	    min = -1;
	}
	    checked_route++;
    }
    else {
	for(i = 0 ; i < prefix ; i++) {
	    if(used[i]==0) {
		path[idx] = i;
		used[i] =1;
		length += m[path[idx-1]][i];
		_prefix(idx+1) ;
		length -= m[path[idx-1]][i];
		used[i] = 0;
	    }
	}
    }
}

void
prefix_start(int start) {
    path[0] = start;
    used[start] = 1;
    _prefix(1) ;
    used[start] = 0 ;
}

    int
factorial(int n) {
    int result = 1;
    for(int i = 1; i<= n ; i++) {
	result = result * i;
    }
return result;
}
    void * 
producer(void * ptr) 
{
    char msg[128] ;
    pthread_t tid ;
    int i ;

    tid = pthread_self() ;
    for (i = 0 ; i < prefix ; i++) {
	prefix_start(i) ;
    }
    return 0x0 ;
}

    void * 
consumer(void * ptr) 
{
    pthread_t tid ;
    char * msg ; 
    int i ;

    tid = pthread_self() ;

    for (i = 0 ; i < factorial(prefix) ; i++) {
	msg = bounded_buffer_dequeue(buf) ;
	if (msg != 0x0) {
	    printf("[%ld] reads %s\n", (unsigned long) tid, msg) ;
	    free(msg) ;
	}
   }
}

    char*
find_N(char* str1)
{
    char* num = malloc(strlen(str1)+1);
    int i=0;
    int j=0;
    while(str1[i] != '\0') {
	if(isdigit(str1[i])) {
	    num[j] = str1[i];
	    j++;
	}
	i++;
    }
    num[j] = '\0';
    return num;
}
    int 
main(int argc, char* argv[]) 
{
    FILE* fp = fopen(argv[1],"r");

    // Get N
    char * str = find_N(argv[1]);
    N = atoi(str);

    // Put Data To Array
    int t;
    for(int i = 0 ; i < N ; i++) {
	for(int j =0;j<N;j++) {
	    fscanf(fp,"%d",&t);
	    m[i][j] = t;
	}
    }
    fclose(fp);

    pthread_t prod ;
    pthread_t cons ;
    int i ;
    prefix = N - 11;

    buf = malloc(sizeof(bounded_buffer)) ;
    bounded_buffer_init(buf, 8) ;

    pthread_create(&(prod), 0x0 , producer, 0x0) ;
    pthread_create(&(cons), 0x0, consumer, 0x0) ;
    //for (i = 0 ; i < atoi(argv[2]) ; i++) {
	//pthread_create(&(cons[i]), 0x0, consumer, 0x0) ;
    //}

    //for (i = 0 ; i < atoi(argv[2]) ; i++) {
	//pthread_join(cons[i], 0x0) ;
    //}
    pthread_join(prod, 0x0) ;
    pthread_join(cons, 0x0) ;
    
    printf("checked route : %d \n",checked_route) ;
    exit(0) ;
}
