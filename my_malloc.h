#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

typedef struct _information_t information_t;
//information block
struct _information_t{
  size_t size;
  information_t *next;
};

//check free list
void *bf_checklist(information_t *onehead,size_t size);

//malloc with sbrk
void *sbrk_malloc(size_t size);

//split helper
void *split_helper(information_t **onehead,information_t *free,size_t size);

//add to free list                                                                                           
void add_helper(information_t **onehead,information_t *block);

//merge adjacent free blocks                                                                                  
void merge_helper(information_t *onehead,information_t *block);

//Thread safe malloc and free: locking version
void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);

//Thread safe malloc and free: non-locking version
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);

#endif
