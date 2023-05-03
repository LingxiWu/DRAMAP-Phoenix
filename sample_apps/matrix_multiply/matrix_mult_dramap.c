#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>

#include <math.h>

#include "stddefines.h"

#include "dram_ap_lib.h"

// typedef struct {
//    int *matrix_A;
//    int *matrix_B;
//    int *matrix_out;
//    int matrix_len;
// } mm_data_t;

#define BLOCK_LEN 100

/** matrix_mult()
 *  Blocked Matrix Multiply Function
 */
void matrix_mult(mm_data_t *data_in)
{
	assert(data_in);
	int i, j, k,a, b, c, end_i, end_j, end_k;

	// for(i = 0;i < (data_in->matrix_len)*data_in->matrix_len ; i++)
	// {
	// 	if(i%data_in->matrix_len == 0)
	// 		dprintf("\n");
	// 	dprintf("%d  ",data_in->matrix_A[i]);
    //   printf("i: %d, data_in->matrix_A[i]: %d\n",i,data_in->matrix_A[i]);
	// }
	// dprintf("\n\n");


	// for(i = 0;i < (data_in->matrix_len)*data_in->matrix_len ; i++)
	// {
	// 	if(i%data_in->matrix_len == 0)
	// 		dprintf("\n");
	// 	dprintf("%d  ",data_in->matrix_B[i]);
	// }
	// dprintf("\n\n");

   for(i = 0; i < data_in->matrix_len; i += BLOCK_LEN)
   for(j = 0; j < data_in->matrix_len; j += BLOCK_LEN)
   for(k = 0; k < data_in->matrix_len; k += BLOCK_LEN)
   {
      end_i = i + BLOCK_LEN; end_j = j + BLOCK_LEN; end_k = k + BLOCK_LEN;
      for (a = i; a < end_i && a < data_in->matrix_len; a++)
      for (b = j; b < end_j && b < data_in->matrix_len; b++)
      for (c = k; c < end_k && c < data_in->matrix_len; c++)
      {
               int temp = ( data_in->matrix_A[ (data_in->matrix_len)*a + c] * data_in->matrix_B[ (data_in->matrix_len)*c + b]);
               // printf("temp: %d\n", temp);
               data_in->matrix_out[(data_in->matrix_len)*a + b] += temp;
                  
      }

   }

   for(i = 0; i < data_in->matrix_len; i++)
   {
      for(j = 0; j < data_in->matrix_len; j++)
      {
         printf("%d ", data_in->matrix_out[(data_in->matrix_len)*i + j]);
      }
      
      printf("\n");
   }
}



int main(int argc, char *argv[]) 
{
   
   int fd_A, fd_B, fd_out;
   char * fdata_A, *fdata_B, *fdata_out;
   int matrix_len, file_size;
   struct stat finfo_A, finfo_B;
   char * fname_A, *fname_B,*fname_out;

   srand( (unsigned)time( NULL ) );

   // Make sure a filename is specified
   if (argv[1] == NULL)
   {
      printf("USAGE: %s [side of matrix]\n", argv[0]);
      exit(1);
   }

   fname_A = "matrix_file_A.txt";
   fname_B = "matrix_file_B.txt";
   fname_out = "matrix_file_out_serial.txt";
   CHECK_ERROR ( (matrix_len = atoi(argv[1])) < 0);
   file_size = ((matrix_len*matrix_len))*sizeof(int);

   printf("MatrixMult: Side of the matrix is %d \n", matrix_len);
   printf("MatrixMult: Running...\n");
   
   

   // Read in the file
   CHECK_ERROR((fd_A = open(fname_A,O_RDONLY)) < 0);
   // Get the file info (for file length)
   CHECK_ERROR(fstat(fd_A, &finfo_A) < 0);
   // Memory map the file
   CHECK_ERROR((fdata_A= mmap(0, file_size + 1,
      PROT_READ | PROT_WRITE, MAP_PRIVATE, fd_A, 0)) == NULL);

   // Read in the file
   CHECK_ERROR((fd_B = open(fname_B,O_RDONLY)) < 0);
   // Get the file info (for file length)
   CHECK_ERROR(fstat(fd_B, &finfo_B) < 0);
   // Memory map the file
   CHECK_ERROR((fdata_B= mmap(0, file_size + 1,
      PROT_READ | PROT_WRITE, MAP_PRIVATE, fd_B, 0)) == NULL);
   
   // Create File
   CHECK_ERROR((fd_out = open(fname_out,O_CREAT | O_RDWR,S_IRWXU)) < 0);
   // Resize
   CHECK_ERROR(ftruncate(fd_out, file_size) < 0);
   // Memory Map
   CHECK_ERROR((fdata_out= mmap(0, file_size + 1,
      PROT_READ | PROT_WRITE, MAP_PRIVATE, fd_out, 0)) == NULL);

   // Setup splitter args
   mm_data_t mm_data;
   mm_data.matrix_len = matrix_len;
   mm_data.matrix_A  = ((int *)fdata_A);
   mm_data.matrix_B  = ((int *)fdata_B);
   mm_data.matrix_out  = ((int *)fdata_out);

   /* START DRAM_AP MATMUL */
   printf("\n+++ DRAM-AP +++ MatrixMult\n");

   // emulated file handler returned by dram_ap_fopen(..., fname, ...);
   matrix_file_handler matrix_fd_A;
   matrix_fd_A.data = ((int *)fdata_A);
   matrix_fd_A.size = matrix_len; // this should NOT be matrix_len

   matrix_file_handler matrix_fd_B;
   matrix_fd_B.data = ((int *)fdata_B);
   matrix_fd_B.size = matrix_len;

   const int BLOCK_FACTOR = 2;
	int vl = matrix_len;
	int bit_len = 32;

	int group_id_0 = 0;
	int group_id_1 = 1;

	int* src_A_0;
	int* src_A_1; // a copy of src_A_0;
	int* src_B_0;
	int* src_B_1;
	int* tmp_res_0;
	int* tmp_res_1;
	int* res; // non DRAM_AP region


	res = malloc(file_size);
	memset(res, 0, file_size); // initialize result matrix to zeros

	dram_ap_valloc(&src_A_0, group_id_0, vl, bit_len);
	dram_ap_valloc(&src_A_1, group_id_1, vl, bit_len);
	dram_ap_valloc(&src_B_0, group_id_0, vl, bit_len);
	dram_ap_valloc(&src_B_1, group_id_1, vl, bit_len);
	dram_ap_valloc(&tmp_res_0, group_id_0, vl, bit_len);
	dram_ap_valloc(&tmp_res_1, group_id_0, vl, bit_len);

	for (int i = 0; i<matrix_len; i++) { // matrix_A row
		dram_ap_fld(&matrix_fd_A, i, matrix_len, src_A_0, vl, bit_len, 1); 
		dram_ap_fld(&matrix_fd_A, i, matrix_len, src_A_1, vl, bit_len, 1);
		// matrix_B column. NOTE: Simultaneously handle BLOCK_FACTOR cols of B 
		for (int j=0; j<matrix_len; j+=BLOCK_FACTOR){ 
			
			dram_ap_fld(&matrix_fd_B, j, matrix_len, src_B_0, vl, bit_len, 0);
			dram_ap_fld(&matrix_fd_B, (j+1), matrix_len, src_B_1, vl, bit_len, 0);

			dram_ap_vmul(tmp_res_0, src_A_0, src_B_0, vl, bit_len);
			dram_ap_vmul(tmp_res_1, src_A_1, src_B_1, vl, bit_len);

			int tmp0 = 0;
			int tmp1 = 0;

			dram_ap_vredsum(&tmp0, tmp_res_0, vl, bit_len);
			dram_ap_vredsum(&tmp1, tmp_res_1, vl, bit_len);

			res[(matrix_len)*i + j] += tmp0;
			res[(matrix_len)*i + (j+1)] += tmp1;
			
		}
	}

	for(int row = 0; row < vl; row++) {
		for(int col = 0; col < vl; col++) {
			printf("%d ", res[matrix_len * row + col]);
		}
		printf(" \n");
	}

	free(src_A_0);
	free(src_A_1);
	free(src_B_0);
	free(src_B_1);
	free(tmp_res_0);
	free(tmp_res_1);
	free(res);


	// free(res);

   /* END DRAM_AP MATMUL */
   

   printf("\n+++ CPU BASELINE +++ MatrixMult: Calling Serial Matrix Multiplication\n");

   //gettimeofday(&starttime,0);

   
   /* START CPU MATMUL */
   memset(mm_data.matrix_out, 0, file_size);
   matrix_mult(&mm_data);
   /* END CPU MATMUL */


   

   //printf("MatrixMult: Multiply Completed time = %ld\n", (endtime.tv_sec - starttime.tv_sec));

   CHECK_ERROR(munmap(fdata_A, file_size + 1) < 0);
   CHECK_ERROR(close(fd_A) < 0);

   CHECK_ERROR(munmap(fdata_B, file_size + 1) < 0);
   CHECK_ERROR(close(fd_B) < 0);

   CHECK_ERROR(close(fd_out) < 0);

   return 0;
}