#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

void *functionC();
void function();
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
int  counter = 0;

main(){
   int rc1;
   pthread_t thread1;

   clock_t start = clock();
   if( (rc1=pthread_create( &thread1, NULL, &functionC, NULL)) ){
      printf("Thread creation failed: %d\n", rc1);
   }

   pthread_join( thread1, NULL);

   clock_t end = clock();
   float seconds = (float)(end - start) / CLOCKS_PER_SEC;

   printf("Time spent on pthread: %f seconds\n", seconds);

   clock_t start2 = clock();
   function();
   clock_t end2 = clock();
   float seconds2 = (float)(end2 - start2) / CLOCKS_PER_SEC;

   printf("Time spent on normal func call: %f seconds\n", seconds2);

   exit(0);
}

void *functionC(){
   //pthread_mutex_lock( &mutex1 );
   //counter++;
   //printf("Counter value: %d\n",counter);
   //pthread_mutex_unlock( &mutex1 );
}

void function(){

}
