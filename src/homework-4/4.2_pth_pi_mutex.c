#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include "timer.h"

const int MAX_THREADS = 1024;

long thread_count;
long long n;
double sum;

sem_t sem;

void *Thread_sum(void *rank);

/* Only executed by main thread */
void Get_args(int argc, char *argv[]);

void Usage(char *prog_name);

double Serial_pi(long long n);

int main(int argc, char *argv[]) {
  long thread;  /* Use long in case of a 64-bit system */
  pthread_t *thread_handles;
  double start, finish, elapsed;
  
  /* Get number of threads from command line */
  Get_args(argc, argv);
  
  thread_handles = malloc(thread_count * sizeof(pthread_t));
  
  GET_TIME(start);
  printf("Serial sum: %.8lf\n", Serial_pi(n));
  GET_TIME(finish)
  elapsed = finish - start;
  printf("Serial sum execution time: %.8fs\n", elapsed);
  sum = 0;
  GET_TIME(start);
  for (thread = 0; thread < thread_count; thread++) {
    pthread_create(&thread_handles[thread], NULL, Thread_sum, (void *) thread);
  }
  for (thread = 0; thread < thread_count; thread++) {
    pthread_join(thread_handles[thread], NULL);
  }
  sum *= 4;
  GET_TIME(finish);
  elapsed = finish - start;
  
  printf("\nThread sum: %.8lf\n", sum);
  printf("Thread sum execution time: %.8fs\n", elapsed);
  
  return 0;
}  /* main */

/*------------------------------------------------------------------*/
void *Thread_sum(void *rank) {
  long my_rank = (long) rank;
  double factor, my_sum = 0;
  long long i;
  long long my_n = n / thread_count;
  long long my_first_i = my_n * my_rank;
  long long my_last_i = my_first_i + my_n;
  
  if (my_first_i % 2 == 0) {
    factor = 1;
  } else {
    factor = -1;
  }
  
  for (i = my_first_i; i < my_last_i; i++, factor = -factor) {
    my_sum += factor / (2 * i + 1);
    if (i == (my_last_i - 1)) {
      sem_wait(&sem); // locked
      sum += my_sum;
      sem_post(&sem); // unlocked
    }
  }
  
  return NULL;
}  /* Thread_sum */

/*------------------------------------------------------------------
 * Function:   Serial_pi
 * Purpose:    Estimate pi using 1 thread
 * In arg:     n
 * Return val: Estimate of pi using n terms of Maclaurin series
 */
double Serial_pi(long long n) {
  double sum = 0.0;
  long long i;
  double factor = 1.0;
  
  for (i = 0; i < n; i++, factor = -factor) {
    sum += factor / (2 * i + 1);
  }
  return 4.0 * sum;
  
}  /* Serial_pi */

/*------------------------------------------------------------------
 * Function:    Get_args
 * Purpose:     Get the command line args
 * In args:     argc, argv
 * Globals out: thread_count, n
 */
void Get_args(int argc, char *argv[]) {
  if (argc != 3) Usage(argv[0]);
  thread_count = strtol(argv[1], NULL, 10);
  if (thread_count <= 0 || thread_count > MAX_THREADS) Usage(argv[0]);
  n = strtoll(argv[2], NULL, 10);
  if (n <= 0) Usage(argv[0]);
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
