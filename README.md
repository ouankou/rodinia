# Rodinia Benchmark Suite

This repository now builds with CMake. Legacy Makefile-based builds and configs have been removed.

## Requirements

- CMake 3.20+
- C/C++ compiler (GCC, Clang, or Intel oneAPI)
- OpenMP runtime for OpenMP and OpenCL benchmarks
- OpenCL SDK (headers + libs) for OpenCL benchmarks
- CUDA toolkit for CUDA benchmarks
- OpenGL (GL/GLU) for the mummergpu benchmarks

## Configure and Build

```bash
cmake -S . -B build
cmake --build build
```

Binaries are placed under `build/bin` by default, grouped per backend:

- `build/bin/omp`
- `build/bin/opencl`
- `build/bin/cuda`

You can override the output root:

```bash
cmake -S . -B build -DRODINIA_OUTPUT_ROOT=/path/to/bin
```

## Backend Options

Enable or disable backends as needed:

```bash
cmake -S . -B build \
  -DRODINIA_ENABLE_OMP_CPU=ON \
  -DRODINIA_ENABLE_OPENCL=ON \
  -DRODINIA_ENABLE_CUDA=OFF
```

For Clang OpenMP GPU offload targets that need explicit target flags, pass them
through `RODINIA_OMP_GPU_OFFLOAD_FLAGS`:

```bash
cmake -S . -B build-omp-gpu \
  -DRODINIA_ENABLE_OMP_CPU=OFF \
  -DRODINIA_ENABLE_OMP_GPU=ON \
  -DRODINIA_ENABLE_OPENCL=OFF \
  -DRODINIA_ENABLE_CUDA=OFF \
  -DRODINIA_OMP_GPU_OFFLOAD_FLAGS="-fopenmp-targets=amdgcn-amd-amdhsa --offload-arch=gfx906"
```

Repeat `--offload-arch` in that string to build one binary for multiple AMD
GPU architectures, such as `--offload-arch=gfx803 --offload-arch=gfx906`.

CUDA-specific configuration:

- `CMAKE_CUDA_ARCHITECTURES` controls GPU architectures (recommended).
- `RODINIA_CUDA_SAMPLES_INCLUDE_DIR` sets the CUDA samples include directory
  (needed for `cuda/cfd` and `cuda/hybridsort` which include `helper_cuda.h`).
- `RODINIA_ENABLE_CUDA_BACKEND=OFF` skips building `cuda/` benchmarks while
  still enabling CUDA for OpenMP `mummergpu`.

Example:

```bash
cmake -S . -B build \
  -DRODINIA_ENABLE_CUDA=ON \
  -DCMAKE_CUDA_ARCHITECTURES=80 \
  -DRODINIA_CUDA_SAMPLES_INCLUDE_DIR=/usr/local/cuda/samples/common/inc
```

## Data

Input datasets are downloaded on demand into `data/` (ignored by git). Run:

```bash
scripts/generate_data.sh
```

The script fetches the full data bundle from the upstream Rodinia data
repository; override with `RODINIA_DATA_URL` or `--url`. See `data/README.md`
for details.

## Notes

- OpenCL benchmarks include `omp.h` in several places; OpenMP must be available.
- If a dependency is missing, CMake will skip the corresponding backend or target
  and print a warning during configuration.
- CUDA 12+ toolkits are supported; CUDA benchmarks no longer rely on legacy
  texture references.
