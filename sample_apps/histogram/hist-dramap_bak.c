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
* DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE FOR ANY
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

#include <math.h>

#include "stddefines.h"
#include "dram_ap_lib.h"

#define IMG_DATA_OFFSET_POS 10
#define BITS_PER_PIXEL_POS 28


typedef struct {
   
   char *img_data;
   struct stat file_stats;
   int imgdata_bytes;
   int data_pos;

} input_struct_t;


int swap;      // to indicate if we need to swap byte order of header information


/* test_endianess
 *
 */
void test_endianess() {
   unsigned int num = 0x12345678;
   char *low = (char *)(&(num));
   if (*low ==  0x78) {
      dprintf("No need to swap\n");
      swap = 0;
   }
   else if (*low == 0x12) {
      dprintf("Need to swap\n");
      swap = 1;
   }
   else {
      printf("Error: Invalid value found in memory\n");
      exit(1);
   } 
}

/* swap_bytes
 *
 */
void swap_bytes(char *bytes, int num_bytes) {
   int i;
   char tmp;
   
   for (i = 0; i < num_bytes/2; i++) {
      dprintf("Swapping %d and %d\n", bytes[i], bytes[num_bytes - i - 1]);
      tmp = bytes[i];
      bytes[i] = bytes[num_bytes - i - 1];
      bytes[num_bytes - i - 1] = tmp;   
   }
}

void __DRAM_AP_Kernel_Launch__(input_struct_t *file_data, dram_ap_kernel_config *kernel_config_obj){

   // note: one strategy is to separate RGB pixels into different regions and aggregate separately
   // but the original pixels are interleaved BGRBGRBGR..., separating them out would be hard
   // can't allocate individual regions/vectors for each color pixel
   // Alternatively, use indexed mask to indicate working on RGB

   printf("\nDRAM-AP setup kernel\n");
   printf("file_data->file_stats.st_size: %ld\n",file_data->file_stats.st_size); // 102 bytes total
   printf("file_data->imgdata_bytes: %d\n",file_data->imgdata_bytes); // 48 bytes, 16 pixel, 3 channels, one byte per pixel
   printf("file_data->data_pos: %d\n",file_data->data_pos); // 54. data starting at the 54th bytes

   dram_ap_arch_config arch_config;
   arch_config = _make_DRAM_AP_arch_config();

   int total_num_pix = file_data->imgdata_bytes; 
   int num_pix_vec = (int)ceil((double)total_num_pix / arch_config.max_vl); // # of src vectors for storing image pixels
   printf("total_num_pix: %d, max_vl: %d, num_pix_vec: %d\n", total_num_pix, arch_config.max_vl, num_pix_vec);


   /* 1. allocate DRAM-AP space */
   kernel_config_obj->source_vecs = malloc((num_pix_vec+3) * sizeof(char*)); // three more for mask vectors
   kernel_config_obj->num_vec = num_pix_vec + 3 + 3; // num_pix_vec pix vectors, 3 mask vectors, 3 result hist vectors.

   for(int i=0; i<num_pix_vec; i++){
      int vl = MIN(arch_config.max_vl, total_num_pix-arch_config.max_vl*i);
      char* pix_segment = &file_data->img_data[file_data->data_pos + i*vl];

      kernel_config_obj->source_vecs[i] = malloc(vl * sizeof(char));
      _DRAM_AP_vld_char(kernel_config_obj->source_vecs[i], pix_segment, vl, 8);

   }

   /* 2. build mask vector, 0 at column position i means don't care */
   // pixel order is Blue Green Red
   char mask_blue[arch_config.max_vl];
   char mask_green[arch_config.max_vl];
   char mask_red[arch_config.max_vl];
   
   for (int i=0;i<arch_config.max_vl;i++){
      mask_blue[i] = mask_green[i] = mask_red[i] = 0;
   }
   int remainder = arch_config.max_vl % 3;

   int i;
   for (i=0; i < (arch_config.max_vl - remainder); i+=3) {   
         mask_blue[i] = 1;
   }
   for (i=1; i < (arch_config.max_vl - remainder); i+=3) {   
         mask_green[i] = 1;
   }
   for (i=2; i < (arch_config.max_vl - remainder); i+=3) {   
         mask_red[i] = 1;
   }


   if(remainder == 0){} else if(remainder == 1){}
   else if(remainder == 2){
         mask_blue[arch_config.max_vl-remainder] = 1;
         mask_green[arch_config.max_vl-remainder+1] = 1;
         // mask_red[i+2] = 1;
   }
   kernel_config_obj->source_vecs[num_pix_vec] = malloc(arch_config.max_vl * sizeof(char));
   kernel_config_obj->source_vecs[num_pix_vec+1] = malloc(arch_config.max_vl * sizeof(char));
   kernel_config_obj->source_vecs[num_pix_vec+2] = malloc(arch_config.max_vl * sizeof(char));

   _DRAM_AP_vld_char(kernel_config_obj->source_vecs[num_pix_vec], mask_blue, arch_config.max_vl, 8);
   _DRAM_AP_vld_char(kernel_config_obj->source_vecs[num_pix_vec+1], mask_green, arch_config.max_vl, 8);
   _DRAM_AP_vld_char(kernel_config_obj->source_vecs[num_pix_vec+2], mask_red, arch_config.max_vl, 8);


   /* 3. allocate result histogram vectors */
   kernel_config_obj->results_vecs = malloc(3 * sizeof(int *));
   kernel_config_obj->results_vecs[0] = malloc(256 * sizeof(int)); // result for blue
   kernel_config_obj->results_vecs[1] = malloc(256 * sizeof(int)); // result for green
   kernel_config_obj->results_vecs[2] = malloc(256 * sizeof(int)); // result for red

   int blue_res[256];
   int green_res[256];
   int red_res[256];

   for (int i=0; i< 256; i++){
      blue_res[i] = 0;
      green_res[i] = 0;
      red_res[i] = 0;
   }

   _DRAM_AP_vld_int(kernel_config_obj->results_vecs[0], blue_res, 256, sizeof(int));
   _DRAM_AP_vld_int(kernel_config_obj->results_vecs[1], green_res, 256, sizeof(int));
   _DRAM_AP_vld_int(kernel_config_obj->results_vecs[2], red_res, 256, sizeof(int));

   


   /* 4. simulating execution of DRAM-AP kernel */
   // we could try both masked search or masked aggregate

   int blue_hits = 0;
   int green_hits = 0;
   int red_hits = 0;

   for(int i=0;i<num_pix_vec;i++){

      int vl = MIN(arch_config.max_vl, total_num_pix-arch_config.max_vl*i);

      // for(char pix=0;pix<1;pix++){} // search every pix pattern
         
      char pix = 0;
      char match_res[vl];
      char pix_vec[vl];

      _DRAM_AP_vbrdcst(pix_vec, pix, vl, 8); // search pixel pattern 00000000
      _DRAM_AP_vsch(match_res, kernel_config_obj->source_vecs[i], pix_vec, vl, 8);
         
      int tmp_blue_hits = 0;
      int tmp_green_hits = 0;
      int tmp_red_hits = 0;

      _DRAM_AP_pcl(&tmp_blue_hits, match_res, kernel_config_obj->source_vecs[num_pix_vec], vl, 8);   // masked bit count    
      _DRAM_AP_pcl(&tmp_green_hits, match_res, kernel_config_obj->source_vecs[num_pix_vec+1], vl, 8);
      _DRAM_AP_pcl(&tmp_red_hits, match_res, kernel_config_obj->source_vecs[num_pix_vec+2], vl, 8);

      blue_hits += tmp_blue_hits;
      green_hits += tmp_green_hits;
      red_hits += tmp_red_hits;

   }
   // int red = 0;
   // for(int i=0;i<8192;i++){
   //    if(mask_red[i] == 1){
   //       red += 1;
   //       printf("[%d]: %d",i,mask_red[i]);
   //    }
   // }
   // printf("red: %d\n", red);


   kernel_config_obj->results_vecs[0][0] = blue_hits;
   kernel_config_obj->results_vecs[1][0] = green_hits;
   kernel_config_obj->results_vecs[2][0] = red_hits;


   printf("----------\n\n");
   printf("blue pixel 00000000 hits: %d\n", kernel_config_obj->results_vecs[0][0]);
   printf("green pixel 00000000 hits: %d\n", kernel_config_obj->results_vecs[1][0]);
   printf("red pixel 00000000 hits: %d\n", kernel_config_obj->results_vecs[2][0]);

}





int main(int argc, char *argv[]) {

   printf("\n Execution in Main() \n");
      
   int i;
   int fd;
   char *fdata;
   struct stat finfo;
   char * fname;
   int red[256];
   int green[256];
   int blue[256];


   // Make sure a filename is specified
   if (argv[1] == NULL) {
      printf("USAGE: %s <bitmap filename>\n", argv[0]);
      exit(1);
   }
   
   fname = argv[1];
   
   // Read in the file
   CHECK_ERROR((fd = open(fname, O_RDONLY)) < 0);
   // Get the file info (for file length)
   CHECK_ERROR(fstat(fd, &finfo) < 0);
   // Memory map the file
   CHECK_ERROR((fdata = mmap(0, finfo.st_size + 1, 
      PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) == NULL);
   
   if ((fdata[0] != 'B') || (fdata[1] != 'M')) {
      printf("File is not a valid bitmap file. Exiting\n");
      exit(1);
   }
   
   test_endianess();    // will set the variable "swap"
   
   unsigned short *bitsperpixel = (unsigned short *)(&(fdata[BITS_PER_PIXEL_POS]));
   if (swap) {
      swap_bytes((char *)(bitsperpixel), sizeof(*bitsperpixel));
   }
   if (*bitsperpixel != 24) {    // ensure its 3 bytes per pixel
      printf("Error: Invalid bitmap format - ");
      printf("This application only accepts 24-bit pictures. Exiting\n");
      exit(1);
   }
   
   unsigned short *data_pos = (unsigned short *)(&(fdata[IMG_DATA_OFFSET_POS]));
   if (swap) {
      swap_bytes((char *)(data_pos), sizeof(*data_pos));
   }
   
   int imgdata_bytes = (int)finfo.st_size - (int)(*(data_pos));

   printf("-- DEBUG -- finfo.st_size: %ld\n", finfo.st_size);
   printf("-- DEBUG -- data_pos: %d\n", *data_pos);

   printf("This file has %d bytes of image data, %d pixels\n", imgdata_bytes,
                                                            imgdata_bytes / 3);
                                                            
   printf("Starting sequential histogram\n");                                                     
   memset(&(red[0]), 0, sizeof(int) * 256);
   memset(&(green[0]), 0, sizeof(int) * 256);
   memset(&(blue[0]), 0, sizeof(int) * 256);
   
   

   /* start CPU BASELINE start */
   for (i=*data_pos; i < finfo.st_size; i+=3) {      
      unsigned char *val = (unsigned char *)&(fdata[i]);
      blue[*val]++;
      //printf("val: %d\n",*val);
      val = (unsigned char *)&(fdata[i+1]);
      green[*val]++;
      val = (unsigned char *)&(fdata[i+2]);
      red[*val]++;   
   }
   /* end CPU BASELINE end */

   // make the input struct
   input_struct_t file_data;
   file_data.img_data = fdata;
   file_data.file_stats = finfo;
   file_data.imgdata_bytes = imgdata_bytes;
   file_data.data_pos = *data_pos;

   // instantiate an kernel_config
   dram_ap_kernel_config kernel_config;
   __DRAM_AP_Kernel_Launch__(&file_data, &kernel_config);





   // clean up space
   dram_ap_arch_config arch_config;
   arch_config = _make_DRAM_AP_arch_config();
   int num_pix_vec = (int)ceil((double)file_data.imgdata_bytes / arch_config.max_vl); // # of src vectors

   // free source vecs
   for(int i=0; i<num_pix_vec+3; i++){
      free(kernel_config.source_vecs[i]);
   }
   // free res vecs
   for(int j=0; i<3; j++){
      free(kernel_config.results_vecs[j]);
   }

   free(kernel_config.source_vecs);
   free(kernel_config.results_vecs);
   


   printf("\n\nBlue\n");
   printf("----------\n\n");
   for (i = 0; i < 256; i++) {
      if(blue[i] != 0)
         printf("%d - %d\n", i, blue[i]);        
   }
   
   printf("\n\nGreen\n");
   printf("----------\n\n");
   for (i = 0; i < 256; i++) {
      if(green[i] != 0)
         printf("%d - %d\n", i, green[i]);        
   }
   
   printf("\n\nRed\n");
   printf("----------\n\n");
   for (i = 0; i < 256; i++) {
      if(red[i] != 0)
         printf("%d - %d\n", i, red[i]);        
   }

   CHECK_ERROR(munmap(fdata, finfo.st_size + 1) < 0);
   CHECK_ERROR(close(fd) < 0);

   return 0;
}
