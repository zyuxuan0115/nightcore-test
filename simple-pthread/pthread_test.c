#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <chrono>
#include <iostream>
#include <signal.h>
using namespace std::chrono;

void *functionCond(void*);
void *functionMutex(void*);
void *functionSignal(void*);
void function();
int stick_this_thread_to_core(int core_id);

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; 

sigset_t fSigSet;

bool condition = false;
std::chrono::_V2::system_clock::time_point start;

main(){
   sigemptyset(&fSigSet);
   sigaddset(&fSigSet, SIGUSR1);
   sigaddset(&fSigSet, SIGSEGV);
   pthread_sigmask(SIG_BLOCK, &fSigSet, NULL);

   if (stick_this_thread_to_core(0)==EINVAL) exit(-1);

   auto start0 = high_resolution_clock::now();
   pthread_mutex_lock(&mutex2);
   auto end0 = high_resolution_clock::now();
   auto duration = duration_cast<nanoseconds>(end0 - start0);
   std::cout << "Time spent on acquiring a free lock: "<<duration.count()<<" ns" << std::endl;
   pthread_mutex_unlock(&mutex2);

   //--------------------------------------//
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
   start = high_resolution_clock::now(); 
   pthread_cond_signal(&cond);
   pthread_mutex_unlock( &mutex1);
 
   pthread_join( thread, NULL);


   //--------------------------------------//
   // creating thread (condition variable version)
   int rc3;
   pthread_t thread3;
   if( (rc3 = pthread_create (&thread3, NULL, &functionSignal, NULL)) ){
      printf("Thread creation failed: %d\n", rc3);
   }
   sleep(1);
   // do something 
   // the data for serverless function is ready
   start = high_resolution_clock::now(); 
   pthread_kill( thread3, SIGUSR1);
 
   pthread_join( thread3, NULL);

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
   start = high_resolution_clock::now(); 
   pthread_mutex_unlock( &mutex2);
   pthread_join( thread2, NULL);



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
  // if (stick_this_thread_to_core(3)==EINVAL) exit(-1);
   pthread_mutex_lock( &mutex1 );
   while(!condition){
      pthread_cond_wait(&cond, &mutex1); //wait for the condition
   }
   // do something (serverless function)
   pthread_mutex_unlock( &mutex1);
   auto end = high_resolution_clock::now();
   auto duration = duration_cast<nanoseconds>(end - start);
   std::cout << "Time spent on pthread (CondVar):"<<duration.count()<<" ns" << std::endl;
   pthread_exit(0);
}

void *functionMutex(void*){
  // if (stick_this_thread_to_core(7)==EINVAL) exit(-1);
   pthread_mutex_lock( &mutex2 );
   // do something (serverless function)
   pthread_mutex_unlock( &mutex2);
   auto end = high_resolution_clock::now();
   auto duration = duration_cast<nanoseconds>(end - start);
   std::cout << "Time spent on pthread (mutex):"<<duration.count()<<" ns" << std::endl;
   pthread_exit(0);
}

void *functionSignal(void*){
  // if (stick_this_thread_to_core(15)==EINVAL) exit(-1);
   int fSigReceived;
   sigwait(&fSigSet, &fSigReceived); 
   // do something (serverless function)
   auto end = high_resolution_clock::now();
   auto duration = duration_cast<nanoseconds>(end - start);
   std::cout << "Time spent on pthread (signal):"<<duration.count()<<" ns" << std::endl;
   pthread_exit(0);
}

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

void function(){

}
