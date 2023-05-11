#ifndef _DRAMAPLIB_H_
#define _DRAMAPLIB_H_

typedef struct {
   int **points;
   unsigned long long size;
} input_file_handler;

#define min(a,b)            (((a) < (b)) ? (a) : (b))


// emulate loading from file to a vector
inline void dram_ap_fld(int** src_v, input_file_handler* fd, int group_id, unsigned long long vl, int pts_bit_len)
{	
	for (int i = 0; i < vl; i++) 
   {
      for (int j = 0; j < pts_bit_len / 4 / 8; j++)
      {
         src_v[i][j] = fd->points[i][j];
      }
   }

}

int** dram_ap_valloc_1(int group_id, unsigned long long vl, int pts_bit_len) 
{
	int** points;
	points = (int **)malloc(sizeof(int *) * vl);
   for (int i=0; i<vl; i++) 
   {
      points[i] = (int *)malloc(sizeof(int) * pts_bit_len / 4 / 8);
   }
   return points;
}

int* dram_ap_valloc_2(int group_id, unsigned long long vl, int dim_bit_len)  
{
	int* vec;
	vec = (int *)malloc(sizeof(int) * vl);

	return vec;
}

int** dram_ap_valloc_3(int group_id, unsigned long long vl, int num_means) 
{
	int** points;
	points = (int **)malloc(sizeof(int *) * num_means);
   for (int i=0; i<num_means; i++) 
   {
      points[i] = (int *)malloc(sizeof(int) * vl);
   }
   return points;
}

inline void dram_ap_brdcst_1(int* array, int** src_v, unsigned long long vl, int pts_bit_len)
{
	for (int i = 0; i < vl; i++) {
		for (int j = 0; j < pts_bit_len / 4 / 8 ; j++) { // get the proper dimension
			src_v[i][j] = array[j];
		}
	}
}

inline void dram_ap_brdcst_2(int scalar, int* src_v, unsigned long long vl, int pts_bit_len) 
{
	for (int i = 0; i < vl; i++) {
		src_v[i] = scalar;
	}
}

inline void dram_ap_vsub(int* dist_v, int** src1_v, int** src2_v, unsigned long long vl, int start_bit, int dim_bit_len) 
{
	for (int i = 0; i < vl; i++) {
		int j = start_bit / 32; // calc dimension
		dist_v[i] = src1_v[i][j] - src2_v[i][j];
	}
}

inline void dram_ap_vabs(int* src_v, unsigned long long vl, int start_bit, int dim_bit_len) {
	for (int i = 0; i < vl; i++) {
		src_v[i] = abs(src_v[i]);
	}
}

inline void dram_ap_vacc(int* dst_v, int* src_v, unsigned long long vl, int start_bit, int bit_len) {
	for (int i = 0; i < vl; i++) {
		dst_v[i] += src_v[i];
	}
}

void dram_ap_vmin(int* src1_v, int* src2_v, unsigned long long vl, int bit_len) 
{
	for (int i = 0; i < vl; i++) {
		if (src1_v[i] < src2_v[i])
			src1_v[i] = src1_v[i];
		else
			src1_v[i] = src2_v[i];
	}
}	

void dram_ap_vmatch(int* mask_v, int* src1_v, int* src2_v, unsigned long long vl, int bit_len)
{
	for (int i = 0; i < vl; i++) {
		// printf("src1_v[%d]: %d, src2_v[%d]: %d\n", i, src1_v[i], i, src2_v[i]);
		if (src1_v[i] == src2_v[i]) {
			mask_v[i] = 1;
		} 
	}
	
}

void dram_ap_vcpy(int* dist_v, int* src_v, int num_element, int bit_len)
{
	for (int i = 0; i < num_element; i++)
	{
		dist_v[i] = src_v[i];
	}
}

int dram_ap_vredsum(int* pts_dim_v, int* mask_v, unsigned long long num_points, int start_bit, int pts_bit_len)
{
	int res = 0;
	for (int i = 0; i < num_points; i ++) {
		if (mask_v[i] == 1) {
			// printf("pts_dim_v[%d]: %d ",i,pts_dim_v[i]);
			res += pts_dim_v[i];
		}
	}
	// printf("\n");
	return res;
}

int dram_ap_pcl(int* mask_v, unsigned long long vl)
{
	int res = 0;
	for (int i = 0; i < vl; i++) {
		res += mask_v[i];
	}
	return res;
}


#endif // _DRAMAPLIB_H_