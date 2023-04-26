#ifndef _DRAMAPLIB_H_
#define _DRAMAPLIB_H_


typedef struct {
   int *data;
   unsigned long long size;
} input_file_handler;

inline void dram_ap_mmap(int** input_matrix, input_file_handler* fd_struct, int num_rows, int num_cols) // helper function to emulate loading from file into input_file_handler
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

inline void dram_ap_valloc(int **src_v, int group_id, unsigned long long vl, int bit_len)
{	
	*src_v = malloc(vl * sizeof(int));
	return;
}

inline void dram_ap_fld(input_file_handler* matrix_fd, int offset, int stride_dist, int *src_v, unsigned long long vl, int bit_len, int orientation)
{
	for (unsigned long long i = 0; i < vl; i++) {
		src_v[i] = matrix_fd->data[offset * stride_dist + i];
	}
}

inline void dram_ap_vredsum(int *scalar, int* src_v, unsigned long long vl, int bit_len)
{
	for (int i = 0; i < vl; i++) {
		*scalar += src_v[i];
	}
}

inline void dram_ap_brdcst(int scalar, int* src_v, unsigned long long vl, int bit_len) 
{
	for (int i = 0; i < vl; i++) {
		src_v[i] = scalar;
	}
}

inline void dram_ap_vsub(int* res_v, int* src1_v, int* src2_v, unsigned long long vl, int bit_len)
{
	for (int i = 0; i < vl; i++) {
		res_v[i] = src1_v[i] - src2_v[i];
	}
}

inline void dram_ap_vmul(int* res_v, int* src1_v, int* src2_v, unsigned long long vl, int bit_len) 
{
	for (int i = 0; i < vl; i++) {
		res_v[i] = src1_v[i] * src2_v[i];
	}
}










#endif // _DRAMAPLIB_H_