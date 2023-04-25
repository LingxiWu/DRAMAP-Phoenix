#ifndef _DRAMAPLIB_H_
#define _DRAMAPLIB_H_


typedef struct {
   int *data;
   unsigned long long size;
} input_file_handler;

inline void dram_ap_fld(int** input_matrix, input_file_handler* fd_struct, int num_rows, int num_cols) // helper function to emulate loading from file into input_file_handler
{
	int i, j;
   
   	for (i=0; i<num_rows; i++) 
   	{
    	for (j=0; j<num_cols; j++) 
      	{
        	fd_struct->data[i * num_rows + j] = input_matrix[i][j];
     	}
   	}
}








#endif // _DRAMAPLIB_H_