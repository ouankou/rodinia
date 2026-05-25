#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build-cuda"
OUT_DIR=""
DEVICE=0
ONLY=()

usage() {
  cat <<'USAGE'
Usage: scripts/run_cuda.sh [options]

Options:
  --build-dir <path>   CMake build directory (default: build-cuda)
  --output-dir <path>  Output directory for generated files (default: <build-dir>/run_outputs/cuda)
  --device <id>        CUDA device id exposed to the benchmarks (default: 0)
  --only <name>        Run only the named benchmark (repeatable)
  -h, --help           Show this help

Benchmarks names (for --only):
  backprop bfs b+tree cfd cfd_double cfd_pre cfd_pre_double dwt2d_192 dwt2d_rgb
  gaussian heartwall hotspot hotspot3d huffman hybridsort kmeans lavaMD leukocyte
  lud mummergpu myocyte nn nw particlefilter_float particlefilter_naive pathfinder
  srad_v1 srad_v2 streamcluster
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
    --device)
      DEVICE="$2"
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
  OUT_DIR="${BUILD_DIR}/run_outputs/cuda"
fi
mkdir -p "${OUT_DIR}"
OUT_DIR="$(cd "${OUT_DIR}" && pwd)"
export CUDA_VISIBLE_DEVICES="${DEVICE}"
BIN_DIR="${BUILD_DIR}/bin/cuda"
DATA_DIR="${ROOT_DIR}/data"

if [[ ! -d "${BIN_DIR}" ]]; then
  echo "Missing CUDA bin directory: ${BIN_DIR}" >&2
  exit 1
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
  run_cmd backprop "${ROOT_DIR}/cuda/backprop" "${BIN_DIR}/backprop 65536"
fi

if should_run bfs; then
  require_bin bfs
  require_data "${DATA_DIR}/bfs/graph1MW_6.txt"
  run_cmd bfs "${ROOT_DIR}/cuda/bfs" "${BIN_DIR}/bfs ${DATA_DIR}/bfs/graph1MW_6.txt"
fi

if should_run b+tree; then
  require_bin b+tree.out
  require_data "${DATA_DIR}/b+tree/mil.txt"
  require_data "${DATA_DIR}/b+tree/command_k.txt"
  run_cmd b+tree "${ROOT_DIR}/cuda/b+tree" \
    "${BIN_DIR}/b+tree.out file ${DATA_DIR}/b+tree/mil.txt command ${DATA_DIR}/b+tree/command_k.txt"
fi

if should_run cfd; then
  require_bin euler3d
  require_data "${DATA_DIR}/cfd/fvcorr.domn.097K"
  run_cmd cfd "${ROOT_DIR}/cuda/cfd" "${BIN_DIR}/euler3d ${DATA_DIR}/cfd/fvcorr.domn.097K"
fi

if should_run cfd_double; then
  require_bin euler3d_double
  require_data "${DATA_DIR}/cfd/fvcorr.domn.097K"
  run_cmd cfd_double "${ROOT_DIR}/cuda/cfd" "${BIN_DIR}/euler3d_double ${DATA_DIR}/cfd/fvcorr.domn.097K"
fi

if should_run cfd_pre; then
  require_bin pre_euler3d
  require_data "${DATA_DIR}/cfd/fvcorr.domn.097K"
  run_cmd cfd_pre "${ROOT_DIR}/cuda/cfd" "${BIN_DIR}/pre_euler3d ${DATA_DIR}/cfd/fvcorr.domn.097K"
fi

if should_run cfd_pre_double; then
  require_bin pre_euler3d_double
  require_data "${DATA_DIR}/cfd/fvcorr.domn.097K"
  run_cmd cfd_pre_double "${ROOT_DIR}/cuda/cfd" "${BIN_DIR}/pre_euler3d_double ${DATA_DIR}/cfd/fvcorr.domn.097K"
fi

if should_run dwt2d_192; then
  require_bin dwt2d
  require_data "${DATA_DIR}/dwt2d/192.bmp"
  run_cmd dwt2d_192 "${ROOT_DIR}/cuda/dwt2d" \
    "${BIN_DIR}/dwt2d ${DATA_DIR}/dwt2d/192.bmp \${RODINIA_BENCH_OUTPUT_DIR}/dwt2d_192.dwt -d 192x192 -f -5 -l 3"
fi

if should_run dwt2d_rgb; then
  require_bin dwt2d
  require_data "${DATA_DIR}/dwt2d/rgb.bmp"
  run_cmd dwt2d_rgb "${ROOT_DIR}/cuda/dwt2d" \
    "${BIN_DIR}/dwt2d ${DATA_DIR}/dwt2d/rgb.bmp \${RODINIA_BENCH_OUTPUT_DIR}/dwt2d_rgb.dwt -d 1024x1024 -f -5 -l 3"
fi

if should_run gaussian; then
  require_bin gaussian
  run_cmd gaussian "${ROOT_DIR}/cuda/gaussian" "${BIN_DIR}/gaussian -s 256 -q"
fi

if should_run heartwall; then
  require_bin heartwall
  require_data "${DATA_DIR}/heartwall/test.avi"
  run_cmd heartwall "${ROOT_DIR}/cuda/heartwall" "${BIN_DIR}/heartwall ${DATA_DIR}/heartwall/test.avi 20"
fi

if should_run hotspot; then
  require_bin hotspot
  require_data "${DATA_DIR}/hotspot/temp_512"
  require_data "${DATA_DIR}/hotspot/power_512"
  run_cmd hotspot "${ROOT_DIR}/cuda/hotspot" \
    "${BIN_DIR}/hotspot 512 2 2 ${DATA_DIR}/hotspot/temp_512 ${DATA_DIR}/hotspot/power_512 \${RODINIA_BENCH_OUTPUT_DIR}/hotspot.out"
fi

if should_run hotspot3d; then
  require_bin 3D
  require_data "${DATA_DIR}/hotspot3D/power_512x8"
  require_data "${DATA_DIR}/hotspot3D/temp_512x8"
  run_cmd hotspot3d "${ROOT_DIR}/cuda/hotspot3D" \
    "${BIN_DIR}/3D 512 8 100 ${DATA_DIR}/hotspot3D/power_512x8 ${DATA_DIR}/hotspot3D/temp_512x8 \${RODINIA_BENCH_OUTPUT_DIR}/hotspot3d.out"
fi

if should_run huffman; then
  require_bin pavle
  require_data "${DATA_DIR}/huffman/test1024_H2.206587175259.in"
  run_cmd huffman "${ROOT_DIR}/cuda/huffman" "${BIN_DIR}/pavle ${DATA_DIR}/huffman/test1024_H2.206587175259.in"
fi

if should_run hybridsort; then
  require_bin hybridsort
  run_cmd hybridsort "${ROOT_DIR}/cuda/hybridsort" "${BIN_DIR}/hybridsort r"
fi

if should_run kmeans; then
  require_bin kmeans
  require_data "${DATA_DIR}/kmeans/kdd_cup"
  run_cmd kmeans "${ROOT_DIR}/cuda/kmeans" "${BIN_DIR}/kmeans -o -i ${DATA_DIR}/kmeans/kdd_cup"
fi

if should_run lavaMD; then
  require_bin lavaMD
  run_cmd lavaMD "${ROOT_DIR}/cuda/lavaMD" "${BIN_DIR}/lavaMD -boxes1d 10"
fi

if should_run leukocyte; then
  require_bin leukocyte
  require_data "${DATA_DIR}/leukocyte/testfile.avi"
  run_cmd leukocyte "${ROOT_DIR}/cuda/leukocyte" "${BIN_DIR}/leukocyte ${DATA_DIR}/leukocyte/testfile.avi 5"
fi

if should_run lud; then
  require_bin lud_cuda
  run_cmd lud "${ROOT_DIR}/cuda/lud" "${BIN_DIR}/lud_cuda -s 1024 -v"
fi

if should_run mummergpu; then
  require_bin mummergpu
  require_data "${DATA_DIR}/mummergpu/NC_003997.20k.fna"
  require_data "${DATA_DIR}/mummergpu/NC_003997_q25bp.50k.fna"
  run_cmd mummergpu "${ROOT_DIR}/cuda/mummergpu" \
    "${BIN_DIR}/mummergpu -C ${DATA_DIR}/mummergpu/NC_003997.20k.fna ${DATA_DIR}/mummergpu/NC_003997_q25bp.50k.fna > \${RODINIA_BENCH_OUTPUT_DIR}/mummergpu.out"
fi

if should_run myocyte; then
  require_bin myocyte.out
  run_cmd myocyte "${ROOT_DIR}/cuda/myocyte" "${BIN_DIR}/myocyte.out 100 1 0"
fi

if should_run nn; then
  require_bin nn
  require_data "${DATA_DIR}/nn/filelist.txt"
  run_cmd nn "${DATA_DIR}/nn" "${BIN_DIR}/nn filelist.txt -r 5 -lat 30 -lng 90"
fi

if should_run nw; then
  require_bin needle
  run_cmd nw "${ROOT_DIR}/cuda/nw" "${BIN_DIR}/needle 2048 10"
fi

if should_run particlefilter_float; then
  require_bin particlefilter_float
  run_cmd particlefilter_float "${ROOT_DIR}/cuda/particlefilter" \
    "${BIN_DIR}/particlefilter_float -x 128 -y 128 -z 10 -np 10000"
fi

if should_run particlefilter_naive; then
  require_bin particlefilter_naive
  run_cmd particlefilter_naive "${ROOT_DIR}/cuda/particlefilter" \
    "${BIN_DIR}/particlefilter_naive -x 128 -y 128 -z 10 -np 10000"
fi

if should_run pathfinder; then
  require_bin pathfinder
  run_cmd pathfinder "${ROOT_DIR}/cuda/pathfinder" \
    "${BIN_DIR}/pathfinder 100000 100 20 > \${RODINIA_BENCH_OUTPUT_DIR}/pathfinder.txt"
fi

if should_run srad_v1; then
  require_bin srad_v1
  run_cmd srad_v1 "${ROOT_DIR}/cuda/srad/srad_v1" "${BIN_DIR}/srad_v1 100 0.5 502 458"
fi

if should_run srad_v2; then
  require_bin srad_v2
  run_cmd srad_v2 "${ROOT_DIR}/cuda/srad/srad_v2" "${BIN_DIR}/srad_v2 2048 2048 0 127 0 127 0.5 2"
fi

if should_run streamcluster; then
  require_bin sc_gpu
  run_cmd streamcluster "${ROOT_DIR}/cuda/streamcluster" \
    "${BIN_DIR}/sc_gpu 10 20 256 65536 65536 1000 none \${RODINIA_BENCH_OUTPUT_DIR}/streamcluster.txt 1"
fi
