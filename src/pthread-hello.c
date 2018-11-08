//
// Created by Kevin Tong on 31/10/2018.
//

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

// Global variable
long thread_count;

void *Hello(void *rank); // Thread function

int main(int argc, char *argv[]) {
  
  long thread; // Use long in case of 64-bit system
  pthread_t *thread_handles;
  
  // Get number of threads from command line
  thread_count = strtol(argv[1], NULL, 10);
  
  thread_handles = malloc(thread_count * sizeof(pthread_t));
  
  for (thread = 0; thread < thread_count; thread++) {
    pthread_create(thread_handles + thread, NULL, Hello, (void *) thread);
  }
  
  printf("Hello from the main thread\n");
  
  for (thread = 0; thread < thread_count; thread++) {
    pthread_join(thread_handles[thread], NULL);
  }
  
  free(thread_handles);
  return 0;
}

void *Hello(void *rank) {
  long my_rank = (long) rank;
  printf("Hello from thread %ld of %ld\n", my_rank, thread_count);
  return NULL;
}