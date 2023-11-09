#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "mpt/mpt.h"
#include "mpt/hash.h"
#include "mpt/pkey.h"

void *functionMutex(void*);
void function();
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
clock_t start;

main(){
   // creating thread (mutex version)
   int rc;
   pthread_mutex_lock(&mutex);
   pthread_t thread;
   if( (rc = pthread_create( &thread, NULL, &functionMutex, NULL)) ){
      printf("Thread creation failed: %d\n", rc);
   }
   // do something 
   // the data for serverless function is ready
   start = clock();
   pthread_mutex_unlock( &mutex);
 
   pthread_join( thread, NULL);

   exit(0);
}

void *functionMutex(void*){
   pthread_mutex_lock( &mutex );
   // do something (serverless function)
   pthread_mutex_unlock( &mutex);
   clock_t end = clock();
   float seconds = (float)(end - start) / CLOCKS_PER_SEC;
   printf("Time spent on pthread (Mutex): %f seconds\n", seconds);
}

void function(){

}
