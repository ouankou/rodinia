set(RODINIA_OPENMP_AVAILABLE OFF)
set(RODINIA_OPENMP_LIBS "")
if(OpenMP_C_FOUND)
  set(RODINIA_OPENMP_AVAILABLE ON)
  list(APPEND RODINIA_OPENMP_LIBS OpenMP::OpenMP_C)
endif()
if(OpenMP_CXX_FOUND)
  set(RODINIA_OPENMP_AVAILABLE ON)
  list(APPEND RODINIA_OPENMP_LIBS OpenMP::OpenMP_CXX)
endif()

set(RODINIA_MATH_LIB m)
set(RODINIA_CUDA_LIBS CUDA::cudart)
if(TARGET CUDA::cuda_driver)
  list(APPEND RODINIA_CUDA_LIBS CUDA::cuda_driver)
endif()

option(RODINIA_CUDA_HYBRIDSORT_VERIFY "Enable hybrid sort verification" OFF)
option(RODINIA_CUDA_HYBRIDSORT_OUTPUT "Enable hybrid sort output" OFF)
option(RODINIA_CUDA_HYBRIDSORT_TIMER "Enable hybrid sort timing output" OFF)
set(RODINIA_CUDA_HYBRIDSORT_HISTO_WG_SIZE_0 "" CACHE STRING "Hybrid sort HISTO_WG_SIZE_0")
set(RODINIA_CUDA_HYBRIDSORT_BUCKET_WG_SIZE_0 "" CACHE STRING "Hybrid sort BUCKET_WG_SIZE_0")
set(RODINIA_CUDA_HYBRIDSORT_BUCKET_WG_SIZE_1 "" CACHE STRING "Hybrid sort BUCKET_WG_SIZE_1")
set(RODINIA_CUDA_HYBRIDSORT_MERGE_WG_SIZE_0 "" CACHE STRING "Hybrid sort MERGE_WG_SIZE_0")
set(RODINIA_CUDA_HYBRIDSORT_MERGE_WG_SIZE_1 "" CACHE STRING "Hybrid sort MERGE_WG_SIZE_1")

set(RODINIA_CUDA_HYBRIDSORT_DEFINES "")
if(RODINIA_CUDA_HYBRIDSORT_VERIFY)
  list(APPEND RODINIA_CUDA_HYBRIDSORT_DEFINES VERIFY)
endif()
if(RODINIA_CUDA_HYBRIDSORT_OUTPUT)
  list(APPEND RODINIA_CUDA_HYBRIDSORT_DEFINES OUTPUT)
endif()
if(RODINIA_CUDA_HYBRIDSORT_TIMER)
  list(APPEND RODINIA_CUDA_HYBRIDSORT_DEFINES TIMER)
endif()
if(RODINIA_CUDA_HYBRIDSORT_HISTO_WG_SIZE_0)
  list(APPEND RODINIA_CUDA_HYBRIDSORT_DEFINES HISTO_WG_SIZE_0=${RODINIA_CUDA_HYBRIDSORT_HISTO_WG_SIZE_0})
endif()
if(RODINIA_CUDA_HYBRIDSORT_BUCKET_WG_SIZE_0)
  list(APPEND RODINIA_CUDA_HYBRIDSORT_DEFINES BUCKET_WG_SIZE_0=${RODINIA_CUDA_HYBRIDSORT_BUCKET_WG_SIZE_0})
endif()
if(RODINIA_CUDA_HYBRIDSORT_BUCKET_WG_SIZE_1)
  list(APPEND RODINIA_CUDA_HYBRIDSORT_DEFINES BUCKET_WG_SIZE_1=${RODINIA_CUDA_HYBRIDSORT_BUCKET_WG_SIZE_1})
endif()
if(RODINIA_CUDA_HYBRIDSORT_MERGE_WG_SIZE_0)
  list(APPEND RODINIA_CUDA_HYBRIDSORT_DEFINES MERGE_WG_SIZE_0=${RODINIA_CUDA_HYBRIDSORT_MERGE_WG_SIZE_0})
endif()
if(RODINIA_CUDA_HYBRIDSORT_MERGE_WG_SIZE_1)
  list(APPEND RODINIA_CUDA_HYBRIDSORT_DEFINES MERGE_WG_SIZE_1=${RODINIA_CUDA_HYBRIDSORT_MERGE_WG_SIZE_1})
endif()

option(RODINIA_CUDA_HUFFMAN_TESTING "Enable Huffman TESTING macro" OFF)
option(RODINIA_CUDA_HUFFMAN_CACHECWLUT "Enable Huffman CACHECWLUT macro" OFF)

set(RODINIA_CUDA_HELPER_AVAILABLE OFF)
if(RODINIA_CUDA_SAMPLES_INCLUDE_DIR AND EXISTS "${RODINIA_CUDA_SAMPLES_INCLUDE_DIR}/helper_cuda.h")
  set(RODINIA_CUDA_HELPER_AVAILABLE ON)
endif()
if(NOT RODINIA_CUDA_HELPER_AVAILABLE)
  message(WARNING "CUDA samples helper headers not found; skipping cuda/cfd and cuda/hybridsort.")
endif()

if(RODINIA_OPENMP_AVAILABLE)
  rodinia_add_cuda_executable(cuda_backprop
    BACKEND cuda
    OUTPUT_NAME backprop
    SOURCES
      backprop/backprop.c
      backprop/backprop_cuda.cu
      backprop/facetrain.c
      backprop/imagenet.c
    LIBS
      ${RODINIA_CUDA_LIBS}
      ${RODINIA_OPENMP_LIBS}
      ${RODINIA_MATH_LIB}
  )
else()
  message(WARNING "OpenMP not found; skipping cuda/backprop.")
endif()

rodinia_add_cuda_executable(cuda_bfs
  BACKEND cuda
  OUTPUT_NAME bfs
  SOURCES bfs/bfs.cu
  LIBS ${RODINIA_CUDA_LIBS}
)

rodinia_add_cuda_executable(cuda_bplustree
  BACKEND cuda
  OUTPUT_NAME b+tree.out
  SOURCES
    b+tree/main.c
    b+tree/kernel/kernel_gpu_cuda_wrapper.cu
    b+tree/kernel/kernel_gpu_cuda_wrapper_2.cu
    b+tree/util/timer/timer.c
    b+tree/util/num/num.c
    b+tree/util/cuda/cuda.cu
  LIBS
    ${RODINIA_CUDA_LIBS}
    ${RODINIA_MATH_LIB}
)

if(RODINIA_CUDA_HELPER_AVAILABLE)
  rodinia_add_cuda_executable(cuda_cfd_euler3d
    BACKEND cuda
    OUTPUT_NAME euler3d
    SOURCES cfd/euler3d.cu
    INCLUDES ${RODINIA_CUDA_SAMPLES_INCLUDE_DIR}
    LIBS ${RODINIA_CUDA_LIBS}
  )

  rodinia_add_cuda_executable(cuda_cfd_euler3d_double
    BACKEND cuda
    OUTPUT_NAME euler3d_double
    SOURCES cfd/euler3d_double.cu
    INCLUDES ${RODINIA_CUDA_SAMPLES_INCLUDE_DIR}
    LIBS ${RODINIA_CUDA_LIBS}
  )

  rodinia_add_cuda_executable(cuda_cfd_pre_euler3d
    BACKEND cuda
    OUTPUT_NAME pre_euler3d
    SOURCES cfd/pre_euler3d.cu
    INCLUDES ${RODINIA_CUDA_SAMPLES_INCLUDE_DIR}
    LIBS ${RODINIA_CUDA_LIBS}
  )

  rodinia_add_cuda_executable(cuda_cfd_pre_euler3d_double
    BACKEND cuda
    OUTPUT_NAME pre_euler3d_double
    SOURCES cfd/pre_euler3d_double.cu
    INCLUDES ${RODINIA_CUDA_SAMPLES_INCLUDE_DIR}
    LIBS ${RODINIA_CUDA_LIBS}
  )
endif()

rodinia_add_cuda_executable(cuda_dwt2d
  BACKEND cuda
  OUTPUT_NAME dwt2d
  SOURCES
    dwt2d/main.cu
    dwt2d/dwt.cu
    dwt2d/components.cu
    dwt2d/dwt_cuda/fdwt53.cu
    dwt2d/dwt_cuda/fdwt97.cu
    dwt2d/dwt_cuda/common.cu
    dwt2d/dwt_cuda/rdwt97.cu
    dwt2d/dwt_cuda/rdwt53.cu
  DEFINES OUTPUT
  LIBS
    ${RODINIA_CUDA_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_cuda_executable(cuda_gaussian
  BACKEND cuda
  OUTPUT_NAME gaussian
  SOURCES gaussian/gaussian.cu
  LIBS ${RODINIA_CUDA_LIBS}
)

rodinia_add_cuda_executable(cuda_heartwall
  BACKEND cuda
  OUTPUT_NAME heartwall
  SOURCES
    heartwall/main.cu
    heartwall/AVI/avilib.c
    heartwall/AVI/avimod.c
  INCLUDES heartwall/AVI
  LIBS
    ${RODINIA_CUDA_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_cuda_executable(cuda_hotspot
  BACKEND cuda
  OUTPUT_NAME hotspot
  SOURCES hotspot/hotspot.cu
  LIBS ${RODINIA_CUDA_LIBS}
)

rodinia_add_cuda_executable(cuda_hotspot3d
  BACKEND cuda
  OUTPUT_NAME 3D
  SOURCES hotspot3D/3D.cu
  LIBS ${RODINIA_CUDA_LIBS}
)

set(RODINIA_CUDA_HUFFMAN_DEFINES "")
if(RODINIA_CUDA_HUFFMAN_TESTING)
  list(APPEND RODINIA_CUDA_HUFFMAN_DEFINES TESTING)
endif()
if(RODINIA_CUDA_HUFFMAN_CACHECWLUT)
  list(APPEND RODINIA_CUDA_HUFFMAN_DEFINES CACHECWLUT)
endif()

rodinia_add_cuda_executable(cuda_huffman
  BACKEND cuda
  OUTPUT_NAME pavle
  SOURCES
    huffman/main_test_cu.cu
    huffman/stats_logger.cpp
    huffman/cpuencode.cpp
  INCLUDES huffman
  DEFINES ${RODINIA_CUDA_HUFFMAN_DEFINES}
  LIBS ${RODINIA_CUDA_LIBS}
)

if(RODINIA_CUDA_HELPER_AVAILABLE)
  rodinia_add_cuda_executable(cuda_hybridsort
    BACKEND cuda
    OUTPUT_NAME hybridsort
    SOURCES
      hybridsort/main.cu
      hybridsort/bucketsort.cu
      hybridsort/mergesort.cu
    INCLUDES ${RODINIA_CUDA_SAMPLES_INCLUDE_DIR}
    DEFINES ${RODINIA_CUDA_HYBRIDSORT_DEFINES}
    LIBS ${RODINIA_CUDA_LIBS}
  )
endif()

if(RODINIA_OPENMP_AVAILABLE)
  rodinia_add_cuda_executable(cuda_kmeans
    BACKEND cuda
    OUTPUT_NAME kmeans
    SOURCES
      kmeans/cluster.c
      kmeans/getopt.c
      kmeans/kmeans.c
      kmeans/kmeans_clustering.c
      kmeans/rmse.c
      kmeans/kmeans_cuda.cu
    LIBS
      ${RODINIA_CUDA_LIBS}
      ${RODINIA_OPENMP_LIBS}
      ${RODINIA_MATH_LIB}
  )
else()
  message(WARNING "OpenMP not found; skipping cuda/kmeans.")
endif()

rodinia_add_cuda_executable(cuda_lavaMD
  BACKEND cuda
  OUTPUT_NAME lavaMD
  SOURCES
    lavaMD/main.c
    lavaMD/kernel/kernel_gpu_cuda_wrapper.cu
    lavaMD/util/num/num.c
    lavaMD/util/timer/timer.c
    lavaMD/util/device/device.cu
  LIBS
    ${RODINIA_CUDA_LIBS}
    ${RODINIA_MATH_LIB}
)

include(${CMAKE_SOURCE_DIR}/cmake/MeschachSources.cmake)
set(RODINIA_CUDA_MESCHACH_SOURCES)
foreach(source_file IN LISTS RODINIA_MESCHACH_SOURCE_FILES)
  list(APPEND RODINIA_CUDA_MESCHACH_SOURCES "leukocyte/meschach_lib/${source_file}")
endforeach()
add_library(cuda_leukocyte_meschach STATIC ${RODINIA_CUDA_MESCHACH_SOURCES})
target_include_directories(cuda_leukocyte_meschach PRIVATE leukocyte/meschach_lib)

rodinia_add_cuda_executable(cuda_leukocyte
  BACKEND cuda
  OUTPUT_NAME leukocyte
  SOURCES
    leukocyte/CUDA/detect_main.c
    leukocyte/CUDA/avilib.c
    leukocyte/CUDA/find_ellipse.c
    leukocyte/CUDA/track_ellipse.c
    leukocyte/CUDA/misc_math.c
    leukocyte/CUDA/find_ellipse_kernel.cu
    leukocyte/CUDA/track_ellipse_kernel.cu
  INCLUDES
    leukocyte/CUDA
    leukocyte/meschach_lib
  LIBS
    cuda_leukocyte_meschach
    ${RODINIA_CUDA_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_cuda_executable(cuda_lud
  BACKEND cuda
  OUTPUT_NAME lud_cuda
  SOURCES
    lud/cuda/lud.cu
    lud/cuda/lud_kernel.cu
    lud/common/common.c
  INCLUDES lud/common
  DEFINES GPU_TIMER
  LIBS ${RODINIA_CUDA_LIBS}
)

if(RODINIA_ENABLE_MUMMERGPU)
  if(RODINIA_OPENMP_AVAILABLE)
    find_package(OpenGL)
    if(OpenGL_FOUND)
      add_library(cuda_mummergpu_lib STATIC
        mummergpu/src/mummergpu.cu
        mummergpu/src/mummergpu_gold.cpp
        mummergpu/src/suffix-tree.cpp
        mummergpu/src/PoolMalloc.cpp
      )
      target_include_directories(cuda_mummergpu_lib PRIVATE mummergpu/src)
      set_target_properties(cuda_mummergpu_lib PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
      target_link_libraries(cuda_mummergpu_lib PRIVATE ${RODINIA_OPENMP_LIBS})

      rodinia_add_cuda_executable(cuda_mummergpu
        BACKEND cuda
        OUTPUT_NAME mummergpu
        SOURCES mummergpu/src/mummergpu_main.cpp
        INCLUDES mummergpu/src
        LIBS
          cuda_mummergpu_lib
          ${RODINIA_CUDA_LIBS}
          ${RODINIA_OPENMP_LIBS}
          ${RODINIA_MATH_LIB}
          OpenGL::GL
      )
      if(TARGET OpenGL::GLU)
        target_link_libraries(cuda_mummergpu PRIVATE OpenGL::GLU)
      endif()
    else()
      message(WARNING "OpenGL not found; skipping cuda/mummergpu.")
    endif()
  else()
    message(WARNING "OpenMP not found; skipping cuda/mummergpu.")
  endif()
endif()

rodinia_add_cuda_executable(cuda_myocyte
  BACKEND cuda
  OUTPUT_NAME myocyte.out
  SOURCES myocyte/main.cu
  LIBS
    ${RODINIA_CUDA_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_cuda_executable(cuda_nn
  BACKEND cuda
  OUTPUT_NAME nn
  SOURCES nn/nn_cuda.cu
  LIBS ${RODINIA_CUDA_LIBS}
)

rodinia_add_cuda_executable(cuda_nw
  BACKEND cuda
  OUTPUT_NAME needle
  SOURCES nw/needle.cu
  LIBS ${RODINIA_CUDA_LIBS}
)

rodinia_add_cuda_executable(cuda_particlefilter_naive
  BACKEND cuda
  OUTPUT_NAME particlefilter_naive
  SOURCES particlefilter/ex_particle_CUDA_naive_seq.cu
  LIBS
    ${RODINIA_CUDA_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_cuda_executable(cuda_particlefilter_float
  BACKEND cuda
  OUTPUT_NAME particlefilter_float
  SOURCES particlefilter/ex_particle_CUDA_float_seq.cu
  LIBS
    ${RODINIA_CUDA_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_cuda_executable(cuda_pathfinder
  BACKEND cuda
  OUTPUT_NAME pathfinder
  SOURCES pathfinder/pathfinder.cu
  LIBS ${RODINIA_CUDA_LIBS}
)

rodinia_add_cuda_executable(cuda_srad_v1
  BACKEND cuda
  OUTPUT_NAME srad_v1
  SOURCES srad/srad_v1/main.cu
  LIBS
    ${RODINIA_CUDA_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_cuda_executable(cuda_srad_v2
  BACKEND cuda
  OUTPUT_NAME srad_v2
  SOURCES srad/srad_v2/srad.cu
  LIBS ${RODINIA_CUDA_LIBS}
)

rodinia_add_cuda_executable(cuda_streamcluster
  BACKEND cuda
  OUTPUT_NAME sc_gpu
  SOURCES
    streamcluster/streamcluster_cuda_cpu.cpp
    streamcluster/streamcluster_cuda.cu
    streamcluster/streamcluster_header.cu
  LIBS ${RODINIA_CUDA_LIBS}
)
