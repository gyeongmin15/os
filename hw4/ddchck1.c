#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX 10

int num_node ;
int num_resource  ;

int array[MAX+MAX][MAX+MAX] ;
unsigned long array_tid[MAX] ;
int address[MAX] ;
    
    int
add_node(unsigned long id)
{
    // Check same id in array_tid[]
    int i ;
    for( i = 0 ; i < MAX ; i++) {
	if(array_tid[i] == id) return i;
    }

    // Add node
    array_tid[num_node] = id ;
    num_node++;
    return num_node - 1;
}
    int 
add_resource(int in_address)
{
    // Check same address in array address[]
    int i ;
    for( i = 0; i < MAX ; i++) {
	if(address[i] == in_address) return i;
    }

    // Add resource
    address[num_resource] = in_address ;
    num_resource++;
    return num_resource - 1;
}
    int 
remove_node(unsigned long id)
{
    // Check sam id in array_tid[]
    int i ;
    for( i = 0 ; i < MAX ; i++) {
	if(array_tid[i] == id)	return i;
    }
    return -1;
}
    int
remove_resource(int in_address)
{
    // Check same address in array address[]
    int i ;
    for( i = 0 ; i< MAX ; i++) {
	if(address[i] == in_address) return i;
    }
    return -1;
}
    int
dfs(int i, int *visited)
{
    int j;
    for(j = 0 ; j < MAX+MAX ; j++){
	if(array[i][j]) {
	    if(visited[j]) return 1 ;

	    visited[j] = 1 ;
	    //return dfs(j,visited) ;
	    if(dfs(j,visited))	return 1;
	}
    }
    return 0;
}
    int 
check_cycle(unsigned long id) 
{
    int i;
    int visited[MAX+MAX] = {0, } ;
    for(i = 0 ; i < MAX ; i++) 
    {   
	if(id == array_tid[i]) break;
    }

    visited[i] = 1;
    if(dfs(i,visited)) return 1;
    return 0;
}
    int 
main (int argc, char * argv[])
	{
    int fd = open(".ddtrace", O_RDONLY | O_SYNC) ;

    while (1) {
	char s[256] ;
	int len ;
	char * in_lock ; 
	unsigned long in_thread_id ;
	int in_owner ;
	int in_address ;

	if ((len = read(fd, s, 256)) == -1)
	    break ;
	if (len <= 0) continue; 

	char * ptr = strtok(s, " ") ; // Extract lock or unlock
	in_lock = ptr ;
	printf("-->%s ", in_lock) ;
	
	ptr = strtok(NULL," ") ;	// Extract thread_id
	in_thread_id = strtoul(ptr, NULL, 10) ;	// string to unsigned long
	printf("-->%ld ", in_thread_id) ;

	ptr = strtok(NULL," ") ;
	in_owner = atoi(ptr) ;
	printf("-->%d ", in_owner) ;

	ptr = strtok(NULL," ") ;
	in_address = strtoul(ptr, NULL, 16) ;
	printf("-->%d ", in_address);
	

	if(strcmp(in_lock,"lock") == 0)	
	{
	    int i = add_node(in_thread_id) ;
	    int j = add_resource(in_address) + 10 ;
	    printf("[%d, %d] Edge 생성\n", i , j) ;
	    array[i][j] = 1 ;

	    if(in_owner == 0) {
		array[i][j] = 0 ;
		array[j][i] = 1 ;
	    }

	    if(check_cycle(in_thread_id)) {
		printf("***** DeadLock *****\n");
	    	printf("Bye\n");
		exit(1);
	    }
	   //for(int k=0;k<10;k++){
	//	printf("address[%d] = %d\n", k, address[k]);
	  //	printf("array_tid[%d] = %ld\n", k , array_tid[k]); 
	   //}
	   
	}
	else if(strcmp(in_lock,"unlock") == 0 ) 
	{
	   int i = remove_node(in_thread_id) ;
	   int j = remove_resource(in_address) + 10 ;
	   if(i == -1 || j == 9) {
	       printf("해당 Edge를 찾을 수 없습니다\n") ;
	       continue ;
	   }
	   //for(int k=0;k<10;k++){
		//printf("address[%d] = %d\n", k, address[k]);
	  //	printf("array_tid[%d] = %ld\n", k , array_tid[k]); 
	   //}
	   
//	   printf("[%d, %d] Edge 삭제\n", i, j) ;
//	   array[i][j] = 0 ;
	   if(array[i][j]) {
	   	printf("[%d, %d] Edge 삭제\n", i, j) ;
	   	array[i][j] = 0 ;
	   }else {
		printf("[%d, %d] Edge 삭제\n", j, i) ;
		array[j][i] = 0 ;
	   }
	}


    }
    close(fd) ;
    return 0 ;
}
