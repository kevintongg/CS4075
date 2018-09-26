#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <mpi.h>

void Gen_data(float local_data[], int local_data_count, int data_count, float min_value, float max_value, int my_rank, MPI_Comm comm);

void Get_input(int *bin_count, float *min_value, float *max_value, int *data_count, int *local_data_count, int my_rank, int comm_sz, MPI_Comm comm);

int main() {
  int comm_sz;
  int my_rank;
  int bin_count;
  int *bin_counts;
  int *bin_count_cumulative = NULL;
  int data_count;
  int local_data_count;
  int i;
  int j;
  float *min_value;
  float *max_value;
  float *bin_min;
  float *data = NULL;
  float *local_data = NULL;
  float bin_width;
  MPI_Comm comm;
  
  comm = MPI_COMM_WORLD;
  MPI_Init(NULL, NULL);
  MPI_Comm_size(comm, &comm_sz);
  MPI_Comm_rank(comm, &comm_sz);
  
  
  Get_input(&bin_count, min_value, max_value, &data_count, &local_data_count, my_rank, comm_sz, comm)
}

void Get_input(int *bin_count, float *min_value, float *max_value, int *data_count, int *local_data_count, int my_rank,
           int comm_sz, MPI_Comm comm) {
  if (my_rank == 0) {
    printf("Number of ints\n");
    scanf("%d", bin_count);
    printf("Min value\n");
    scanf("%f", min_value);
    printf("Max value\n");
    scanf("%f", max_value);
    if (*max_value < *min_value) {
      float *temp = max_value;
      max_value = min_value;
      min_value = temp
    }
    printf("Number of ints\n");
    scanf("%d", data_count);
    
    // make data count a multiple of comm sz
    *local_data_count = *data_count / comm_sz;
    *data_count = *data_count / comm_sz;
    printf("\n");
    
    MPI_Bcast(bin_count, 1, MPI_INT, 0, comm);
    MPI_Bcast(min_value, 1, MPI_FLOAT, 0, comm);
    MPI_Bcast(max_value, 1, MPI_FLOAT, 0, comm);
    MPI_Bcast(data_count, 1, MPI_INT, 0, comm);
    MPI_Bcast(local_data_count, 1, MPI_INT, 0, comm);
  }
}

void Gen_data(float *local_data, int local_data_count, int data_count, float min_value, float max_value, int my_rank, MPI_Comm comm) {
  float *data;
  
  if (my_rank == 0) {
    float range = max_value - min_value;
    data = malloc(data_count*sizeof(float));
    
    for (int i = 0; i < data_count; i++) {
      data[i] = (float) rand() / (float) RAND_MAX * range + min_value;
    }
  }
  
  MPI_Scatter(data, local_data_count, MPI_FLOAT, local_data, local_data_count, MPI_FLOAT, 0, comm);
  
  if (my_rank == 0) {
    free(data);
  }
}
