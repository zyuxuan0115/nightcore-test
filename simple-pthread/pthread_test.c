#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

void *functionCond(void*);
void *functionMutex(void*);
void function();

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; 

bool condition = false;
clock_t start;

int stick_this_thread_to_core(int core_id) {
   int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
   if (core_id < 0 || core_id >= num_cores)
      return EINVAL;

   cpu_set_t cpuset;
   CPU_ZERO(&cpuset);
   CPU_SET(core_id, &cpuset);

   pthread_t current_thread = pthread_self();    
   return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

main(){
   stick_this_thread_to_core(0);

   //--------------------------------------//
   // creating thread (mutex version)
   int rc2;
   pthread_mutex_lock(&mutex2);
   pthread_t thread2;
   if( (rc2 = pthread_create( &thread2, NULL, &functionMutex, NULL)) ){
      printf("Thread creation failed: %d\n", rc2);
   }
   // do something 
   // the data for serverless function is ready
   sleep(1);
   start = clock();
   pthread_mutex_unlock( &mutex2);
 
   pthread_join( thread2, NULL);

   // creating thread (condition variable version)
   int rc1;
   pthread_t thread;
   if( (rc1 = pthread_create( &thread, NULL, &functionCond, NULL)) ){
      printf("Thread creation failed: %d\n", rc1);
   }
   sleep(1);
   pthread_mutex_lock(&mutex1);
   // do something 
   // the data for serverless function is ready
   condition = true;
   start = clock();
   pthread_cond_signal(&cond);
   pthread_mutex_unlock( &mutex1);
 
   pthread_join( thread, NULL);

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
   stick_this_thread_to_core(2);
   pthread_mutex_lock( &mutex1 );
   while(!condition){
      pthread_cond_wait(&cond, &mutex1); //wait for the condition
   }
   // do something (serverless function)
   pthread_mutex_unlock( &mutex1);
   clock_t end = clock();
   float seconds = (float)(end - start) / CLOCKS_PER_SEC;
   printf("Time spent on pthread (CondVar): %f seconds\n", seconds);
}

void *functionMutex(void*){
   stick_this_thread_to_core(8);
   pthread_mutex_lock( &mutex2 );
   // do something (serverless function)
   pthread_mutex_unlock( &mutex2);
   clock_t end = clock();
   float seconds = (float)(end - start) / CLOCKS_PER_SEC;
   printf("Time spent on pthread (Mutex): %f seconds\n", seconds);
}

void function(){

}
