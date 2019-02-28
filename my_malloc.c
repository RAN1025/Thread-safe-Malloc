#include "my_malloc.h"

pthread_mutex_t lock;

//free list: locking version
information_t *listhead=NULL;

//free list: no-lock version
__thread information_t *threadhead=NULL;

//check free list
//if requires sbrk, return NULL
//else return the pointer to the free block
void *bf_checklist(information_t *onehead,size_t size){
  information_t *current=onehead;
  information_t *bestfit=NULL;
  //exist free block
  while(current!=NULL){
    //best free block with enough space
    if(current->size >= size){
      //return directly
      if(current->size <=(size+sizeof(information_t))){
	bestfit=current;
	return bestfit;
      }
      //record
      if(bestfit==NULL){
	bestfit=current;
      }
      else if(current->size < bestfit->size){
	bestfit=current;
      }
    }
    current=current->next;
  }
  return bestfit;
}

//malloc with sbrk
//set the information block
void *sbrk_malloc(size_t size){
  information_t *newblock=sbrk(0);
  void *realblock=sbrk(size+sizeof(information_t));
  //sbrk fail
  if(realblock==(void*)-1){
    return NULL;
  }
  newblock->size=size;
  newblock->next=NULL;
  return newblock;
}

//no-lock verion
void *sbrk_malloc_nolock(size_t size){
  pthread_mutex_lock(&lock);
  information_t *newblock=sbrk(0);
  void *realblock=sbrk(size+sizeof(information_t));
  pthread_mutex_unlock(&lock);
  //sbrk fail
  if(realblock==(void*)-1){
    return NULL;
  }
  newblock->size=size;
  newblock->next=NULL;
  return newblock;
}

//split helper
//adjust the free list
//return the answer for the malloc function
void *split_helper(information_t **onehead,information_t *free,size_t size){
  //check wether requires split
  if(free->size <=(size+sizeof(information_t))){
    //do not need to split, return the pointer
    //locates at head
    if(free==*onehead){
      *onehead=free->next;
      return free+1;
    }
    else{
      information_t *current = *onehead;
      //find the previous block
      while(current->next!=free){
	current=current->next;
      }
      current->next=free->next;
      return free+1;
    }
  }
  else{
    //split
    free->size=free->size-size-sizeof(information_t);
    //new information block
    information_t *answer=(information_t*)((char *)free+sizeof(information_t)+free->size);
    answer->size=size;
    answer->next=NULL;
    return answer+1;
  }
}

//add helper
//add newly freed block to the free list
void add_helper(information_t **onehead,information_t *block){
  information_t *current=*onehead;
  if(current>block){
    *onehead=block;
    block->next=current;
  }
  else{
    while(current->next!=NULL && current->next<block){
	current=current->next;
    }
    block->next=current->next;
    current->next=block;
  }
}

//merge adjacent free blocks
void merge_helper(information_t *onehead,information_t *block){
  char *current=(char *)block;
  //merge block with the next block
  if(block->next!=NULL){
    if((current+sizeof(information_t)+block->size)==(char*)block->next){
      block->size=block->size+sizeof(information_t)+(block->next)->size;
      block->next=(block->next)->next;
    }
  }
  //merge block with the previous block
  if(block!=onehead){
    information_t *previous=onehead;
    while(previous->next!=block){
      previous=previous->next;
    }
    if((current-previous->size-sizeof(information_t))==(char*)previous){
      previous->size=previous->size+sizeof(information_t)+block->size;
      previous->next=block->next;
    }
  }
}

//Thraed safe malloc and free: locking version
//best fit malloc
void *ts_malloc_lock(size_t size){
  if(size<=0){
    return NULL;
  }
  pthread_mutex_lock(&lock);
  //check free list
  information_t *free =(information_t*)bf_checklist(listhead,size);
  //sbrk                                                                                                      
  if(free==NULL){
    information_t *answer=(information_t*)sbrk_malloc(size);
    pthread_mutex_unlock(&lock);
    return answer+1;
  }
  else{
    information_t *answer=(information_t*)split_helper(&listhead,free,size);
    pthread_mutex_unlock(&lock);
    return answer;
  }
}

//best fit free
void ts_free_lock(void *ptr){
  if(ptr==NULL){
    return;
  }
  pthread_mutex_lock(&lock);
  //empty free list                                                                                      
  if(listhead==NULL){
    information_t *block=(information_t*)((char *)ptr-sizeof(information_t));
    listhead=block;
  }
  else{
    information_t *block=(information_t*)((char *)ptr-sizeof(information_t));
    add_helper(&listhead,block);
    merge_helper(listhead,block);
  }
   pthread_mutex_unlock(&lock);
}

//Thraed safe malloc and free: non-locking version
//best fit malloc
void *ts_malloc_nolock(size_t size){
  if(size<=0){
    return NULL;
  }
  //check free list
  information_t *free =(information_t*)bf_checklist(threadhead,size);
  //sbrk                                                                                                             
  if(free==NULL){
    information_t *answer=(information_t*)sbrk_malloc_nolock(size);
    return answer+1;
  }
  else{
    information_t *answer=(information_t*)split_helper(&threadhead,free,size);
    return answer;
  }
}

//best fit free
void ts_free_nolock(void *ptr){
  if(ptr==NULL){
    return;
  }
  //empty free list
  if(threadhead==NULL){
    information_t *block=(information_t*)((char *)ptr-sizeof(information_t));
    threadhead=block;
  }
  else{
    information_t *block=(information_t*)((char *)ptr-sizeof(information_t));
    add_helper(&threadhead,block);
    merge_helper(threadhead,block);
  }
}
