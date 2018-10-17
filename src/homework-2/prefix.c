#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <time.h>

void Read_n(int *n, int *local_n, int my_rank, int comm_sz, MPI_Comm comm);

void Generate_data(int *prefix, int n, int local_n, int my_rank, MPI_Comm comm);

void Prefix_sum(int *prefix, int *final_prefix_sum, int n, int local_n, int my_rank, MPI_Comm comm);

int main(int argc, char **argv) {
  
  int my_rank;
  int comm_sz;
  int n;
  int local_n;
  int local_prefix_sum;
  int *prefix;
  int *final_prefix_sum;
  MPI_Comm comm;
  
  comm = MPI_COMM_WORLD;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(comm, &comm_sz);
  MPI_Comm_rank(comm, &my_rank);
  
  // read n
  Read_n(&n, &local_n, my_rank, comm_sz, comm);

  prefix = malloc(local_n * sizeof(int));
  final_prefix_sum = malloc(local_n * sizeof(int));
  
  Generate_data(prefix, n, local_n, my_rank, comm);
  
  if (my_rank == 0) {
    printf("prefix array: [");
    for (int i = 0; i < n; ++i) {
      printf(" %d ", prefix[i]);
    }
    printf("]\n");
  }
  
  MPI_Scan(&prefix, &local_prefix_sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  MPI_Gather(&local_prefix_sum, 1, MPI_INT, final_prefix_sum, 1, MPI_INT, 0, comm);

  if (my_rank == 0) {
    printf("prefix sums");
    for (int i = 0; i < n; i++) {
      printf("\nsum[%d] = %d", i, final_prefix_sum[i]);
    }
    printf("\n");
  }
  
  free(prefix);
  free(final_prefix_sum);
  MPI_Finalize();
}

void Read_n(int *n, int *local_n, int my_rank, int comm_sz, MPI_Comm comm) {
  
  MPI_Datatype type;
  
  MPI_Type_contiguous(1, MPI_INT, &type);
  MPI_Type_commit(&type);
  
  // get input
  if (my_rank == 0) {
    printf("How many elements do you want (size of array)?\n");
    scanf("%d", n);
  }
  
  MPI_Bcast(n, 1, type, 0, comm);
  
  *local_n = *n / comm_sz;
}

void Generate_data(int *prefix, int n, int local_n, int my_rank, MPI_Comm comm) {
  int *data = NULL;
  
  if (my_rank == 0) {
    data = malloc(local_n * sizeof(int));
    
    srand((unsigned int) time(0));
    
    for (int i = 0; i < local_n; i++) {
      data[i] = rand() % 23;
    }
  }
  
  MPI_Bcast( data, 1, MPI_INT, 0, comm);
  MPI_Scatter(data, n, MPI_INT, prefix, n, MPI_INT, 0, comm);
  
  if (my_rank == 0) {
    free(data);
  }
}

void Prefix_sum(int *prefix, int *final_prefix_sum, int n, int local_n, int my_rank, MPI_Comm comm) {
  int *data = NULL;
  
  if (my_rank == 0) {
    data = malloc(local_n * sizeof(int));
    prefix[0] = data[0];

    for (int i = 1; i < local_n; i++) {
      data[i] = data[i - 1] + prefix[i];
    }
  }
  
  MPI_Scatter(final_prefix_sum, n, MPI_INT, prefix, n, MPI_INT, 0, comm);

  if (my_rank == 0) {
    free(final_prefix_sum);
  }
}
