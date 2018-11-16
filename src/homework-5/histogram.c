#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"

#define MAX_THREADS 1024

long thread_count;
long long n;

int bin_count; // num of bins
float min_meas; // min meas min value for bin containing smallest values
float max_meas; // max meas max value for bin containing smallest values
float *bin_maxes; // array of bin count floats
int *bin_counts; // array of bin count ints
int data_count; // num of measurements
float *data; // array of data count floats
int my_rank;
pthread_mutex_t mutex;

void Get_args(int argc, char *argv[]);

void Usage(char *prog_name);

void Gen_data();

void Get_input();

void *Set_bins(void *rank);

void Find_bin();

int Which_bin();

void Print();


int main(int argc, char *argv[]) {
  long thread;  /* Use long in case of a 64-bit system */
  pthread_t *thread_handles;
  
  // memory allocation
  bin_maxes = malloc(bin_count * sizeof(float));
  bin_counts = malloc(bin_count * sizeof(int));
  data = malloc(data_count * sizeof(float));
  thread_handles = malloc(thread_count * sizeof(pthread_t));
  
  /* Get number of threads from command line */
  Get_args(argc, argv);
  
  Get_input();
  Gen_data();
  
  pthread_mutex_init(&mutex, NULL);
  for (thread = 0; thread < thread_count; thread++) {
    pthread_create(&thread_handles[thread], NULL, Set_bins, (void *) thread);
  }
  for (thread = 0; thread < thread_count; thread++) {
    pthread_join(thread_handles[thread], NULL);
  }
  pthread_mutex_destroy(&mutex);
  Find_bin();
  Print();
  printf("\nthe range is from %10.2f to %10.2f\n", min_meas, max_meas);
  
  free(thread_handles);
  free(bin_counts);
  free(data);
}

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

// get user input
void Get_input() {
  printf("number of bins\n");
  scanf("%d", &bin_count);
  printf("min value\n");
  scanf("%f", &min_meas);
  printf("max value\n");
  scanf("%f", &max_meas);
  if (max_meas < min_meas) {
    float temp = max_meas;
    max_meas = min_meas;
    min_meas = temp;
  }
  printf("number of measurements\n");
  scanf("%d", &data_count);
  
}

// generate random data
void Gen_data() {
  unsigned int seed = (unsigned) time(NULL);
  
  float range = max_meas - min_meas;
  
  for (int i = 0; i < data_count; i++) {
    data[i] = (float) rand_r(&seed) / (float) RAND_MAX * range + min_meas;
  }
}

/* Find out the appropriate bin for each data in local_data and increase the number of data in this bin  */
void *Set_bins(void *rank) {
//  long my_rank = (long) rank;
//  int index = 0;
//  long length = data_count / thread_count;
//  float interval = (max_meas - min_meas) / bin_count;
//
//  for (int i = 0; i < length; i++) {
////    bin_maxes[i] = interval * (float) (i + 1) + min_meas;
//    index = (data[i + my_rank] - min_meas) / interval;
//    pthread_mutex_lock(&mutex);
//    bin_counts[index]++;
//    pthread_mutex_unlock(&mutex);
//  }

  float range = max_meas - min_meas;
  float interval = range / bin_count;

  for (int i = 0; i < bin_count; i++) {
    bin_maxes[i] = interval * (float) (i + 1) + min_meas;
    pthread_mutex_lock(&mutex);
    bin_counts[i]++;
    pthread_mutex_unlock(&mutex);
  }
  
  return NULL;
}

/* Find out the appropriate bin for each data in local_data and increase the number of data in this bin  */
void Find_bin() {
  int bin;

  for (int i = 0; i < data_count; i++) {
    bin = Which_bin();
    pthread_mutex_lock(&mutex);
    bin_counts[bin]++;
    pthread_mutex_unlock(&mutex);
  }
}

/* Find out the appropriate bin for each data */
int Which_bin() {
  int i;

  for (i = 0; i < bin_count - 1; i++) {
    if (*data < bin_maxes[i]) {
      break;
    }
  }

  return i;
}

// print data
void Print() {
  int width = 40;
  int max = 0;
  int row_width;
  
  for (int i = 0; i < bin_count; i++) {
    if (bin_counts[i] > max) {
      max = bin_counts[i];
    }
  }
  
  for (int i = 0; i < bin_count; ++i) {
    printf("%10.2f | ", bin_maxes[i]);
    row_width = (int) ((float) bin_counts[i] / (float) max * (float) width);
    for (int j = 0; j < row_width; ++j) {
      printf("#");
    }
    printf(" ");
    printf("%d\n", bin_counts[i]);
  }
}
