#include <unistd.h>
#include <stdio.h>
#include "smalloc.h" 
#include <limits.h>
#include <string.h>

sm_container_t sm_head = {
    0,
    &sm_head, 
    &sm_head,
    0 
} ;

static 
    void * 
_data (sm_container_ptr e)
{
    return ((void *) e) + sizeof(sm_container_t) ;
}

static 
    void 
sm_container_split (sm_container_ptr hole, size_t size)
{
    sm_container_ptr remainder = (sm_container_ptr) (_data(hole) + size) ;

    remainder->dsize = hole->dsize - size - sizeof(sm_container_t) ;	
    remainder->status = Unused ;
    remainder->next = hole->next ;
    remainder->prev = hole ;
    hole->dsize = size ;
    hole->next->prev = remainder ;
    hole->next = remainder ;
}

static 
    void * 
retain_more_memory (int size)
{
    sm_container_ptr hole ;
    int pagesize = getpagesize() ;
    int n_pages = 0 ;

    n_pages = (sizeof(sm_container_t) + size + sizeof(sm_container_t)) / pagesize  + 1 ;
    hole = (sm_container_ptr) sbrk(n_pages * pagesize) ;
    if (hole == 0x0)
	return 0x0 ;

    hole->status = Unused ;
    hole->dsize = n_pages * getpagesize() - sizeof(sm_container_t) ;
    return hole ;
}

    void * 
smalloc (size_t size) 
{
    sm_container_ptr hole = 0x0, itr = 0x0 ;
    int best_fit = INT_MAX;
    sm_container_ptr best = 0x0 ;

    for (itr = sm_head.next ; itr != &sm_head ; itr = itr->next) {
	if (itr->status == Busy)
	    continue ;

	printf("itr->dsize = %lu, size = %lu\n", itr->dsize , size);
	if ((itr->dsize == size) || (size + sizeof(sm_container_t) < itr->dsize)) {
	    if(best_fit > (itr->dsize - (size + sizeof(sm_container_t)))) {
		best_fit = itr->dsize - (size + sizeof(sm_container_t)) ;	    
		printf("best_fit = %d\n", best_fit);
		best = itr;
	    }
	}
    }

    for (itr = sm_head.next ; itr != &sm_head ; itr = itr->next) {
	if(best == itr) {
	    hole = itr;
	    break;
	}
    }


    if (hole == 0x0) {
	hole = retain_more_memory(size) ;
	if (hole == 0x0)
	    return 0x0 ;
	hole->next = &sm_head ;
	hole->prev = sm_head.prev ;
	(sm_head.prev)->next = hole ;
	sm_head.prev = hole ;
    }
    if (size < hole->dsize) 
	sm_container_split(hole, size) ;
    hole->status = Busy ;
    return _data(hole) ;
}

    void 
sfree (void * p)
{
    sm_container_ptr itr , hole = 0x0;

    for (itr = sm_head.next ; itr != &sm_head ; itr = itr->next) {
	if (p == _data(itr)) {
	    itr->status = Unused ;
	    break ;
	}
    }

    if(itr != sm_head.prev && itr->next->status == Unused) {
	itr->next = itr->next->next ;
	itr->next->next->prev = itr->next->prev ;
	itr->dsize += (itr->next->prev->dsize + sizeof(sm_container_t)) ;
    }

    if(itr != sm_head.next && itr->prev->status == Unused) {
	itr->prev->next = itr->next ;
	itr->next->prev = itr->prev ;
	itr->prev->dsize += (itr->dsize + sizeof(sm_container_t)) ;
	itr = itr->prev ;
    }

    //    if((int)itr->dsize >=  getpagesize()) {
    //	printf("itr->dszie = %lu, int version = %d\n", itr->dsize, (int)itr->dsize);
    //	int size = (int)itr->dsize - getpagesize() ;
    //	
    //	if(itr != sm_head.next) {
    //		sm_container_split(itr, size) ;
    //		itr->prev->next = itr->next ;
    //		itr->next->prev = itr->prev ;
    //		//itr->dsize -= (getpagesize());
    //	}	
    //
    //	if(itr != sm_head.prev) {
    //		sm_container_split(itr, size) ;
    //		itr->next = itr->next->next ;
    //		itr->next->next->prev = itr->next->prev ;
    //		itr->dsize -= getpagesize() ;
    //	}
    //    }
    //
    //
}

    void 
print_sm_containers ()
{
    sm_container_ptr itr ;
    int i ;

    printf("==================== sm_containers ====================\n") ;
    for (itr = sm_head.next, i = 0 ; itr != &sm_head ; itr = itr->next, i++) {
	printf("%3d:%p:%s:", i, _data(itr), itr->status == Unused ? "Unused" : "  Busy") ;
	printf("%8d:", (int) itr->dsize) ;

	int j ;
	char * s = (char *) _data(itr) ;
	for (j = 0 ; j < (itr->dsize >= 8 ? 8 : itr->dsize) ; j++) 
	    printf("%02x ", s[j]) ;
	printf("\n") ;
    }
    printf("\n") ;

}

    void
print_mem_uses() 
{
    sm_container_ptr itr ;
    int total_heap_memory = 0 ;
    int allocated_heap_memory = 0 ;
    int remaining_heap_memory = 0 ;

    for(itr = sm_head.next; itr != &sm_head ; itr = itr->next) {
	total_heap_memory += ((sizeof(sm_container_t)) + itr->dsize) ;
	if(itr->status == Busy) {
	    allocated_heap_memory += itr->dsize ;
	}else {
	    remaining_heap_memory += itr->dsize ;
	}
    }

    fprintf(stderr, "Total allcated memory by smalloc = %d bytes\n", total_heap_memory) ;
    fprintf(stderr, "Used = %d bytes\n", allocated_heap_memory) ;
    fprintf(stderr, "Available = %d bytes\n", remaining_heap_memory) ;
    fprintf(stderr, "--------------------------------------------------------\n\n");

}

    void *
srealloc (void * p, size_t newsize)
{
    sm_container_ptr itr , itr2;

    for(itr = sm_head.next; itr != &sm_head; itr = itr->next) {
	if (p == _data(itr)) {
	    break ;
	}
    }

    if(newsize == itr->dsize) return _data(itr) ;

    if((newsize + sizeof(sm_container_t)) < itr->dsize) {
    	sm_container_split(itr, newsize) ;
	sfree(_data(itr->next)) ;	
	
	if(itr->next == sm_head.prev) {
	    sshrink();
	}

	return _data(itr);
    }

    int extends = 0 ;
    for(itr2 = itr->next; itr2 != &sm_head ; itr2 = itr2->next) {
	if(itr2->status == Busy) break;
	extends += (sizeof(sm_container_t) + itr2->dsize) ;
	
	if(extends > newsize) break ;
    }
    
    if(extends >= newsize) {
	itr2->next->prev = itr ;
	itr->next = itr2->next ;
	itr->dsize += extends ;

	sm_container_split(itr, newsize) ;
    	return _data(itr) ;
    }


    sm_container_ptr new ;
    new = smalloc(newsize) ;
    memcpy(new, _data(itr), itr->dsize) ;
    return new ;


}

    void
sshrink()
{
    int reduce = 0 ;
    sm_container_ptr itr ;

    for(itr = sm_head.prev; itr != &sm_head; itr = itr->prev) {
	if(itr->status == Busy) break ;

	reduce += (itr->dsize + sizeof(sm_container_t)) ;
	sm_head.prev = itr->prev;
	itr->prev->next = &sm_head ;
    }
    if(reduce == 0) return;

    sbrk(reduce * (int)(-1)) ;
}
