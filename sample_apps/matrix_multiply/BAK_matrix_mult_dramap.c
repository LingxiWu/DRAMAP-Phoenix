/* Copyright (c) 2007, Stanford University
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Stanford University nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY  BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/ 

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

typedef struct {
   int *matrix_A;
   int *matrix_B;
   int *matrix_out;
   int matrix_len;
} mm_data_t;

#define BLOCK_LEN 100

/** matrix_mult()
 *  Blocked Matrix Multiply Function
 */
void matrix_mult(mm_data_t *data_in)
{
	assert(data_in);
	int i, j, k,a, b, c, end_i, end_j, end_k;

	for(i = 0;i < (data_in->matrix_len)*data_in->matrix_len ; i++)
	{
		if(i%data_in->matrix_len == 0)
			dprintf("\n");
		dprintf("%d  ",data_in->matrix_A[i]);
      // printf("i: %d, data_in->matrix_A[i]: %d\n",i,data_in->matrix_A[i]);
	}
	dprintf("\n\n");


	for(i = 0;i < (data_in->matrix_len)*data_in->matrix_len ; i++)
	{
		if(i%data_in->matrix_len == 0)
			dprintf("\n");
		dprintf("%d  ",data_in->matrix_B[i]);
	}
	dprintf("\n\n");

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
         // dprintf("%d  ", data_in->matrix_out[(data_in->matrix_len)*i + j]);
         printf("matrix_out[%d]: %d\n", (data_in->matrix_len)*i + j, data_in->matrix_out[(data_in->matrix_len)*i + j]);
      }
      
      dprintf("\n");
   }
}

void matrix_mult_dramap_1(mm_data_t *data_in)
{

   // transposing the matrix_B
   int matrix_B[data_in->matrix_len * data_in->matrix_len];
   _DRAM_AP_matrix_trans(matrix_B, data_in->matrix_B, data_in->matrix_len);

   /* 0. declare result matrix_C */
   int matrix_C[data_in->matrix_len * data_in->matrix_len];

   for(int i = 0; i < data_in->matrix_len; i++) // ith row in MA
   {
      for(int j = 0; j < data_in->matrix_len; j++) // jth row in MB_t
      {

         int max_vl = 8192; // max physical bank width == max vector length
         int iter = (int)ceil((double)data_in->matrix_len / max_vl); // inner most loop iterations
         int sum = 0; // value at matrix_C[i][j]

         // DRAM_AP: each time perform vector multiplication 
         for(int k = 0; k < iter; k++)
         {
            /* 1. set vl */
            int vl = MIN(max_vl, data_in->matrix_len - k * max_vl);

            /* 2. load vec_1 */
            int vec_1[vl];
            int* matrix_a_seg = &data_in->matrix_A[(data_in->matrix_len)*i];
            _DRAM_AP_vld(vec_1, matrix_a_seg, vl, sizeof(int)*8);

            /* 3. load vec_2 */
            int vec_2[vl];
            int* matrix_b_seg = &matrix_B[(data_in->matrix_len)*j];
            _DRAM_AP_vld(vec_2, matrix_b_seg, vl, sizeof(int)*8);

            /* 4. mat mult */
            int vec_3[vl];
            _DRAM_AP_vmul(vec_3, vec_1, vec_2, vl, sizeof(int)*8);

            /* 5. reduction on vec_3 across column*/
            int temp_sum = 0;
            _DRAM_AP_vredsum(&temp_sum, vec_3, vl, sizeof(int)*8);
            sum += temp_sum;

            // printf("matrix_a_seg: %d, %d, %d\n", matrix_a_seg[0], matrix_a_seg[1], matrix_a_seg[2]);
            // printf("       vec_1: %d, %d, %d \n", vec_1[0], vec_1[1], vec_1[2]);
            // printf("matrix_b_seg: %d, %d, %d\n", matrix_b_seg[0], matrix_b_seg[1], matrix_b_seg[2]);
            // printf("       vec_2: %d, %d, %d \n", vec_2[0], vec_2[1], vec_2[2]);
            // printf("       vec_3: %d, %d, %d \n", vec_3[0], vec_3[1], vec_3[2]);
            // printf("    temp_sum: %d", temp_sum);

         }

         /* 6. assign reduction to */
         matrix_C[i*data_in->matrix_len+j] = sum;
         printf("matrix_C[%d]: %d\n", i*data_in->matrix_len+j, matrix_C[i*data_in->matrix_len+j]);

      }
   }
}

// setup a dram-ap compute kernel to be launched
// no arithmetic operations, only DRAM-AP memory operations
void __DRAM_AP_Kernel_Setup__(mm_data_t *data_in, dram_ap_kernel_config *kernel_config_obj){

   // transposing the matrix_B
   int matrix_B[data_in->matrix_len * data_in->matrix_len];
   _DRAM_AP_matrix_trans(matrix_B, data_in->matrix_B, data_in->matrix_len);

   dram_ap_arch_config arch_config;
   arch_config = _make_DRAM_AP_arch_config();


   /* 1.  declare a DRAM-AP kernel config object */ 

   /* determine how many vectors are needed */
   // for now, assuming its programmer's responsibility

   int iter = (int)ceil((double)data_in->matrix_len / arch_config.max_vl); // inner most loop iterations
   int vl = arch_config.max_vl;
   if(iter == 1)
      vl = MIN(arch_config.max_vl, data_in->matrix_len); // // quick and dirty, not the optimal solution all matrix columns fit inside one vector

   int total_vec_needed = data_in->matrix_len * data_in->matrix_len * 2 + data_in->matrix_len; 
   // int dram_ap_vec_srcs[total_vec_needed][vl]; // each matrix_row replicate matrix_len times, there are 2 matrices

   // check enough vectors
   assert(arch_config.num_bank * arch_config.num_sa_per_bank >= data_in->matrix_len * data_in->matrix_len * 2);

   /* 2. vectorizing code & make DRAM-AP kernel config obj */ 
   kernel_config_obj->vl = vl;
   kernel_config_obj->bit_len = sizeof(int)*8;
   kernel_config_obj->num_vec = total_vec_needed; 

   kernel_config_obj->source_vecs = malloc(kernel_config_obj->num_vec * sizeof(int*));
   for(int i=0;i<kernel_config_obj->num_vec;i++){
      kernel_config_obj->source_vecs[i] = malloc(kernel_config_obj->vl * sizeof(int));
   }

   // vectorize MA
   for (int i=0;i<data_in->matrix_len * data_in->matrix_len;i++){
      int seg_idx = i / data_in->matrix_len;
      int* matrix_a_seg = &data_in->matrix_A[seg_idx * data_in->matrix_len];
      // _DRAM_AP_vld(dram_ap_vec_srcs[i], matrix_a_seg, vl, sizeof(int)*8);
      _DRAM_AP_vld(kernel_config_obj->source_vecs[i], matrix_a_seg, vl, sizeof(int)*8); // DMA, no cache r/w
   }

   // vectorize MB
   for (int j=0;j<data_in->matrix_len * data_in->matrix_len;j++){
      int seg_idx = j / data_in->matrix_len;
      int* matrix_b_seg = &matrix_B[seg_idx * data_in->matrix_len];
      // _DRAM_AP_vld(dram_ap_vec_srcs[j + data_in->matrix_len * data_in->matrix_len], matrix_b_seg, vl, sizeof(int)*8);
      _DRAM_AP_vld(kernel_config_obj->source_vecs[j + data_in->matrix_len * data_in->matrix_len], matrix_b_seg, vl, sizeof(int)*8);
   }

   // vectorize MC, the result matrix. MC represented in three vectors, each vec is a row, intialize to zero
   for (int k=0;k<data_in->matrix_len;k++){
      int* res_row;
      res_row = malloc(kernel_config_obj->vl * sizeof(int));
      for (int m=0;m<kernel_config_obj->vl;m++){
         res_row[m] = 0;
      }
      _DRAM_AP_vld(kernel_config_obj->source_vecs[data_in->matrix_len * data_in->matrix_len * 2 + k], res_row, vl, sizeof(int)*8);
   }



   // // DEBUG print
   // printf("DEBUG output: kernel_config.source_vecs \n");
   // for (int i=0;i<kernel_config_obj->num_vec;i++){
   //    printf("kernel_config.source_vecs[%d] ",i);
   //    for (int j=0;j<vl;j++){
   //       printf("%d ",kernel_config_obj->source_vecs[i][j]);
   //    }
   //    printf(" \n");
   // }

   return;

}

// this function contains the procedure to be launched onto DRAM-AP
// right now just some dummy code to simulate 2 3x3 matrix multiplication
int __DRAM_AP_Kernel_Launch__(dram_ap_kernel_config* kernel_config, mm_data_t *data_in){

   printf("Launching DRAM-AP to compute kernel\n");

   // compute each row of matrix_output
   int vl = kernel_config->vl;
   int num_src_vecs = vl*vl*2;

   // PARALLEL compute for all temp_vec
   // compute for the first result row
   for(int i=0; i<vl; i++){ // row index of MA

      int temp_vec[vl];
      int temp_sum=0;

      _DRAM_AP_vmul(temp_vec, kernel_config->source_vecs[i], kernel_config->source_vecs[i*vl+vl*vl], vl, sizeof(int)*8);
      _DRAM_AP_vredsum(&temp_sum, temp_vec, vl, sizeof(int)*8);
      _DRAM_AP_vput(kernel_config->source_vecs[num_src_vecs], i, temp_sum, vl, sizeof(int)*8);

   }

   printf("1st row: ");
   for(int i=0;i<vl;i++){
      printf("%d ", kernel_config->source_vecs[num_src_vecs][i]);
   }
   printf("\n");

   // compute for the second result row
   for(int i=0; i<vl; i++){ // row index of MA

      int temp_vec[vl];
      int temp_sum=0;

      _DRAM_AP_vmul(temp_vec, kernel_config->source_vecs[i+vl], kernel_config->source_vecs[i*vl+vl*vl+1], vl, sizeof(int)*8);
      _DRAM_AP_vredsum(&temp_sum, temp_vec, vl, sizeof(int)*8);
      _DRAM_AP_vput(kernel_config->source_vecs[num_src_vecs+1], i, temp_sum, vl, sizeof(int)*8);

   }

   printf("2nd row: ");
   for(int i=0;i<vl;i++){
      printf("%d ", kernel_config->source_vecs[num_src_vecs+1][i]);
   }
   printf("\n");

   // compute for the second result row
   for(int i=0; i<vl; i++){ // row index of MA

      int temp_vec[vl];
      int temp_sum=0;

      _DRAM_AP_vmul(temp_vec, kernel_config->source_vecs[i+vl+vl], kernel_config->source_vecs[i*vl+vl*vl+2], vl, sizeof(int)*8);
      _DRAM_AP_vredsum(&temp_sum, temp_vec, vl, sizeof(int)*8);
      _DRAM_AP_vput(kernel_config->source_vecs[num_src_vecs+2], i, temp_sum, vl, sizeof(int)*8);

   }

   printf("3rd row: ");
   for(int i=0;i<vl;i++){
      printf("%d ", kernel_config->source_vecs[num_src_vecs+2][i]);
   }
   printf("\n");


   


   // DEBUG print
   printf("\nDEBUG output: kernel_config.vecs \n");
   for (int i=0;i<kernel_config->num_vec;i++){
      printf("kernel_config.source_vecs[%d] ",i);
      for (int j=0;j<vl;j++){
         printf("%d ",kernel_config->source_vecs[i][j]);
      }
      printf(" \n");
   }

   // put the value back from AP to normal address
   _DRAM_AP_vst(&data_in->matrix_out[0], &kernel_config->source_vecs[num_src_vecs][0], vl, sizeof(int)*8);
   _DRAM_AP_vst(&data_in->matrix_out[3], &kernel_config->source_vecs[num_src_vecs+1][0], vl, sizeof(int)*8);
   _DRAM_AP_vst(&data_in->matrix_out[6], &kernel_config->source_vecs[num_src_vecs+2][0], vl, sizeof(int)*8);

   // data_in->matrix_out[(data_in->matrix_len)*a + b]
   for(int i = 0; i < data_in->matrix_len; i++)
   {
      for(int j = 0; j < data_in->matrix_len; j++)
      {
         // dprintf("%d  ", data_in->matrix_out[(data_in->matrix_len)*i + j]);
         printf("DRAM-AP result -> matrix_out[%d]: %d\n", (data_in->matrix_len)*i + j, data_in->matrix_out[(data_in->matrix_len)*i + j]);
      }
      
      dprintf("\n");
   }




   return 0;


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
   

   printf("MatrixMult: Calling Serial Matrix Multiplication\n");

   //gettimeofday(&starttime,0);
   

   
   memset(mm_data.matrix_out, 0, file_size);

   printf(" --- CPU Baseline --- \n");
   matrix_mult(&mm_data);

   // printf(" --- DRAM-AP_v1 --- \n");
   // matrix_mult_dramap_1(&mm_data);

   printf(" --- DRAM-AP --- \n");

   dram_ap_kernel_config kernel_config;
   __DRAM_AP_Kernel_Setup__(&mm_data, &kernel_config);
   __DRAM_AP_Kernel_Launch__(&kernel_config, &mm_data);

   
   


   //printf("MatrixMult: Multiply Completed time = %ld\n", (endtime.tv_sec - starttime.tv_sec));
   // matrix_mult_dramap_1(&mm_data); // test run

   CHECK_ERROR(munmap(fdata_A, file_size + 1) < 0);
   CHECK_ERROR(close(fd_A) < 0);

   CHECK_ERROR(munmap(fdata_B, file_size + 1) < 0);
   CHECK_ERROR(close(fd_B) < 0);

   CHECK_ERROR(close(fd_out) < 0);

   return 0;
}
