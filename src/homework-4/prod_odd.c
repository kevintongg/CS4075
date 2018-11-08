#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "timer.h"

const int MAX_THREADS = 1024;
const int MAX_MESSAGES = 64;

long thread_count;
long long n;
double sum = 0;
pthread_mutex_t mutex;
char **messages;

void *Producer_consumer(void *rank);

/* Only executed by main thread */
void Get_args(int argc, char *argv[]);

void Usage(char *prog_name);

double Serial_pi(long long n);

int main(int argc, char *argv[]) {
  long thread;  /* Use long in case of a 64-bit system */
  pthread_t *thread_handles;
  
  /* Get number of threads from command line */
  Get_args(argc, argv);
  
  thread_handles = malloc(thread_count * sizeof(pthread_t));
  messages = malloc(thread_count * MAX_MESSAGES * sizeof(char));
  
  pthread_mutex_init(&mutex, NULL);
  
  for (thread = 0; thread < thread_count; thread++) {
    pthread_create(&thread_handles[thread], NULL, Producer_consumer, (void *) thread);
  }
  for (thread = 0; thread < thread_count; thread++) {
    pthread_join(thread_handles[thread], NULL);
  }
  
  free(thread_handles);
  free(messages);
  pthread_mutex_destroy(&mutex);
  return 0;
}  /* main */

/*------------------------------------------------------------------*/
void *Producer_consumer(void *rank) {
  long my_rank = (long) rank;
  long destination = (my_rank + 1) % thread_count;
  char *my_msg = malloc(MAX_MESSAGES * sizeof(char));
  
  // producer
  pthread_mutex_lock(&mutex);
  sprintf(my_msg, "Hello from %ld", my_rank);
  messages[destination] = my_msg;
  pthread_mutex_unlock(&mutex);
  
  while (1) {
    // consumer
    if (messages[my_rank] != NULL) {
      pthread_mutex_lock(&mutex);
      printf("Thread %ld > %s\n", my_rank, messages[my_rank]);
      pthread_mutex_unlock(&mutex);
      break;
    } else { /* my_rank == producer */
      messages[destination] = my_msg;
      pthread_mutex_unlock(&mutex);
    }
  }
  
  return NULL;
}  /* Thread_sum */

/*------------------------------------------------------------------
 * Function:   Serial_pi
 * Purpose:    Estimate pi using 1 thread
 * In arg:     n
 * Return val: Estimate of pi using n terms of MacLaurin series
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
  if (argc != 3) {
    Usage(argv[0]);
  }
  thread_count = strtol(argv[1], NULL, 10);
  if (thread_count <= 0 || thread_count > MAX_THREADS) {
    Usage(argv[0]);
  }
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
