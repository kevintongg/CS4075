//
// Created by Kevin Tong on 28/10/2018.
//

#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INT MPI_INT

void Get_input(int argc, char *argv[], int myRank, long *total_num_tosses, MPI_Comm comm);

long Toss(long number_of_tosses, int myRank);

int main(int argc, char **argv) {
  int my_rank, comm_sz;
  long total_num_tosses, num_of_tosses, num_in_circle, total_num_in_circle;
  double piEstimate;
  MPI_Comm comm = MPI_COMM_WORLD;
  
  MPI_Init(&argc, &argv);
  MPI_Comm_size(comm, &comm_sz);
  MPI_Comm_rank(comm, &my_rank);
  
  Get_input(argc, argv, my_rank, &total_num_tosses, comm);
  
  num_of_tosses = total_num_tosses / comm_sz;
  
  MPI_Barrier(comm);
  num_in_circle = Toss(num_of_tosses, my_rank);
  
  MPI_Reduce(&num_in_circle, &total_num_in_circle, 1, MPI_LONG, MPI_SUM, 0, comm);
  
  if (my_rank == 0) {
    piEstimate = (4 * total_num_in_circle) / ((double) total_num_tosses);
    printf("π is approximately %.8f\n", piEstimate);
  }
  MPI_Finalize();
  return 0;
}

void Get_input(int argc, char *argv[], int myRank, long *total_num_tosses, MPI_Comm comm) {
  if (myRank == 0) {
    if (argc != 2) {
      fprintf(stderr, "usage: mpiexec -n <N> %s <number of tosses> \n", argv[0]);
      fflush(stderr);
      *total_num_tosses = 0;
    } else {
      *total_num_tosses = atoi(argv[1]);
    }
  }
  MPI_Bcast(total_num_tosses, 1, MPI_LONG, 0, comm);
  
  if (*total_num_tosses == 0) {
    MPI_Finalize();
    exit(-1);
  }
}

long Toss(long number_of_tosses, int myRank) {
  long toss, number_in_circle = 0;
  double x, y, distance_squared;
  unsigned int seed = (unsigned) time(NULL);
  srand(seed + myRank);
  for (toss = 0; toss < number_of_tosses; toss++) {
    x = rand_r(&seed) / (double) RAND_MAX;
    y = rand_r(&seed) / (double) RAND_MAX;
    distance_squared = pow(x, 2) + pow(y, 2);
    if (distance_squared <= 1.0) {
      number_in_circle++;
    }
  }
  return number_in_circle;
}