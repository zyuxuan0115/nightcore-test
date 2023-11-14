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
#define _GNU_SOURCE

void *functionMutex(void*);
void function();
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
clock_t start;
int pkey;

main(){
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

   int ret = pkey_mprotect(ptr, MMAP_PAGE_SIZE, PROT_NONE, pkey); 
   if (ret < 0) {printf("pkey_mprotect failed\n");}

//   *ptr_int  = 5;
   pthread_mutex_lock(&mutex);
   int* ptr_int = (int*) ptr;
   if ((pkey_set(pkey, PKEY_DISABLE_WRITE)<0) && (pkey>=0)) {printf("pkey_set error\n");}
   
   *ptr_int = 7;
   pthread_mutex_unlock(&mutex);

   // creating thread (mutex version)
   int rc;
   pthread_mutex_lock(&mutex);
   pthread_t thread;
   if( (rc = pthread_create( &thread, NULL, &functionMutex, ptr)) ){
      printf("Thread creation failed: %d\n", rc);
   }

   // do something 
   // the data for serverless function is ready
   start = clock();
   pthread_mutex_unlock( &mutex);
 
   pthread_join( thread, NULL);

   exit(0);
}

void *functionMutex(void* arg){
   int ret = pkey_mprotect(arg, MMAP_PAGE_SIZE, PROT_READ | PROT_WRITE, pkey); 
   int* ptr_int = (int*) arg;
   *ptr_int = 5;

   pthread_mutex_lock( &mutex );
   // do something (serverless function)
   pthread_mutex_unlock( &mutex);
   clock_t end = clock();
   float seconds = (float)(end - start) / CLOCKS_PER_SEC;
   printf("Time spent on pthread (Mutex): %f seconds\n", seconds);
}