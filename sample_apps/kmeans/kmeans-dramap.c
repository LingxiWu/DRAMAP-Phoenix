/* Copyright (c) 2007, Stanford University
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Stanford University nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/ 

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "stddefines.h"

#include "dram_ap_lib.h"

// #define DEF_NUM_POINTS 100000
#define DEF_NUM_POINTS 10
// #define DEF_NUM_MEANS 100
#define DEF_NUM_MEANS 3
#define DEF_DIM 3
#define DEF_GRID_SIZE 1000

#define MAX_INT 2147483647
#define false 0
#define true 1

int num_points; // number of vectors
int dim;       // Dimension of each vector
int num_means; // number of clusters
int grid_size; // size of each dimension of vector space
int modified;

/** parse_args()
 *  Parse the user arguments
 */
// void parse_args(int argc, char **argv) 
// {   
//    int c;
//    extern char *optarg;
//    extern int optind;
   
//    num_points = DEF_NUM_POINTS;
//    num_means = DEF_NUM_MEANS;
//    dim = DEF_DIM;
//    grid_size = DEF_GRID_SIZE;
   
//    while ((c = getopt(argc, argv, "d:c:p:s:")) != EOF) 
//    {
//       switch (c) {
//          case 'd':
//             dim = atoi(optarg);
//             break;
//          case 'c':
//             num_means = atoi(optarg);
//             break;
//          case 'p':
//             num_points = atoi(optarg);
//             break;
//          case 's':
//             grid_size = atoi(optarg);
//             break;
//          case '?':
//             printf("Usage: %s -d <vector dimension> -c <num clusters> -p <num points> -s <grid size>\n", argv[0]);
//             exit(1);
//       }
//    }
   
//    if (dim <= 0 || num_means <= 0 || num_points <= 0 || grid_size <= 0) {
//       printf("Illegal argument value. All values must be numeric and greater than 0\n");
//       exit(1);
//    }
   
//    printf("Dimension = %d\n", dim);
//    printf("Number of clusters = %d\n", num_means);
//    printf("Number of points = %d\n", num_points);
//    printf("Size of each dimension = %d\n", grid_size);   
// }

/** generate_points()
 *  Generate the points
 */
void generate_points(int **pts, int size) 
{   
   int i, j;
   
   for (i=0; i<size; i++) 
   {
      for (j=0; j<dim; j++) 
      {
         pts[i][j] = rand() % grid_size;
      }
   }
}

/** get_sq_dist()
 *  Get the squared distance between 2 points
 */
inline unsigned int get_sq_dist(int *v1, int *v2)
{
   int i;
   
   unsigned int sum = 0;
   for (i = 0; i < dim; i++) 
   {
      sum += ((v1[i] - v2[i]) * (v1[i] - v2[i])); 
   }
   return sum;
}

/** add_to_sum()
 *	Helper function to update the total distance sum
 */
void add_to_sum(int *sum, int *point)
{
   int i;
   
   for (i = 0; i < dim; i++)
   {
      sum[i] += point[i];   
   }   
}

/** find_clusters()
 *  Find the cluster that is most suitable for a given set of points
 */
void find_clusters(int **points, int **means, int *clusters) 
{
   int i, j;
   unsigned int min_dist, cur_dist;
   int min_idx;

   for (i = 0; i < num_points; i++) 
   {
      min_dist = get_sq_dist(points[i], means[0]);
      min_idx = 0; 
      for (j = 1; j < num_means; j++)
      {
         cur_dist = get_sq_dist(points[i], means[j]);
         if (cur_dist < min_dist) 
         {
            min_dist = cur_dist;
            min_idx = j;   
         }
      }
      
      if (clusters[i] != min_idx) 
      {
         clusters[i] = min_idx;
         modified = true;
      }
   }   
}

/** calc_means()
 *  Compute the means for the various clusters
 */
void calc_means(int **points, int **means, int *clusters)
{
   int i, j, grp_size;
   int *sum;
   
   sum = (int *)malloc(dim * sizeof(int));
   
   for (i = 0; i < num_means; i++) 
   {
      memset(sum, 0, dim * sizeof(int));
      grp_size = 0;
      
      for (j = 0; j < num_points; j++)
      {
         if (clusters[j] == i) 
         {
            add_to_sum(sum, points[j]);
            grp_size++;
         }   
      }
      
      for (j = 0; j < dim; j++)
      {
         //dprintf("div sum = %d, grp size = %d\n", sum[j], grp_size);
         if (grp_size != 0)
         { 
            means[i][j] = sum[j] / grp_size;
         }
      }       
   }
}

/** dump_matrix()
 *  Helper function to print out the points
 */
void dump_matrix(int **vals, int rows, int cols)
{
   int i, j;
   
   for (i = 0; i < rows; i++) 
   {
      for (j = 0; j < cols; j++)
      {
         dprintf("%5d ",vals[i][j]);
      }
      dprintf("\n");
   }
}

/** 
* This application groups 'num_points' row-vectors (which are randomly
* generated) into 'num_means' clusters through an iterative algorithm - the 
* k-means algorith 
*/
int main(int argc, char **argv) 
{

   int **points;
   int **means;
   int *clusters;
   
   int i;
   
   // parse_args(argc, argv);
   num_points = DEF_NUM_POINTS;
   num_means = DEF_NUM_MEANS;
   dim = DEF_DIM;
   grid_size = DEF_GRID_SIZE;

   printf("num_points: %d, num_means: %d, dim: %d\n", num_points, num_means, dim);

   points = (int **)malloc(sizeof(int *) * num_points);
   for (i=0; i<num_points; i++) 
   {
      points[i] = (int *)malloc(sizeof(int) * dim);
   }
   dprintf("Generating points\n");
   generate_points(points, num_points);   

   for (int i = 0; i < dim; i++) {
      printf("dim %d: ",i);
      for (int j = 0; j < num_points; j++) {
         printf("%5d ", points[j][i]);
      }
      printf("\n");
   }
   printf("\n\n");

   means = (int **)malloc(sizeof(int *) * num_means);
   for (i=0; i<num_means; i++) 
   {
      means[i] = (int *)malloc(sizeof(int) * dim);
   }
   dprintf("Generating means\n");
   generate_points(means, num_means);

   for (int i = 0; i < dim; i++) {
      printf("initial means_dim %d: ",i);
      for (int j = 0; j < num_means; j++) {
         printf("%5d ", means[j][i]);
      }
      printf("\n");
   }
   printf("\n\n");
   
   clusters = (int *)malloc(sizeof(int) * num_points);
   memset(clusters, -1, sizeof(int) * num_points);
   
   modified = true;

   /* START DRAMAP */
   
   input_file_handler points_fd;
   points_fd.points = points;
   points_fd.size = num_points;

   unsigned long long vl = num_points;
   int group_id = 0;
   int pts_bit_len = dim * 4 * 8; // 4-byte integer
   int dim_bit_len = pts_bit_len / dim;

   int** pts_v = dram_ap_valloc_1(group_id, vl, pts_bit_len);
   dram_ap_fld(pts_v, &points_fd, group_id, vl, pts_bit_len);

   for (int i = 0; i < dim; i++) {
      printf("pts_dim %d: ",i);
      for (int j = 0; j < num_points; j++) {
         printf("%5d ", pts_v[j][i]);
      }
      printf("\n");
   }
   printf("\n\n");


   // for (int itr = 0; itr < 5; itr++) {

   // }

   


   
   int* min_dist_v = dram_ap_valloc_2(group_id, vl, dim_bit_len);
   int** dist_matrix = dram_ap_valloc_3(group_id, vl, num_means);
   dram_ap_brdcst_2(MAX_INT, min_dist_v, vl, pts_bit_len); 

   // distance calculation
   for (int i = 0; i < num_means; i++) { // for all centroid

      int** mean_v = dram_ap_valloc_1(group_id, vl, pts_bit_len);
      
      // brdcst means[i] to mean_v
      dram_ap_brdcst_1(means[i], mean_v, vl, pts_bit_len);    

      // compute dist of pts_v to means_v
      int* dist_v = dram_ap_valloc_2(group_id, vl, pts_bit_len);
      dram_ap_brdcst_2(0, dist_v, vl, pts_bit_len);

      for (int j = 0; j < dim; j++) { // calculate the distance between all points and centroid[i]

         printf("means_v_dim_%d: ",j);
         for (int k = 0; k < num_points; k++) {
            printf("%5d ",mean_v[k][j]);
         }
         printf("\n");

         int* dist_dim_v = dram_ap_valloc_2(group_id, vl, dim_bit_len);
         dram_ap_vsub(dist_dim_v, pts_v, mean_v, vl, j * dim_bit_len, dim_bit_len);
         dram_ap_vabs(dist_dim_v, vl, j * dim_bit_len, dim_bit_len);
         dram_ap_vacc(dist_v, dist_dim_v, vl, j * dim_bit_len, pts_bit_len);

         printf("dist_dim_v_%d: ",j);
         for(int i=0;i<vl;i++){
            printf("%5d ",dist_dim_v[i]);
         }
         printf("\n");


         free(dist_dim_v);
      }
      
      dram_ap_vmin(min_dist_v, dist_v, vl, pts_bit_len);

      printf("total_dist_v: ");
      for(int i=0;i<vl;i++){
         printf("%5d ",dist_v[i]);
      }
      printf("\n");

      dram_ap_vcpy(dist_matrix[i], dist_v, num_points, pts_bit_len); // use vcpy


      free(dist_v);
      printf("\n");
   }

   printf("min_dist_v\n");
   for(int i=0;i<vl;i++){
      printf("%5d ",min_dist_v[i]);
   }
   printf("\n");
   

   // for each centroid, produce one-hot encoding, aggregate results, and make new centroids
   for (int i = 0; i < num_means; i++) { // produce new means
      int* mask_v = dram_ap_valloc_2(group_id, vl, 1); // for each centroid, produce a mask indicating the closest centroid to each point
      dram_ap_brdcst_2(0, mask_v, vl, pts_bit_len);
      
      dram_ap_vmatch(mask_v, min_dist_v, dist_matrix[i], num_points, pts_bit_len);

      printf("dist_matrix[%d]: ",i);
      for (int j = 0; j < num_points; j++){
         printf("%d ", dist_matrix[i][j]);
      }
      printf("\n");

      printf("mask_v:");
      for (int j = 0; j < num_points; j++){
         printf("%d ", mask_v[j]);
      }
      printf("\n\n");

      
      for (int j=0; j<dim; j++) {
         
         printf("pts_dim %d: ",j);
         int* pts_dim = (int *)malloc(sizeof(int) * vl); // for all dim_x of all points
         for (int i=0; i<num_points; i++){
            pts_dim[i] = pts_v[i][j];
            printf("%5d ",pts_dim[i]);
         }
         int dim_sum = dram_ap_vredsum(pts_dim, mask_v, num_points, j * dim_bit_len, pts_bit_len);
         int num_pts = dram_ap_pcl(mask_v, num_points);
         int new_mean_dim = dim_sum / num_pts;
         printf(" dim_%d_sum: %5d, new_mean_dim_%d: %d",j, dim_sum, j, new_mean_dim);
         printf("\n");

         means[i][j] = new_mean_dim;


         free(pts_dim);
      }

      printf("\n");


      free(mask_v);
   }

   for (int i = 0; i < dim; i++) {
      printf("new means_dim %d: ",i);
      for (int j = 0; j < num_means; j++) {
         printf("%5d ", means[j][i]);
      }
      printf("\n");
   }


   free(dist_matrix);
   free(min_dist_v);

   
   for (int j = 0; j < dim; j++) {
      printf("pts_dim %d: ",j);
      for (int i = 0; i < num_points; i++) {
         printf("%5d ", pts_v[i][j]);
      }
      printf("\n");
   }


   free(pts_v);





   /* END DRAMAP */


   /* START CPU BASELINE */
   dprintf("\n\nStarting CPU baseline iterative algorithm\n");
   
   
   while (modified) 
   {
      modified = false;
      dprintf(".");
      
      find_clusters(points, means, clusters);
      calc_means(points, means, clusters);
   }
   
   
   dprintf("\n\nFinal Means:\n");
   dump_matrix(means, num_means, dim);
   
   dprintf("Cleaning up\n");
   for (i=0; i<num_means; i++) {
      free(means[i]);
   }
   free(means);
   for (i=0; i<num_points; i++) {
      free(points[i]);
   }
   free(points);

   /* END CPU BASELINE */
   return 0;  
}
