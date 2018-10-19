#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <stdlib.h>

void Read_n(int *n, int *local_n, int my_rank, int comm_sz, MPI_Comm comm);

void Generate_data(int *prefix, int n, int local_n, int my_rank, MPI_Comm comm);

void Prefix_sum(int *prefix, int *final_prefix_sum, int n, int local_n, int my_rank, MPI_Comm comm);

int main(int argc, char **argv) {
  
  int my_rank;
  int comm_sz;
  int n;
  int local_n;
  int *local_prefix_sum;
  int local_sum = 0;
  int final_sum = 0;
  int *prefix;
  int *final_prefix_sums;
  MPI_Comm comm;
  
  comm = MPI_COMM_WORLD;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(comm, &comm_sz);
  MPI_Comm_rank(comm, &my_rank);
  
  // read n
  Read_n(&n, &local_n, my_rank, comm_sz, comm);
  
  prefix = malloc(n * sizeof(int));
  local_prefix_sum = malloc(local_n * sizeof(int));
  final_prefix_sums = malloc(n * sizeof(int));
  
  Generate_data(prefix, n, local_n, my_rank, comm);
  
  if (my_rank == 0) {
    printf("prefix array: [");
    for (int i = 0; i < n; ++i) {
      printf(" %d ", prefix[i]);
    }
    printf("]\n");
  }
  
  for (int j = 0; j < local_n; j++) {
    local_sum += prefix[j];
    prefix[j] = local_sum;
  }
  
  MPI_Scan(&local_sum, &final_sum, 1, MPI_INT, MPI_SUM, comm);
  
  int temp_sum = final_sum - local_sum;
  for (int l = 0; l < local_n; ++l) {
    prefix[l] += temp_sum;
  }

  MPI_Gather(prefix, local_n, MPI_INT, final_prefix_sums, local_n, MPI_INT, 0, comm);
  
  if (my_rank == 0) {
    printf("prefix sums");
    for (int i = 0; i < n; i++) {
      printf("\nsum[%d] = %d", i + 1, final_prefix_sums[i]);
    }
    printf("\n");
  }
  
  free(prefix);
  free(final_prefix_sums);
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

void Generate_data(int *prefix, int n, int local_n, int my_rank, MPI_Comm comm) {
  int *data = NULL;
  
  if (my_rank == 0) {
    data = malloc(local_n * sizeof(int));
    
    srand((unsigned int) time(0));
    
    for (int i = 0; i < local_n; i++) {
      data[i] = rand() % 24;
    }
  }
  
  MPI_Scatter(data, local_n, MPI_INT, prefix, local_n, MPI_INT, 0, comm);
  
  if (my_rank == 0) {
    free(data);
  }
}

void Prefix_sum(int *prefix, int *final_prefix_sum, int n, int local_n, int my_rank, MPI_Comm comm) {
  int *data = NULL;
  
  if (my_rank == 0) {
    data = malloc(local_n * sizeof(int));
    prefix[0] = data[0];
    
    for (int i = 1; i < n; i++) {
      data[i] = data[i - 1] + prefix[i];
    }
  }
  
  MPI_Scatter(final_prefix_sum, n, MPI_INT, prefix, n, MPI_INT, 0, comm);
  
  if (my_rank == 0) {
    free(final_prefix_sum);
  }
}



//  for (int j = 0; j < local_n; j++) {
//    local_total += prefix[j];
//    MPI_Scan(&pdf_i, &cdf_i, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
//    final_prefix_sums[j] = cdf_i;
//  }

//  printf("%d", local_total);

//  local_total = prefix[0];

//  MPI_Scan(&local_total, &local_prefix_sum[0], 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
//  MPI_Gather(&local_prefix_sum, local_n, MPI_INT, final_prefix_sums, local_n, MPI_INT, 0, comm);

//  MPI_Scan(&local_total, &final_sum, local_n, MPI_INT, MPI_SUM, MPI_COMM_WORLD);