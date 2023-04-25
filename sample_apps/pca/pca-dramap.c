#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "stddefines.h"

#include "dram_ap_lib.h"

#define DEF_GRID_SIZE 100  // all values in the matrix are from 0 to this value 
#define DEF_NUM_ROWS 10
#define DEF_NUM_COLS 10

int num_rows;
int num_cols;
int grid_size;

/** parse_args()
 *  Parse the user arguments to determine the number of rows and colums
 */
void parse_args(int argc, char **argv) 
{
   int c;
   extern char *optarg;
   extern int optind;
   
   num_rows = DEF_NUM_ROWS;
   num_cols = DEF_NUM_COLS;
   grid_size = DEF_GRID_SIZE;
   
   while ((c = getopt(argc, argv, "r:c:s:")) != EOF) 
   {
      switch (c) {
         case 'r':
            num_rows = atoi(optarg);
            break;
         case 'c':
            num_cols = atoi(optarg);
            break;
         case 's':
            grid_size = atoi(optarg);
            break;
         case '?':
            printf("Usage: %s -r <num_rows> -c <num_cols> -s <max value>\n", argv[0]);
            exit(1);
      }
   }
   
   if (num_rows <= 0 || num_cols <= 0 || grid_size <= 0) {
      printf("Illegal argument value. All values must be numeric and greater than 0\n");
      exit(1);
   }

   printf("Number of rows = %d\n", num_rows);
   printf("Number of cols = %d\n", num_cols);
   printf("Max value for each element = %d\n", grid_size);   
}

/** dump_points()
 *  Print the values in the matrix to the screen
 */
void dump_points(int **vals, int rows, int cols)
{
   int i, j;
   
   for (i = 0; i < rows; i++) 
   {
      for (j = 0; j < cols; j++)
      {
         printf("%5d ",vals[i][j]);
      }
      printf("\n");
   }
}

/** generate_points()
 *  Create the values in the matrix
 */
void generate_points(int **pts, int rows, int cols) 
{   
   int i, j;
   
   for (i=0; i<rows; i++) 
   {
      for (j=0; j<cols; j++) 
      {
         pts[i][j] = rand() % grid_size;
      }
   }
}

/** calc_mean()
 *  Compute the mean for each row
 */
void calc_mean(int **matrix, int *mean) {
   int i, j;
   int sum = 0;
   
   for (i = 0; i < num_rows; i++) {
      sum = 0;
      for (j = 0; j < num_cols; j++) {
         sum += matrix[i][j];
      }
      mean[i] = sum / num_cols;   
   }
}

/** calc_cov()
 *  Calculate the covariance
 */
void calc_cov(int **matrix, int *mean, int **cov) {
   int i, j, k;
   int sum;
   
   for (i = 0; i < num_rows; i++) {
      for (j = i; j < num_rows; j++) {
         sum = 0;
         for (k = 0; k < num_cols; k++) {
            sum = sum + ((matrix[i][k] - mean[i]) * (matrix[j][k] - mean[j]));
         }
         cov[i][j] = cov[j][i] = sum/(num_cols-1);
      }   
   }   
}


int main(int argc, char **argv) {
   
   int i;
   int **matrix, **cov;
   int *mean;
   
   parse_args(argc, argv);   
   
   // Create the matrix to store the points
   matrix = (int **)malloc(sizeof(int *) * num_rows);
   for (i=0; i<num_rows; i++) 
   {
      matrix[i] = (int *)malloc(sizeof(int) * num_cols);
   }

   // printf("num")
   
   //Generate random values for all the points in the matrix
   generate_points(matrix, num_rows, num_cols);
   
   // Print the points
   dump_points(matrix, num_rows, num_cols);

   /* ++++++++++ START DRAM-AP  ++++++++++ */ 

   int* meanVec; // allocate at non-DRAM_AP region
   int* src1_v;
   int* src2_v;

   input_file_handler matrix_fd;
   matrix_fd.data = malloc(sizeof(int) * num_rows * num_cols);
   matrix_fd.size = (unsigned long long) num_rows * num_cols; // file handler containing metadata data and size


   const int BLOCK_FACTOR = 2; // concurrently two vec ops
   int vl = num_cols; // matrix_len
   int bit_len = 32;
   int group_1 = 0;
   int group_2 = 1;

   meanVec = malloc(num_rows); // host memory region
   
   dram_ap_mmap(matrix, &matrix_fd, num_rows, num_cols);
   dram_ap_valloc(&src1_v, group_1, vl, bit_len);
   dram_ap_valloc(&src2_v, group_2, vl, bit_len);



   // calculate mean vector

   for (int r = 0; r < num_rows; r += BLOCK_FACTOR) {

   		int tmp1 = 0, tmp2 = 0;
   		dram_ap_fld(&matrix_fd, r * num_cols, 1, src1_v, vl, bit_len, 0);
   		dram_ap_fld(&matrix_fd, (r + 1) * num_cols, 1, src2_v, vl, bit_len, 0);

   		dram_ap_vredsum(&tmp1, src1_v, vl, bit_len);
   		dram_ap_vredsum(&tmp2, src2_v, vl, bit_len);
   		
   		tmp1 = tmp1 / num_cols; // is this calculated at the host?
   		tmp2 = tmp2 / num_cols; // is this calculated at the host?

   		meanVec[r] = tmp1; // send to host mem region leveraging the DCA?
   		meanVec[r+1] = tmp2;
   }





   



   


   /* ++++++++++ END DRAM-AP  ++++++++++ */ 
   
   /* ++++++++++ START CPU BASELINE  ++++++++++ */ 
   // Allocate Memory to store the mean and the covariance matrix
   mean = (int *)malloc(sizeof(int) * num_rows);
   cov = (int **)malloc(sizeof(int *) * num_rows);
   for (i=0; i<num_rows; i++) 
   {
      cov[i] = (int *)malloc(sizeof(int) * num_rows);
   }
   
   /* CPU kernel */
   // Compute the mean and the covariance
   calc_mean(matrix, mean);

   // for (int i=0;i<num_rows;i++) {
   // 	printf("%d ", mean[i]);
   // }
   // printf("\n");

   calc_cov(matrix, mean, cov); 

   dump_points(cov, num_rows, num_rows);
   /* ++++++++++ END CPU BASELINE  ++++++++++ */ 


   return 0;
}

