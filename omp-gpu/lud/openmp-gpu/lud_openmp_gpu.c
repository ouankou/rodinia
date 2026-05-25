#include <stdio.h>

void lud_openmp_gpu(float *a, int size)
{
     int i,j,k;
     printf("%d\n",size);
     /*
      * The outer i loop has true loop-carried dependencies. Keep the device
      * work split at each i so every row update completes before the dependent
      * column update and the next iteration begin.
      */
#pragma omp target data map(tofrom : a [0:size * size]) use_device_ptr(a)
     for (i=0; i <size; i++){
#pragma omp target teams distribute parallel for is_device_ptr(a) \
    firstprivate(i, size) private(j,k)
         for (j=i; j <size; j++){
             float sum=a[i*size+j];
             for (k=0; k<i; k++) sum -= a[i*size+k]*a[k*size+j];
             a[i*size+j]=sum;
         }
#pragma omp target teams distribute parallel for is_device_ptr(a) \
    firstprivate(i, size) private(j,k)
         for (j=i+1;j<size; j++){
             float sum=a[j*size+i];
             for (k=0; k<i; k++) sum -=a[j*size+k]*a[k*size+i];
             a[j*size+i]=sum/a[i*size+i];
         }
     }
}
