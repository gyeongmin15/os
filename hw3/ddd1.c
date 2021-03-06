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
#include <sched.h>
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
bounded_buffer * buf2 = 0x0 ; 

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER ;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER ;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER ;

int a_m[11][11] ;
int a_path[11] ;
int a_used[11] ;
int a_length = 0;
int a_min = -1;
long check;

int m[50][50];
int ans = INT_MAX;
////////////////
int path[50] ;
int used[50] ;
int length = 0 ;
int min = -1 ;

int prefix;
int N;
int checked_route ;
long bound;
long number_of_subtask;

int sig ;
int number_of_del ;

void _travel(int idx) {
    int i;

    if (idx == 11) {
	a_length += a_m[a_path[10]][a_path[0]] ;
	if (a_min == -1 || a_min > a_length) {
	    a_min = a_length ;
	}
	a_length -= a_m[a_path[10]][a_path[0]] ;
	check++;
    }
    else {
	for (i = 0 ; i < 11 ; i++) {
	    if (a_used[i] == 0) {
		a_path[idx] = i ;
		a_used[i] = 1 ;
		a_length += a_m[a_path[idx-1]][i] ;
		_travel(idx+1) ;
		a_length -= a_m[a_path[idx-1]][i] ;
		a_used[i] = 0 ;
	    }
	}
    }
}

void travel(int start) {
    a_path[0] = start ;
    a_used[start] = 1 ;
    _travel(1) ;
    a_used[start] = 0 ;
}
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
	char s1[10];
	sprintf(s1, "%d", length);
	bounded_buffer_queue(buf, strdup(s1)) ;
	if(min==-1 || min > length) {
	    min = length ;
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
// handler setting
void 
handler (int sig) {
    if(sig == SIGINT) {
	printf(" =>Current Best Solution: %d , Checked Route: %ld\n",ans,check);
	exit(0);
    }
}
void spin()
{
    int i ;
    for (i = 0 ; i < 50000 ; i++) ;
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
    	//printf("->>%d\n",checked_route) ;
    }
    return 0x0 ;
}

int turn;

    void * 
consumer(void * ptr) 
{
    pthread_t tid ;
    char * msg ; 
    int i ;

    tid = pthread_self() ;

    while(bound > 0) {
	msg = bounded_buffer_dequeue(buf) ;

	pthread_mutex_lock(&mutex) ;

	if(number_of_del >0) {
	    pthread_cancel(pthread_self());
	    number_of_del--;
	}
	for(int i=0;i<11;i++){
	    travel(i);
	}
	int sub_result = a_min + atoi(msg);
	if(ans > sub_result)	ans = sub_result;

	//printf("[%ld] result, %d, check : %ld\n",(unsigned long) tid, sub_result, check);
	bound--;

	pthread_mutex_unlock(&mutex) ;
	
	if(sig>0) {
	    bounded_buffer_dequeue(buf2) ;
	    sig--;
	    printf("Thread ID : [%ld], The number of subtasks processed so far : %ld, The number of checked routes : %ld\n", (unsigned long) tid, number_of_subtask - bound ,factorial(11)*(number_of_subtask - bound)) ; 
	}
	spin();

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
    signal(SIGINT, handler) ;
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

    // for consumer thread
    for(int i=0;i<11;i++) {
	for(int j=0;j<11;j++) {
	    a_m[i][j] = m[N-11+i][N-11+j] ;
	}
    }

    pthread_t prod ;
    pthread_t cons[8] ;
    int number_of_thread = atoi(argv[2]);	// number of threads

    int i ;
    prefix = N - 11;
    number_of_subtask = factorial(prefix) ;
    bound = number_of_subtask ;

    buf = malloc(sizeof(bounded_buffer)) ;
    bounded_buffer_init(buf, 30) ;

    buf2 = malloc(sizeof(bounded_buffer)) ;
    
    pthread_create(&(prod), 0x0 , producer, 0x0) ;
    for (i = 0 ; i < number_of_thread ; i++) {
	pthread_create(&(cons[i]), 0x0, consumer, 0x0) ;
    }

    printf("1. state   2.print the information   3.Change the Number of Thread\n");
    while(1) {
	int input;
	//printf("1. state   2.print the information   3.Change the Number of Thread\n");
	scanf("%d",&input);
	if(input == 1) {
	    printf("Best solution : %d, The number of checked routes : %ld\n", ans, check);
	}else if(input == 2){
	    bounded_buffer_init(buf2,number_of_thread) ;
	    for(int i=0;i<number_of_thread;i++) {
		bounded_buffer_queue(buf2, "A") ;
	    }
	    sig = number_of_thread ;
	}else if(input == 3) {
	    printf("Enter the Number of Thread :");
	    int input_thread;
	    scanf("%d",&input_thread);

	    if(input_thread > number_of_thread){
		for(int i=number_of_thread; i<input_thread;i++) {
		    pthread_create(&cons[i], 0x0, consumer, 0x0) ;
		}
		printf("The number of Thread changed to %d\n",input_thread);
		number_of_thread = input_thread ;
	    }else {
		number_of_del = number_of_thread - input_thread;;
		printf("The number of Thread changed to %d\n",input_thread);
		number_of_thread = input_thread ;
	    	while(number_of_del > 0) ;
	    }
	}else {
	    printf("Wrong input Please try again\n");
	}
	
	//printf("1. state   2.print the information   3.Change the Number of Thread\n");
	if(bound == 0) break;
    }
    for (i = 0 ; i < number_of_thread; i++) {
	pthread_join(cons[i], 0x0) ;
    }
    pthread_join(prod, 0x0) ;


    printf("Best Solution : %d, checked route : %ld \n",ans ,check) ;
    exit(0) ;
}
