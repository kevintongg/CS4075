#include <semaphore.h>
#include <math.h>
#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"

#define MAX_THREADS 1024

long thread_count;
double a;
double b;
int n;
double h;
double temp_a;
double temp_b;
long temp_n;
double sum;
pthread_mutex_t mutex;
sem_t sem;

void Get_input(double *a, double *b, int *n);

void *Trapezoid(void *rank);

void *Pth_mat_vec(void *rank);

/* Only executed by main thread */
void Get_args(int argc, char *argv[]);

void Usage(char *prog_name);

int main(int argc, char *argv[]) {
  long thread;
  
  pthread_t *thread_handles;
  
  Get_args(argc, argv);
  Get_input(&a, &b, &n);
  
  thread_handles = malloc(thread_count * sizeof(pthread_t));
  
  pthread_mutex_init(&mutex, NULL);
  for (thread = 0; thread < thread_count; thread++) {
    pthread_create(&thread_handles[thread], NULL, Trapezoid, (void *) thread);
  }
  for (thread = 0; thread < thread_count; thread++) {
    pthread_join(thread_handles[thread], NULL);
  }
  
  printf("\nWith n = %d trapezoids, our estimate of the integral from %f to %f = %.4f\n", n, a, b, sum);
  
  free(thread_handles);
  pthread_mutex_destroy(&mutex);
}

double f(double x) {
  return pow(x, 3);
}

void *Trapezoid(void *rank) {
  long my_rank = (long) rank;
  double estimate;
  double x;
  int i;
  
  h = (b - a) / n; // base length
  temp_n = (n / thread_count); // trapezoid count
  temp_a = a + my_rank * temp_n * h; // left endpoint
  temp_b = temp_a + temp_n * h; // right endpoint
  
  estimate = (f(temp_a) + f(temp_b)) / 2;
  for (i = 1; i <= temp_n - 1; i++) {
    x = temp_a + i * h;
    estimate += f(x);
  }
  
  pthread_mutex_lock(&mutex);
  sem_wait(&sem);
  sum += estimate * h;
  pthread_mutex_unlock(&mutex);
  sem_post(&sem);
  
  return NULL;
}

void Get_input(double *a, double *b, int *n) {
  printf("Input a, b, and n\n");
  printf("Enter a\n");
  scanf("%lf", a);
  printf("Enter b\n");
  scanf("%lf", b);
  printf("Enter n\n");
  scanf("%d", n);
}

void Get_args(int argc, char *argv[]) {
  if (argc != 2) {
    Usage(argv[0]);
  }
  thread_count = strtol(argv[1], NULL, 10);
  if (thread_count <= 0 || thread_count > MAX_THREADS) {
    Usage(argv[0]);
  }
}
/* Get_args */

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