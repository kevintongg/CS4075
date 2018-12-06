#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"

#define MAX_THREADS 1024

typedef struct node {
  int *data;
  struct node *next;
  pthread_mutex_t mutex;
} node;

long thread_count;
long long n;
double sum;
int a;

void Get_input(int *a, long long *n);

/* Only executed by main thread */
void Get_args(int argc, char *argv[]);

void Usage(char *prog_name);

void *Queue(void *rank);

int main(int argc, char *argv[]) {
  long thread;
  pthread_t *thread_handles;
  
  int *list = malloc(20 * sizeof(int));
  
  Get_input(&a, &n);
  
  thread_handles = malloc(thread_count * sizeof(pthread_t));
  
  for (thread = 0; thread < thread_count; thread++) {
    pthread_create(&thread_handles[thread], NULL, Queue, (void *) thread);
  }
  for (thread = 0; thread < thread_count; thread++) {
    pthread_join(thread_handles[thread], NULL);
  }
}

void *Queue(void *rank) {
  
  node *head = NULL;
  node *temp = NULL;
  
}

void Get_input(int *a, long long *n) {
  printf("Input a, b, and n\n");
  printf("Enter a - # of tasks\n");
  scanf("%d", a);
  printf("Enter n - # of threads\n");
  scanf("$%lli", n);
}

void Get_args(int argc, char *argv[]) {
  if (argc != 2) {
    Usage(argv[0]);
  }
  thread_count = strtol(argv[1], NULL, 10);
  if (thread_count <= 0 || thread_count > MAX_THREADS) {
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

void selectTask(int *list) {
  
  unsigned int seed = (unsigned) time(NULL);
  
  for (int i = 0; i < a; ++i) {
    switch (rand_r(&seed) / 3 + 1) {
      case 1:
    }
  }
  /*
   * rng
   * switch
   * 3 cases
   * each function put a random number
   *
   */
}

int insert(int value, node **head) {
  node *curr = *head;
  node *prev = NULL;
  node *temp;
  
  while (curr != NULL && curr->data < value) {
    prev = curr;
    curr = curr->next;
  }
  
  if (curr == NULL || curr->data > value) {
    temp = malloc(sizeof(node));
    temp->data = value;
    temp->next = curr;
    if (prev == NULL) {
      *head = temp;
    } else {
      prev->next = temp;
    }
    return 1;
  } else {
    return 0;
  }
}

int delete(int value, node **head) {
  node *curr = *head;
  node *prev = NULL;
  
  while (curr != NULL && curr->data < value) {
    prev = curr;
    curr = curr->next;
  }
  
  if (curr != NULL && curr->data == value) {
    if (prev == NULL) {
      *head = curr->next;
      free(curr);
    } else {
      prev->next = curr->next;
      free(curr);
    }
    return 1;
  } else {
    return 0;
  }
}

int member(int value, node *head) {
  node *curr = head;
  
  while (curr != NULL && curr->data < value) {
    curr = curr->next;
  }
  
  if (curr == NULL || curr->data > value) {
    return 0;
  } else {
    return 1;
  }
}




