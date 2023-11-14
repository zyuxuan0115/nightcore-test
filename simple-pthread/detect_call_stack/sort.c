#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#define ARRAY_LEN 50000
#define TIMES 20
static struct timeval tm1;

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

int main(){
   printf("[sort] bubble sort is done\n");
   return 0;
}
