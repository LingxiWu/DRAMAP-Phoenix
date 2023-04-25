#ifndef _DRAMAPLIB_H_
#define _DRAMAPLIB_H_

typedef struct {
   int *matrix_A;
   int *matrix_B;
   int *matrix_out;
   int matrix_len;
} mm_data_t;

typedef struct {
   int *data;
   int size; // matrix_len
} matrix_file_handler;

inline void dram_ap_valloc(int **src_v, int group_id, unsigned long long vl, int bit_len)
{	
	*src_v = malloc(vl * sizeof(int));
	return;
}

inline void dram_ap_vcpy(int *dst_v, int *src_v, unsigned long long vl, int bit_len)
{
	for(int i = 0; i < vl; i++){
		src_v[i] = dst_v[i];
	}
}

inline void dram_ap_vmul(int *dst_v, int *src1_v, int *src2_v, unsigned long long vl, int bit_len)
{
	for (int i = 0; i < vl; i++) {
		dst_v[i] = src1_v[i] * src2_v[i];
	}
}

inline void dram_ap_vredsum(int *scalar, int* src_v, unsigned long long vl, int bit_len)
{
	for (int i = 0; i < vl; i++) {
		*scalar += src_v[i];
	}
}

// load matrix data file into DRAM_AP vector
// we assume DRAM_AP provides a specific file handler structure that contains input file meta data, and can be created by calling a special function dram_ap_mmap()
// inline void dram_ap_fld(matrix_file_handler* matrix_fd, int offset, int *src_v, unsigned long long vl, int bit_len, int orientation)
inline void dram_ap_fld(matrix_file_handler* matrix_fd, int offset, int stride_dist, int *src_v, unsigned long long vl, int bit_len, int orientation)
{

	if (orientation == 1) { // load data in a row major layout
	    for (unsigned long long i = 0; i < vl; i++) {
	    	// src_v[i] = matrix_fd->data[offset * matrix_fd->size + i];
	    	src_v[i] = matrix_fd->data[offset * stride_dist + i];
	    }
	} else if (orientation == 0) { // load data in a column major layout
		for (unsigned long long i = 0; i < vl; i++){
			// src_v[i] = matrix_fd->data[i * matrix_fd->size + offset];
			src_v[i] = matrix_fd->data[i * stride_dist + offset];
		}
	}


	return;
}





#endif // _DRAMAPLIB_H_