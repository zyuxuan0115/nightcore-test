#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <chrono>
#include <iostream>

using namespace std::chrono;

void *functionCond(void*);
void *functionMutex(void*);
void function();

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; 

bool condition = false;
std::chrono::_V2::system_clock::time_point start1;

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
   if (stick_this_thread_to_core(0)==EINVAL) exit(-1);

   start1 = high_resolution_clock::now();
   pthread_mutex_lock(&mutex2);
   auto end0 = high_resolution_clock::now();
   auto duration = duration_cast<nanoseconds>(end0 - start1);
   std::cout << "Time spent on acquire a clock: "<<duration.count()<<" ns" << std::endl;
   pthread_mutex_unlock(&mutex2);

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
   start1 = high_resolution_clock::now(); 
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
   start1 = high_resolution_clock::now(); 
   pthread_cond_signal(&cond);
   pthread_mutex_unlock( &mutex1);
 
   pthread_join( thread, NULL);

   //--------------------------------------//
   // normal function call
   auto start2 = high_resolution_clock::now(); 
   function();
   auto end2 = high_resolution_clock::now();
   auto duration2 = duration_cast<nanoseconds>(end2 - start2);
   std::cout <<"Time spent on normal func call: "<<duration2.count()<<" ns" << std::endl;

   exit(0);
}

void *functionCond(void*){
   if (stick_this_thread_to_core(2)==EINVAL) exit(-1);
   pthread_mutex_lock( &mutex1 );
   while(!condition){
      pthread_cond_wait(&cond, &mutex1); //wait for the condition
   }
   // do something (serverless function)
   pthread_mutex_unlock( &mutex1);
   auto end1 = high_resolution_clock::now();
   auto duration = duration_cast<nanoseconds>(end1 - start1);
   std::cout << "Time spent on pthread (CondVar):"<<duration.count()<<" ns" << std::endl;

}

void *functionMutex(void*){
   if (stick_this_thread_to_core(8)==EINVAL) exit(-1);
   pthread_mutex_lock( &mutex2 );
   // do something (serverless function)
   pthread_mutex_unlock( &mutex2);
   auto end1 = high_resolution_clock::now();
   auto duration = duration_cast<nanoseconds>(end1 - start1);
   std::cout << "Time spent on pthread (mutex):"<<duration.count()<<" ns" << std::endl;
}

void function(){

}
