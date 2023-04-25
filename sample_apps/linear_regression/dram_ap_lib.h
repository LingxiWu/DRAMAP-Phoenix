#ifndef _DRAMAPLIB_H_
#define _DRAMAPLIB_H_



typedef struct {
   char *data;
   unsigned long long size;
} input_file_handler;

inline void dram_ap_valloc(char **src_v, int group_id, unsigned long long vl, int bit_len)
{	
	*src_v = malloc(vl * sizeof(char));
	return;
}

inline void dram_ap_valloc_l(long **src_v, int group_id, unsigned long long vl, int bit_len)
{	
	*src_v = malloc(vl * sizeof(long long));
	return;
}

// load POINT data from file into DRAM_AP vector
// we assume DRAM_AP provides a specific file handler structure that contains input file meta data (input_file_handler), and can be created by calling a special function dram_ap_mmap()
inline void dram_ap_fld(input_file_handler* points_fd, int offset, int stride_dist, char *src_v, unsigned long long vl, int bit_len, int orientation)
{

	for (unsigned long long i = 0; i < vl; i++) {
		src_v[i] = (char) points_fd->data[offset + i * stride_dist];
	}
	
	return;
}

inline void dram_ap_vcpy(char *dst_v, char *src_v, unsigned long long vl, int bit_len)
{
	for(int i = 0; i < vl; i++){
		dst_v[i] = (char) src_v[i];
	}
}

inline void dram_ap_vmul(long *dst_v, char *src1_v, char *src2_v, unsigned long long vl, int bit_len)
{
	for(int i = 0; i < vl; i++){
		dst_v[i] = (long) ((long)src1_v[i] * (long)src2_v[i]);
	}
}

inline void dram_ap_vredsum_l(long long *scalar, long* src_v, unsigned long long vl, int bit_len)
{
	for (int i = 0; i < vl; i++) {
		*scalar += (long long) src_v[i];
	}
}

inline void dram_ap_vredsum_c(long long *scalar, char* src_v, unsigned long long vl, int bit_len)
{
	for (int i = 0; i < vl; i++) {
		*scalar += (long long) src_v[i];
	}
}




#endif // _DRAMAPLIB_H_