#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <err.h>
//#include <mpt/mpt.h>
//#include <mpt/hash.h>
//#include <mpt/pkey.h>

#define MMAP_PAGE_SIZE 4*1024 

void *functionMutex(void*);
void function();

typedef struct pthread_args{
  int pkey;
  int* ptr;
} arg;

int main(){
   void* ptr = mmap(NULL, MMAP_PAGE_SIZE, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
   if (ptr==NULL){
      printf("mmap fails\n");
   }

   // memory cannot be access at all in main thread
   int pkey = pkey_alloc(0, PKEY_DISABLE_ACCESS);
   if (pkey<0){ 
      printf("pkey = %d, no available keys\n", pkey);
      err(EXIT_FAILURE, "pkey_alloc");
   }

   printf("pkey = %d\n", pkey);

   int ret = pkey_mprotect(ptr, MMAP_PAGE_SIZE, PROT_READ | PROT_WRITE, pkey); 
   if (ret < 0) {
     printf("pkey_mprotect failed\n");
   }

   int* ptr_int = (int*) ptr;

   arg a1;
   a1.pkey = pkey;
   a1.ptr = ptr_int;

   // creating thread (mutex version)
   int rc;
   pthread_t thread;
   if( (rc = pthread_create( &thread, NULL, &functionMutex, (void*)&a1)) ){
      printf("Thread creation failed: %d\n", rc);
   }

   // do something 
   // the data for serverless function is ready
 
   pthread_join( thread, NULL);

   exit(0);
}

void *functionMutex(void* arguments){
   arg* args = (arg*)arguments;
   int pkey = args->pkey;
   int *ptr = args->ptr;

   if (pkey_set(pkey, 0)<0)  {
     printf("pkey_set error\n");
   }

   *ptr = 7;

   pthread_exit(0);
}
