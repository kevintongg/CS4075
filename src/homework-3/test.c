//
// Created by Kevin Tong on 30/10/2018.
//

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

void Generate_matrix(int local_matrix[], int n, int local_n, int my_rank, MPI_Comm comm, int comm_sz);

void Generate_vector(int local_vector[], int n, int local_n, int my_rank, MPI_Comm comm);

void Print_matrix(int local_matrix[], int n, int local_n, char title[], int my_rank, MPI_Comm comm, int comm_sz);

void Print_vector(int local_vector[], int n, int local_n, char title[], int my_rank, MPI_Comm comm);

void
Matrix_vector_mult(int local_matrix[], int local_vector[], int local_result[], int x[], int local_m, int n, int local_n,
                   MPI_Comm comm);

int main(int argc, char* argv[]) {
  int n, local_n;
  int *local_matrix;
  int *local_vector;
  int *local_result;
  int *x;
  
  int comm_size, my_rank;
  
  MPI_Comm comm;
  
  MPI_Init(&argc, &argv);
  comm = MPI_COMM_WORLD;
  
  MPI_Comm_size(comm, &comm_size);
  MPI_Comm_rank(comm, &my_rank);
  
  Read_n(&n, &local_n, my_rank, comm_size, comm);
  
//  printf("local n %d n %d comm_sz %d\n", local_n, n, comm_size);
  
  local_matrix = malloc(n * n * sizeof(int));
  local_vector = malloc(n * sizeof(int));
  local_result = malloc(n * sizeof(int));
  x = malloc(n * sizeof(int));
  
  Generate_matrix(local_matrix, n, local_n, my_rank, comm, comm_size);
  Generate_vector(local_vector, n, local_n, my_rank, comm);
  
  if (my_rank == 0) {
    printf("\n- input -\n\n");
  }
  
  Print_matrix(local_matrix, n, local_n, "- matrix -", my_rank, comm, comm_size);
  Print_vector(local_vector, n, local_n, "\n- vector -", my_rank, comm);
  
  Matrix_vector_mult(local_matrix, local_vector, local_result, x, local_n, n, local_n, comm);
  
  if (my_rank == 0) {
    printf("\n- output -\n\n");
  }
  
  Print_vector(local_result, n, local_n, "- matrix-vector multiplication -", my_rank, comm);
  
  free(local_matrix);
  free(local_vector);
  free(local_result);
  free(x);
  MPI_Finalize();
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

void Generate_matrix(int local_matrix[], int n, int local_n, int my_rank, MPI_Comm comm, int comm_sz) {
  srand((unsigned int) time(NULL));
  
  int *data = NULL;
  int dim = n * n;
  
  if (my_rank == 0) {
    data = malloc(dim * sizeof(int));
    
//    printf("Enter your matrix\n");
    for (int i = 0; i < dim; ++i) {
//      scanf("%d", &local_matrix[i]);
      local_matrix[i] = rand() % 10;
//      printf("value %d: %d\n", i + 1, local_matrix[i]);
    }
    MPI_Scatter(data, local_n, INT, local_matrix, local_n, INT, 0, comm);
    free(data);
  } else {
    MPI_Scatter(data, local_n, INT, local_matrix, local_n, INT, 0, comm);
  }
}

void Generate_vector(int local_vector[], int n, int local_n, int my_rank, MPI_Comm comm) {
  srand((unsigned int) time(NULL));
  
  int *data = NULL;
  
  if (my_rank == 0) {
    data = malloc(n * sizeof(int));
    
//    printf("Enter your vector\n");
    for (int i = 0; i < n; ++i) {
//      scanf("%d", &local_vector[i]);
      local_vector[i] = rand() % 4;
//      printf("value %d: %d\n", i + 1, local_vector[i]);
    }
    MPI_Scatter(data, local_n, INT, local_vector, local_n, INT, 0, comm);
    free(data);
  } else {
    MPI_Scatter(data, local_n, INT, local_vector, local_n, INT, 0, comm);
  }
}

void Print_matrix(int local_matrix[], int n, int local_n, char title[], int my_rank, MPI_Comm comm, int comm_sz) {
  int *data = NULL;
  int dim = n * n;
  
  if (my_rank == 0) {
    data = malloc(dim * sizeof(int));
    MPI_Gather(local_matrix, local_n, INT, data, local_n, INT, 0, comm);
    printf("%s\n", title);
    for (int i = 0; i < dim; ++i) {
      printf("index %d: %d\n", i, local_matrix[i]);
    }
    printf("\n");
    free(data);
  } else {
    MPI_Gather(local_matrix, local_n, INT, data, local_n, INT, 0, comm);
  }
}

void Print_vector(int local_vector[], int n, int local_n, char title[], int my_rank, MPI_Comm comm) {
  int *data = NULL;
  
  if (my_rank == 0) {
    data = malloc(n * sizeof(int));
    MPI_Gather(local_vector, local_n, INT, data, local_n, INT, 0, comm);
    printf("%s\n", title);
    for (int i = 0; i < n; ++i) {
      printf("index %d: %d\n", i, local_vector[i]);
    }
    printf("\n");
    free(data);
  } else {
    MPI_Gather(local_vector, local_n, INT, data, local_n, INT, 0, comm);
  }
}

void Matrix_vector_mult(int local_matrix[], int local_vector[], int local_result[], int x[], int local_m, int n, int local_n, MPI_Comm comm) {
  
//  MPI_Allgather(local_matrix, local_n, INT, x, local_n, INT, comm);

  for (int i = 0; i < local_n; ++i) {
    for (int j = 0; j < n; ++j) {
      local_result[i] += local_matrix[i * n + j] * local_vector[j];
    }
  }
}