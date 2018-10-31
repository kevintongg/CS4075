//
// Created by Kevin Tong on 29/10/2018.
//

// what is the order of your square matrix?
// 2
// local n: 1 n: 2 comm_sz: 2

// what is the order of your square matrix?
// 4
// local n: 2 n: 4 comm_sz: 2

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define INT MPI_INT

void Read_n(int *n, int *local_n, int my_rank, int comm_sz,
            MPI_Comm comm);

void Check_for_error(int local_ok, char fname[], char message[],
                     MPI_Comm comm);

void Allocate_arrays(int **local_matrix, int **local_vector, int **local_result, int n, int local_n, MPI_Comm comm);

void Read_data(int local_matrix[], int local_vector[], MPI_Comm comm, int my_rank, int comm_sz, int local_n, int n);

void Generate_matrix(int local_matrix[], int n, int local_n, int my_rank, MPI_Comm comm, int comm_sz);

void Generate_vector(int local_vector[], int n, int local_n, int my_rank, MPI_Comm comm);

void Print_vector(int local_vector[], int n, int local_n, char title[], int my_rank, MPI_Comm comm);

void Print_matrix(int local_matrix[], int n, int local_n, char title[], int my_rank, MPI_Comm comm, int comm_sz);

void
Matrix_vector_mult(int local_matrix[], int local_vector[], int local_result[], int x[], int n, int local_m, int local_n,
                   MPI_Comm comm);

int main(int argc, char* argv[]) {
  int n, local_n, *x;
  int *matrix, *local_matrix;
  int *vector, *local_vector;
  int *result, *local_result;
  
  int comm_size, my_rank;
  
  MPI_Comm comm;
  
  MPI_Init(&argc, &argv);
  comm = MPI_COMM_WORLD;
  
  MPI_Comm_size(comm, &comm_size);
  MPI_Comm_rank(comm, &my_rank);
  
  Read_n(&n, &local_n, my_rank, comm_size, comm);
  
  printf("local n %d n %d comm_sz %d\n", local_n, n, comm_size);

//  matrix = malloc(n * n * sizeof(int));
//  vector = malloc(n * sizeof(int));
  local_matrix = malloc(n * n * sizeof(int));
  local_vector = malloc(n * sizeof(int));
  local_result = malloc(n * sizeof(int));
  
//  Allocate_arrays(&local_matrix, &local_vector, &local_result, n, local_n, comm);
  Generate_matrix(local_matrix, n, local_n, my_rank, comm, comm_size);
  Generate_vector(local_vector, n, local_n, my_rank, comm);

//  Read_data(local_matrix, local_vector, comm, my_rank, comm_size, local_n, n);
  
  /* Print input data */
  if (my_rank == 0) {
    printf("\n–input–\n");
    printf("\n");
  }
  
  Print_matrix(local_matrix, n, local_n, "–matrix–", my_rank, comm, 0);
  Print_vector(local_vector, n, local_n, "–vector–", my_rank, comm);
  
  /* Print results */
  if (my_rank == 0) {
    printf("\n––output––\n");
  }
  
  x = malloc(n * sizeof(int));
  
  /* Compute matrix-vector multiplication */
  Matrix_vector_mult(local_matrix, local_vector, local_result, x, n, 0, local_n, comm);
//  MPI_Gather(local_result, n, INT, result, n, INT, 0, comm);
  Print_vector(local_result, n, local_n, "matrix-vector multiplication", my_rank, comm);
  
//  free(matrix);
//  free(vector);
  free(local_matrix);
  free(local_vector);
  free(local_result);
  
  MPI_Finalize();
  return 0;
}

/*-------------------------------------------------------------------*/
void Check_for_error(int local_ok, char fname[], char message[], MPI_Comm comm) {
  int ok;
  
  MPI_Allreduce(&local_ok, &ok, 1, INT, MPI_MIN, comm);
  if (ok == 0) {
    int my_rank;
    MPI_Comm_rank(comm, &my_rank);
    if (my_rank == 0) {
      fprintf(stderr, "Proc %d > In %s, %s\n", my_rank, fname,
              message);
      fflush(stderr);
    }
    MPI_Finalize();
    exit(-1);
  }
}  /* Check_for_error */

void Read_n(int *n, int *local_n, int my_rank, int comm_sz,
            MPI_Comm comm) {
  int local_ok = 1;
  
  if (my_rank == 0) {
    printf("what is the order of your square matrix? ");
    printf("\n");
    scanf("%d", n);
  }
  
  MPI_Bcast(n, 1, INT, 0, comm);
  
  if (*n < 0 || *n % comm_sz) {
    local_ok = 0;
  }
  
  Check_for_error(local_ok, "Read_n", "n should be > 0 and evenly divisible by comm_sz", comm);
  
  // calculate m and m
  *local_n = *n / comm_sz;
} /* Read_n */

/*-------------------------------------------------------------------*/
void Read_data(int local_matrix[], int local_vector[], MPI_Comm comm, int my_rank, int comm_sz, int local_n, int n) {
  int *a = NULL;
  int *b = NULL;
  int i;
  
  srand((unsigned int) time(NULL));
  
  if (my_rank == 0) {
    a = malloc(local_n * local_n * comm_sz * sizeof(int));
    b = malloc(local_n * comm_sz * sizeof(int));
    
    // matrix
    printf("Enter your matrix\n");
    for (i = 0; i < pow(local_n * 2, 2); i++) {
//      scanf("%d", &a[i]);
      a[i] = rand() % 8 + 1;
    }
    // scatter matrix
    MPI_Scatter(a, local_n, INT, local_matrix, local_n, INT, 0, comm);
    // vector
    printf("Enter your vector\n");
    for (i = 0; i < local_n * comm_sz; i++) {
//      scanf("%d", &b[i]);
      b[i] = rand() % 8 + 1;
    }
    // scatter vector
    MPI_Scatter(b, local_n, INT, local_vector, local_n, INT, 0, comm);
    free(a);
    free(b);
  } else {
    MPI_Scatter(a, local_n, INT, local_matrix, local_n, INT, 0, comm);
    MPI_Scatter(b, local_n, INT, local_vector, local_n, INT, 0, comm);
  }
}  /* Read_data */

/*-------------------------------------------------------------------*/
void Print_matrix(int local_matrix[], int n, int local_n, char title[], int my_rank, MPI_Comm comm, int comm_sz) {
  int *data = NULL;
  int i;
  
  if (my_rank == 0) {
    data = malloc(local_n * local_n * sizeof(int));
    MPI_Gather(local_matrix, local_n, INT, data, local_n, INT, 0, comm);
    printf("%s\n", title);
    for (i = 0; i < pow(n, 2); i++) {
      printf("%d ", data[i]);
    }
    printf("\n");
    free(data);
  } else {
    MPI_Gather(local_matrix, local_n, INT, data, local_n, INT, 0, comm);
  }
  
//  if (my_rank == 0) {
//    data = malloc(local_n * local_n  * sizeof(int));
//    MPI_Gather(local_matrix, local_n, INT, data, local_n * n, INT, 0, comm);
//    printf("%s\n", title);
//    for (i = 0; i < n * n; i++) {
//      for (int j = 0; j < n; j++) {
//        int index = i * n + j;
////        printf("index: %d \n", index);
//        printf("%d", local_matrix[i * n + j]);
////        printf("%d", local_matrix[i]);
//      }
//      printf("\n");
//    }
//    printf("\n");
//    free(data);
//  } else {
//    MPI_Gather(local_matrix, local_n, INT, data, local_n, INT, 0, comm);
//  }
  
}  /* Print_vector */

/*-------------------------------------------------------------------*/
void Print_vector(int local_vector[], int n, int local_n, char title[], int my_rank, MPI_Comm comm) {
  int *a = NULL;
  int i;
  
  if (my_rank == 0) {
    a = malloc(n * sizeof(int));
    MPI_Gather(local_vector, local_n, INT, a, local_n, INT, 0, comm);
    printf("%s\n", title);
    for (i = 0; i < n; i++) {
      printf("%d ", a[i]);
    }
    printf("\n");
    free(a);
  } else {
    MPI_Gather(local_vector, local_n, INT, a, local_n, INT, 0, comm);
  }
  
}  /* Print_vector */

void Matrix_vector_mult(int local_matrix[], int local_vector[], int local_result[], int x[], int n, int local_m, int local_n,
                        MPI_Comm comm) {
  
  int local_i, j;
  
  MPI_Allgather(local_matrix, local_n, INT, x, local_n, INT, comm);
  
  for (local_i = 0; local_i < local_n; local_i++) {
    for (j = 0; j < n; j++) {
      local_result[local_i] += local_matrix[local_i * n + j] * local_vector[j];
//      printf("result %d\n", local_result[local_i]);
    }
  }
}

void Allocate_arrays(int **local_matrix, int **local_vector, int **local_result, int n, int local_n, MPI_Comm comm) {
  int local_ok = 1;
  
  *local_matrix = malloc(local_n * n * sizeof(int));
  *local_vector = malloc(local_n * sizeof(int));
  *local_result = malloc(local_n * sizeof(int));
  
  if (*local_matrix == NULL || *local_vector == NULL || *local_result == NULL) {
    local_ok = 0;
  }
  
  Check_for_error(local_ok, "Allocate arrays", "Cannot allocate arrays", comm);
}

void Generate_matrix(int local_matrix[], int n, int local_n, int my_rank, MPI_Comm comm, int comm_sz) {

//  int *a = NULL;
//  int *b = NULL;
//  int i;
//
//  srand((unsigned int) time(NULL));
//
//  if (my_rank == 0) {
//    a = malloc(local_n * local_n * comm_sz * sizeof(int));
//    b = malloc(local_n * comm_sz * sizeof(int));
//
//    // matrix
//    printf("Enter your matrix\n");
//    for (i = 0; i < pow(local_n * 2, 2); i++) {
////      scanf("%d", &a[i]);
//      a[i] = rand() % 8 + 1;
//    }
//    // scatter matrix
//    MPI_Scatter(a, local_n, INT, local_matrix, local_n, INT, 0, comm);
//    // vector
//    printf("Enter your vector\n");
//    for (i = 0; i < local_n * comm_sz; i++) {
////      scanf("%d", &b[i]);
//      b[i] = rand() % 8 + 1;
//    }
//    // scatter vector
//    MPI_Scatter(b, local_n, INT, local_vector, local_n, INT, 0, comm);
//    free(a);
//    free(b);
//  } else {
//    MPI_Scatter(a, local_n, INT, local_matrix, local_n, INT, 0, comm);
//    MPI_Scatter(b, local_n, INT, local_vector, local_n, INT, 0, comm);
//  }
  
  srand((unsigned int) time(NULL));
  
  int *data = NULL;
  
  if (my_rank == 0) {
    data = malloc(local_n * local_n * comm_sz * sizeof(int));
  
    printf("Enter your matrix\n");
    for (int i = 0; i < pow(local_n, 2); i++) {
      for (int j = 0; j < n; j++) {
//        int index = i * n + j;
//        printf("index: %d\n", index);
//        local_matrix[i * n + j] = rand() % 5;
        scanf("%d", &local_matrix[i]);
      }
    }
    MPI_Scatter(data, local_n, INT, local_matrix, local_n * n, INT, 0, comm);
    free(data);
  } else {
    MPI_Scatter(data, local_n, INT, local_matrix, local_n * n, INT, 0, comm);
  }
}

void Generate_vector(int local_vector[], int n, int local_n, int my_rank, MPI_Comm comm) {
  srand((unsigned int) time(NULL));
  
  int *data = NULL;
  
  if (my_rank == 0) {
    data = malloc(n * sizeof(int));
    
    printf("Enter your vector\n");
    for (int i = 0; i < n; i++) {
      scanf("%d", &local_vector[i]);
//      data[i] = rand() % 10 + 1;
    }
    MPI_Scatter(data, local_n, INT, local_vector, local_n, INT, 0, comm);
    free(data);
  } else {
    MPI_Scatter(data, local_n, INT, local_vector, local_n, INT, 0, comm);
  }
}