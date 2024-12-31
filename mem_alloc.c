#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>

//To align the memory to 16 bytes
typedef char ALIGN[16];

//Header of the memory block which contains meta data
union header{
    struct{
        size_t size;
        unsigned is_free;
        union header *next;
    }mem;

    ALIGN mem_align;
};

typedef union header mem_header;

//Declaring head and tail of the memory linked list and avoiding race conditions
mem_header *head=NULL, *tail=NULL;
pthread_mutex_t global_malloc_lock;

//Finds the next available memory block based on the is_free attribute of each memory header
mem_header *find_free_mem(size_t size){
    mem_header *curr=head;
    while(curr){
        if(curr->mem.is_free && curr->mem.size>=size){
            return curr;
        }
        curr = curr->mem.next;
    }
    return NULL;
    
}
//Equivalent to free() in c which frees a block of memory and sets the is_free=1
void release(void *block){
    mem_header *header, *temp;
    void *break_program;

    if(!block){
        return;
    }
    pthread_mutex_lock(&global_malloc_lock);
    header=(mem_header *)block-1;
    break_program=sbrk(0);

    if((char *)block +header->mem.size==break_program){
        if(head==tail){
            head=tail=NULL;
        }else{
            temp=head;
            while(temp){
                if(temp->mem.next==tail){
                    temp->mem.next=NULL;
                    tail=temp;
                    break;
                }
                temp=temp->mem.next;
            }
        }
        sbrk(0-header->mem.size-sizeof(mem_header));
        pthread_mutex_unlock(&global_malloc_lock);
        return;
    }
    header->mem.is_free=1;
    pthread_mutex_unlock(&global_malloc_lock);
    
}

//Equivalent of malloc in c which sets a block of memory based on the size requested
void *alloc_mem(size_t size){
    size_t total_size;
    void *block;
    mem_header *header;

    if(!size){
        return NULL;
    }
    pthread_mutex_lock(&global_malloc_lock);
    header=find_free_mem(size);
    if(header){
        header->mem.size=0;
        pthread_mutex_unlock(&global_malloc_lock);
        return (void *)(header +1);
    }
    //If free mem is not found then we need to allocate
    total_size=sizeof(mem_header)+size;
    block=sbrk(total_size);
    if(block==(void *)-1){
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }
    header=block;
    header->mem.size=size;
    header->mem.is_free=0;
    header->mem.next=NULL;

    if(!head){
        head=header;
    }
    else{
        tail->mem.next=header;
    }
    tail=header;
    pthread_mutex_unlock(&global_malloc_lock);
    return (void *)(header+1);

}

//Equivalent to Calloc() in c which takes a number of elements to be intialized and the size of each block
void *set_mem_zero(size_t num, size_t nsize){
    size_t size;
    void * block;
    if(!num||!nsize){
        return NULL;
    }
    size=num*nsize;
    if(nsize!=size/num){
        return NULL;
    }
    block=alloc_mem(size);
    if(!block){
        return NULL;
    }
    memset(block,0,size);
}
//Equivalent to recalloc() in c which takes a pointer a mem block and if the size requested is smaller
//than the actual size of the mem block then return that mem block. Otherwise create a new mem block with the 
//size requested and and return a pointer to the memory block.
void *re_alloc(void *block,size_t size){
    mem_header *header;
    void *ret;
    if(!block||!size){
        return alloc_mem(size);
    }
    header=(mem_header*)block-1;
    if(header->mem.size>=size){
        return block;
    }
    ret=alloc_mem(size);
    
    if(ret){
        memcpy(ret,block,header->mem.size);
        release(block);
    }
    return ret;

}
//Printing the contents of the mem header and associated information
void print_mem_list()
{
	mem_header *curr = head;
	printf("head = %p, tail = %p \n", (void*)head, (void*)tail);
	while(curr) {
		printf("addr = %p, size = %zu, is_free=%u, next=%p\n",
			(void*)curr, curr->mem.size, curr->mem.is_free, (void*)curr->mem.next);
		curr = curr->mem.next;
	}
}

int main(){
    pthread_mutex_init(&global_malloc_lock, NULL);

    void *p1 = alloc_mem(32);  // Allocate 32 bytes
    void *p2 = alloc_mem(64);  // Allocate 64 bytes
    void *p3 = alloc_mem(128); // Allocate 128 bytes
    void *p4=set_mem_zero(15,1); //Set 15 bytes to 0
    re_alloc(p2,100); //Recalloc mem block pointer p2.

    printf("After allocation:\n");
    print_mem_list();
    

    return 0;   
}