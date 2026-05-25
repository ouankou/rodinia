cmake_minimum_required(VERSION 3.20)

project(rodinia LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(RODINIA_ENABLE_OPENMP "Build OpenMP benchmarks" ON)
option(RODINIA_ENABLE_OMP_GPU "Build OpenMP GPU offload benchmarks" OFF)
option(RODINIA_ENABLE_OPENCL "Build OpenCL benchmarks" ON)
option(RODINIA_ENABLE_CUDA "Build CUDA benchmarks" ON)
option(RODINIA_ENABLE_CUDA_BACKEND "Build CUDA benchmarks under cuda/" ON)
option(RODINIA_ENABLE_MUMMERGPU "Build mummergpu benchmarks" ON)

set(RODINIA_OUTPUT_ROOT "${CMAKE_BINARY_DIR}/bin" CACHE PATH "Output root for benchmark binaries")
set(RODINIA_CUDA_SAMPLES_INCLUDE_DIR "" CACHE PATH "Path to CUDA samples common/inc for helper_cuda.h")
set(RODINIA_OMP_GPU_OFFLOAD_ARCH "" CACHE STRING "OpenMP GPU offload architecture passed as --offload-arch=<value>; leave empty to auto-detect NVIDIA GPUs when possible")

find_package(Threads)

if(RODINIA_ENABLE_OPENMP OR RODINIA_ENABLE_OPENCL OR RODINIA_ENABLE_OMP_GPU)
  find_package(OpenMP)
  if(RODINIA_ENABLE_OPENMP AND NOT OpenMP_FOUND)
    message(WARNING "OpenMP not found; disabling OpenMP benchmarks.")
    set(RODINIA_ENABLE_OPENMP OFF)
  endif()
  if(RODINIA_ENABLE_OMP_GPU AND NOT OpenMP_FOUND)
    message(FATAL_ERROR "OpenMP not found; RODINIA_ENABLE_OMP_GPU requires an OpenMP offload-capable compiler.")
  endif()
endif()

if(RODINIA_ENABLE_OPENCL)
  if(NOT OpenMP_FOUND)
    message(WARNING "OpenMP not found; disabling OpenCL benchmarks that rely on omp.h.")
    set(RODINIA_ENABLE_OPENCL OFF)
  else()
    find_package(OpenCL)
    if(NOT OpenCL_FOUND)
      message(WARNING "OpenCL not found; disabling OpenCL benchmarks.")
      set(RODINIA_ENABLE_OPENCL OFF)
    endif()
  endif()
endif()

if(RODINIA_ENABLE_CUDA)
  include(CheckLanguage)
  check_language(CUDA)
  if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)
    set(CMAKE_CUDA_STANDARD 11)
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)
    find_package(CUDAToolkit)
    if(NOT CUDAToolkit_FOUND)
      message(WARNING "CUDAToolkit not found; disabling CUDA benchmarks.")
      set(RODINIA_ENABLE_CUDA OFF)
    endif()
  else()
    message(WARNING "CUDA compiler not available; disabling CUDA benchmarks.")
    set(RODINIA_ENABLE_CUDA OFF)
  endif()
endif()

if(RODINIA_ENABLE_CUDA AND NOT RODINIA_CUDA_SAMPLES_INCLUDE_DIR)
  find_path(RODINIA_CUDA_SAMPLES_INCLUDE_DIR
    NAMES helper_cuda.h
    PATHS
      "${CUDAToolkit_ROOT}"
      "$ENV{CUDA_PATH}"
      "$ENV{CUDA_HOME}"
    PATH_SUFFIXES
      "samples/common/inc"
      "extras/demo_suite"
  )
endif()

include(cmake/RodiniaHelpers.cmake)

if(RODINIA_ENABLE_OPENMP)
  add_subdirectory(openmp)
endif()

if(RODINIA_ENABLE_OMP_GPU)
  add_subdirectory(omp-gpu)
endif()

if(RODINIA_ENABLE_OPENCL)
  add_subdirectory(opencl)
endif()

if(RODINIA_ENABLE_CUDA AND RODINIA_ENABLE_CUDA_BACKEND)
  add_subdirectory(cuda)
endif()
