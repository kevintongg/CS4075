#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "math.h"

#define MAX 4
#define MAX_THREADS 1024

long thread_count;
long long n;
double sum;
int a[MAX][MAX];
int x[MAX];
int y[MAX];

void Gen_data();

void *Pth_mat_vec(void *rank);

/* Only executed by main thread */
void Get_args(int argc, char *argv[]);

void Usage(char *prog_name);

int main(int argc, char *argv[]) {
  long thread;  /* Use long in case of a 64-bit system */
  pthread_t *thread_handles;
  
  Gen_data();
  
  printf("–Matrix–\n");
  for (int i = 0; i < MAX; ++i) {
    for (int j = 0; j < MAX; ++j) {
      printf("%d ", a[i][j]);
    }
    printf("\n");
  }
  printf("\n");
  
  printf("–Vector–\n");
  for (int i = 0; i < MAX; ++i) {
    printf("%d ", x[i]);
    printf("\n");
  }
  printf("\n");
  
  /* Get number of threads from command line */
  Get_args(argc, argv);
  
  thread_handles = malloc(thread_count * sizeof(pthread_t));
  for (thread = 0; thread < thread_count; thread++) {
    pthread_create(&thread_handles[thread], NULL, Pth_mat_vec, (void *) thread);
  }
  for (thread = 0; thread < thread_count; thread++) {
    pthread_join(thread_handles[thread], NULL);
  }
  
  printf("–Result–\n");
  for (int i = 0; i < MAX; ++i) {
    printf("%d ", y[i]);
    printf("\n");
  }
  
  free(thread_handles);
}

void Gen_data() {
  unsigned int seed = (unsigned) time(NULL);
  
  for (int i = 0; i < MAX; ++i) {
    for (int j = 0; j < MAX; ++j) {
      a[i][j] = rand_r(&seed) % 10;
    }
  }
  for (int k = 0; k < MAX; ++k) {
    x[k] = rand_r(&seed) % 10;
  }
}

//void *Pth_mat_vec(void *rank) {
//  int temp = step++;
//  for (int i = temp * MAX / 4; i < (temp + 1) * MAX; ++i) {
//    y[i] = 0;
//    for (int j = 0; j < MAX; ++j) {
//      y[i] += a[i][j] * x[j];
//    }
//  }
//  return NULL;
//}

/*------------------------------------------------------------------*/
void *Pth_mat_vec(void *rank) {
  long my_rank = (long) rank;
  long i;
  long j;
  long local_n = MAX / thread_count;
  long my_first_row = my_rank * local_n;
  long my_last_row = (my_rank + 1) * local_n - 1;

  for (i = my_first_row; i <= my_last_row; i++) {
    y[i] = 0;
    for (j = 0; j < MAX; ++j) {
      y[i] += a[i][j] * x[j];
    }
  }

  return NULL;
}  /* Thread_sum */

/*------------------------------------------------------------------
 * Function:    Get_args
 * Purpose:     Get the command line args
 * In args:     argc, argv
 * Globals out: thread_count, n
 */
void Get_args(int argc, char *argv[]) {
  if (argc != 2) {
    Usage(argv[0]);
  }
  thread_count = strtol(argv[1], NULL, 10);
  if (thread_count <= 0 || thread_count > MAX_THREADS) {
    Usage(argv[0]);
  }
//  n = strtoll(argv[2], NULL, 10);
//  if (n <= 0) {
//    Usage(argv[0]);
//  }
}  /* Get_args */

/*------------------------------------------------------------------
 * Function:  Usage
 * Purpose:   Print a message explaining how to run the program
 * In arg:    prog_name
 */
void Usage(char *prog_name) {
  fprintf(stderr, "usage: %s <number of threads> <n>\n", prog_name);
  fprintf(stderr, "   n is the number of terms and should be >= 1\n");
  fprintf(stderr, "   n should be evenly divisible by the number of threads\n");
  exit(0);
}  /* Usage */