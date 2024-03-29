#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <math.h>

double f(double x);

double Trap(double left_endpoint, double right_endpoint, int trapezoid_count, double base_length);

void Get_input(
    int my_rank, // in
    int comm_sz, // in
    double *a_p, // out
    double *b_p, // out
    int *n_p);   // out

int main(void) {
  int my_rank, comm_sz, n, local_n;
  double a, b, h, local_a, local_b;
  double local_int, total_int;
  
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  
  Get_input(my_rank, comm_sz, &a, &b, &n);
  
  h = (b - a) / n; /* h is the same for all processes */
  local_n = n / comm_sz; /* So is the number of trapezoids */
  
  local_a = a + my_rank * local_n * h;
  local_b = local_a + local_n * h;
  local_int = Trap(local_a, local_b, local_n, h);
  
  MPI_Reduce(&local_int, &total_int, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  
  if (my_rank == 0) {
    printf("\nWith n = %d trapezoids, our estimate of the integral from %f to %f = %.4f\n", n, a, b, total_int);
  }
  
  MPI_Finalize();
  return 0;
} /* main */

double f(double x) {
  return pow(x, 3);
}

double Trap(double left_endpoint, double right_endpoint, int trapezoid_count, double base_length) {
  double estimate;
  double x;
  int i;
  
  estimate = (f(left_endpoint) + f(right_endpoint)) / 2.0;
  for (i = 1; i <= trapezoid_count - 1; i++) {
    x = left_endpoint + i * base_length;
    estimate += f(x);
  }
  
  return estimate * base_length;
}

void Get_input(
    int my_rank, // in
    int comm_sz, // in
    double *a_p, // out
    double *b_p, // out
    int *n_p)    // out
{
  int destination;
  
  if (my_rank == 0) {
    printf("Input a, b, and n\n");
    printf("Enter a\n");
    scanf("%lf", a_p);
    printf("Enter b\n");
    scanf("%lf", b_p);
    printf("Enter n\n");
    scanf("%d", n_p);
    for (destination = 1; destination < comm_sz; destination++) {
      MPI_Send(a_p, 1, MPI_DOUBLE, destination, 0, MPI_COMM_WORLD);
      MPI_Send(b_p, 1, MPI_DOUBLE, destination, 0, MPI_COMM_WORLD);
      MPI_Send(n_p, 1, MPI_INT, destination, 0, MPI_COMM_WORLD);
    }
  } else /* my_rank != 0 */ {
    MPI_Recv(a_p, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(b_p, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(n_p, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
} /* get input */