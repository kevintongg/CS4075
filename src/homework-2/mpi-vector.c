#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

void Read_n(int *n_p, int *local_n_p, int my_rank, int comm_sz,
            MPI_Comm comm);

void Check_for_error(int local_ok, char fname[], char message[],
                     MPI_Comm comm);

void Read_data(double local_vec1[], double local_vec2[], int local_n, int my_rank, int comm_sz, MPI_Comm comm);

void Print_vector(double local_vec[], int local_n, int n, char title[],
                  int my_rank, MPI_Comm comm);

void Par_vector_addition(const double *local_vec1, const double *local_vec2, double *local_result, int local_n);

int main(void) {
  int n, local_n;
  double *local_vec1, *local_vec2;
  double *local_addition;
  int comm_sz, my_rank;
  MPI_Comm comm;
  
  
  MPI_Init(NULL, NULL);
  comm = MPI_COMM_WORLD;
  MPI_Comm_size(comm, &comm_sz);
  MPI_Comm_rank(comm, &my_rank);
  
  // read n and initialize local n
  Read_n(&n, &local_n, my_rank, comm_sz, comm);
  
  // need malloc
  local_vec1 = malloc(local_n * sizeof(double));
  local_vec2 = malloc(local_n * sizeof(double));
  local_addition = malloc(local_n * sizeof(double));
  
  // read vectors and scatter to processes
  Read_data(local_vec1, local_vec2, local_n, my_rank, comm_sz, comm);
  
  /* Print input data */
  if (my_rank == 0) {
    printf("\n––input––\n");
  }
  Print_vector(local_vec1, local_n, n, "vector x: ", my_rank, comm);
  Print_vector(local_vec2, local_n, n, "vector y: ", my_rank, comm);
  
  /* Print results */
  if (my_rank == 0) {
    printf("\n––output––\n");
  }
  
  /* Compute addition and print out result */
  Par_vector_addition(local_vec1, local_vec2, local_addition, local_n);
  Print_vector(local_addition, local_n, n, "addition vector of vector x and y:", my_rank, comm);
  
  free(local_addition);
  free(local_vec1);
  free(local_vec2);
  
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
    printf("what is the order of your vector: ");
    printf("\n");
    scanf("%d", n_p);
    strtol()
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
void Read_data(double local_vec1[], double local_vec2[], int local_n, int my_rank, int comm_sz, MPI_Comm comm) {
  double *a = NULL;
  int i;
  MPI_Datatype type;
  
  MPI_Type_contiguous(1, MPI_DOUBLE, &type);
  MPI_Type_commit(&type);
  
  if (my_rank == 0) {
    a = malloc(local_n * comm_sz * sizeof(double));
    
    // first vector
    printf("Enter your x vector\n");
    for (i = 0; i < local_n * comm_sz; i++) {
      scanf("%lf", &a[i]);
    }
    // scatter vector 1
    MPI_Scatter(a, local_n, type, local_vec1, local_n, type, 0, comm);
    // second vector
    printf("Enter your y vector\n");
    for (i = 0; i < local_n * comm_sz; i++) {
      scanf("%lf", &a[i]);
    }
    // scatter vector 2
    MPI_Scatter(a, local_n, type, local_vec2, local_n, type, 0, comm);
    free(a);
  } else {
    MPI_Scatter(a, local_n, type, local_vec1, local_n, type, 0, comm);
    MPI_Scatter(a, local_n, type, local_vec2, local_n, type, 0, comm);
  }
}  /* Read_data */

/*-------------------------------------------------------------------*/
void Print_vector(double local_vec[], int local_n, int n, char title[],
                  int my_rank, MPI_Comm comm) {
  double *a = NULL;
  int i;
  MPI_Datatype type;
  
  MPI_Type_contiguous(1, MPI_DOUBLE, &type);
  MPI_Type_commit(&type);
  
  if (my_rank == 0) {
    a = malloc(n * sizeof(double));
    MPI_Gather(local_vec, local_n, type, a, local_n, type, 0, comm);
    printf("%s\n", title);
    for (i = 0; i < n; i++) {
      printf("%.2f ", a[i]);
    }
    printf("\n");
    free(a);
  } else {
    MPI_Gather(local_vec, local_n, type, a, local_n, type, 0, comm);
  }
  
}  /* Print_vector */

/*-------------------------------------------------------------------*/
void Par_vector_addition(const double *local_vec1, const double *local_vec2, double *local_result, int local_n) {
  
  for (int local_i = 0; local_i < local_n; local_i++) {
    local_result[local_i] = local_vec1[local_i] + local_vec2[local_i];
  }
  
  
}  /* Par_vector_addition */