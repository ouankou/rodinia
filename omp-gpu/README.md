# OpenMP GPU Offload

This backend contains OpenMP target-offload variants selected from
NeoRodinia-Old commit `73124c1`. It intentionally includes only variants that
can be built as regular OpenMP offload executables in this CMake tree.

Selected benchmarks:

- backprop
- bfs
- b+tree
- cfd
- heartwall
- hotspot
- hotspot3D
- kmeans
- lavaMD
- leukocyte
- lud
- myocyte
- nn
- nw
- particlefilter
- pathfinder
- srad
- streamcluster

Configure with LLVM OpenMP offload:

```sh
cmake -S . -B build-omp-gpu \
  -DRODINIA_ENABLE_OPENMP=OFF \
  -DRODINIA_ENABLE_OPENCL=OFF \
  -DRODINIA_ENABLE_CUDA=OFF \
  -DRODINIA_ENABLE_CUDA_BACKEND=OFF \
  -DRODINIA_ENABLE_OMP_GPU=ON \
  -DCMAKE_C_COMPILER=/path/to/clang \
  -DCMAKE_CXX_COMPILER=/path/to/clang++
cmake --build build-omp-gpu -j32
```

When `RODINIA_OMP_GPU_OFFLOAD_ARCH` is empty, CMake tries to detect the first
NVIDIA compute capability with `nvidia-smi` and passes the corresponding
`--offload-arch=sm_XX` to Clang. Set `RODINIA_OMP_GPU_OFFLOAD_ARCH` explicitly
to override that detection.

Run the selected suite:

```sh
scripts/run_omp_gpu.sh --build-dir build-omp-gpu
```

The runner defaults to `OMP_TARGET_OFFLOAD=MANDATORY` and writes generated
outputs under the build tree. Benchmark-generated files should be written below
`RODINIA_OUTPUT_DIR` or `RODINIA_BENCH_OUTPUT_DIR`, which the runner sets to a
per-benchmark subdirectory of `<build-dir>/run_outputs/omp-gpu`.
