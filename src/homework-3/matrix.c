//
// Created by Kevin Tong on 22/10/2018.
//

#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

#define TYPE MPI_DOUBLE

void Read_n(int *n, int *local_n, int my_rank, int comm_sz,
            MPI_Comm comm);

void Check_for_error(int local_ok, char fname[], char message[],
                     MPI_Comm comm);

void Read_data(double local_matrix[], double local_vector[], MPI_Comm comm, int my_rank, int comm_sz, int n, int local_n);

void Print_vector(double local_vector[], int local_n, int n, char title[],
                  int my_rank, MPI_Comm comm);

void Print_matrix(double local_matrix[], int local_n, int n, char *title,
                  int my_rank, MPI_Comm comm);

void
Matrix_vector_mult(double local_a[], double vector[], double local_result[], int local_m, int n, int local_n);

int main(void) {
  int n, local_n;
//  double *local_matrix, *local_vector;
  double *matrix;
  double *vector;
  double *final_result;
  double *local_matrix;
  double *local_result;
  int comm_sz, my_rank;
  MPI_Comm comm;
  
  MPI_Init(NULL, NULL);
  comm = MPI_COMM_WORLD;
  MPI_Comm_size(comm, &comm_sz);
  MPI_Comm_rank(comm, &my_rank);
  
  // read n and initialize local n
  Read_n(&n, &local_n, my_rank, comm_sz, comm);
  
  printf("local n %d n %d comm_sz %d\n", local_n, n, comm_sz);
  
  // need malloc
  matrix = malloc(n * n * sizeof(double));
  vector = malloc(n * sizeof(double));
  final_result = malloc(n * sizeof(double));
  local_matrix = malloc(n * local_n * sizeof(double));
  local_result = malloc(local_n * sizeof(double));
  
  // read vectors and scatter to processes
  Read_data(local_matrix, vector, comm, my_rank, comm_sz, n, local_n);
  
  /* Print input data */
  if (my_rank == 0) {
    printf("\n––input––\n");
  }
  
  Print_matrix(local_matrix, local_n, n, "matrix: ", my_rank, comm);
  Print_vector(vector, local_n, n, "vector: ", my_rank, comm);
  
  /* Print results */
  if (my_rank == 0) {
    printf("\n––output––\n");
  }
  
  /* Compute matrix-vector multiplication and print out result */
  Matrix_vector_mult(local_matrix, vector, local_result, n , n, local_n);
//  MPI_Gather(local_result, 1, TYPE, final_result, 1, TYPE, 0, comm);
  Print_vector(final_result, local_n, n, "matrix-vector multiplication result:", my_rank, comm);
  
  
  free(final_result);
  free(matrix);
  free(vector);
  free(local_matrix);
  free(local_result);
  
  MPI_Finalize();
  return 0;
}

/*-------------------------------------------------------------------*/
void Check_for_error(int local_ok, char fname[], char message[], MPI_Comm comm) {
  int ok;
  
  MPI_Allreduce(&local_ok, &ok, 1, MPI_INT, MPI_MIN, comm);
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

/*-------------------------------------------------------------------*/
void Read_n(int *n, int *local_n, int my_rank, int comm_sz,
            MPI_Comm comm) {
  int local_ok = 1;
  
  if (my_rank == 0) {
    printf("what is the order of your square matrix? ");
    printf("\n");
    scanf("%d", n);
  }
  
  // broadcast n
  MPI_Bcast(n, 1, MPI_INT, 0, comm);
  
  // check for error
  if (*n < 0 || *n % comm_sz) {
    local_ok = 0;
  }
  Check_for_error(local_ok, "Read_n", "n should be > 0 and evenly divisible by comm_sz", comm);
  
  // calculate n_p
  *local_n = *n / comm_sz;
}  /* Read_n */

/*-------------------------------------------------------------------*/
void
Read_data(double local_matrix[], double local_vector[], MPI_Comm comm, int my_rank, int comm_sz, int n, int local_n) {
  double *a, *b = NULL;
  int i;
  
  srand((unsigned int) time(NULL));
  
  if (my_rank == 0) {
    a = malloc(n * n * sizeof(double));
    b = malloc(n * sizeof(double));
    
    // matrix
    printf("Enter your matrix\n");
    for (i = 0; i < n * n; i++) {
      scanf("%lf", &a[i]);
//        a[i] = rand() % 10;
    }
    // scatter matrix
    MPI_Scatter(a, local_n, TYPE, local_matrix, local_n, TYPE, 0, comm);
    // vector
    printf("Enter your vector\n");
    for (i = 0; i < n; i++) {
      scanf("%lf", &b[i]);
//      local_vector[i] = rand() % 10;
//      scanf("%lf", &local_vector[i]);
    }
    // scatter b
    MPI_Scatter(b, local_n, TYPE, local_vector, local_n, TYPE, 0, comm);
    free(a);
    free(b);
  } else {
    MPI_Scatter(a, local_n, TYPE, local_matrix, local_n, TYPE, 0, comm);
//    MPI_Scatter(b, local_n, TYPE, local_vector, local_n, TYPE, 0, comm);
  }
//  MPI_Bcast(local_vector, n, TYPE, 0, comm);
  
}  /* Read_data */

/*-------------------------------------------------------------------*/
void Print_vector(double local_vec[], int local_n, int n, char title[],
                  int my_rank, MPI_Comm comm) {
  double *a = NULL;
  int i;
  
  if (my_rank == 0) {
    a = malloc(n * sizeof(double));
    MPI_Gather(local_vec, local_n, TYPE, a, local_n, TYPE, 0, comm);
    printf("%s\n", title);
    for (i = 0; i < n; i++) {
      printf("%.2f ", local_vec[i]);
    }
    printf("\n");
    free(a);
  } else {
//    MPI_Gather(local_vec, local_n, TYPE, a, local_n, TYPE, 0, comm);
  }
  
}  /* Print_vector */

/*-------------------------------------------------------------------*/
void Print_matrix(double local_matrix[], int local_n, int n, char *title,
                  int my_rank, MPI_Comm comm) {
  double *a = NULL;
  int i;
  
  if (my_rank == 0) {
    a = malloc(local_n * local_n * sizeof(double));
//    for (i = 0; i < pow(n, 2); i++) {
//      a[i] = (double *) malloc(n * sizeof(double));
//    }
//    MPI_Gather(local_matrix, local_n, TYPE, a, local_n, TYPE, 0, comm);
    printf("%s\n", title);
    for (i = 0; i < pow(n, 2); i++) {
      printf("%.2f ", a[i]);
    }
    printf("\n");
    free(a);
  } else {
    MPI_Gather(local_matrix, local_n, TYPE, a, local_n, TYPE, 0, comm);
  }
  
}  /* Print_vector */

/*-------------------------------------------------------------------*/
void
Matrix_vector_mult(double local_a[], double vector[], double local_result[], int local_m, int n, int local_n) {
  int local_i;
  int i;
  
  for (local_i = 0; local_i < local_n; local_i++) {
    for (i = 0; i < n; i++) {
      local_result[local_i] += local_a[local_i * n + i] * vector[i];
//      printf("RESULTS %.2f\n", local_a[i]);
    }
  }
}