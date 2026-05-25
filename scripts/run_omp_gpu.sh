#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build-omp-gpu"
OUT_DIR=""
ONLY=()

usage() {
  cat <<'EOF'
Usage: scripts/run_omp_gpu.sh [options]

Options:
  --build-dir <path>   CMake build directory (default: build-omp-gpu)
  --output-dir <path>  Output directory for generated files (default: <build-dir>/run_outputs/omp-gpu)
  --only <name>        Run only the named benchmark (repeatable)
  -h, --help           Show this help

Benchmark names:
  backprop b+tree cfd cfd_double cfd_pre cfd_pre_double heartwall hotspot
  hotspot3d kmeans lavaMD lud nn nw pathfinder srad_v1 srad_v2
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
  OUT_DIR="${BUILD_DIR}/run_outputs/omp-gpu"
fi
mkdir -p "${OUT_DIR}"
OUT_DIR="$(cd "${OUT_DIR}" && pwd)"
BIN_DIR="${BUILD_DIR}/bin/omp-gpu"
DATA_DIR="${ROOT_DIR}/data"
OMP_TARGET_OFFLOAD_VALUE="${RODINIA_OMP_TARGET_OFFLOAD:-MANDATORY}"

if [[ ! -d "${BIN_DIR}" ]]; then
  echo "Missing OpenMP GPU bin directory: ${BIN_DIR}" >&2
  exit 1
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
  local cmd="$2"
  local bench_out="${OUT_DIR}/${name}"
  mkdir -p "${bench_out}"
  echo "==> ${name}"
  (
    cd "${bench_out}"
    RODINIA_OUTPUT_DIR="${bench_out}" \
    RODINIA_BENCH_OUTPUT_DIR="${bench_out}" \
    RODINIA_DATA_DIR="${DATA_DIR}" \
    OMP_TARGET_OFFLOAD="${OMP_TARGET_OFFLOAD_VALUE}" \
    bash -c "${cmd}" 2>&1 | tee "${bench_out}/run.log"
  )
}

if [[ ! -f "${DATA_DIR}/.downloaded" ]]; then
  "${ROOT_DIR}/scripts/generate_data.sh"
fi

if should_run backprop; then
  require_bin backprop
  run_cmd backprop "${BIN_DIR}/backprop 65536"
fi

if should_run b+tree; then
  require_bin b+tree.out
  run_cmd b+tree \
    "${BIN_DIR}/b+tree.out file ${DATA_DIR}/b+tree/mil.txt command ${DATA_DIR}/b+tree/command_k.txt"
fi

if should_run cfd; then
  require_bin euler3d_cpu
  run_cmd cfd "${BIN_DIR}/euler3d_cpu ${DATA_DIR}/cfd/fvcorr.domn.097K"
fi

if should_run cfd_double; then
  require_bin euler3d_cpu_double
  run_cmd cfd_double "${BIN_DIR}/euler3d_cpu_double ${DATA_DIR}/cfd/fvcorr.domn.097K"
fi

if should_run cfd_pre; then
  require_bin pre_euler3d_cpu
  run_cmd cfd_pre "${BIN_DIR}/pre_euler3d_cpu ${DATA_DIR}/cfd/fvcorr.domn.097K"
fi

if should_run cfd_pre_double; then
  require_bin pre_euler3d_cpu_double
  run_cmd cfd_pre_double "${BIN_DIR}/pre_euler3d_cpu_double ${DATA_DIR}/cfd/fvcorr.domn.097K"
fi

if should_run heartwall; then
  require_bin heartwall
  run_cmd heartwall "${BIN_DIR}/heartwall ${DATA_DIR}/heartwall/test.avi 20"
fi

if should_run hotspot; then
  require_bin hotspot
  run_cmd hotspot "${BIN_DIR}/hotspot 512 512 2 ${DATA_DIR}/hotspot/temp_512 ${DATA_DIR}/hotspot/power_512"
fi

if should_run hotspot3d; then
  require_bin 3D
  run_cmd hotspot3d \
    "${BIN_DIR}/3D 512 8 100 ${DATA_DIR}/hotspot3D/power_512x8 ${DATA_DIR}/hotspot3D/temp_512x8 \${RODINIA_BENCH_OUTPUT_DIR}/hotspot3d.out"
fi

if should_run kmeans; then
  require_bin kmeans
  run_cmd kmeans "${BIN_DIR}/kmeans -i ${DATA_DIR}/kmeans/kdd_cup"
fi

if should_run lavaMD; then
  require_bin lavaMD
  run_cmd lavaMD "${BIN_DIR}/lavaMD -boxes1d 10"
fi

if should_run lud; then
  require_bin lud_omp_gpu
  run_cmd lud "${BIN_DIR}/lud_omp_gpu -i ${DATA_DIR}/lud/512.dat"
fi

if should_run nn; then
  require_bin nn
  nn_filelist="${OUT_DIR}/nn/filelist.txt"
  mkdir -p "$(dirname "${nn_filelist}")"
  printf '%s\n' \
    "${DATA_DIR}/nn/cane4_0.db" \
    "${DATA_DIR}/nn/cane4_1.db" \
    "${DATA_DIR}/nn/cane4_2.db" \
    "${DATA_DIR}/nn/cane4_3.db" > "${nn_filelist}"
  run_cmd nn "${BIN_DIR}/nn ${nn_filelist} 5 30 90"
fi

if should_run nw; then
  require_bin needle
  run_cmd nw "${BIN_DIR}/needle 2048 10"
fi

if should_run pathfinder; then
  require_bin pathfinder
  run_cmd pathfinder "${BIN_DIR}/pathfinder 100000 100 > \${RODINIA_BENCH_OUTPUT_DIR}/pathfinder.out"
fi

if should_run srad_v1; then
  require_bin srad_v1
  run_cmd srad_v1 "${BIN_DIR}/srad_v1 100 0.5 502 458"
fi

if should_run srad_v2; then
  require_bin srad_v2
  run_cmd srad_v2 "${BIN_DIR}/srad_v2 2048 2048 0 127 0 127 0.5 2"
fi
