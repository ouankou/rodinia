set(RODINIA_OPENMP_LIBS "")
if(OpenMP_C_FOUND)
  list(APPEND RODINIA_OPENMP_LIBS OpenMP::OpenMP_C)
endif()
if(OpenMP_CXX_FOUND)
  list(APPEND RODINIA_OPENMP_LIBS OpenMP::OpenMP_CXX)
endif()

set(RODINIA_MATH_LIB m)

set(RODINIA_OPENCL_LINK_DIRS "")
set(RODINIA_OPENCL_RPATH_OPTIONS "")
if(TARGET OpenCL::OpenCL)
  get_target_property(RODINIA_OPENCL_LIB_LOCATION OpenCL::OpenCL IMPORTED_LOCATION)
  if(RODINIA_OPENCL_LIB_LOCATION)
    get_filename_component(RODINIA_OPENCL_LIB_DIR "${RODINIA_OPENCL_LIB_LOCATION}" DIRECTORY)
    set(RODINIA_OPENCL_LINK_DIRS "${RODINIA_OPENCL_LIB_DIR}")
    if(NOT WIN32)
      set(RODINIA_OPENCL_RPATH_OPTIONS "-Wl,-rpath,${RODINIA_OPENCL_LIB_DIR}")
    endif()
  endif()
endif()

function(rodinia_add_opencl_executable target)
  rodinia_add_executable(${target} ${ARGN}
    LINK_DIRECTORIES ${RODINIA_OPENCL_LINK_DIRS}
    LINK_OPTIONS ${RODINIA_OPENCL_RPATH_OPTIONS}
  )
  target_compile_definitions(${target} PRIVATE CL_TARGET_OPENCL_VERSION=200)
  if(RODINIA_OPENCL_FORCE_CPU)
    target_compile_definitions(${target} PRIVATE RODINIA_OPENCL_FORCE_CPU=1)
  endif()
endfunction()

option(RODINIA_OPENCL_HYBRIDSORT_VERIFY "Enable hybrid sort verification" OFF)
option(RODINIA_OPENCL_HYBRIDSORT_OUTPUT "Enable hybrid sort output" OFF)
option(RODINIA_OPENCL_HYBRIDSORT_TIMER "Enable hybrid sort timing output" OFF)
option(RODINIA_OPENCL_FORCE_CPU "Force OpenCL CPU device selection" OFF)
set(RODINIA_OPENCL_HYBRIDSORT_HISTO_WG_SIZE_0 "" CACHE STRING "Hybrid sort HISTO_WG_SIZE_0")
set(RODINIA_OPENCL_HYBRIDSORT_BUCKET_WG_SIZE_0 "" CACHE STRING "Hybrid sort BUCKET_WG_SIZE_0")
set(RODINIA_OPENCL_HYBRIDSORT_BUCKET_WG_SIZE_1 "" CACHE STRING "Hybrid sort BUCKET_WG_SIZE_1")
set(RODINIA_OPENCL_HYBRIDSORT_MERGE_WG_SIZE_0 "" CACHE STRING "Hybrid sort MERGE_WG_SIZE_0")
set(RODINIA_OPENCL_HYBRIDSORT_MERGE_WG_SIZE_1 "" CACHE STRING "Hybrid sort MERGE_WG_SIZE_1")

set(RODINIA_OPENCL_HYBRIDSORT_DEFINES "")
if(RODINIA_OPENCL_HYBRIDSORT_VERIFY)
  list(APPEND RODINIA_OPENCL_HYBRIDSORT_DEFINES VERIFY)
endif()
if(RODINIA_OPENCL_HYBRIDSORT_OUTPUT)
  list(APPEND RODINIA_OPENCL_HYBRIDSORT_DEFINES OUTPUT)
endif()
if(RODINIA_OPENCL_HYBRIDSORT_TIMER)
  list(APPEND RODINIA_OPENCL_HYBRIDSORT_DEFINES TIMER)
endif()
if(RODINIA_OPENCL_HYBRIDSORT_HISTO_WG_SIZE_0)
  list(APPEND RODINIA_OPENCL_HYBRIDSORT_DEFINES HISTO_WG_SIZE_0=${RODINIA_OPENCL_HYBRIDSORT_HISTO_WG_SIZE_0})
endif()
if(RODINIA_OPENCL_HYBRIDSORT_BUCKET_WG_SIZE_0)
  list(APPEND RODINIA_OPENCL_HYBRIDSORT_DEFINES BUCKET_WG_SIZE_0=${RODINIA_OPENCL_HYBRIDSORT_BUCKET_WG_SIZE_0})
endif()
if(RODINIA_OPENCL_HYBRIDSORT_BUCKET_WG_SIZE_1)
  list(APPEND RODINIA_OPENCL_HYBRIDSORT_DEFINES BUCKET_WG_SIZE_1=${RODINIA_OPENCL_HYBRIDSORT_BUCKET_WG_SIZE_1})
endif()
if(RODINIA_OPENCL_HYBRIDSORT_MERGE_WG_SIZE_0)
  list(APPEND RODINIA_OPENCL_HYBRIDSORT_DEFINES MERGE_WG_SIZE_0=${RODINIA_OPENCL_HYBRIDSORT_MERGE_WG_SIZE_0})
endif()
if(RODINIA_OPENCL_HYBRIDSORT_MERGE_WG_SIZE_1)
  list(APPEND RODINIA_OPENCL_HYBRIDSORT_DEFINES MERGE_WG_SIZE_1=${RODINIA_OPENCL_HYBRIDSORT_MERGE_WG_SIZE_1})
endif()

set(RODINIA_OPENCL_CXX_SOURCES
  backprop/backprop.c
  backprop/facetrain.c
  backprop/imagenet.c
  kmeans/cluster.c
  kmeans/getopt.c
  kmeans/read_input.c
  kmeans/kmeans_clustering.c
  kmeans/rmse.c
  lud/common/common.c
  nw/nw.c
)
set_source_files_properties(${RODINIA_OPENCL_CXX_SOURCES} PROPERTIES LANGUAGE CXX)

rodinia_add_opencl_executable(opencl_backprop
  BACKEND opencl
  OUTPUT_NAME backprop
  SOURCES
    backprop/backprop_ocl.cpp
    backprop/backprop.c
    backprop/facetrain.c
    backprop/imagenet.c
  LIBS
    OpenCL::OpenCL
    ${RODINIA_OPENMP_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_opencl_executable(opencl_bfs
  BACKEND opencl
  OUTPUT_NAME bfs
  SOURCES
    bfs/bfs.cpp
    bfs/timer.cc
  LIBS
    OpenCL::OpenCL
    ${RODINIA_OPENMP_LIBS}
)

rodinia_add_opencl_executable(opencl_bplustree
  BACKEND opencl
  OUTPUT_NAME b+tree.out
  SOURCES
    b+tree/main.c
    b+tree/kernel/kernel_gpu_opencl_wrapper.c
    b+tree/kernel/kernel_gpu_opencl_wrapper_2.c
    b+tree/util/timer/timer.c
    b+tree/util/num/num.c
    b+tree/util/opencl/opencl.c
  LIBS
    OpenCL::OpenCL
    ${RODINIA_MATH_LIB}
)

rodinia_add_opencl_executable(opencl_cfd
  BACKEND opencl
  OUTPUT_NAME euler3d
  SOURCES cfd/euler3d.cpp
  LIBS OpenCL::OpenCL
)

rodinia_add_opencl_executable(opencl_dwt2d
  BACKEND opencl
  OUTPUT_NAME dwt2d
  SOURCES
    dwt2d/main.cpp
    dwt2d/components.cpp
  LIBS OpenCL::OpenCL
)

rodinia_add_opencl_executable(opencl_gaussian
  BACKEND opencl
  OUTPUT_NAME gaussian
  SOURCES
    gaussian/clutils.cpp
    gaussian/gaussianElim.cpp
    gaussian/utils.cpp
  LIBS
    OpenCL::OpenCL
    ${RODINIA_OPENMP_LIBS}
)

rodinia_add_opencl_executable(opencl_heartwall
  BACKEND opencl
  OUTPUT_NAME heartwall
  SOURCES
    heartwall/main.c
    heartwall/kernel/kernel_gpu_opencl_wrapper.c
    heartwall/util/avi/avilib.c
    heartwall/util/avi/avimod.c
    heartwall/util/file/file.c
    heartwall/util/timer/timer.c
    heartwall/util/opencl/opencl.c
  LIBS
    OpenCL::OpenCL
    ${RODINIA_MATH_LIB}
)

rodinia_add_opencl_executable(opencl_hotspot
  BACKEND opencl
  OUTPUT_NAME hotspot
  SOURCES
    hotspot/OpenCL_helper_library.c
    hotspot/hotspot.c
  LIBS OpenCL::OpenCL
)

rodinia_add_opencl_executable(opencl_hotspot3d
  BACKEND opencl
  OUTPUT_NAME 3D
  SOURCES
    hotspot3D/3D.c
    hotspot3D/CL_helper.c
  LIBS
    OpenCL::OpenCL
    ${RODINIA_MATH_LIB}
)

rodinia_add_opencl_executable(opencl_hybridsort
  BACKEND opencl
  OUTPUT_NAME hybridsort
  SOURCES
    hybridsort/hybridsort.c
    hybridsort/bucketsort.c
    hybridsort/mergesort.c
  DEFINES ${RODINIA_OPENCL_HYBRIDSORT_DEFINES}
  COMPILE_OPTIONS -fcommon
  LIBS
    OpenCL::OpenCL
    ${RODINIA_MATH_LIB}
)

rodinia_add_opencl_executable(opencl_kmeans
  BACKEND opencl
  OUTPUT_NAME kmeans
  SOURCES
    kmeans/kmeans.cpp
    kmeans/cluster.c
    kmeans/getopt.c
    kmeans/read_input.c
    kmeans/kmeans_clustering.c
    kmeans/rmse.c
  LIBS
    OpenCL::OpenCL
    ${RODINIA_OPENMP_LIBS}
)

rodinia_add_opencl_executable(opencl_lavaMD
  BACKEND opencl
  OUTPUT_NAME lavaMD
  SOURCES
    lavaMD/main.c
    lavaMD/kernel/kernel_gpu_opencl_wrapper.c
    lavaMD/util/num/num.c
    lavaMD/util/timer/timer.c
    lavaMD/util/opencl/opencl.c
  LIBS
    OpenCL::OpenCL
    ${RODINIA_MATH_LIB}
)

include(${CMAKE_SOURCE_DIR}/cmake/MeschachSources.cmake)
set(RODINIA_OPENCL_MESCHACH_SOURCES)
foreach(source_file IN LISTS RODINIA_MESCHACH_SOURCE_FILES)
  list(APPEND RODINIA_OPENCL_MESCHACH_SOURCES "leukocyte/meschach_lib/${source_file}")
endforeach()
add_library(opencl_leukocyte_meschach STATIC ${RODINIA_OPENCL_MESCHACH_SOURCES})
target_include_directories(opencl_leukocyte_meschach PRIVATE leukocyte/meschach_lib)

rodinia_add_opencl_executable(opencl_leukocyte
  BACKEND opencl
  OUTPUT_NAME leukocyte
  SOURCES
    leukocyte/OpenCL/detect_main.c
    leukocyte/OpenCL/avilib.c
    leukocyte/OpenCL/find_ellipse.c
    leukocyte/OpenCL/find_ellipse_opencl.c
    leukocyte/OpenCL/track_ellipse.c
    leukocyte/OpenCL/track_ellipse_opencl.c
    leukocyte/OpenCL/misc_math.c
    leukocyte/OpenCL/OpenCL_helper_library.c
  INCLUDES
    leukocyte/OpenCL
    leukocyte/meschach_lib
  LIBS
    opencl_leukocyte_meschach
    OpenCL::OpenCL
    ${RODINIA_MATH_LIB}
)

rodinia_add_opencl_executable(opencl_lud
  BACKEND opencl
  OUTPUT_NAME lud
  SOURCES
    lud/ocl/lud.cpp
    lud/common/common.c
  INCLUDES lud/common
  LIBS OpenCL::OpenCL
)

rodinia_add_opencl_executable(opencl_myocyte
  BACKEND opencl
  OUTPUT_NAME myocyte.out
  SOURCES
    myocyte/main.c
    myocyte/kernel/kernel_gpu_opencl_wrapper.c
    myocyte/util/file/file.c
    myocyte/util/num/num.c
    myocyte/util/timer/timer.c
    myocyte/util/opencl/opencl.c
  LIBS
    OpenCL::OpenCL
    ${RODINIA_MATH_LIB}
)

rodinia_add_opencl_executable(opencl_nn
  BACKEND opencl
  OUTPUT_NAME nn
  SOURCES
    nn/nearestNeighbor.cpp
    nn/clutils.cpp
    nn/utils.cpp
  LIBS
    OpenCL::OpenCL
    ${RODINIA_OPENMP_LIBS}
)

rodinia_add_opencl_executable(opencl_nw
  BACKEND opencl
  OUTPUT_NAME nw
  SOURCES nw/nw.c
  LIBS OpenCL::OpenCL
)

rodinia_add_opencl_executable(opencl_particlefilter_naive
  BACKEND opencl
  OUTPUT_NAME OCL_particlefilter_naive
  SOURCES particlefilter/ex_particle_OCL_naive_seq.cpp
  LIBS OpenCL::OpenCL
)

rodinia_add_opencl_executable(opencl_particlefilter_double
  BACKEND opencl
  OUTPUT_NAME OCL_particlefilter_double
  SOURCES particlefilter/ex_particle_OCL_double_seq.cpp
  LIBS OpenCL::OpenCL
)

rodinia_add_opencl_executable(opencl_particlefilter_single
  BACKEND opencl
  OUTPUT_NAME OCL_particlefilter_single
  SOURCES particlefilter/ex_particle_OCL_single_seq.cpp
  LIBS OpenCL::OpenCL
)


rodinia_add_opencl_executable(opencl_pathfinder
  BACKEND opencl
  OUTPUT_NAME pathfinder
  SOURCES
    pathfinder/main.cpp
    pathfinder/OpenCL.cpp
  LIBS OpenCL::OpenCL
)

rodinia_add_opencl_executable(opencl_srad
  BACKEND opencl
  OUTPUT_NAME srad
  SOURCES
    srad/main.c
    srad/kernel/kernel_gpu_opencl_wrapper.c
    srad/util/opencl/opencl.c
    srad/util/graphics/graphics.c
    srad/util/graphics/resize.c
    srad/util/timer/timer.c
  LIBS
    OpenCL::OpenCL
    ${RODINIA_MATH_LIB}
)

rodinia_add_opencl_executable(opencl_streamcluster
  BACKEND opencl
  OUTPUT_NAME streamcluster
  SOURCES streamcluster/streamcluster.cpp
  LIBS OpenCL::OpenCL
)
