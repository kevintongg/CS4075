#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "math.h"

const int MAX_THREADS = 1024;

long thread_count;
long long n;
double sum;
double pi_estimate;

void *Thread_sum();

/* Only executed by main thread */
void Get_args(int argc, char *argv[]);

void Usage(char *prog_name);

int main(int argc, char *argv[]) {
  long thread;  /* Use long in case of a 64-bit system */
  pthread_t *thread_handles;
  double start, finish, elapsed;
  
  /* Get number of threads from command line */
  Get_args(argc, argv);
  
  thread_handles = malloc(thread_count * sizeof(pthread_t));
  for (thread = 0; thread < thread_count; thread++) {
    pthread_create(&thread_handles[thread], NULL, Thread_sum, (void *) thread);
  }
  for (thread = 0; thread < thread_count; thread++) {
    pthread_join(thread_handles[thread], NULL);
  }
  
  printf("π estimate: %.8f\n", pi_estimate);
  free(thread_handles);
}

/*------------------------------------------------------------------*/
void *Thread_sum() {
  long long int num_in_circle = 0;
  float x, y, distance_squared;
  unsigned int seed = (unsigned) time(NULL);
  srand(seed);
  for (int toss = 0; toss < n; toss++) {
    x = rand_r(&seed) / (float) RAND_MAX;
    y = rand_r(&seed) / (float) RAND_MAX;
    distance_squared = (x * x) + (y * y);
    if (distance_squared <= 1) {
      num_in_circle++;
    }
  }
  pi_estimate = 4 * num_in_circle / (float) n;
  
  return NULL;
}  /* Thread_sum */

/*------------------------------------------------------------------
 * Function:    Get_args
 * Purpose:     Get the command line args
 * In args:     argc, argv
 * Globals out: thread_count, n
 */
void Get_args(int argc, char *argv[]) {
  if (argc != 3) {
    Usage(argv[0]);
  }
  thread_count = strtol(argv[1], NULL, 10);
  if (thread_count <= 0 || thread_count > MAX_THREADS) {
    Usage(argv[0]);
  }
  n = strtoll(argv[2], NULL, 10);
  if (n <= 0) {
    Usage(argv[0]);
  }
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