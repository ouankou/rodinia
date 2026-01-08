set(RODINIA_OPENMP_LIBS "")
if(OpenMP_C_FOUND)
  list(APPEND RODINIA_OPENMP_LIBS OpenMP::OpenMP_C)
endif()
if(OpenMP_CXX_FOUND)
  list(APPEND RODINIA_OPENMP_LIBS OpenMP::OpenMP_CXX)
endif()

set(RODINIA_MATH_LIB m)
set(RODINIA_OPENMP_CFD_BLOCK_LENGTH "8" CACHE STRING "CFD block_length for OpenMP builds")

rodinia_add_executable(openmp_backprop
  BACKEND omp
  OUTPUT_NAME backprop
  SOURCES
    backprop/backprop.c
    backprop/backprop_kernel.c
    backprop/facetrain.c
    backprop/imagenet.c
  LIBS
    ${RODINIA_OPENMP_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_executable(openmp_bfs
  BACKEND omp
  OUTPUT_NAME bfs
  SOURCES bfs/bfs.cpp
  LIBS ${RODINIA_OPENMP_LIBS}
)

rodinia_add_executable(openmp_bplustree
  BACKEND omp
  OUTPUT_NAME b+tree.out
  SOURCES
    b+tree/main.c
    b+tree/kernel/kernel_cpu.c
    b+tree/kernel/kernel_cpu_2.c
    b+tree/util/timer/timer.c
    b+tree/util/num/num.c
  LIBS
    ${RODINIA_OPENMP_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_executable(openmp_cfd_euler3d_cpu
  BACKEND omp
  OUTPUT_NAME euler3d_cpu
  SOURCES cfd/euler3d_cpu.cpp
  DEFINES block_length=${RODINIA_OPENMP_CFD_BLOCK_LENGTH}
  LIBS ${RODINIA_OPENMP_LIBS}
)

rodinia_add_executable(openmp_cfd_euler3d_cpu_double
  BACKEND omp
  OUTPUT_NAME euler3d_cpu_double
  SOURCES cfd/euler3d_cpu_double.cpp
  DEFINES block_length=${RODINIA_OPENMP_CFD_BLOCK_LENGTH}
  LIBS ${RODINIA_OPENMP_LIBS}
)

rodinia_add_executable(openmp_cfd_pre_euler3d_cpu
  BACKEND omp
  OUTPUT_NAME pre_euler3d_cpu
  SOURCES cfd/pre_euler3d_cpu.cpp
  DEFINES block_length=${RODINIA_OPENMP_CFD_BLOCK_LENGTH}
  LIBS ${RODINIA_OPENMP_LIBS}
)

rodinia_add_executable(openmp_cfd_pre_euler3d_cpu_double
  BACKEND omp
  OUTPUT_NAME pre_euler3d_cpu_double
  SOURCES cfd/pre_euler3d_cpu_double.cpp
  DEFINES block_length=${RODINIA_OPENMP_CFD_BLOCK_LENGTH}
  LIBS ${RODINIA_OPENMP_LIBS}
)

rodinia_add_executable(openmp_heartwall
  BACKEND omp
  OUTPUT_NAME heartwall
  SOURCES
    heartwall/main.c
    heartwall/AVI/avilib.c
    heartwall/AVI/avimod.c
  INCLUDES heartwall/AVI
  LIBS
    ${RODINIA_OPENMP_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_executable(openmp_hotspot
  BACKEND omp
  OUTPUT_NAME hotspot
  SOURCES hotspot/hotspot_openmp.cpp
  LIBS ${RODINIA_OPENMP_LIBS}
)

rodinia_add_executable(openmp_hotspot3d
  BACKEND omp
  OUTPUT_NAME 3D
  SOURCES hotspot3D/3D.c
  LIBS
    ${RODINIA_OPENMP_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_executable(openmp_kmeans
  BACKEND omp
  OUTPUT_NAME kmeans
  SOURCES
    kmeans/kmeans_openmp/cluster.c
    kmeans/kmeans_openmp/getopt.c
    kmeans/kmeans_openmp/kmeans.c
    kmeans/kmeans_openmp/kmeans_clustering.c
  LIBS
    ${RODINIA_OPENMP_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_executable(openmp_lavaMD
  BACKEND omp
  OUTPUT_NAME lavaMD
  SOURCES
    lavaMD/main.c
    lavaMD/kernel/kernel_cpu.c
    lavaMD/util/num/num.c
    lavaMD/util/timer/timer.c
  LIBS
    ${RODINIA_OPENMP_LIBS}
    ${RODINIA_MATH_LIB}
)

include(${CMAKE_SOURCE_DIR}/cmake/MeschachSources.cmake)
set(RODINIA_OPENMP_MESCHACH_SOURCES)
foreach(source_file IN LISTS RODINIA_MESCHACH_SOURCE_FILES)
  list(APPEND RODINIA_OPENMP_MESCHACH_SOURCES "leukocyte/meschach_lib/${source_file}")
endforeach()
add_library(openmp_leukocyte_meschach STATIC ${RODINIA_OPENMP_MESCHACH_SOURCES})
target_include_directories(openmp_leukocyte_meschach PRIVATE leukocyte/meschach_lib)

rodinia_add_executable(openmp_leukocyte
  BACKEND omp
  OUTPUT_NAME leukocyte
  SOURCES
    leukocyte/OpenMP/detect_main.c
    leukocyte/OpenMP/avilib.c
    leukocyte/OpenMP/find_ellipse.c
    leukocyte/OpenMP/track_ellipse.c
    leukocyte/OpenMP/misc_math.c
  INCLUDES
    leukocyte/OpenMP
    leukocyte/meschach_lib
  LIBS
    openmp_leukocyte_meschach
    ${RODINIA_OPENMP_LIBS}
    ${RODINIA_MATH_LIB}
    Threads::Threads
)

rodinia_add_executable(openmp_lud_omp
  BACKEND omp
  OUTPUT_NAME lud_omp
  SOURCES
    lud/omp/lud.c
    lud/omp/lud_omp.c
    lud/common/common.c
  INCLUDES lud/common
  LIBS
    ${RODINIA_OPENMP_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_executable(openmp_lud_base
  BACKEND omp
  OUTPUT_NAME lud_base
  SOURCES
    lud/base/lud.c
    lud/base/lud_base.c
    lud/common/common.c
  INCLUDES lud/common
  LIBS ${RODINIA_MATH_LIB}
)

if(RODINIA_ENABLE_CUDA AND RODINIA_ENABLE_MUMMERGPU)
  set(RODINIA_CUDA_LIBS CUDA::cudart)
  add_library(openmp_mummergpu_lib STATIC
    mummergpu/src/mummergpu.cu
    mummergpu/src/mummergpu_gold.cpp
    mummergpu/src/suffix-tree.cpp
    mummergpu/src/PoolMalloc.cpp
  )
  target_include_directories(openmp_mummergpu_lib PRIVATE mummergpu/src)
  set_target_properties(openmp_mummergpu_lib PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
  target_link_libraries(openmp_mummergpu_lib PRIVATE ${RODINIA_CUDA_LIBS} ${RODINIA_OPENMP_LIBS})

  rodinia_add_cuda_executable(openmp_mummergpu
    BACKEND omp
    OUTPUT_NAME mummergpu
    SOURCES mummergpu/src/mummergpu_main.cpp
    INCLUDES mummergpu/src
    LIBS
      openmp_mummergpu_lib
      ${RODINIA_CUDA_LIBS}
      ${RODINIA_OPENMP_LIBS}
      ${RODINIA_MATH_LIB}
  )
endif()

rodinia_add_executable(openmp_myocyte
  BACKEND omp
  OUTPUT_NAME myocyte.out
  SOURCES myocyte/main.c
  LIBS
    ${RODINIA_OPENMP_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_executable(openmp_nn
  BACKEND omp
  OUTPUT_NAME nn
  SOURCES nn/nn_openmp.c
  LIBS
    ${RODINIA_OPENMP_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_executable(openmp_nw
  BACKEND omp
  OUTPUT_NAME needle
  SOURCES nw/needle.cpp
  LIBS ${RODINIA_OPENMP_LIBS}
)

rodinia_add_executable(openmp_particlefilter
  BACKEND omp
  OUTPUT_NAME particle_filter
  SOURCES particlefilter/ex_particle_OPENMP_seq.c
  LIBS
    ${RODINIA_OPENMP_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_executable(openmp_pathfinder
  BACKEND omp
  OUTPUT_NAME pathfinder
  SOURCES pathfinder/pathfinder.cpp
  LIBS ${RODINIA_OPENMP_LIBS}
)

rodinia_add_executable(openmp_srad_v1
  BACKEND omp
  OUTPUT_NAME srad_v1
  SOURCES srad/srad_v1/main.c
  LIBS
    ${RODINIA_OPENMP_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_executable(openmp_srad_v2
  BACKEND omp
  OUTPUT_NAME srad_v2
  SOURCES srad/srad_v2/srad.cpp
  LIBS
    ${RODINIA_OPENMP_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_executable(openmp_streamcluster
  BACKEND omp
  OUTPUT_NAME sc_omp
  SOURCES streamcluster/streamcluster_omp.cpp
  LIBS ${RODINIA_OPENMP_LIBS}
)
