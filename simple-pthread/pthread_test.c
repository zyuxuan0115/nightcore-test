#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

void *functionCond(void*);
void *functionMutex(void*);
void function();
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; 
int  counter = 0;
bool condition = false;
clock_t start;

main(){
   // creating thread (condition variable version)
   int rc1;
   pthread_t thread;
   if( (rc1 = pthread_create( &thread, NULL, &functionCond, NULL)) ){
      printf("Thread creation failed: %d\n", rc1);
   }

   pthread_mutex_lock(&mutex);
   // do something 
   // the data for serverless function is ready
   condition = true;
   start = clock();
   pthread_cond_signal(&cond);
   pthread_mutex_unlock( &mutex);
 
   pthread_join( thread, NULL);

   //--------------------------------------//
   // creating thread (mutex version)
   int rc2;
   pthread_mutex_lock(&mutex);
   pthread_t thread2;
   if( (rc2 = pthread_create( &thread2, NULL, &functionMutex, NULL)) ){
      printf("Thread creation failed: %d\n", rc2);
   }
   // do something 
   // the data for serverless function is ready
   start = clock();
   pthread_mutex_unlock( &mutex);
 
   pthread_join( thread2, NULL);

   //--------------------------------------//
   // normal function call
   clock_t start2 = clock();
   function();
   clock_t end2 = clock();
   float seconds2 = (float)(end2 - start2) / CLOCKS_PER_SEC;

   printf("Time spent on normal func call: %f seconds\n", seconds2);

   exit(0);
}

void *functionCond(void*){
   pthread_mutex_lock( &mutex );
   while(!condition){
      pthread_cond_wait(&cond, &mutex); //wait for the condition
   }
   // do something (serverless function)
   pthread_mutex_unlock( &mutex);
   clock_t end = clock();
   float seconds = (float)(end - start) / CLOCKS_PER_SEC;
   printf("Time spent on pthread (CondVar): %f seconds\n", seconds);
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
