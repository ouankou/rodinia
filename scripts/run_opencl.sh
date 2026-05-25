#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build-opencl"
OUT_DIR=""
ONLY=()

usage() {
  cat <<'USAGE'
Usage: scripts/run_opencl.sh [options]

Options:
  --build-dir <path>   CMake build directory (default: build-opencl)
  --output-dir <path>  Output directory for generated files (default: <build-dir>/run_outputs/opencl)
  --only <name>        Run only the named benchmark (repeatable)
  -h, --help           Show this help

Benchmarks names (for --only):
  backprop bfs b+tree cfd dwt2d_192 dwt2d_rgb gaussian heartwall hotspot hotspot3d
  hybridsort kmeans lavaMD leukocyte lud myocyte nn nw particlefilter_double
  particlefilter_naive particlefilter_single pathfinder srad streamcluster
USAGE
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
    --only)
      ONLY+=("$2")
      shift
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
  OUT_DIR="${BUILD_DIR}/run_outputs/opencl"
fi
mkdir -p "${OUT_DIR}"
OUT_DIR="$(cd "${OUT_DIR}" && pwd)"
BIN_DIR="${BUILD_DIR}/bin/opencl"
DATA_DIR="${ROOT_DIR}/data"

if [[ ! -d "${BIN_DIR}" ]]; then
  echo "Missing OpenCL bin directory: ${BIN_DIR}" >&2
  exit 1
fi

ONEAPI_COMPILER_LIB=""
if [[ -d /opt/intel/oneapi/compiler ]]; then
  ONEAPI_COMPILER_LIB="$(ls -d /opt/intel/oneapi/compiler/*/lib 2>/dev/null | sort -V | tail -1 || true)"
  if [[ -n "${ONEAPI_COMPILER_LIB}" ]]; then
    export LD_LIBRARY_PATH="${ONEAPI_COMPILER_LIB}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
  fi
fi

if [[ ! -f "${DATA_DIR}/.downloaded" ]]; then
  "${ROOT_DIR}/scripts/generate_data.sh"
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

require_data() {
  local path="$1"
  if [[ ! -f "${path}" ]]; then
    echo "Missing data file: ${path}" >&2
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
  (cd "${dir}" && RODINIA_OUTPUT_DIR="${bench_out}" RODINIA_BENCH_OUTPUT_DIR="${bench_out}" bash -c "${cmd}" 2>&1 | tee "${bench_out}/run.log")
}

if should_run backprop; then
  require_bin backprop
  run_cmd backprop "${ROOT_DIR}/opencl/backprop" "${BIN_DIR}/backprop 65536"
fi

if should_run bfs; then
  require_bin bfs
  require_data "${DATA_DIR}/bfs/graph1MW_6.txt"
  run_cmd bfs "${ROOT_DIR}/opencl/bfs" "${BIN_DIR}/bfs ${DATA_DIR}/bfs/graph1MW_6.txt"
fi

if should_run b+tree; then
  require_bin b+tree.out
  require_data "${DATA_DIR}/b+tree/mil.txt"
  require_data "${DATA_DIR}/b+tree/command_k.txt"
  run_cmd b+tree "${ROOT_DIR}/opencl/b+tree" \
    "${BIN_DIR}/b+tree.out file ${DATA_DIR}/b+tree/mil.txt command ${DATA_DIR}/b+tree/command_k.txt"
fi

if should_run cfd; then
  require_bin euler3d
  require_data "${DATA_DIR}/cfd/fvcorr.domn.097K"
  run_cmd cfd "${ROOT_DIR}/opencl/cfd" "${BIN_DIR}/euler3d ${DATA_DIR}/cfd/fvcorr.domn.097K -t gpu -d 0"
fi

if should_run dwt2d_192; then
  require_bin dwt2d
  require_data "${DATA_DIR}/dwt2d/192.bmp"
  run_cmd dwt2d_192 "${ROOT_DIR}/opencl/dwt2d" \
    "${BIN_DIR}/dwt2d ${DATA_DIR}/dwt2d/192.bmp -d 192x192 -f -5 -l 3"
fi

if should_run dwt2d_rgb; then
  require_bin dwt2d
  require_data "${DATA_DIR}/dwt2d/rgb.bmp"
  run_cmd dwt2d_rgb "${ROOT_DIR}/opencl/dwt2d" \
    "${BIN_DIR}/dwt2d ${DATA_DIR}/dwt2d/rgb.bmp -d 1024x1024 -f -5 -l 3"
fi

if should_run gaussian; then
  require_bin gaussian
  run_cmd gaussian "${ROOT_DIR}/opencl/gaussian" "${BIN_DIR}/gaussian -s 256"
fi

if should_run heartwall; then
  require_bin heartwall
  require_data "${DATA_DIR}/heartwall/test.avi"
  run_cmd heartwall "${ROOT_DIR}/opencl/heartwall" "${BIN_DIR}/heartwall 20"
fi

if should_run hotspot; then
  require_bin hotspot
  require_data "${DATA_DIR}/hotspot/temp_512"
  require_data "${DATA_DIR}/hotspot/power_512"
  run_cmd hotspot "${ROOT_DIR}/opencl/hotspot" \
    "${BIN_DIR}/hotspot 512 2 2 ${DATA_DIR}/hotspot/temp_512 ${DATA_DIR}/hotspot/power_512 \${RODINIA_BENCH_OUTPUT_DIR}/hotspot.out"
fi

if should_run hotspot3d; then
  require_bin 3D
  require_data "${DATA_DIR}/hotspot3D/power_512x8"
  require_data "${DATA_DIR}/hotspot3D/temp_512x8"
  run_cmd hotspot3d "${ROOT_DIR}/opencl/hotspot3D" \
    "${BIN_DIR}/3D 512 8 100 ${DATA_DIR}/hotspot3D/power_512x8 ${DATA_DIR}/hotspot3D/temp_512x8 \${RODINIA_BENCH_OUTPUT_DIR}/hotspot3d.out"
fi

if should_run hybridsort; then
  require_bin hybridsort
  run_cmd hybridsort "${ROOT_DIR}/opencl/hybridsort" "${BIN_DIR}/hybridsort r"
fi

if should_run kmeans; then
  require_bin kmeans
  require_data "${DATA_DIR}/kmeans/kdd_cup"
  run_cmd kmeans "${ROOT_DIR}/opencl/kmeans" "${BIN_DIR}/kmeans -o -i ${DATA_DIR}/kmeans/kdd_cup"
fi

if should_run lavaMD; then
  require_bin lavaMD
  run_cmd lavaMD "${ROOT_DIR}/opencl/lavaMD" "${BIN_DIR}/lavaMD -boxes1d 10"
fi

if should_run leukocyte; then
  require_bin leukocyte
  require_data "${DATA_DIR}/leukocyte/testfile.avi"
  run_cmd leukocyte "${ROOT_DIR}/opencl/leukocyte/OpenCL" \
    "${BIN_DIR}/leukocyte ${DATA_DIR}/leukocyte/testfile.avi 5"
fi

if should_run lud; then
  require_bin lud
  run_cmd lud "${ROOT_DIR}/opencl/lud/ocl" "${BIN_DIR}/lud -s 1024 -v"
fi

if should_run myocyte; then
  require_bin myocyte.out
  run_cmd myocyte "${ROOT_DIR}/opencl/myocyte" "${BIN_DIR}/myocyte.out -time 100"
fi

if should_run nn; then
  require_bin nn
  require_data "${DATA_DIR}/nn/cane4_0.db"
  run_cmd nn "${ROOT_DIR}/opencl/nn" "${BIN_DIR}/nn filelist.txt -r 5 -lat 30 -lng 90"
fi

if should_run nw; then
  require_bin nw
  run_cmd nw "${ROOT_DIR}/opencl/nw" "${BIN_DIR}/nw 2048 10 ./nw.cl"
fi

if should_run particlefilter_double; then
  require_bin OCL_particlefilter_double
  run_cmd particlefilter_double "${ROOT_DIR}/opencl/particlefilter" \
    "${BIN_DIR}/OCL_particlefilter_double -x 128 -y 128 -z 10 -np 400000"
  if [[ -f "${OUT_DIR}/particlefilter_double/output.txt" ]]; then
    mv "${OUT_DIR}/particlefilter_double/output.txt" "${OUT_DIR}/particlefilter_double/particlefilter_double_output.txt"
  fi
fi

if should_run particlefilter_single; then
  require_bin OCL_particlefilter_single
  run_cmd particlefilter_single "${ROOT_DIR}/opencl/particlefilter" \
    "${BIN_DIR}/OCL_particlefilter_single -x 128 -y 128 -z 10 -np 400000"
  if [[ -f "${OUT_DIR}/particlefilter_single/output.txt" ]]; then
    mv "${OUT_DIR}/particlefilter_single/output.txt" "${OUT_DIR}/particlefilter_single/particlefilter_single_output.txt"
  fi
fi

if should_run particlefilter_naive; then
  require_bin OCL_particlefilter_naive
  run_cmd particlefilter_naive "${ROOT_DIR}/opencl/particlefilter" \
    "${BIN_DIR}/OCL_particlefilter_naive -x 128 -y 128 -z 10 -np 10000"
  if [[ -f "${OUT_DIR}/particlefilter_naive/output.txt" ]]; then
    mv "${OUT_DIR}/particlefilter_naive/output.txt" "${OUT_DIR}/particlefilter_naive/particlefilter_naive_output.txt"
  fi
fi

if should_run pathfinder; then
  require_bin pathfinder
  run_cmd pathfinder "${ROOT_DIR}/opencl/pathfinder" \
    "${BIN_DIR}/pathfinder 100000 100 20 > \${RODINIA_BENCH_OUTPUT_DIR}/pathfinder.txt"
fi

if should_run srad; then
  require_bin srad
  run_cmd srad "${ROOT_DIR}/opencl/srad" \
    "${BIN_DIR}/srad 100 0.5 502 458 \"\${RODINIA_BENCH_OUTPUT_DIR}/image_out.pgm\""
fi

if should_run streamcluster; then
  require_bin streamcluster
  run_cmd streamcluster "${ROOT_DIR}/opencl/streamcluster" \
    "${BIN_DIR}/streamcluster 10 20 256 65536 65536 1000 none \${RODINIA_BENCH_OUTPUT_DIR}/streamcluster.txt 1 -t gpu -d 0"
fi
