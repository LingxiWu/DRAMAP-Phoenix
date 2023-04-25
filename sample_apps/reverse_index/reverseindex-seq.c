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

#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include "stddefines.h"


#define UNIT_SIZE (1024 * 5)      // approx 5kB per html file

enum {
   START,
   IN_TAG,
   IN_ATAG,
   FOUND_HREF,
   START_LINK
};

typedef struct link_elem {
   char *filename;
   struct link_elem *next;   
} link_elem_t;


typedef struct link_head {
   char *link;
   struct link_elem *elem;
   //struct link_head *next; 
} link_head_t;


typedef struct flist{
   char *data;
   char *name;
   int fd;
   off_t size;
   struct flist *next;
} filelist_t;

typedef struct {
   filelist_t *filelist;
   filelist_t *currfile;
   int num_files;
   int unit_size;
} ri_data_t;


link_head_t *links;
int use_len;
int length;

ri_data_t ri_data; // track list of files, # of files, and their sizes
int data_size;
int req_data;

#define START_ARRAY_SIZE 2000

/** cleanup()
 *  
 */
void cleanup() {
   filelist_t *curr_ptr = ri_data.filelist;
   filelist_t *prev_ptr;
   
   while(curr_ptr != NULL) {
      munmap(curr_ptr->data, curr_ptr->size + 1);
      close(curr_ptr->fd);
      free(curr_ptr->name);

      prev_ptr = curr_ptr;
      curr_ptr = curr_ptr->next;
      free(prev_ptr);   
   }
}

/** addtolist()
 *  Create and update the list from links to files
 */
void addtolist(filelist_t *file) {
   
   file->next = NULL;
   
   if (ri_data.filelist == NULL) {
      ri_data.filelist = file;
      ri_data.currfile = file;
   }
   else {
      ri_data.currfile->next = file;
      ri_data.currfile = file;
   }
   
   ri_data.num_files++;
}

/** recursedirs()
 *  Recursively go througn the directories with the HTML files and memory
 *	 map them
 */
void recursedirs(char *name) {
   
   DIR *dp;
   struct dirent *ep;
   struct stat finfo;
   char *path;
   
   if (((req_data > 0) && (data_size < req_data)) || (req_data <= 0)) {
      dp = opendir (name);
      if (dp != NULL)      // it's a directory
      {
         while ((ep = readdir (dp)) != NULL) {
            if (strcmp(ep->d_name, ".") == 0) continue;
            if (strcmp(ep->d_name, "..") == 0) continue;
            
            CHECK_ERROR((path = (char *)malloc(strlen(ep->d_name) + strlen(name) + 2)) == NULL);
            sprintf(path, "%s/%s",name,ep->d_name);
            
            recursedirs(path);
            free(path);
         }
         (void) closedir (dp);
      }
      else  // it's not a directory, mmap file and add it to the list
      {
         filelist_t *file;
         CHECK_ERROR((file = (filelist_t *)malloc(sizeof(filelist_t))) == NULL);
         file->fd = open(name, O_RDONLY);
         if (file->fd < 0) {
            free(file);   
         }
         else {
            int ret = fstat(file->fd, &finfo);
            if (ret < 0) {
               free(file);
            }
            else {
               file->size = finfo.st_size;
               char *f_data = mmap(0, file->size + 1, PROT_READ | PROT_WRITE, 
                                                       MAP_PRIVATE, file->fd, 0);
               if (f_data == NULL) {
                  free(file);
               }
               else {
                  CHECK_ERROR((file->name = (char *)malloc(strlen(name) + 1)) == NULL);
                  data_size += finfo.st_size;
                  strcpy(file->name, name);
                  CHECK_ERROR((file->data = (char *)malloc(finfo.st_size + 1)) == NULL);
                  memcpy(file->data, f_data, finfo.st_size);
                  addtolist(file);
                  munmap(f_data, file->size + 1);
                  close(file->fd);
               }
            }
         }
      }
   }
   return;
}

/** dobsearch()
 *  Looking for a specific link
 */
int dobsearch(char* link)
{
   int cmp, high = use_len, low = -1, next;

   // Binary search the array to find the key
   while (high - low > 1)
   {
       next = (high + low) / 2;   
       cmp = strcmp(link, links[next].link);
       if (cmp == 0)
          return next;
       else if (cmp < 0)
           high = next;
       else
           low = next;
   }

   return high;
}

/** insert_sorted()
 *  Look at the current links and insert into the lists in a sorted manner
 */
void insert_sorted(char * link, char *filename) {
   
   dprintf("Inserting %s.... ", link);
   
   // struct timeval tv_bsearch_start;
   // gettimeofday(&tv_bsearch_start, NULL);

   int pos = dobsearch(link);

   // struct timeval tv_bsearch_end;
   // gettimeofday(&tv_bsearch_end, NULL);

   // long tv_bsearch_elapsed = 0;
   // tv_bsearch_elapsed += (tv_bsearch_end.tv_sec - tv_bsearch_start.tv_sec) * 1000000 + tv_bsearch_end.tv_usec - tv_bsearch_start.tv_usec;

   // FILE *fp;
   // fp = fopen("bsearch_interval.txt", "a");
   // fprintf(fp, "%ld\n", tv_bsearch_elapsed);
   // fclose(fp);
   

   if (pos >= use_len)
   {
      // at end
      
      // struct timeval tv_insert_start;
      // gettimeofday(&tv_insert_start, NULL);

      dprintf("Inserting at end\n");
      links[use_len].link = link; // insert

      // struct timeval tv_insert_end;
      // gettimeofday(&tv_insert_end, NULL);

      // long tv_insert_elapsed = 0;
      // tv_insert_elapsed += (tv_insert_end.tv_sec - tv_insert_start.tv_sec) * 1000000 + tv_insert_end.tv_usec - tv_insert_start.tv_usec;

      // FILE *fp;
      // fp = fopen("insert_interval.txt", "a");
      // fprintf(fp, "%ld\n", tv_insert_elapsed);
      // fclose(fp);

      // struct timeval tv_file_start;
      // gettimeofday(&tv_file_start, NULL);

	   link_elem_t *new_elem;
      CHECK_ERROR((new_elem = (link_elem_t *)malloc(sizeof(link_elem_t))) == NULL);
      new_elem->next = NULL;
      new_elem->filename = filename; 
      links[use_len].elem = new_elem; // links are equivalent to words in wordCount

  	   use_len++;

      // struct timeval tv_file_end;
      // gettimeofday(&tv_file_end, NULL);

      // long tv_file_elapsed = 0;
      // tv_file_elapsed += (tv_file_end.tv_sec - tv_file_start.tv_sec) * 1000000 + tv_file_end.tv_usec - tv_file_start.tv_usec;

      // FILE *fp;
      // fp = fopen("file_interval.txt", "a");
      // fprintf(fp, "%ld\n", tv_file_elapsed);
      // fclose(fp);

	}
   else if (pos < 0)
   {
      // at front

      // struct timeval tv_insert_start;
      // gettimeofday(&tv_insert_start, NULL);
      
      dprintf("Inserting at front\n");
      memmove(&links[1], links, use_len*sizeof(link_head_t));
      links[0].link = link; // insert

      // struct timeval tv_insert_end;
      // gettimeofday(&tv_insert_end, NULL);

      // long tv_insert_elapsed = 0;
      // tv_insert_elapsed += (tv_insert_end.tv_sec - tv_insert_start.tv_sec) * 1000000 + tv_insert_end.tv_usec - tv_insert_start.tv_usec;

      // FILE *fp;
      // fp = fopen("insert_interval.txt", "a");
      // fprintf(fp, "%ld\n", tv_insert_elapsed);
      // fclose(fp);

      // struct timeval tv_file_start;
      // gettimeofday(&tv_file_start, NULL);
      
      link_elem_t *new_elem;
      CHECK_ERROR((new_elem = (link_elem_t *)malloc(sizeof(link_elem_t))) == NULL);
      new_elem->next = NULL;
      new_elem->filename = filename; 
      links[0].elem = new_elem;
      
	   use_len++;

      // struct timeval tv_file_end;
      // gettimeofday(&tv_file_end, NULL);

      // long tv_file_elapsed = 0;
      // tv_file_elapsed += (tv_file_end.tv_sec - tv_file_start.tv_sec) * 1000000 + tv_file_end.tv_usec - tv_file_start.tv_usec;

      // FILE *fp;
      // fp = fopen("file_interval.txt", "a");
      // fprintf(fp, "%ld\n", tv_file_elapsed);
      // fclose(fp);

   }
   else if (strcmp(link, links[pos].link) == 0)
   {
      // match

      // struct timeval tv_file_start;
      // gettimeofday(&tv_file_start, NULL);      

      dprintf("link exists\n");
      assert(links[pos].elem);
      
      link_elem_t *new_elem;
      CHECK_ERROR((new_elem = (link_elem_t *)malloc(sizeof(link_elem_t))) == NULL);
      new_elem->filename = filename;
      new_elem->next = links[pos].elem;
      links[pos].elem = new_elem;

      // struct timeval tv_file_end;
      // gettimeofday(&tv_file_end, NULL);

      // long tv_file_elapsed = 0;
      // tv_file_elapsed += (tv_file_end.tv_sec - tv_file_start.tv_sec) * 1000000 + tv_file_end.tv_usec - tv_file_start.tv_usec;

      // FILE *fp;
      // fp = fopen("file_interval.txt", "a");
      // fprintf(fp, "%ld\n", tv_file_elapsed);
      // fclose(fp);

	}
   else
   {
      // insert at pos

      // struct timeval tv_insert_start;
      // gettimeofday(&tv_insert_start, NULL);

      dprintf("Inserting in middle\n");
      memmove(&links[pos+1], &links[pos], (use_len-pos)*sizeof(link_head_t));
      links[pos].link = link; // insert

      // struct timeval tv_insert_end;
      // gettimeofday(&tv_insert_end, NULL);

      // long tv_insert_elapsed = 0;
      // tv_insert_elapsed += (tv_insert_end.tv_sec - tv_insert_start.tv_sec) * 1000000 + tv_insert_end.tv_usec - tv_insert_start.tv_usec;

      // FILE *fp;
      // fp = fopen("insert_interval.txt", "a");
      // fprintf(fp, "%ld\n", tv_insert_elapsed);
      // fclose(fp);
	   
      // struct timeval tv_file_start;
      // gettimeofday(&tv_file_start, NULL);  

	   link_elem_t *new_elem;
      CHECK_ERROR((new_elem = (link_elem_t *)malloc(sizeof(link_elem_t))) == NULL);
      new_elem->next = NULL;
      new_elem->filename = filename; 
      links[pos].elem = new_elem;
      
	   use_len++;

      // struct timeval tv_file_end;
      // gettimeofday(&tv_file_end, NULL);

      // long tv_file_elapsed = 0;
      // tv_file_elapsed += (tv_file_end.tv_sec - tv_file_start.tv_sec) * 1000000 + tv_file_end.tv_usec - tv_file_start.tv_usec;

      // FILE *fp;
      // fp = fopen("file_interval.txt", "a");
      // fprintf(fp, "%ld\n", tv_file_elapsed);
      // fclose(fp);      
   }


	if(use_len == length)
	{
      // struct timeval tv_expand_start;
      // gettimeofday(&tv_expand_start, NULL);

		length *= 2;
	   links = (link_head_t*)realloc(links,length*sizeof(link_head_t));
      
      // struct timeval tv_expand_end;
      // gettimeofday(&tv_expand_end, NULL);

      // long tv_expand_elapsed = 0;
      // tv_expand_elapsed += (tv_expand_end.tv_sec - tv_expand_start.tv_sec) * 1000000 + tv_expand_end.tv_usec - tv_expand_start.tv_usec;

      // FILE *fp;
      // fp = fopen("expand_interval.txt", "a");
      // fprintf(fp, "%ld\n", tv_expand_elapsed);
      // fclose(fp);
	}
   
   
   dprintf("Inserted\n");  
}

/** getlinks()
 *  Get the links in the files and update the relevant lists
 */
void getlinks() 
{
   int i;
   off_t j;
   char *link_end;
   int state = START;
   
   links = (link_head_t*)calloc(START_ARRAY_SIZE,sizeof(link_head_t));
   length = START_ARRAY_SIZE;
   use_len = 0;
   
   filelist_t *file = ri_data.filelist;
   
   // go through each file and look for links.
   for (i=0; i<ri_data.num_files; i++) 
   {
      assert(file);
      dprintf("Looking at file %s, size %d\n",file->name, file->size);
      
      for (j=0; j<file->size; j++) 
      {
         switch (state)
         {
            case START:
               if (file->data[j] == '<') state = IN_TAG;
               break;
               
            case IN_TAG:
               //dprintf("Found a tag\n");
               if (file->data[j] == 'a') state = IN_ATAG;
               else if (file->data[j] == ' ') state = IN_TAG;
               else state = START;               
               break;
         
            case IN_ATAG:
               //dprintf("Found <a\n");
               if (file->data[j] == 'h')
               {
                  if (strncmp(&file->data[j], "href", 4) == 0) 
                  {
                     state = FOUND_HREF;   
                     j += 3;
                  }
                  else state = START;
               }
               else if (file->data[j] == ' ') state = IN_ATAG;
               else state = START;
               break;
               
            case FOUND_HREF:
               //dprintf("Found href\n");
               if (file->data[j] == ' ') state = FOUND_HREF;
               else if (file->data[j] == '=') state = FOUND_HREF;
               else if (file->data[j] == '\"') state  = START_LINK;
               else state = START;
               break;
            
            case START_LINK:
               //dprintf("Found a link\n");
               link_end = NULL;
               link_end = strchr(&(file->data[j]), '\"');
               if (link_end != NULL)
               {
                  link_end[0] = 0;

                  // actual cpu kernel
                  // struct timeval tv_cpu_kernel_start;
                  // gettimeofday(&tv_cpu_kernel_start, NULL);

                  insert_sorted(&(file->data[j]), file->name);  // file->data[j] is the link. file contains a bunch of links  
                  printf("Found key %s in file %s\n", &(file->data[j]), file->name);    
                  
                  // struct timeval tv_cpu_kernel_end;
                  // gettimeofday(&tv_cpu_kernel_end, NULL);

                  // long tv_cpu_kernel_elapsed = 0;
                  // tv_cpu_kernel_elapsed += (tv_cpu_kernel_end.tv_sec - tv_cpu_kernel_start.tv_sec) * 1000000 + tv_cpu_kernel_end.tv_usec - tv_cpu_kernel_start.tv_usec;

                  // FILE *fp;
                  // fp = fopen("insert_sorted_interval.txt", "a");
                  // fprintf(fp, "%ld\n", tv_cpu_kernel_elapsed);
                  // fclose(fp);

                  
                  // FILE *fp;
                  // fp = fopen("links.txt", "a");
                  // fprintf(fp, "%s\n", &(file->data[j]));
                  // fclose(fp);

                  j += strlen(&(file->data[j]));
               }
               state = START;
               break;
         }
      }
            
      file = file->next;
   }
}


int main(int argc, char **argv)
{  

   struct timeval tv1;
   gettimeofday(&tv1, NULL);
    
   if (argc != 2) {
      fprintf(stderr, "Usage: %s <start directory>\n",argv[0]); 
      exit(-1);  
   }   
   CHECK_ERROR(argv[1] == NULL);
   
   ri_data.filelist = NULL;
   ri_data.currfile = NULL;
   ri_data.num_files = 0;
   ri_data.unit_size = UNIT_SIZE;
   
   char *req_data_str = getenv("RI_DATASIZE");
   if (req_data_str != NULL)
      req_data = atoi(req_data_str);
   else
      req_data = 0;
   
   printf("Reqd data = %d\n", req_data);
   
   // preprocessing timer start
   struct timeval tv5;
   gettimeofday(&tv5, NULL);

   recursedirs(argv[1]); // preprocessing the files

   struct timeval tv6;
   gettimeofday(&tv6, NULL);
   long preprocessing_time_elapsed = 0;
   preprocessing_time_elapsed += (tv6.tv_sec - tv5.tv_sec) * 1000000 + tv6.tv_usec - tv5.tv_usec;
   printf("preprocessing_time_elapsed: %ld\n",preprocessing_time_elapsed);  

   // preprocessing timer stop
   
   printf("Number of files added = %d, total size = %d\n", ri_data.num_files, data_size);
   printf("Files are:\n");
   ri_data.currfile = ri_data.filelist;
   while (ri_data.currfile!= NULL) {
      // printf("%s\n", ri_data.currfile->name);
      ri_data.currfile = ri_data.currfile->next;
   }

   ri_data.currfile = ri_data.filelist; 
   
   printf("\n\nGetting links\n");
   
   // kernel timer start
   struct timeval tv3;
   gettimeofday(&tv3, NULL);

   getlinks(); // kernel

   struct timeval tv4;
   gettimeofday(&tv4, NULL);
   long kernel_time_elapsed = 0;
   kernel_time_elapsed += (tv4.tv_sec - tv3.tv_sec) * 1000000 + tv4.tv_usec - tv3.tv_usec;
   printf("kernel_time_elapsed: %ld\n",kernel_time_elapsed);   
   // kernel timer stop
   
   link_elem_t *curr_elem;
   int i;
   
   for (i = 0; i < use_len; i++)
   {
      // printf("\nLink to \"%s\" found in:\n", links[i].link);
      curr_elem = links[i].elem;
      while(curr_elem != NULL)
      {
         // printf("\t%s\n",curr_elem->filename);
         curr_elem = curr_elem->next;   
      }   
   }
   
   struct timeval tv2;
   gettimeofday(&tv2, NULL);
   // printf("sec: %ld,usec: %ld \n",tv2.tv_sec, tv2.tv_usec);
   long total_time_elapsed = 0;
   total_time_elapsed += (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
   printf("total_time_elapsed: %ld\n",total_time_elapsed);

   return 0;
}
