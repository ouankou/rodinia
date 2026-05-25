/*****************************************************************************/
/*IMPORTANT:  READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.         */
/*By downloading, copying, installing or using the software you agree        */
/*to this license.  If you do not agree to this license, do not download,    */
/*install, copy or use the software.                                         */
/*                                                                           */
/*                                                                           */
/*Copyright (c) 2005 Northwestern University                                 */
/*All rights reserved.                                                       */

/*Redistribution of the software in source and binary forms,                 */
/*with or without modification, is permitted provided that the               */
/*following conditions are met:                                              */
/*                                                                           */
/*1       Redistributions of source code must retain the above copyright     */
/*        notice, this list of conditions and the following disclaimer.      */
/*                                                                           */
/*2       Redistributions in binary form must reproduce the above copyright   */
/*        notice, this list of conditions and the following disclaimer in the */
/*        documentation and/or other materials provided with the distribution.*/
/*                                                                            */
/*3       Neither the name of Northwestern University nor the names of its    */
/*        contributors may be used to endorse or promote products derived     */
/*        from this software without specific prior written permission.       */
/*                                                                            */
/*THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS    */
/*IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED      */
/*TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT AND         */
/*FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL          */
/*NORTHWESTERN UNIVERSITY OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,       */
/*INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES          */
/*(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR          */
/*SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)          */
/*HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,         */
/*STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN    */
/*ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE             */
/*POSSIBILITY OF SUCH DAMAGE.                                                 */
/******************************************************************************/
/*************************************************************************/
/**   File:         kmeans_clustering.c                                 **/
/**   Description:  Implementation of regular k-means clustering        **/
/**                 algorithm                                           **/
/**   Author:  Wei-keng Liao                                            **/
/**            ECE Department, Northwestern University                  **/
/**            email: wkliao@ece.northwestern.edu                       **/
/**                                                                     **/
/**   Edited by: Jay Pisharath                                          **/
/**              Northwestern University.                               **/
/**                                                                     **/
/**   ================================================================  **/
/**
 * **/
/**   Edited by: Sang-Ha  Lee
 * **/
/**				 University of Virginia
 * **/
/**
 * **/
/**   Description:	No longer supports fuzzy c-means clustering;
 * **/
/**					only regular k-means clustering.
 * **/
/**					Simplified for main functionality:
 * regular k-means	**/
/**					clustering.
 * **/
/**                                                                     **/
/*************************************************************************/

#include "kmeans.h"
#include <float.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define RANDOM_MAX 2147483647

#ifndef FLT_MAX
#define FLT_MAX 3.40282347e+38
#endif

extern double wtime(void);
#pragma omp declare target
int find_nearest_point(float *pt,                  /* [nfeatures] */
                       int nfeatures, float **pts, /* [npts][nfeatures] */
                       int npts) {
  int index, i;
  float min_dist = FLT_MAX;

  /* find the cluster center id with min distance to pt */
  for (i = 0; i < npts; i++) {
    float dist;
    dist = euclid_dist_2(pt, pts[i], nfeatures); /* no need square root */
    if (dist < min_dist) {
      min_dist = dist;
      index = i;
    }
  }
  return (index);
}

/*----< euclid_dist_2() >----------------------------------------------------*/
/* multi-dimensional spatial Euclid distance square */
__inline float euclid_dist_2(float *pt1, float *pt2, int numdims) {
  int i;
  float ans = 0.0;

  for (i = 0; i < numdims; i++) {
    ans += (pt1[i] - pt2[i]) * (pt1[i] - pt2[i]);
  }

  return (ans);
}

static int find_nearest_point_flat(float *pt, int nfeatures, float *pts,
                                   int npts) {
  int index = 0;
  float min_dist = FLT_MAX;

  for (int i = 0; i < npts; i++) {
    float dist = euclid_dist_2(pt, pts + i * nfeatures, nfeatures);
    if (dist < min_dist) {
      min_dist = dist;
      index = i;
    }
  }
  return index;
}
#pragma omp end declare target

/*----< kmeans_clustering() >---------------------------------------------*/
float **kmeans_clustering(float **h_feature, /* in: [npoints][nfeatures] */
                          int nfeatures, int npoints, int nclusters,
                          float threshold, int *membership) /* out: [npoints] */
{

  int i, j, n = 0, loop = 0;
  int *new_centers_len;  /* [nclusters]: no. of points in each cluster */
  float **h_clusters;    /* out: [nclusters][nfeatures] */
  float *feature;
  float *clusters;
  float *new_centers;
  float *partial_centers;
  int *partial_centers_len;
  float delta;
  const int center_size = nclusters * nfeatures;
  const int feature_size = npoints * nfeatures;
  int num_chunks = center_size;
  if (num_chunks > npoints)
    num_chunks = npoints;

  feature = (float *)malloc(feature_size * sizeof(float));
  for (i = 0; i < npoints; i++)
    for (j = 0; j < nfeatures; j++)
      feature[i * nfeatures + j] = h_feature[i][j];

  /* allocate space for returning variable clusters[] */
  h_clusters = (float **)malloc(nclusters * sizeof(float *));
  h_clusters[0] = (float *)malloc(nclusters * nfeatures * sizeof(float));
  for (i = 1; i < nclusters; i++)
    h_clusters[i] = h_clusters[i - 1] + nfeatures;

  /* randomly pick cluster centers */
  for (i = 0; i < nclusters; i++) {
    // n = (int)rand() % npoints;
    for (j = 0; j < nfeatures; j++)
      h_clusters[i][j] = h_feature[n][j];
    n++;
  }

  clusters = (float *)malloc(center_size * sizeof(float));
  for (i = 0; i < nclusters; i++)
    for (j = 0; j < nfeatures; j++)
      clusters[i * nfeatures + j] = h_clusters[i][j];

  for (i = 0; i < npoints; i++)
    membership[i] = -1;

  /* need to initialize new_centers_len and new_centers[0] to all 0 */
  new_centers_len = (int *)calloc(nclusters, sizeof(int));
  new_centers = (float *)calloc(center_size, sizeof(float));
  partial_centers_len =
      (int *)calloc(num_chunks * nclusters, sizeof(int));
  partial_centers =
      (float *)calloc(num_chunks * center_size, sizeof(float));

#pragma omp target data map(to                                                 \
                            : feature [0:feature_size], membership [0:npoints])\
    map(tofrom : clusters [0:center_size])                                     \
    map(alloc                                                                 \
        : new_centers [0:center_size], new_centers_len [0:nclusters],          \
          partial_centers [0:num_chunks * center_size],                        \
          partial_centers_len [0:num_chunks * nclusters])
  {
    do {
      delta = 0.0;

#pragma omp target teams distribute parallel for private(j)                    \
    firstprivate(npoints, nclusters, nfeatures)                                \
    map(to : feature [0:feature_size], clusters [0:center_size])               \
    map(tofrom : membership [0:npoints]) reduction(+ : delta)
      for (i = 0; i < npoints; i++) {
        /* find the index of nestest cluster centers */
        int index = find_nearest_point_flat(feature + i * nfeatures, nfeatures,
                                            clusters, nclusters);
        /* if membership changes, increase delta by 1 */
        if (membership[i] != index)
          delta += 1.0;

        /* assign the membership to object i */
        membership[i] = index;
      }

#pragma omp target teams distribute parallel for collapse(2)                  \
    firstprivate(npoints, nclusters, num_chunks)                              \
    map(to : membership [0:npoints])                                          \
    map(tofrom : partial_centers_len [0:num_chunks * nclusters])
      for (int chunk = 0; chunk < num_chunks; chunk++) {
        for (int cluster = 0; cluster < nclusters; cluster++) {
          int start = (int)(((long long)chunk * npoints) / num_chunks);
          int end = (int)(((long long)(chunk + 1) * npoints) / num_chunks);
          int count = 0;
          for (int point = start; point < end; point++) {
            if (membership[point] == cluster)
              count++;
          }
          partial_centers_len[chunk * nclusters + cluster] = count;
        }
      }

#pragma omp target teams distribute parallel for collapse(3)                  \
    firstprivate(npoints, nclusters, nfeatures, num_chunks, center_size)      \
    map(to : feature [0:feature_size], membership [0:npoints])                \
    map(tofrom : partial_centers [0:num_chunks * center_size])
      for (int chunk = 0; chunk < num_chunks; chunk++) {
        for (int cluster = 0; cluster < nclusters; cluster++) {
          for (int feature_idx = 0; feature_idx < nfeatures; feature_idx++) {
            int start = (int)(((long long)chunk * npoints) / num_chunks);
            int end = (int)(((long long)(chunk + 1) * npoints) / num_chunks);
            float sum = 0.0f;
            for (int point = start; point < end; point++) {
              if (membership[point] == cluster)
                sum += feature[point * nfeatures + feature_idx];
            }
            partial_centers[chunk * center_size + cluster * nfeatures +
                            feature_idx] = sum;
          }
        }
      }

#pragma omp target teams distribute parallel for                              \
    firstprivate(nclusters, num_chunks)                                       \
    map(to : partial_centers_len [0:num_chunks * nclusters])                  \
    map(tofrom : new_centers_len [0:nclusters])
      for (i = 0; i < nclusters; i++) {
        int count = 0;
        for (int chunk = 0; chunk < num_chunks; chunk++)
          count += partial_centers_len[chunk * nclusters + i];
        new_centers_len[i] = count;
      }

#pragma omp target teams distribute parallel for collapse(2)                  \
    firstprivate(nclusters, nfeatures, num_chunks, center_size)               \
    map(to : partial_centers [0:num_chunks * center_size])                    \
    map(tofrom : new_centers [0:center_size])
      for (i = 0; i < nclusters; i++) {
        for (j = 0; j < nfeatures; j++) {
          float sum = 0.0f;
          for (int chunk = 0; chunk < num_chunks; chunk++)
            sum += partial_centers[chunk * center_size + i * nfeatures + j];
          new_centers[i * nfeatures + j] = sum;
        }
      }

      /* replace old cluster centers with new_centers */
#pragma omp target teams distribute parallel for collapse(2)                  \
    firstprivate(nclusters, nfeatures)                                        \
    map(to : new_centers [0:center_size], new_centers_len [0:nclusters])      \
    map(tofrom : clusters [0:center_size])
      for (i = 0; i < nclusters; i++) {
        for (j = 0; j < nfeatures; j++) {
          if (new_centers_len[i] > 0)
            clusters[i * nfeatures + j] =
                new_centers[i * nfeatures + j] / new_centers_len[i];
        }
      }
    } while (delta > threshold && loop++ < 500);
  }

  for (i = 0; i < nclusters; i++)
    for (j = 0; j < nfeatures; j++)
      h_clusters[i][j] = clusters[i * nfeatures + j];

  free(feature);
  free(clusters);
  free(new_centers);
  free(partial_centers);
  free(partial_centers_len);
  free(new_centers_len);

  return h_clusters;
}
