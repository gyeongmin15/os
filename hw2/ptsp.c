#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

int pipes[2]; 
int m[50][50];
bool visited[50];
int city[50];

int N;
int lowcost = INT_MAX;
int checked;

int ans = INT_MAX;
int prefix;
int sub_result;

// handler setting
void
handler2 (int sig) { 
    printf(" =>Current Best Solution: %d , Checked Route: %d\n",ans,checked);
    exit(0);
}

void 
handler (int sig) {
    char buf[32];
    if(sig == SIGINT) {
	sprintf(buf,"%d %d",sub_result,checked);
	write(pipes[1], buf, strlen(buf)+1);
	
	exit(0);
    }
}

int 
min(int a, int b) {
    if(a>=b) 
	return b;
    else 
	return a;
}

void 
tsp(int num, int sum, int count, int start, int end) {
    visited[num] = true;
    city[count - 1] = num;
    if (count == end-start) {
	for(int i=start;i<end;i++) {
	    // comback
	    if(i==end-1) {
		sum += m[city[i-start]][city[0]];
	    }
	}
	visited[num] = false;
	city[count - 1] = -1;
	lowcost = min(lowcost,sum);
	checked++;
	return;
    }

    for(int i=start;i<end;i++){
	if(!visited[i] && m[num][i] != 0){
	    tsp(i, sum+m[num][i], count+1, start, end);
	}
    }
    visited[num] = false;
    city[count - 1] = -1;
}

void
parent_process()
{
    signal(SIGINT, handler2);
    char buf[32];
    ssize_t s;

    close(pipes[1]);    

    while((s=read(pipes[0], buf, 31))>0) {
	buf[s-1] = 0x0;
	// Get best solution and checked route
	char * ptr = strtok(buf," ");
	char *temp[2] = {NULL, };
	int i = 0;
	while(ptr!=NULL) {
	    temp[i] = ptr;
	    i++;
	    ptr = strtok(NULL, " ");
	}

	ans = min(ans, atoi(temp[0]));
	checked += atoi(temp[1]);

//	printf("in parents :%d\n",checked);
    }
    close(pipes[0]);
}

void
child_process(int idx)
{
    signal(SIGINT, handler);

    close(pipes[0]);
    char buf[32];
    ssize_t s;
   
    prefix = N-12;

    int j=0;
    sub_result = 0;
    for(int i=0;i<prefix;i++) {
	j = i+1+idx;
	if(j>prefix) {
	    j-=prefix;
	}
	sub_result += m[i][j];

	if(i==prefix-1) {
	    sub_result += m[0][prefix];
	    tsp(i+1,0,1,i+1,i+13);
//	    printf("lowcost: %d, sub result: %d\n",lowcost,sub_result);
	    sub_result += lowcost;
	    printf("Hello I'm child, my result is that  = %d\n",sub_result);
//	    printf("in child :%d\n",checked);

	    sprintf(buf,"%d %d",sub_result,checked);

	    write(pipes[1], buf, strlen(buf)+1);
	}
    }

    close(pipes[1]);
    exit(0);
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
main(int argc, char* argv[]){
    FILE* fp = fopen(argv[1],"r");
    int t;
    pid_t child_pid[12];

    // Get N
    char* str = find_N(argv[1]);
    N = atoi(str);

    // Put Data To Array
    for (int i = 0; i < N ; i++) {
	city[i] = -1;
	visited[i] = false;
	for (int j = 0; j < N; j++){
	    fscanf(fp, "%d", &t);
	    m[i][j] = t;
	}
    }
    fclose(fp);

    // Create Parent Pipe
    if(pipe(pipes) != 0) {
	perror("Error") ;
	exit(1);
    }

    // Create Child
    for(int i=0;i<atoi(argv[2]);i++) {
	child_pid[i] = fork();
	if(child_pid[i]==0) { // child
	    printf("child %d generated by %d\n", getpid(), getppid());
	    child_process(i);
	}else if(child_pid[i]>0) { //parent
	    printf("parent %d spawned child %d\n",getpid(), child_pid[i]);
	}else {
	    printf("Fork Failed\n");
	    exit(1);
	}
    }

    parent_process();
    sleep(2);

    pid_t term_pid ;
    int exit_code;

    for(int i=0;i<atoi(argv[2]); i++) {
	term_pid = wait(&exit_code);
	printf("process %d is finished with exit code %d\n", term_pid, exit_code);
    }

    printf(" =>Best Solution: %d ,Checked Route: %d\n", ans, checked);

    return 0;
}
