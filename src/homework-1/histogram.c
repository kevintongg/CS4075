#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <mpi.h>

void Gen_data(float local_data[], int local_data_count, int data_count, float min_meas, float max_meas, int my_rank,
              MPI_Comm comm);

void Get_input(int *bin_count, float *min_meas, float *max_meas, int *data_count, int *local_data_count, int my_rank,
               int comm_sz, MPI_Comm comm);

void Set_bins(float bin_maxes[], int loc_bin_cts[], float min_meas, float max_meas, int bin_count, int my_rank,
              MPI_Comm comm);

void Find_bin(int bin_counts[], float local_data[], int loc_bin_cts[], int local_data_count, float bin_maxes[],
              int bin_count, float min_meas, MPI_Comm comm);

int Which_bin(float data, const float bin_maxes[], int bin_count, float min_meas);

void Print(float bin_maxes[], int bin_counts[], int bin_count, float min_meas);

int main() {
  int bin_count; // num of bins
  float min_meas; // min meas min value for bin containing smallest values
  float max_meas; // max meas max value for bin containing smallest values
  float *bin_maxes; // array of bin count floats
  int *bin_counts; // array of bin count ints
  int *loc_bin_cts;
  int data_count; // num of measurements
  int local_data_count;
  float *data; // array of data count floats
  float *local_data;
  int my_rank;
  int comm_sz;
  MPI_Comm comm;

  comm = MPI_COMM_WORLD;
  MPI_Init(NULL, NULL);
  MPI_Comm_size(comm, &comm_sz);
  MPI_Comm_rank(comm, &my_rank);

  Get_input(&bin_count, &min_meas, &max_meas, &data_count, &local_data_count, my_rank, comm_sz, comm);

  // allocate memory for arrays
  bin_maxes = malloc(bin_count * sizeof(float));
  bin_counts = malloc(bin_count * sizeof(int));
  loc_bin_cts = malloc(bin_count * sizeof(int));
  data = malloc(data_count * sizeof(float));
  local_data = malloc(local_data_count * sizeof(float));
  
  Set_bins(bin_maxes, loc_bin_cts, min_meas, max_meas, bin_count, my_rank, comm);
  Gen_data(local_data, local_data_count, data_count, min_meas, max_meas, my_rank, comm);
  Find_bin(bin_counts, local_data, loc_bin_cts, local_data_count, bin_maxes, bin_count, min_meas, comm);
  MPI_Reduce(loc_bin_cts, bin_counts, bin_count, MPI_INT, MPI_SUM, 0, comm);

  if (my_rank == 0) {
    Print(bin_maxes, bin_counts, bin_count, min_meas);
  }

  free(bin_maxes);
  free(bin_counts);
  free(loc_bin_cts);
  free(data);
  free(local_data);

  MPI_Finalize();

  return 0;
}

void Get_input(int *bin_count, float *min_meas, float *max_meas, int *data_count, int *local_data_count, int my_rank,
               int comm_sz, MPI_Comm comm) {
  if (my_rank == 0) {
    printf("Number of ints\n");
    scanf("%d", bin_count);
    printf("Min value\n");
    scanf("%f", min_meas);
    printf("Max value\n");
    scanf("%f", max_meas);
    if (*max_meas < *min_meas) {
      float *temp = max_meas;
      max_meas = min_meas;
      min_meas = temp;
    }
    printf("Number of ints\n");
    scanf("%d", data_count);

    // make data count a multiple of comm sz
    *local_data_count = *data_count / comm_sz;
    *data_count = *data_count / comm_sz;
    printf("\n");

    MPI_Bcast(bin_count, 1, MPI_INT, 0, comm);
    MPI_Bcast(min_meas, 1, MPI_FLOAT, 0, comm);
    MPI_Bcast(max_meas, 1, MPI_FLOAT, 0, comm);
    MPI_Bcast(data_count, 1, MPI_INT, 0, comm);
    MPI_Bcast(local_data_count, 1, MPI_INT, 0, comm);
  }
}

void Gen_data(float *local_data, int local_data_count, int data_count, float min_meas, float max_meas, int my_rank,
              MPI_Comm comm) {
  float *data;

  if (my_rank == 0) {
    float range = max_meas - min_meas;
    data = malloc(data_count * sizeof(float));

    for (int i = 0; i < data_count; i++) {
      data[i] = (float) rand() / (float) RAND_MAX * range + min_meas;
    }
  }

  MPI_Scatter(data, local_data_count, MPI_FLOAT, local_data, local_data_count, MPI_FLOAT, 0, comm);

  if (my_rank == 0) {
    free(data);
  }
}

/* Find out the appropriate bin for each data in local_data and increase the number of data in this bin  */
void Set_bins(float bin_maxes[], int loc_bin_cts[], float min_meas, float max_meas, int bin_count, int my_rank,
              MPI_Comm comm) {
  float range = max_meas - min_meas;
  float interval = range / bin_count;

  for (int i = 0; i < bin_count; i++) {
    bin_maxes[i] = interval * (float) (i + 1) + min_meas;
    loc_bin_cts[i] = 0;
  }
}

/* Find out the appropriate bin for each data in local_data and increase the number of data in this bin  */
void Find_bin(int bin_counts[], float local_data[], int loc_bin_cts[], int local_data_count, float bin_maxes[],
              int bin_count, float min_meas, MPI_Comm comm) {
  int bin;

  for (int i = 0; i < local_data_count; i++) {
    bin = Which_bin(local_data[i], bin_maxes, bin_count, min_meas);
    loc_bin_cts[bin]++;
  }
}

/* Find out the appropriate bin for each data */
int Which_bin(float data, const float *bin_maxes, int bin_count, float min_meas) {
  int i;

  for (i = 0; i < bin_count - 1; i++) {
    if (data < bin_maxes[i]) {
      break;
    }
  }

  return i;
}

void Print(float bin_maxes[], int bin_counts[], int bin_count, float min_meas) {
  int width = 40;
  int max = 0;
  int row_width;
  int i;
  int j;

  for (i = 0; i < bin_count; i++) {
    if (bin_counts[i] > max) {
      max = bin_counts[i];
    }
  }

  for (i = 0; i < bin_count; ++i) {
    printf("%10.2f |", bin_maxes[i]);
    row_width = (int) ((float) bin_counts[i] / (float) max * (float) width);
    for (j = 0; j < row_width; ++j) {
      printf("#");
    }
    printf("%d\n", bin_counts[i]);
  }
}
