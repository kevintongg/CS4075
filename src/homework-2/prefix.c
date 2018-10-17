#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

void Read_n(int *n, int *local_n, int my_rank, int comm_sz, MPI_Comm comm);

void Generate_data(int vector, int local_vector, int n, int my_rank, int comm_sz, MPI_Comm comm);

int main(int argc, char **argv) {
  
  int my_rank;
  int comm_sz;
  int n;
  int local_n;
  int *vector;
  int *local_vector;
  int *prefix;
  int *local_prefix;
  MPI_Comm comm;
  
  comm = MPI_COMM_WORLD;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(comm, &comm_sz);
  MPI_Comm_rank(comm, &my_rank);
  
  // read n
  Read_n(&n, &local_n, my_rank, comm_sz, comm);
  
  vector = malloc(n * sizeof(int));
  local_vector = malloc(n * sizeof(int));
  prefix = malloc(n * sizeof(int));
  local_prefix = malloc(n * sizeof(int));
  
  Generate_data(*vector, *local_vector, n, my_rank, comm_sz, comm);
  
  for (int i = 0; i < *vector; ++i) {
    printf("%d\n", vector[i]);
  }
  
  MPI_Scan(prefix, local_prefix, 1, MPI_INT, MPI_SUM, comm);
  
  MPI_Finalize();
}

void Read_n(int *n, int *local_n, int my_rank, int comm_sz, MPI_Comm comm) {
  
  // get input
  if (my_rank == 0) {
    printf("How many elements do you want (size of array)?\n");
    scanf("%d", n);
  }
  
  MPI_Bcast(n, 1, MPI_INT, 0, comm);
  
  *local_n = *n / comm_sz;
}

void Generate_data(int vector, int local_vector, int n, int my_rank, int comm_sz, MPI_Comm comm) {
  int *data;
  
  if (my_rank == 0) {
    data = malloc(n * sizeof(int));
    for (int i = 0; i <= n; ++i) {
      data[i] = rand() % 10 + 1;
    }
  }
  
  MPI_Scatter(data, local_vector, MPI_INT, n, my_rank, MPI_INT, 0, comm);
  
  if (my_rank == 0) {
    free(data);
  }
}