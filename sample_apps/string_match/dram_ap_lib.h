#ifndef _DRAMAPLIB_H_
#define _DRAMAPLIB_H_

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

typedef struct {
  int keys_file_len;
  char * keys_file; // data
} str_data_t; // this emulates file handler returned by dramap


#define MAX_REC_LEN 1024

#define MAX_CHAR_LEN 128



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

		if( file[i] == '\n' ){ // linux new line character
			// printf("i: %d, word: %s \n", i, output);
			return (i+1);
		}
			

		output[i] = file[i];
		i++;
	}
	file+=i;
	return i;
}

unsigned long long getVlHelper(char *filename) {

	unsigned long long count = 0;
	char c;
	FILE *fp;
	fp = fopen(filename, "r");
	// Check if file exists
    if (fp == NULL)
    {
        printf("Could not open file %s", filename);
        return 0;
    }
	// Extract characters from file and store in character c
    for (c = getc(fp); c != EOF; c = getc(fp))
        if (c == '\n') // Increment count if this character is newline
            count = count + 1;
   fclose(fp);
   printf("The file %s has %lld lines\n", filename, count);

	return count;
}

inline char** dram_ap_valloc_char(int group_id, unsigned long long vl, int char_len) // NOTE, vl should be omitted in real implementation. 
{	
	char** src_v = malloc(vl * sizeof(char*));
	// for (int i = 0; i < vl; i++) {
	// 	src_v[i] = malloc(char_len * sizeof(char));
	// }
	return src_v;
}

// for allocating result 
inline void dram_ap_valloc(int** src_v, int group_id, unsigned long long vl, int bit_len)
{
	*src_v = malloc(vl * sizeof(int));
	return;
}

inline void dram_ap_brdcst(char* query, char** src_v, unsigned long long vl, int char_len) 
{
	for (int i = 0; i < vl; i++) {
		// src_v[i] = query;
		src_v[i] = malloc(strlen(query) * sizeof(char));
		memcpy(src_v[i], query, strlen(query));
	}
}





void dram_ap_fld(str_data_t* file_handler, char** src_v, unsigned long long vl, int char_len)
{
	int key_len;
	int i = 0;
	char * cur_word = malloc(char_len);
	char * key_file = file_handler->keys_file;
	bzero(cur_word, char_len);

	while( (key_len = getnextline(cur_word, char_len, key_file))>=0 ) {

		src_v[i] = malloc(key_len * sizeof(char));
		memcpy(src_v[i], cur_word, strlen(cur_word)); 
		key_file = key_file + key_len;
		bzero(cur_word, char_len);
		i += 1;
	}

}

void dram_ap_match(int* res_v, char** src1_v, char** src2_v, unsigned long long vl, int char_len)
{
	for (int i = 0; i < vl; i++) {
		if (!strcmp(src1_v[i], src2_v[i])) {
			res_v[i] = 1;
		}
		else {
			res_v[i] = 0;
		}
	}
}

void dram_ap_pcl(int* res, int* src_v, unsigned long long vl, int char_len)
{
	for (int i = 0; i < vl; i++) {
		*res += src_v[i];
	}
}



#endif // _DRAMAPLIB_H_