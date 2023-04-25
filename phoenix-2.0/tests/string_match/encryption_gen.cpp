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
#include <iostream>
#include <fstream>
#include <list>
#include <vector>

#define SALT_SIZE 2
#define MAX_REC_LEN 1024
#define OFFSET 5

using namespace std;
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

int main(int argc, char *argv[]) 
{
	string line;
	ifstream words_alpha_f ("words_alpha.txt");

	// if(remove("encrypted_small.txt")==0){
	// 	printf("previous encrypted_small.txt removed ...\n");
	// }
	// if(remove("encrypted_medium.txt")==0){
	// 	printf("previous encrypted_medium.txt removed ...\n");
	// }
	// if(remove("encrypted_large.txt")==0){
	// 	printf("previous encrypted_large.txt removed ...\n");
	// }
	// ofstream encrypted_f("encrypted_medium.txt");
	// ofstream encrypted_f("encrypted_small.txt");
	// ofstream encrypted_f("encrypted_large.txt");

	srand(time(0)); 
	int random;
	// int temp = 0;

	int MAX_WORD_LEN = 50;
	std::vector<long long> word_len_hist(++MAX_WORD_LEN,0);
	

	// int max_len = 0; // max DDT 31 chars
	// string max_word;
	if(words_alpha_f.is_open()){
		while(getline(words_alpha_f, line)){ // iterate each word
			int n = line.length();
			word_len_hist[n] ++;
			// if (n > max_len){
			// 	max_len = n;
			// 	max_word = line;
			// }
			// printf("word: %s, len: %d\n", line.c_str(), n);

/*
			random = rand() % 100 + 1;
			if(random < 31) { // encrypt 30% of the words_alpha
				int n = line.length();
				// cout << line << endl;
				char char_array[n+1];
				strcpy(char_array, line.c_str());
				char * encrypted = (char*)malloc(MAX_REC_LEN);
				compute_hashes(char_array, encrypted);
				encrypted_f << encrypted << endl;
				free((char*)encrypted);
				// printf("encrypted: %s\n", encrypted);
			}
*/			
		}

	} else {
		printf("error open plain text file\n");
	}
	// printf("max word: %s, max_length: %d\n", max_word.c_str(), max_len);
	for (long long x : word_len_hist)
		cout << x << " ";

	// printf("new encrypted_small file generated ...\n");
	// printf("new encrypted_medium file generated ...\n");
	// printf("new encrypted_large file generated ...\n");

	// char *key1 = "Helloworld";
	// char *key2 = "howareyou";
	// char *key3 = "ferrari";
	// char *key4 = "whotheman";

	// char *key1_final = malloc(strlen(key1) + 1);
	// char *key2_final = malloc(strlen(key2) + 1);
	// char *key3_final = malloc(strlen(key3) + 1);
	// char *key4_final = malloc(strlen(key4) + 1);

	// compute_hashes(key1, key1_final);
	// compute_hashes(key2, key2_final);
	// compute_hashes(key3, key3_final);
	// compute_hashes(key4, key4_final);

	// encrypted_f << key1_final << endl;
	// encrypted_f << key2_final << endl;
	// encrypted_f << key3_final << endl;
	// encrypted_f << key4_final << endl;

	words_alpha_f.close();
	// encrypted_f.close();
	return 0;
}