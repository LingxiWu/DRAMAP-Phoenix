
#ifndef _DRAMAPLIB_H_
#define _DRAMAPLIB_H_


typedef struct {
   
   char *img_data;
   int imgdata_bytes;

   int bits_per_pixel_pos;
   int img_data_offset_pos;

   int data_pos;

} file_handler;


inline void dram_ap_valloc(unsigned char **src_v, int group_id, unsigned long long vl, int bit_len)
{	
	*src_v = malloc(vl * sizeof(char));
	return;
}

// load image data into img_v DRAM_AP vector
inline void dram_ap_fld(file_handler* file_handler, unsigned char *src_v, unsigned long long vl, int bit_len)
{
	// unsigned char* img_pixels = malloc(vl * sizeof(char));
	assert(src_v != NULL);

	for (int i=file_handler->data_pos; i < file_handler->imgdata_bytes+file_handler->data_pos; i++) {      
      unsigned char *val = (unsigned char *)&(file_handler->img_data[i]);
      src_v[i-file_handler->data_pos] = *val; 
   }

	return;
}

inline void dram_ap_vfill(int pattern, unsigned char *mask_v, unsigned long long vl)
{
	for(int i=0;i<vl;i++){
		mask_v[i] = 0;
	}
	if (pattern == 100){ // 100
		for(int i=0;i<vl;i+=3){
			mask_v[i] = 1;
		}
	} else if (pattern == 10){
		for(int i=1;i<vl;i+=3){
			mask_v[i] = 1;
		}
	} else if (pattern == 1){
		for(int i=2;i<vl;i+=3){
			mask_v[i] = 1;
		}
	}
	return;
}

inline void dram_ap_brdcst(unsigned char val, unsigned char * src_v, unsigned long long vl, int bit_len)
{
	for(int i=0;i<vl;i++){
		src_v[i] = val;
	}
	return;
}

inline void dram_ap_xnor(unsigned char *dst_v, unsigned char *src1_v, unsigned char *src2_v, unsigned long long vl, int bit_len)
{
	for(int i=0;i<vl;i++){
		if(src1_v[i] == src2_v[i]){
			dst_v[i] = 1;
		}
	}
}

inline void dram_ap_cpy(unsigned char *dst_v, unsigned char *src_v, unsigned long long vl, int bit_len)
{
	for(int i=0;i<vl;i++){
		src_v[i] = dst_v[i];
	}
}

inline void dram_ap_and(unsigned char *dst_v, unsigned char *src1_v, unsigned char *src2_v, unsigned long long vl, int bit_len)
{
	for(int i=0;i<vl;i++){
		if(src1_v[i] == 1 && src2_v[i] == 1){
			dst_v[i] = 1;
		} else {
			dst_v[i] = 0;
		}
	}
}

inline void dram_ap_pcl(int *counter, unsigned char * src_v, unsigned long long vl, int bit_len)
{
	for(int i=0;i<vl;i++){
		if(src_v[i] == 1){
			*counter += 1;
		}
	}
}
#endif // _DRAMAPLIB_H_