//
// Created by Kevin Tong on 22/10/2018.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <math.h>

#define TYPE MPI_DOUBLE

void Read_n(int *n_p, int *local_n_p, int my_rank, int comm_sz,
            MPI_Comm comm);

void Check_for_error(int local_ok, char fname[], char message[],
                     MPI_Comm comm);

void Read_data(double *local_matrix, double *local_vector, int local_n, int my_rank, int comm_sz, MPI_Comm comm);

void Print_vector(double local_vec[], int local_n, int n, char title[],
                  int my_rank, MPI_Comm comm);

void Print_vector_2d(double local_vec[], int local_n, int n, char title[],
                     int my_rank, MPI_Comm comm);

void
Mat_vect_mult(const double *local_a, const double *local_x, double *local_result, int local_m, int n, int local_n);

int main(void) {
  int n, local_n;
//  double *local_matrix, *local_vector;
  double *local_matrix;
  double *local_vector;
  double *final_vector;
  int comm_sz, my_rank;
  MPI_Comm comm;
  
  MPI_Init(NULL, NULL);
  comm = MPI_COMM_WORLD;
  MPI_Comm_size(comm, &comm_sz);
  MPI_Comm_rank(comm, &my_rank);
  
  // read n and initialize local n
  Read_n(&n, &local_n, my_rank, comm_sz, comm);
  
  // need malloc
  local_matrix = malloc(local_n * sizeof(double));
  local_vector = malloc(local_n * sizeof(double));
  final_vector = malloc(local_n * sizeof(double));
  
  // read vectors and scatter to processes
  Read_data(local_matrix, local_vector, local_n, my_rank, comm_sz, comm);
  
  /* Print input data */
  if (my_rank == 0) {
    printf("\n––input––\n");
  }
  
  Print_vector_2d(local_matrix, local_n, n, "matrix: ", my_rank, comm);
  Print_vector(local_vector, local_n, n, "vector: ", my_rank, comm);
  
  /* Print results */
  if (my_rank == 0) {
    printf("\n––output––\n");
  }
  
  /* Compute matrix-vector multiplication and print out result */
  Mat_vect_mult(local_matrix, local_vector, final_vector, n, n, local_n);
  Print_vector(final_vector, local_n, n, "matrix-vector multiplication result:", my_rank, comm);
  
  free(final_vector);
  free(local_matrix);
  free(local_vector);
  
  MPI_Finalize();
  return 0;
}

/*-------------------------------------------------------------------*/
void Check_for_error(
    int local_ok   /* in */,
    char fname[]    /* in */,
    char message[]  /* in */,
    MPI_Comm comm       /* in */) {
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
void Read_n(int *n_p, int *local_n_p, int my_rank, int comm_sz,
            MPI_Comm comm) {
  int local_ok = 1;
  
  if (my_rank == 0) {
    printf("what is the order of your square matrix? ");
    printf("\n");
    scanf("%d", n_p);
  }
  
  // broadcast n
  MPI_Bcast(n_p, 1, MPI_INT, 0, comm);
  
  // check for error
  if (*n_p < 0 || *n_p % comm_sz) {
    local_ok = 0;
  }
  Check_for_error(local_ok, "Read_n", "n should be > 0 and evenly divisible by comm_sz", comm);
  
  // calculate n_p
  *local_n_p = *n_p / comm_sz;
}  /* Read_n */

/*-------------------------------------------------------------------*/
void Read_data(double *local_matrix, double *local_vector, int local_n, int my_rank, int comm_sz, MPI_Comm comm) {
  double *a = NULL;
  double *b = NULL;
  int i;
  
  srand((unsigned int) time(NULL));
  
  if (my_rank == 0) {
    a = malloc(local_n * local_n * comm_sz * sizeof(double));
    b = malloc(local_n * comm_sz * sizeof(double));
    
    // first vector
    printf("Enter your matrix\n");
    for (i = 0; i < pow(local_n, 2) * comm_sz; i++) {
        scanf("%lf", &a[i]);
//        a[i] = rand() % 10;
    }
    // scatter vector 1
    MPI_Scatter(a, local_n, TYPE, local_matrix, local_n, TYPE, 0, comm);
    // second vector
    printf("Enter your vector\n");
    for (i = 0; i < local_n * comm_sz; i++) {
      scanf("%lf", &b[i]);
//      b[i] = rand() % 10;
    }
    // scatter vector 2
    MPI_Scatter(b, local_n, TYPE, local_vector, local_n, TYPE, 0, comm);
    free(a);
    free(b);
  } else {
    MPI_Scatter(a, local_n, TYPE, local_matrix, local_n, TYPE, 0, comm);
    MPI_Scatter(b, local_n, TYPE, local_vector, local_n, TYPE, 0, comm);
  }
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
      printf("%.2f ", a[i]);
    }
    printf("\n");
    free(a);
  } else {
    MPI_Gather(local_vec, local_n, TYPE, a, local_n, TYPE, 0, comm);
  }
  
}  /* Print_vector */

/*-------------------------------------------------------------------*/
void Print_vector_2d(double local_vec[], int local_n, int n, char title[],
                     int my_rank, MPI_Comm comm) {
  double **a = NULL;
  int i;
  int j;
  
  if (my_rank == 0) {
    a = malloc(n * sizeof(double));
    for (i = 0; i < pow(n, 2); i++) {
      a[i] = (double *) malloc(n * sizeof(double));
    }
    MPI_Gather(local_vec, local_n, TYPE, a, local_n, TYPE, 0, comm);
    printf("%s\n", title);
    for (i = 0; i < n; i++) {
      for (j = 0; j < n; ++j) {
        printf("%.2f ", *&a[i][j]);
      }
    }
    printf("\n");
    free(a);
  } else {
    MPI_Gather(local_vec, local_n, TYPE, a, local_n, TYPE, 0, comm);
  }
  
}  /* Print_vector */

/*-------------------------------------------------------------------*/
void
Mat_vect_mult(const double *local_a, const double *local_x, double *local_result, int local_m, int n, int local_n) {
  int local_i;
  int i;
  
  for (local_i = 0; local_i < local_m; local_i++) {
    for (i = 0; i < n; i++) {
      local_result[local_i] += local_a[local_i * n + i] * local_x[i];
    }
  }
}