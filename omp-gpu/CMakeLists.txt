set(RODINIA_OMP_GPU_LIBS "")
if(OpenMP_C_FOUND)
  list(APPEND RODINIA_OMP_GPU_LIBS OpenMP::OpenMP_C)
endif()
if(OpenMP_CXX_FOUND)
  list(APPEND RODINIA_OMP_GPU_LIBS OpenMP::OpenMP_CXX)
endif()

set(RODINIA_MATH_LIB m)

set(RODINIA_OMP_GPU_EFFECTIVE_OFFLOAD_ARCH "${RODINIA_OMP_GPU_OFFLOAD_ARCH}")
if(NOT RODINIA_OMP_GPU_EFFECTIVE_OFFLOAD_ARCH AND CMAKE_C_COMPILER_ID MATCHES "Clang")
  find_program(RODINIA_NVIDIA_SMI nvidia-smi)
  if(RODINIA_NVIDIA_SMI)
    execute_process(
      COMMAND "${RODINIA_NVIDIA_SMI}" --query-gpu=compute_cap --format=csv,noheader
      OUTPUT_VARIABLE RODINIA_NVIDIA_COMPUTE_CAP
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(RODINIA_NVIDIA_COMPUTE_CAP)
      string(REGEX REPLACE "\n.*" "" RODINIA_NVIDIA_COMPUTE_CAP "${RODINIA_NVIDIA_COMPUTE_CAP}")
      string(REGEX REPLACE "\\." "" RODINIA_NVIDIA_COMPUTE_CAP "${RODINIA_NVIDIA_COMPUTE_CAP}")
      set(RODINIA_OMP_GPU_EFFECTIVE_OFFLOAD_ARCH "sm_${RODINIA_NVIDIA_COMPUTE_CAP}")
      message(STATUS "Detected NVIDIA OpenMP GPU offload architecture: ${RODINIA_OMP_GPU_EFFECTIVE_OFFLOAD_ARCH}")
    endif()
  endif()
endif()

set(RODINIA_OMP_GPU_OPTIONS "")
if(RODINIA_OMP_GPU_EFFECTIVE_OFFLOAD_ARCH)
  list(APPEND RODINIA_OMP_GPU_OPTIONS "--offload-arch=${RODINIA_OMP_GPU_EFFECTIVE_OFFLOAD_ARCH}")
endif()
if(RODINIA_OMP_GPU_OFFLOAD_FLAGS)
  separate_arguments(RODINIA_OMP_GPU_EXTRA_OPTIONS UNIX_COMMAND "${RODINIA_OMP_GPU_OFFLOAD_FLAGS}")
  if(RODINIA_OMP_GPU_EXTRA_OPTIONS)
    list(APPEND RODINIA_OMP_GPU_OPTIONS ${RODINIA_OMP_GPU_EXTRA_OPTIONS})
  endif()
endif()

set(RODINIA_OMP_GPU_C_OPENMP_OPTIONS "")
set(RODINIA_OMP_GPU_CXX_OPENMP_OPTIONS "")
if(OpenMP_C_FLAGS)
  separate_arguments(RODINIA_OMP_GPU_C_OPENMP_OPTIONS UNIX_COMMAND "${OpenMP_C_FLAGS}")
endif()
if(OpenMP_CXX_FLAGS)
  separate_arguments(RODINIA_OMP_GPU_CXX_OPENMP_OPTIONS UNIX_COMMAND "${OpenMP_CXX_FLAGS}")
endif()

function(rodinia_add_omp_gpu_executable target)
  rodinia_add_executable(${target} BACKEND omp-gpu ${ARGN})
  if(RODINIA_OMP_GPU_C_OPENMP_OPTIONS)
    target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:C>:${RODINIA_OMP_GPU_C_OPENMP_OPTIONS}>)
    target_link_options(${target} PRIVATE ${RODINIA_OMP_GPU_C_OPENMP_OPTIONS})
  endif()
  if(RODINIA_OMP_GPU_CXX_OPENMP_OPTIONS)
    target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${RODINIA_OMP_GPU_CXX_OPENMP_OPTIONS}>)
    target_link_options(${target} PRIVATE ${RODINIA_OMP_GPU_CXX_OPENMP_OPTIONS})
  endif()
  if(RODINIA_OMP_GPU_OPTIONS)
    target_compile_options(${target} PRIVATE ${RODINIA_OMP_GPU_OPTIONS})
    target_link_options(${target} PRIVATE ${RODINIA_OMP_GPU_OPTIONS})
  endif()
endfunction()

rodinia_add_omp_gpu_executable(omp_gpu_backprop
  OUTPUT_NAME backprop
  SOURCES
    backprop/backprop.c
    backprop/backprop_kernel.c
    backprop/facetrain.c
    backprop/imagenet.c
  LIBS
    ${RODINIA_OMP_GPU_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_omp_gpu_executable(omp_gpu_bfs
  OUTPUT_NAME bfs
  SOURCES bfs/bfs.cpp
  LIBS ${RODINIA_OMP_GPU_LIBS}
)

rodinia_add_omp_gpu_executable(omp_gpu_bplustree
  OUTPUT_NAME b+tree.out
  SOURCES
    b+tree/main.c
    b+tree/kernel/kernel_cpu.c
    b+tree/kernel/kernel_cpu_2.c
    b+tree/util/timer/timer.c
    b+tree/util/num/num.c
  LIBS
    ${RODINIA_OMP_GPU_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_omp_gpu_executable(omp_gpu_cfd_euler3d_cpu
  OUTPUT_NAME euler3d_cpu
  SOURCES cfd/euler3d_cpu.cpp
  LIBS ${RODINIA_OMP_GPU_LIBS}
)

rodinia_add_omp_gpu_executable(omp_gpu_cfd_euler3d_cpu_double
  OUTPUT_NAME euler3d_cpu_double
  SOURCES cfd/euler3d_cpu_double.cpp
  LIBS ${RODINIA_OMP_GPU_LIBS}
)

rodinia_add_omp_gpu_executable(omp_gpu_cfd_pre_euler3d_cpu
  OUTPUT_NAME pre_euler3d_cpu
  SOURCES cfd/pre_euler3d_cpu.cpp
  LIBS ${RODINIA_OMP_GPU_LIBS}
)

rodinia_add_omp_gpu_executable(omp_gpu_cfd_pre_euler3d_cpu_double
  OUTPUT_NAME pre_euler3d_cpu_double
  SOURCES cfd/pre_euler3d_cpu_double.cpp
  LIBS ${RODINIA_OMP_GPU_LIBS}
)

rodinia_add_omp_gpu_executable(omp_gpu_heartwall
  OUTPUT_NAME heartwall
  SOURCES
    heartwall/main.c
    heartwall/AVI/avilib.c
    heartwall/AVI/avimod.c
  INCLUDES heartwall/AVI
  LIBS
    ${RODINIA_OMP_GPU_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_omp_gpu_executable(omp_gpu_hotspot
  OUTPUT_NAME hotspot
  SOURCES hotspot/hotspot.cpp
  LIBS ${RODINIA_OMP_GPU_LIBS}
)

rodinia_add_omp_gpu_executable(omp_gpu_hotspot3d
  OUTPUT_NAME 3D
  SOURCES hotspot3D/3D.c
  LIBS
    ${RODINIA_OMP_GPU_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_omp_gpu_executable(omp_gpu_kmeans
  OUTPUT_NAME kmeans
  SOURCES
    kmeans/cluster.c
    kmeans/getopt.c
    kmeans/kmeans.c
    kmeans/kmeans_clustering.c
  LIBS
    ${RODINIA_OMP_GPU_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_omp_gpu_executable(omp_gpu_lavaMD
  OUTPUT_NAME lavaMD
  SOURCES
    lavaMD/main.c
    lavaMD/kernel/kernel_gpu.c
    lavaMD/util/num/num.c
    lavaMD/util/timer/timer.c
  LIBS
    ${RODINIA_OMP_GPU_LIBS}
    ${RODINIA_MATH_LIB}
)

include(${CMAKE_SOURCE_DIR}/cmake/MeschachSources.cmake)
set(RODINIA_OMP_GPU_MESCHACH_SOURCES)
foreach(source_file IN LISTS RODINIA_MESCHACH_SOURCE_FILES)
  list(APPEND RODINIA_OMP_GPU_MESCHACH_SOURCES "${CMAKE_SOURCE_DIR}/openmp/leukocyte/meschach_lib/${source_file}")
endforeach()
add_library(omp_gpu_leukocyte_meschach STATIC ${RODINIA_OMP_GPU_MESCHACH_SOURCES})
target_include_directories(omp_gpu_leukocyte_meschach PRIVATE ${CMAKE_SOURCE_DIR}/openmp/leukocyte/meschach_lib)

rodinia_add_omp_gpu_executable(omp_gpu_leukocyte
  OUTPUT_NAME leukocyte
  SOURCES
    leukocyte/OpenMP/detect_main.c
    leukocyte/OpenMP/avilib.c
    leukocyte/OpenMP/find_ellipse.c
    leukocyte/OpenMP/track_ellipse.c
    leukocyte/OpenMP/misc_math.c
  INCLUDES
    leukocyte/OpenMP
    ${CMAKE_SOURCE_DIR}/openmp/leukocyte/meschach_lib
  LIBS
    omp_gpu_leukocyte_meschach
    ${RODINIA_OMP_GPU_LIBS}
    ${RODINIA_MATH_LIB}
    Threads::Threads
)

rodinia_add_omp_gpu_executable(omp_gpu_lud
  OUTPUT_NAME lud_omp_gpu
  SOURCES
    lud/openmp-gpu/lud.c
    lud/openmp-gpu/lud_openmp_gpu.c
    lud/common/common.c
  INCLUDES lud/common
  LIBS
    ${RODINIA_OMP_GPU_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_omp_gpu_executable(omp_gpu_myocyte
  OUTPUT_NAME myocyte.out
  SOURCES myocyte/main.c
  LIBS
    ${RODINIA_OMP_GPU_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_omp_gpu_executable(omp_gpu_nn
  OUTPUT_NAME nn
  SOURCES nn/nn.c
  LIBS
    ${RODINIA_OMP_GPU_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_omp_gpu_executable(omp_gpu_nw
  OUTPUT_NAME needle
  SOURCES nw/needle.cpp
  LIBS ${RODINIA_OMP_GPU_LIBS}
)

rodinia_add_omp_gpu_executable(omp_gpu_particlefilter
  OUTPUT_NAME particle_filter
  SOURCES particlefilter/ex_particle_OPENMP_seq.c
  LIBS
    ${RODINIA_OMP_GPU_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_omp_gpu_executable(omp_gpu_pathfinder
  OUTPUT_NAME pathfinder
  SOURCES pathfinder/pathfinder.c
  LIBS ${RODINIA_OMP_GPU_LIBS}
)

rodinia_add_omp_gpu_executable(omp_gpu_srad_v1
  OUTPUT_NAME srad_v1
  SOURCES srad/srad_v1/main.c
  LIBS
    ${RODINIA_OMP_GPU_LIBS}
    ${RODINIA_MATH_LIB}
)

rodinia_add_omp_gpu_executable(omp_gpu_streamcluster
  OUTPUT_NAME sc_omp
  SOURCES streamcluster/streamcluster_omp.cpp
  LIBS ${RODINIA_OMP_GPU_LIBS}
)

rodinia_add_omp_gpu_executable(omp_gpu_srad_v2
  OUTPUT_NAME srad_v2
  SOURCES srad/srad_v2/srad.cpp
  LIBS
    ${RODINIA_OMP_GPU_LIBS}
    ${RODINIA_MATH_LIB}
)
