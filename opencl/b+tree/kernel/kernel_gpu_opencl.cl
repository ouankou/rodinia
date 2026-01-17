// #ifdef __cplusplus
// extern "C" {
// #endif

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

//#define DEFAULT_ORDER 256

//======================================================================================================================================================150
//	STRUCTURES (had to bring from ../common.h here because feature of including headers in clBuildProgram does not work for some reason)
//======================================================================================================================================================150

// Type representing the record to which a given key refers. In a real B+ tree system, the record would hold data (in a database) or a file (in an operating system) or some other information.
// Users can rewrite this part of the code to change the type and content of the value field.
typedef struct record {
	int value;
} record;

// ???
typedef struct knode {
	int location;
	int indices [DEFAULT_ORDER + 1];
	int  keys [DEFAULT_ORDER + 1];
	bool is_leaf;
	int num_keys;
} knode; 

//========================================================================================================================================================================================================200
//	findK function
//========================================================================================================================================================================================================200

__kernel void 
findK(	long height,
		__global knode *knodesD,
		long knodes_elem,
		__global record *recordsD,

		__global long *currKnodeD,
		__global long *offsetD,
		__global int *keysD, 
		__global record *ansD)
{

	// private thread IDs
	int thid = get_local_id(0);
	int bid = get_group_id(0);
	int threadsPerBlock = get_local_size(0);

	// processtree levels
	int i;
	for(i = 0; i < height; i++){

		// Avoid data races by letting a single work-item update traversal state.
		if(thid == 0){
			long curr = currKnodeD[bid];
			long next = curr;
			int k;
			for(k = 0; k < threadsPerBlock; k++){
				int key_left = knodesD[curr].keys[k];
				int key_right = knodesD[curr].keys[k+1];
				if((key_left <= keysD[bid]) && (key_right > keysD[bid])){
					long candidate = (long)knodesD[curr].indices[k];
					// Guard against invalid indices.
					if(candidate < knodes_elem){
						next = candidate;
					}
					break;
				}
			}
			offsetD[bid] = next;
			currKnodeD[bid] = next;
		}
		barrier(CLK_GLOBAL_MEM_FENCE);

	}

	//At this point, we have a candidate leaf node which may contain
	//the target record.  Check each key to hopefully find the record
	if(knodesD[currKnodeD[bid]].keys[thid] == keysD[bid]){
		ansD[bid].value = recordsD[knodesD[currKnodeD[bid]].indices[thid]].value;
	}

}

//========================================================================================================================================================================================================200
//	End
//========================================================================================================================================================================================================200

// #ifdef __cplusplus
// }
// #endif
