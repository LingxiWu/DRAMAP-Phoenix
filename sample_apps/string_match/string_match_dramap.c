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


//#include <readline/readline.h>
//#include <readline/history.h>

#include "stddefines.h"

#include "dram_ap_lib.h"


/** string_match()
 *  function that goes through file looking for matches to the given hashes 
 */
void string_match(str_data_t *data_in)
{
	assert(data_in);

	 char *key1 = "Helloworld";
	 char *key2 = "howareyou";
	 char *key3 = "ferrari";
	 char *key4 = "whotheman";


	int key_len;
	char * key_file;
	key_file = data_in->keys_file;
	char * cur_word = malloc(MAX_REC_LEN);
	bzero(cur_word, MAX_REC_LEN);

    while( (key_len = getnextline(cur_word, MAX_REC_LEN, key_file))>=0)
    {

	   if(!strcmp(key1, cur_word)){
		   printf("FOUND: WORD IS %s\n", cur_word);
	   }

		if(!strcmp(key2, cur_word)){
		   printf("FOUND: WORD IS %s\n", cur_word);
		}

		if(!strcmp(key3, cur_word)){
		   printf("FOUND: WORD IS %s\n", cur_word);
		}

		if(!strcmp(key4, cur_word)){
		   printf("FOUND: WORD IS %s\n", cur_word);
		}

		
	   key_file = key_file + key_len;
		bzero(cur_word, MAX_REC_LEN);
   }
   free(cur_word);
}


// dram_ap_fld(str_data_t* fd, char* src_v, unsigned long long vl, MAX_REC_LEN, 0) 

int main(int argc, char *argv[]) 
{   
   int fd;
   char *fdata;
   struct stat finfo;
   char *fname;

   
   if (argv[1] == NULL)
   {
      printf("USAGE: %s <keys filename>\n", argv[0]);
      exit(1);
   }
   fname = argv[1];

   // Read in the file
   CHECK_ERROR((fd = open(fname,O_RDONLY)) < 0);
   // Get the file info (for file length)
   CHECK_ERROR(fstat(fd, &finfo) < 0);
   // Memory map the file
   CHECK_ERROR((fdata= mmap(0, finfo.st_size + 1,
      PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) == NULL);

	printf("file Size is %ld\n",finfo.st_size);


   str_data_t str_data;
   str_data.keys_file_len = finfo.st_size;
   str_data.keys_file  = ((char *)fdata);

   /* START DRAMAP BASELINE */

   printf("\nSTART DRAMAP: \n");
   // str_data = dramap_fopen(fname) --> returns a fd. str_data emulates the dramap fd

   unsigned long long vl = getVlHelper(fname); // NOTE, vl should be dynamically determined by the runtime/compiler. It should not be set by the programmer. this vl is explicitly set for functional simulation only
   

   char* query1 = "Helloworld";
   char* query2 = "howareyou";
   // char* query3 = "ferrari";
   // char* query4 = "whotheman";

   int num_match1 = 0;
   int num_match2 = 0;

   int group_1 = 0;
   char** key_v;
   char** query1_v;
   char** query2_v;
   int* res1_v;
   int* res2_v;

   // NOTE, vl should be omitted in real implementation 
   key_v = dram_ap_valloc_char(group_1, vl, MAX_CHAR_LEN); 

   query1_v = dram_ap_valloc_char(group_1, vl, strlen(query1)); 
   query2_v = dram_ap_valloc_char(group_1, vl, strlen(query2)); 
   dram_ap_valloc(&res1_v, group_1, vl, 1); 
   dram_ap_valloc(&res2_v, group_1, vl, 1); 


   dram_ap_fld(&str_data, key_v, vl, MAX_CHAR_LEN); 
   dram_ap_brdcst(query1, query1_v, vl, strlen(query1)); 
   dram_ap_brdcst(query2, query2_v, vl, strlen(query2)); 

   dram_ap_match(res1_v, key_v, query1_v, vl, strlen(query1)); 
   dram_ap_match(res2_v, key_v, query2_v, vl, strlen(query2)); 
   dram_ap_pcl(&num_match1, res1_v, vl, 1); 
   dram_ap_pcl(&num_match2, res2_v, vl, 1); 
   

   if (num_match1) {
   	printf("FOUND Query Word: %s\n", query1);
   } else {
   	printf("DID NOT Find Query Word: %s\n", query1);
   }
   if (num_match2) {
   	printf("FOUND Query Word: %s\n", query2);
   } else {
   	printf("DID NOT Find Query Word: %s\n", query2);
   }
   



   free(key_v);
   free(query1_v);
   free(query2_v);
   free(res1_v);
   free(res2_v);
   
   printf("END DRAMAP: \n");



   



   /* START CPU BASELINE */
   printf("\nCPU Baseline String Match: Running...\n");
   string_match(&str_data);
   printf("CPU baseline String Match: Completed \n");
   /* END CPU BASELINE */


   CHECK_ERROR(munmap(fdata, finfo.st_size + 1) < 0);
   CHECK_ERROR(close(fd) < 0);

   return 0;
}
