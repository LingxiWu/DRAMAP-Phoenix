#ifndef _DRAMAPLIB_H_
#define _DRAMAPLIB_H_

typedef struct {
   
   int num_vec; // number of vectors to be used in this kernel

   int vl; // max num of elements in a vector
   int bit_len;

   int **source_vecs;

   int **results_vecs;

} dram_ap_kernel_config;

typedef struct {

	int num_bank;
	int num_sa_per_bank;
	int max_bit_len;
	int max_vl; // 8192 --> 16385

} dram_ap_arch_config; // basic configurations of the DRAM-AP architecture

inline void _DRAM_AP_bit_transpose(){
	
}

// helper function that transpose elements of a vector
inline void _DRAM_AP_matrix_trans(int* matrix_t, int* matrix_src, int ml)
{
	for(int i=0; i<ml; i++){
		for(int j=0; j<ml; j++){
			matrix_t[i * ml + j] = matrix_src[j * ml + i];
			// printf("matrix_t[%d]: %d\n", i*ml+j, matrix_src[j * ml + i]);
		}
	}
	return;
}

inline dram_ap_arch_config _make_DRAM_AP_arch_config(){
	dram_ap_arch_config arch_config_obj;

	arch_config_obj.num_bank = 128;
	arch_config_obj.num_sa_per_bank = 128;
	arch_config_obj.max_bit_len = 512;
	arch_config_obj.max_vl = 8192;

	return arch_config_obj;
}

// copy from *addr to *vec_reg
inline void _DRAM_AP_vld(int* vec_reg, int* addr, int vl, int bit_len)
{	
	// other func args are useful for C++ libs
	for(int i=0;i<vl;i++){
		vec_reg[i] = addr[i];
	}
	// printf("addr: %d, %d, %d\n", addr[0], addr[1], addr[2]);
	// printf("vec_reg: %d, %d, %d\n", vec_reg[0], vec_reg[1], vec_reg[2]);
	return;
}


inline void _DRAM_AP_vst(int* addr, int* vec_reg, int vl, int bit_len)
{
	for(int i=0;i<vl;i++){
		addr[i] = vec_reg[i];
	}
	return;
}

inline void _DRAM_AP_vcp(int* vec_dst, int* vec_src, int vl, int bit_len)
{
	for(int i=0;i<vl;i++){
		vec_dst[i] = vec_src[i];
	}
	return;
}

inline void _DRAM_AP_vput(int* vec, int index, int val, int vl, int bit_len)
{
	vec[index] = val;
}



inline void _DRAM_AP_vmul(int* vec_res, int* vec_src1, int* vec_src2, int vl, int bit_len)
{
	
	// printf("vec_1: %d, %d, %d\n", vec_src1[0],vec_src1[1],vec_src1[2]);
	// printf("vec_2: %d, %d, %d\n", vec_src2[0],vec_src2[1],vec_src2[2]);
	
	for(int i=0;i<vl;i++){
		vec_res[i] = vec_src1[i] * vec_src2[i];
	}
	return;
    
}

// future work: provide a mask array for gather operation
inline void _DRAM_AP_vredsum(int* val, int* vec, int vl, int bit_len)
{
	for(int i=0;i<vl;i++){
		*val += vec[i];
	}
	return;
}



#define MIN(x, y) (((x) < (y)) ? (x) : (y))


#endif // _DRAMAPLIB_H_