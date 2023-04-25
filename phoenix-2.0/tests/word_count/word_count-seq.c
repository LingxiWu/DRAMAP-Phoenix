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
#include <fcntl.h>
#include <ctype.h>
#include <pthread.h>

#include "map_reduce.h"
#include "stddefines.h"
#include "sort-pthread.h"

#define DEFAULT_DISP_NUM 10
#define START_ARRAY_SIZE 2000

typedef struct {
	char* word;
	int count;
} wc_count_t;

typedef struct {
   long flen;
   char *fdata;
} wc_data_t;

enum {
   IN_WORD,
   NOT_IN_WORD
};

wc_count_t* words;
int use_len;
int length;

void wordcount_getword(void *args_in);
void wordcount_addword(char* word, int len) ;

/** wordcount_cmp()
 *  Comparison function to compare 2 words
 */
int wordcount_cmp(const void *v1, const void *v2)
{
   wc_count_t* w1 = (wc_count_t*)v1;
   wc_count_t* w2 = (wc_count_t*)v2;

   int i1 = w1->count;
   int i2 = w2->count;

   if (i1 < i2) return 1;
   else if (i1 > i2) return -1;
   else return 0;
}

/** wordcount_splitter()
 *  Memory map the file and divide file on a word border i.e. a space.
 */
void wordcount_splitter(void *data_in)
{
	int i;
   wc_data_t * data = (wc_data_t *)data_in; 
   words = (wc_count_t*)calloc(START_ARRAY_SIZE,sizeof(wc_count_t));
   length = START_ARRAY_SIZE;
   use_len = 0;

   for(i=0;i<START_ARRAY_SIZE;i++)
	   words[i].count = 0;

   map_args_t* out = (map_args_t*)malloc(sizeof(map_args_t));
   out->data = data->fdata;
   out->length = data->flen;

   wordcount_getword(out);
   free(out);
}

/** wordcount_map()
 * Go through the file and update the count for each unique word
 */
void wordcount_getword(void *args_in) 
{
   map_args_t* args = (map_args_t*)args_in;

   char *curr_start, curr_ltr;
   int state = NOT_IN_WORD;
   int i;
   assert(args);

   char *data = (char *)(args->data);
   curr_start = data;
   assert(data);

   // printf("args_len is %ld\n", args->length); // file size in bytes. each char is one byte

   for (i = 0; i < args->length; i++) // scan each char one-by-one
   {
      curr_ltr = toupper(data[i]); // take the current char
      switch (state)
      {
      case IN_WORD: 
         data[i] = curr_ltr;
         if ((curr_ltr < 'A' || curr_ltr > 'Z') && curr_ltr != '\'')
         {  // we have obtained a whole word
            data[i] = 0;
			   // printf("\nthe word is %s\n",curr_start);
            // printf("the curr_ltr is %c\n",curr_ltr); 
			   wordcount_addword(curr_start, &data[i] - curr_start + 1);
            state = NOT_IN_WORD;
         }
      break;

      default:
      case NOT_IN_WORD: 
         if (curr_ltr >= 'A' && curr_ltr <= 'Z')
         {
            curr_start = &data[i];
            data[i] = curr_ltr;
            state = IN_WORD;
         }
         break;
      }
   }

   // Add the last word
   if (state == IN_WORD)
   {
			data[args->length] = 0;
			//printf("\nthe word is %s\n\n",curr_start);
			wordcount_addword(curr_start, &data[i] - curr_start + 1);
   }
}

/** dobsearch()
 *  Search for a specific word in the array
 */
int dobsearch(char* word)
{
   int cmp, high = use_len, low = -1, next;

   // Binary search the array to find the key
   while (high - low > 1)
   {
       next = (high + low) / 2;   
       cmp = strcmp(word, words[next].word);
       if (cmp == 0)
          return next;
       else if (cmp < 0)
           high = next;
       else
           low = next;
   }

   return high;
}

/** wordcount_addword()
 * Add a new word to the array in the correct sorted postion
 */
void wordcount_addword(char* word, int len) 
{
   // printf("word: %s\n",word);
	int pos = dobsearch(word);
   // printf("use_len: %d\n", use_len);
   if (pos >= use_len)
   {
      // at end
      words[use_len].word = word;
	   words[use_len].count = 1;
	   use_len++;
	}
   else if (pos < 0)
   {
      // at front
      memmove(&words[1], words, use_len*sizeof(wc_count_t));
      words[0].word = word;
	   words[0].count = 1;
	   use_len++;
   }

   else if (strcmp(word, words[pos].word) == 0) // hit, update counter
   {
      // match
      words[pos].count++;
	}

   else
   {
      // insert at pos
      memmove(&words[pos+1], &words[pos], (use_len-pos)*sizeof(wc_count_t));
      words[pos].word = word;
	   words[pos].count = 1;
	   use_len++;
   }

	if(use_len == length) // expand the array by doubling it
	{ 
		length *= 2;
	   words = (wc_count_t*)realloc(words,length*sizeof(wc_count_t));
	}
}

int main(int argc, char *argv[]) {
   
   struct timeval tv1;
   gettimeofday(&tv1, NULL);

   int i;
   int fd;
   char * fdata;
   int disp_num;
   struct stat finfo;
   char * fname, * disp_num_str;

   // Make sure a filename is specified
   if (argv[1] == NULL)
   {
      printf("USAGE: %s <filename> [Top # of results to display]\n", argv[0]);
      exit(1);
   }
   
   fname = argv[1];
   disp_num_str = argv[2]; // display top X results

   printf("Wordcount: Running...\n");
   
   // Read in the file
   CHECK_ERROR((fd = open(fname, O_RDONLY)) < 0);
   // Get the file info (for file length)
   CHECK_ERROR(fstat(fd, &finfo) < 0);
   // Memory map the file
   CHECK_ERROR((fdata = mmap(0, finfo.st_size + 1, 
      PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) == NULL);
   
   // Get the number of results to display
   CHECK_ERROR((disp_num = (disp_num_str == NULL) ? 
      DEFAULT_DISP_NUM : atoi(disp_num_str)) <= 0);
   
   // Setup splitter args
   wc_data_t wc_data;
   wc_data.flen = finfo.st_size;
   wc_data.fdata = fdata;

   printf("Wordcount Serial: Running\n");
   
   /// kernel start
   struct timeval tv3;
   gettimeofday(&tv3, NULL);

   wordcount_splitter(&wc_data); // add words to the array, at the same time update the counter 
   
   struct timeval tv4;
   gettimeofday(&tv4, NULL);
   long kernel_time_elapsed = 0;
   kernel_time_elapsed += (tv4.tv_sec - tv3.tv_sec) * 1000000 + tv4.tv_usec - tv3.tv_usec;
   printf("kernel_time_elapsed: %ld\n",kernel_time_elapsed);

   qsort(words, use_len, sizeof(wc_count_t), wordcount_cmp);


   // kernel end
   
	dprintf("Use len is %d\n", use_len); // # of unique words.
   
   // print and also calculate total num of words
   long long int total_words = 0;
   // for(i=0; i< disp_num && i < use_len ; i++)
   for(i=0; i < use_len ; i++)
   {
		wc_count_t* temp = &(words[i]);
      total_words += temp->count;
   }
   printf("total_words: %lld\n",total_words);
   
   free(words);


   CHECK_ERROR(munmap(fdata, finfo.st_size + 1) < 0);
   CHECK_ERROR(close(fd) < 0);

   // total time elapsed.
   struct timeval tv2;
   gettimeofday(&tv2, NULL);
   // printf("sec: %ld,usec: %ld \n",tv2.tv_sec, tv2.tv_usec);
   long total_time_elapsed = 0;
   total_time_elapsed += (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
   printf("total_time_elapsed: %ld\n",total_time_elapsed);

   double percentage;
   percentage = (double)kernel_time_elapsed / (double)total_time_elapsed;
   printf("kernel percentage: %.2f\n",percentage);

   return 0;
}
