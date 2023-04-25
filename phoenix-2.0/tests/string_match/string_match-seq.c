/* Copyright (c) 2007-2009, Stanford University
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Stanford University nor the names of its 
*       contributors may be used to endorse or promote products derived from 
*       this software without specific prior written permission.
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
#include <sys/time.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <inttypes.h>

#include "stddefines.h"

#define SALT_SIZE 2
#define MAX_REC_LEN 1024
#define OFFSET 5

typedef struct {
  int keys_file_len;
  int encrypted_file_len;
  long bytes_comp;
  char * keys_file;
  char * encrypt_file;
  char* salt;
} str_data_t;

void compute_hashes(char* word, char* final_word);

/** getnextline()
 *  Function to get the next word
 */
int getnextline(char* output, int max_len, char* file)
{
	int i=0;
	while(i<max_len-1)
	{
		if( file[i] == '\0')
		{
			if(i==0)
				return -1;
			else
				return i;
		}
		if( file[i] == '\r')
			return (i+2);

		if( file[i] == '\n' )
			return (i+1);

		output[i] = file[i];
		i++;
	}
	file+=i;
	return i;
}

/** compute_hashes()
 *  Simple Cipher to generate a hash of the word 
 */
void compute_hashes(char* word, char* final_word)
{
	int i;

	for(i=0;i<strlen(word);i++) {
		final_word[i] = word[i]+OFFSET;
	}
	final_word[i] = '\0';
}

/** string_match()
 *  function that goes through file looking for matches to the given hashes 
 */
void string_match(str_data_t *data_in)
{
	assert(data_in);

/*
	 char *key1 = "Helloworld";
	 char *key2 = "howareyou";
	 char *key3 = "ferrari";
	 char *key4 = "whotheman";

	// char *key1 = "gastromyxorrhea";
	// char *key2 = "gastronephritis";
	// char *key3 = "gastronome";
	// char *key4 = "gastronomer";


	 char *key1_final = malloc(strlen(key1) + 1);
	 char *key2_final = malloc(strlen(key2) + 1);
	 char *key3_final = malloc(strlen(key3) + 1);
	 char *key4_final = malloc(strlen(key4) + 1);

 // printf("%d %d\n", strlen(key1), strlen(key2));
	compute_hashes(key1, key1_final);
	compute_hashes(key2, key2_final);
	compute_hashes(key3, key3_final);
	compute_hashes(key4, key4_final);
*/


	int key_len;
	int encrypted_len;
	char * key_file;
	char * encrypt_file;
	key_file = data_in->keys_file;
	encrypt_file = data_in->encrypt_file;
	char * cur_word = malloc(MAX_REC_LEN);
	char * cur_word_final = malloc(MAX_REC_LEN);
	char * cur_encrypt = malloc(MAX_REC_LEN);
	bzero(cur_word, MAX_REC_LEN);
	bzero(cur_word_final, MAX_REC_LEN);
	bzero(cur_encrypt, MAX_REC_LEN);

// long long hit_counter = 0;
// long long key_counter = 0;
// long long encrypt_counter = 0;

    while( (key_len = getnextline(cur_word, MAX_REC_LEN, key_file))>=0) // iterate through keys in key_file (from commandline)
    {
    	// printf("cur_word: %s\n", cur_word);
      // key_counter ++;

		  compute_hashes(cur_word, cur_word_final); // encrypt current key word

		  // encrypt_counter = 0; // reset counter
		  while( (encrypted_len = getnextline(cur_encrypt, MAX_REC_LEN, encrypt_file))>=0)
		  {
			 // encrypt_counter ++;
			 // printf("cur_word: %s, cur_encrypt: %s, cur_word_final: %s\n", cur_word, cur_encrypt,cur_word_final);
			 // printf("cur_encrypt: %s\n", cur_encrypt);
			 if(!strcmp(cur_encrypt, cur_word_final)){
				  hit_counter ++;
				  // printf("counter %d, FOUND: WORD IS %s\n", counter, cur_word);
			 }

			 encrypt_file = encrypt_file + encrypted_len;
			 bzero(cur_encrypt, MAX_REC_LEN);

		  }

		  encrypt_file = data_in->encrypt_file; // reset file pointer
  		/*
  	  	if(!strcmp(key1_final, cur_word_final))
  		   dprintf("FOUND: WORD IS %s\n", cur_word);

  	   	if(!strcmp(key2_final, cur_word_final))
  		   dprintf("FOUND: WORD IS %s\n", cur_word);

  	   	if(!strcmp(key3_final, cur_word_final))
  		   dprintf("FOUND: WORD IS %s\n", cur_word);

  	   	if(!strcmp(key4_final, cur_word_final))
  		   dprintf("FOUND: WORD IS %s\n", cur_word);
  		*/
  	  key_file = key_file + key_len;
		  bzero(cur_word, MAX_REC_LEN);
		  bzero(cur_word_final, MAX_REC_LEN);
    }
    free(cur_word);
    free(cur_word_final);
    free(cur_encrypt);
     /*
     free(key1_final);
     free(key2_final);
     free(key3_final);
     free(key4_final);
     */

   printf("#_key: %lld, #_encrypt: %lld, hits: %lld\n", key_counter, encrypt_counter, hit_counter);
}

int main(int argc, char *argv[]) 
{   

	struct timeval tv1;
   	gettimeofday(&tv1, NULL);

   int fd_keys;
   int fd_encrypt;
   char *fdata_keys;
   char *fdata_encrypt;
   struct stat finfo_keys;
   struct stat finfo_encrypt;
   char *fname_keys;
   char *fname_encrypt;

	 /* Option to provide the encrypted words in a file as opposed to source code */
   // fname_encrypt = "encrypted_small.txt";
   // fname_encrypt = "encrypted_large.txt";
   fname_encrypt = "encrypted_medium.txt";
   
   if (argv[1] == NULL)
   {
      printf("USAGE: %s <keys filename>\n", argv[0]);
      exit(1);
   }
   fname_keys = argv[1];

   // struct timeval starttime,endtime;
   srand( (unsigned)time( NULL ) );

   printf("String Match: Running...\n");

   // Read in the file
   CHECK_ERROR((fd_encrypt = open(fname_encrypt,O_RDONLY)) < 0);
   // Get the file info (for file length)
   CHECK_ERROR(fstat(fd_encrypt, &finfo_encrypt) < 0);
   // Memory map the file
   CHECK_ERROR((fdata_encrypt= mmap(0, finfo_encrypt.st_size + 1,
      PROT_READ | PROT_WRITE, MAP_PRIVATE, fd_encrypt, 0)) == NULL);


   // Read in the file
   CHECK_ERROR((fd_keys = open(fname_keys,O_RDONLY)) < 0);
   // Get the file info (for file length)
   CHECK_ERROR(fstat(fd_keys, &finfo_keys) < 0);
   // Memory map the file
   CHECK_ERROR((fdata_keys= mmap(0, finfo_keys.st_size + 1,
      PROT_READ | PROT_WRITE, MAP_PRIVATE, fd_keys, 0)) == NULL);

   // Setup splitter args

	dprintf("Encrypted Size is %ld\n",finfo_encrypt.st_size);
	dprintf("Keys Size is %" PRId64 "\n",finfo_keys.st_size);

   str_data_t str_data;

   str_data.keys_file_len = finfo_keys.st_size;
   str_data.encrypted_file_len = finfo_encrypt.st_size;
   str_data.bytes_comp = 0;
   str_data.keys_file  = ((char *)fdata_keys);
   str_data.encrypt_file = ((char *)fdata_encrypt);
   str_data.salt = malloc(SALT_SIZE);

   //printf("String Match: Calling Serial String Match\n");
	 // gettimeofday(&starttime,0);
	 /// kernel start
   struct timeval tv3;
   gettimeofday(&tv3, NULL);
   
   string_match(&str_data); // kernel
   
   struct timeval tv4;
   gettimeofday(&tv4, NULL);

   long kernel_time_elapsed = 0;
   kernel_time_elapsed += (tv4.tv_sec - tv3.tv_sec) * 1000000 + tv4.tv_usec - tv3.tv_usec;
   printf("kernel_time_elapsed: %ld\n",kernel_time_elapsed);

	 // gettimeofday(&endtime,0);
  //  printf("String Match: Completed %ld\n",(endtime.tv_sec - starttime.tv_sec));

   free(str_data.salt);

   /*CHECK_ERROR(munmap(fdata_encrypt, finfo_encrypt.st_size + 1) < 0);
   CHECK_ERROR(close(fd_encrypt) < 0);*/

   CHECK_ERROR(munmap(fdata_keys, finfo_keys.st_size + 1) < 0);
   CHECK_ERROR(close(fd_keys) < 0);
   CHECK_ERROR(close(fd_encrypt) < 0);
   
   struct timeval tv2;
   gettimeofday(&tv2, NULL);
   long total_time_elapsed = 0;
   total_time_elapsed += (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
   printf("total_time_elapsed: %ld\n",total_time_elapsed);

   double percentage;
   percentage = (double)kernel_time_elapsed / (double)total_time_elapsed;
   printf("kernel percentage: %.2f\n",percentage);


   return 0;
}
