#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if [[ -d "${ROOT_DIR}/build-omp-clang-cuda" ]]; then
  BUILD_DIR="${ROOT_DIR}/build-omp-clang-cuda"
else
  BUILD_DIR="${ROOT_DIR}/build-omp-clang"
fi
OUT_DIR=""
default_threads() {
  getconf _NPROCESSORS_ONLN 2>/dev/null || nproc 2>/dev/null || echo 1
}
THREADS="${RODINIA_OPENMP_THREADS:-$(default_threads)}"
RUN_MUMMERGPU=1
ONLY=()

usage() {
  cat <<'EOF'
Usage: scripts/run_openmp.sh [options]

Options:
  --build-dir <path>   CMake build directory (default: build-omp-clang)
  --output-dir <path>  Output directory for generated files (default: <build-dir>/run_outputs/omp)
  --threads <n>        OpenMP thread count to use (default: online CPU count)
  --only <name>        Run only the named benchmark (repeatable)
  --skip-mummergpu     Skip mummergpu even if built
  -h, --help           Show this help

Benchmarks names (for --only):
  backprop bfs b+tree cfd cfd_double cfd_pre cfd_pre_double heartwall hotspot
  hotspot3d kmeans lavaMD leukocyte lud_omp lud_base myocyte nn nw particlefilter
  pathfinder streamcluster srad_v1 srad_v2 mummergpu
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-dir)
      BUILD_DIR="$2"
      shift
      ;;
    --output-dir)
      OUT_DIR="$2"
      shift
      ;;
    --threads)
      THREADS="$2"
      shift
      ;;
    --only)
      ONLY+=("$2")
      shift
      ;;
    --skip-mummergpu)
      RUN_MUMMERGPU=0
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown argument: $1" >&2
      usage >&2
      exit 1
      ;;
  esac
  shift
done

BUILD_DIR="$(cd "${BUILD_DIR}" && pwd)"
if [[ -z "${OUT_DIR}" ]]; then
  OUT_DIR="${BUILD_DIR}/run_outputs/omp"
fi
mkdir -p "${OUT_DIR}"
OUT_DIR="$(cd "${OUT_DIR}" && pwd)"
BIN_DIR="${BUILD_DIR}/bin/omp"
DATA_DIR="${ROOT_DIR}/data"
HAS_MUMMERGPU=0

if [[ ! -d "${BIN_DIR}" ]]; then
  echo "Missing OpenMP bin directory: ${BIN_DIR}" >&2
  exit 1
fi
if [[ -x "${BIN_DIR}/mummergpu" ]]; then
  HAS_MUMMERGPU=1
else
  RUN_MUMMERGPU=0
fi

should_run() {
  local name="$1"
  if [[ ${#ONLY[@]} -eq 0 ]]; then
    return 0
  fi
  for entry in "${ONLY[@]}"; do
    if [[ "${entry}" == "${name}" ]]; then
      return 0
    fi
  done
  return 1
}

require_bin() {
  local bin="$1"
  if [[ ! -x "${BIN_DIR}/${bin}" ]]; then
    echo "Missing binary: ${BIN_DIR}/${bin}" >&2
    exit 1
  fi
}

run_cmd() {
  local name="$1"
  local dir="$2"
  local cmd="$3"
  local bench_out="${OUT_DIR}/${name}"
  mkdir -p "${bench_out}"
  echo "==> ${name}"
  (cd "${dir}" && RODINIA_OUTPUT_DIR="${bench_out}" RODINIA_BENCH_OUTPUT_DIR="${bench_out}" OMP_NUM_THREADS="${THREADS}" bash -c "${cmd}" 2>&1 | tee "${bench_out}/run.log")
}

if [[ ! -f "${DATA_DIR}/.downloaded" ]]; then
  "${ROOT_DIR}/scripts/generate_data.sh"
fi

if should_run backprop; then
  require_bin backprop
  run_cmd backprop "${ROOT_DIR}/openmp/backprop" "${BIN_DIR}/backprop 65536"
fi

if should_run bfs; then
  require_bin bfs
  run_cmd bfs "${ROOT_DIR}/openmp/bfs" "${BIN_DIR}/bfs ${THREADS} ${DATA_DIR}/bfs/graph1MW_6.txt"
fi

if should_run b+tree; then
  require_bin b+tree.out
  run_cmd b+tree "${ROOT_DIR}/openmp/b+tree" \
    "${BIN_DIR}/b+tree.out cores ${THREADS} file ${DATA_DIR}/b+tree/mil.txt command ${DATA_DIR}/b+tree/command_k.txt"
fi

if should_run cfd; then
  require_bin euler3d_cpu
  run_cmd cfd "${ROOT_DIR}/openmp/cfd" "${BIN_DIR}/euler3d_cpu ${DATA_DIR}/cfd/fvcorr.domn.097K"
fi

if should_run cfd_double; then
  require_bin euler3d_cpu_double
  run_cmd cfd_double "${ROOT_DIR}/openmp/cfd" "${BIN_DIR}/euler3d_cpu_double ${DATA_DIR}/cfd/fvcorr.domn.097K"
fi

if should_run cfd_pre; then
  require_bin pre_euler3d_cpu
  run_cmd cfd_pre "${ROOT_DIR}/openmp/cfd" "${BIN_DIR}/pre_euler3d_cpu ${DATA_DIR}/cfd/fvcorr.domn.097K"
fi

if should_run cfd_pre_double; then
  require_bin pre_euler3d_cpu_double
  run_cmd cfd_pre_double "${ROOT_DIR}/openmp/cfd" "${BIN_DIR}/pre_euler3d_cpu_double ${DATA_DIR}/cfd/fvcorr.domn.097K"
fi

if should_run heartwall; then
  require_bin heartwall
  run_cmd heartwall "${ROOT_DIR}/openmp/heartwall" \
    "${BIN_DIR}/heartwall ${DATA_DIR}/heartwall/test.avi 20 ${THREADS}"
fi

if should_run hotspot3d; then
  require_bin 3D
  run_cmd hotspot3d "${ROOT_DIR}/openmp/hotspot3D" \
    "${BIN_DIR}/3D 512 8 100 ${DATA_DIR}/hotspot3D/power_512x8 ${DATA_DIR}/hotspot3D/temp_512x8 \${RODINIA_BENCH_OUTPUT_DIR}/hotspot3d.out"
fi

if should_run hotspot; then
  require_bin hotspot
  run_cmd hotspot "${ROOT_DIR}/openmp/hotspot" \
    "${BIN_DIR}/hotspot 512 512 2 ${THREADS} ${DATA_DIR}/hotspot/temp_512 ${DATA_DIR}/hotspot/power_512 \${RODINIA_BENCH_OUTPUT_DIR}/hotspot.out"
fi

if should_run kmeans; then
  require_bin kmeans
  run_cmd kmeans "${ROOT_DIR}/openmp/kmeans" \
    "${BIN_DIR}/kmeans -n ${THREADS} -i ${DATA_DIR}/kmeans/kdd_cup"
fi

if should_run lavaMD; then
  require_bin lavaMD
  run_cmd lavaMD "${ROOT_DIR}/openmp/lavaMD" "${BIN_DIR}/lavaMD -cores ${THREADS} -boxes1d 10"
fi

if should_run leukocyte; then
  require_bin leukocyte
  run_cmd leukocyte "${ROOT_DIR}/openmp/leukocyte" \
    "${BIN_DIR}/leukocyte 5 ${THREADS} ${DATA_DIR}/leukocyte/testfile.avi"
fi

if should_run lud_omp; then
  require_bin lud_omp
  run_cmd lud_omp "${ROOT_DIR}/openmp/lud" "${BIN_DIR}/lud_omp -n ${THREADS} -s 8000"
fi

if should_run lud_base; then
  require_bin lud_base
  run_cmd lud_base "${ROOT_DIR}/openmp/lud" "${BIN_DIR}/lud_base -i ${DATA_DIR}/lud/512.dat"
fi

if should_run myocyte; then
  require_bin myocyte.out
  run_cmd myocyte "${ROOT_DIR}/openmp/myocyte" "${BIN_DIR}/myocyte.out 100 32 1 ${THREADS}"
fi

if should_run nn; then
  require_bin nn
  run_cmd nn "${ROOT_DIR}/openmp/nn" "${BIN_DIR}/nn filelist_4 5 30 90"
fi

if should_run nw; then
  require_bin needle
  run_cmd nw "${ROOT_DIR}/openmp/nw" "${BIN_DIR}/needle 2048 10 ${THREADS}"
fi

if should_run particlefilter; then
  require_bin particle_filter
  run_cmd particlefilter "${ROOT_DIR}/openmp/particlefilter" \
    "${BIN_DIR}/particle_filter -x 128 -y 128 -z 10 -np 10000"
fi

if should_run pathfinder; then
  require_bin pathfinder
  run_cmd pathfinder "${ROOT_DIR}/openmp/pathfinder" \
    "${BIN_DIR}/pathfinder 100000 100 > \${RODINIA_BENCH_OUTPUT_DIR}/pathfinder.out"
fi

if should_run streamcluster; then
  require_bin sc_omp
  run_cmd streamcluster "${ROOT_DIR}/openmp/streamcluster" \
    "${BIN_DIR}/sc_omp 10 20 256 65536 65536 1000 none \${RODINIA_BENCH_OUTPUT_DIR}/streamcluster.txt ${THREADS}"
fi

if should_run srad_v1; then
  require_bin srad_v1
  run_cmd srad_v1 "${ROOT_DIR}/openmp/srad/srad_v1" \
    "${BIN_DIR}/srad_v1 100 0.5 502 458 ${THREADS}"
fi

if should_run srad_v2; then
  require_bin srad_v2
  run_cmd srad_v2 "${ROOT_DIR}/openmp/srad/srad_v2" \
    "${BIN_DIR}/srad_v2 2048 2048 0 127 0 127 ${THREADS} 0.5 2"
fi

if should_run mummergpu; then
  if [[ ${RUN_MUMMERGPU} -eq 1 ]]; then
    run_cmd mummergpu "${ROOT_DIR}/openmp/mummergpu" \
      "${BIN_DIR}/mummergpu -C ${DATA_DIR}/mummergpu/NC_003997.fna ${DATA_DIR}/mummergpu/NC_003997_q100bp.fna > \${RODINIA_BENCH_OUTPUT_DIR}/mummergpu.out"
  elif [[ ${HAS_MUMMERGPU} -eq 0 ]]; then
    echo "Skipping mummergpu: missing binary in ${BIN_DIR}; configure with CUDA to enable." >&2
  fi
fi
