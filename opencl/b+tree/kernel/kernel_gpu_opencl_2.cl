//========================================================================================================================================================================================================200
//	DEFINE/INCLUDE
//========================================================================================================================================================================================================200

//======================================================================================================================================================150
//	DEFINE
//======================================================================================================================================================150

// double precision support (switch between as needed for NVIDIA/AMD)
#ifdef AMDAPP
#ifdef cl_amd_fp64
#pragma OPENCL EXTENSION cl_amd_fp64 : enable
#endif
#else
#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif
#endif

// clBuildProgram compiler cannot link this file for some reason, so had to redefine constants and structures below
// #include ../common.h						// (in directory specified to compiler)			main function header

//======================================================================================================================================================150
//	DEFINE (had to bring from ../common.h here because feature of including headers in clBuildProgram does not work for some reason)
//======================================================================================================================================================150

// change to double if double precision needed
#define fp float

//#define DEFAULT_ORDER_2 256

//======================================================================================================================================================150
//	STRUCTURES (had to bring from ../common.h here because feature of including headers in clBuildProgram does not work for some reason)
//======================================================================================================================================================150

// ???
typedef struct knode {
	int location;
	int indices [DEFAULT_ORDER_2 + 1];
	int  keys [DEFAULT_ORDER_2 + 1];
	bool is_leaf;
	int num_keys;
} knode; 

//========================================================================================================================================================================================================200
//	findRangeK function
//========================================================================================================================================================================================================200

__kernel void 
findRangeK(	long height,
			__global knode *knodesD,
			long knodes_elem,

			__global long *currKnodeD,
			__global long *lastKnodeD,
			__global int *startD,
			__global int *endD,
			__global int *RecstartD, 
			__global int *ReclenD)
{

	// private thread IDs
	int thid = get_local_id(0);
	int bid = get_group_id(0);
	int threadsPerBlock = get_local_size(0);

	// ???
	int i;
	for(i = 0; i < height; i++){

		// Avoid data races by letting a single work-item update traversal state.
		if(thid == 0){
			long curr = currKnodeD[bid];
			long last = lastKnodeD[bid];
			long next = curr;
			long next_last = last;
			int k;
			int start_found = 0;
			int end_found = 0;
			for(k = 0; k < threadsPerBlock; k++){
				int curr_left = knodesD[curr].keys[k];
				int curr_right = knodesD[curr].keys[k+1];
				if(!start_found && (curr_left <= startD[bid]) && (curr_right > startD[bid])){
					long candidate = (long)knodesD[curr].indices[k];
					if(candidate < knodes_elem){
						next = candidate;
					}
					start_found = 1;
				}
				int last_left = knodesD[last].keys[k];
				int last_right = knodesD[last].keys[k+1];
				if(!end_found && (last_left <= endD[bid]) && (last_right > endD[bid])){
					long candidate = (long)knodesD[last].indices[k];
					if(candidate < knodes_elem){
						next_last = candidate;
					}
					end_found = 1;
				}
				if(start_found && end_found){
					break;
				}
			}
			currKnodeD[bid] = next;
			lastKnodeD[bid] = next_last;
		}
		barrier(CLK_GLOBAL_MEM_FENCE);
	}

	// Find the index of the starting record
	if(knodesD[currKnodeD[bid]].keys[thid] == startD[bid]){
		RecstartD[bid] = knodesD[currKnodeD[bid]].indices[thid];
	}
	//	__syncthreads();
	barrier(CLK_LOCAL_MEM_FENCE);

	// Find the index of the ending record
	if(knodesD[lastKnodeD[bid]].keys[thid] == endD[bid]){
		ReclenD[bid] = knodesD[lastKnodeD[bid]].indices[thid] - RecstartD[bid]+1;
	}

}

//========================================================================================================================================================================================================200
//	End
//========================================================================================================================================================================================================200
