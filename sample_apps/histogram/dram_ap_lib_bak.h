#ifndef _DRAMAPLIB_H_
#define _DRAMAPLIB_H_

typedef struct {
   
   int num_vec; // number of vectors to be used in this kernel

   char **source_vecs;

   int **results_vecs;

   // maybe another field indicate orientation of the results? V, H

} dram_ap_kernel_config;

typedef struct {

	int num_bank;
	int num_sa_per_bank;
	int max_bit_len;
	int max_vl; // 8192 --> 16385

} dram_ap_arch_config; // basic configurations of the DRAM-AP architecture


inline dram_ap_arch_config _make_DRAM_AP_arch_config(){
	dram_ap_arch_config arch_config_obj;

	arch_config_obj.num_bank = 128;
	arch_config_obj.num_sa_per_bank = 128;
	arch_config_obj.max_bit_len = 512;
	arch_config_obj.max_vl = 8192;

	return arch_config_obj;
}




// copy from *addr to *vec_reg
inline void _DRAM_AP_vld_char(char* vec_reg, char* addr, int vl, int bit_len)
{	
	// use bit_len to determine data type

	for(int i=0;i<vl;i++){
		vec_reg[i] = addr[i];
	}
	// printf("addr: %d, %d, %d\n", addr[0], addr[1], addr[2]);
	// printf("vec_reg: %d, %d, %d\n", vec_reg[0], vec_reg[1], vec_reg[2]);
	return;
}

inline void _DRAM_AP_vld_int(int* vec_reg, int* addr, int vl, int bit_len)
{	
	// use bit_len to determine data type

	for(int i=0;i<vl;i++){
		vec_reg[i] = addr[i];
	}
	// printf("addr: %d, %d, %d\n", addr[0], addr[1], addr[2]);
	// printf("vec_reg: %d, %d, %d\n", vec_reg[0], vec_reg[1], vec_reg[2]);
	return;
}

// do a parallel search across src_vec. If src_vec[i] matches key, set dst_vec[i] = 1
inline void _DRAM_AP_vsch(char* dst_vec, char * src_vec, char* key_vec, int vl, int bit_len)
{
	int hits = 0;
	for(int i=0;i<vl;i++){
		if(src_vec[i] == key_vec[i]){
			dst_vec[i] = 1;
			hits += 1;
		} else {
			dst_vec[i] = 0;
		}
	}
	printf("_DRAM_AP_vsch --> hits: %d\n",hits);
	
}

inline void _DRAM_AP_pcl(int* hits, char* vec_reg, char* mask, int vl, int bit_len){
	for(int i=0;i<vl;i++){
		if(vec_reg[i] == 1 && mask[i] == 1){
		// if(vec_reg[i] == 1 ){
			*hits += 1;
		}
	}
}

inline void _DRAM_AP_vbrdcst(char* vec_dst, char key, int vl, int bit_len){
	for(int i=0;i<vl;i++){
		vec_dst[i] = key;
	}
}



#define MIN(x, y) (((x) < (y)) ? (x) : (y))


#endif // _DRAMAPLIB_H_