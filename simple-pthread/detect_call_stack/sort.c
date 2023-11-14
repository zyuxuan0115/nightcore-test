#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#define ARRAY_LEN 50000
#define TIMES 20
static struct timeval tm1;

void bubble_sort(int* a, int n);
void bubble_sort2(int* a, int n);
void bubble_sort3(int* a, int n);
void* sort_array(void*);

int main(){
   printf("[sort] bubble sort is done\n");
   return 0;
}


void* sort_array(void* arg) {
   int data[ARRAY_LEN], j=0;
   while (j<TIMES){
      for(int i=0; i<ARRAY_LEN; ++i){
         data[i] = rand();
      }
      bubble_sort2(data, ARRAY_LEN);
      for(int i=0; i<ARRAY_LEN; ++i){
         data[i] = rand();
      }
      bubble_sort(data, ARRAY_LEN);
      for(int i=0; i<ARRAY_LEN; ++i){
         data[i] = rand();
      }
      bubble_sort3(data, ARRAY_LEN);
         j++;
   }
}

void bubble_sort (int *a, int n) {
   int i, t, s = 1;
   while (s) {
      s = 0;
      for (i = 1; i < n; i++) {
         if (a[i] < a[i - 1]) {
            t = a[i];
            a[i] = a[i - 1];
            a[i - 1] = t;
            s = 1;
         }
      }
   }
}

void bubble_sort2 (int *a, int n) {
   for (int i = 0; i<n; i++){
      for (int j=i+1; j<n; j++){
         if (a[j] < a[i]){
            int tmp = a[i];
            a[i] = a[j];
            a[j] = tmp;
         }
      }
   }
}

void bubble_sort3 (int *a, int n) {
   for (int i = 0; i<n; i++){
      for (int j=i+1; j<n; j++){
         if (a[j] < a[i]){
            int tmp = a[i];
            a[i] = a[j];
            a[j] = tmp;
         }
      }
   }
}

