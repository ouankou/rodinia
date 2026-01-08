#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build/data"

force=0
only=""

usage() {
  cat <<'EOF'
Usage: scripts/generate_data.sh [--force] [--only <kmeans|leukocyte|mummergpu>]

Generates large Rodinia input datasets locally without downloading.
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --force)
      force=1
      ;;
    --only)
      shift
      only="${1:-}"
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

if [[ -n "${only}" ]]; then
  case "${only}" in
    mummergpu|kmeans|leukocyte)
      ;;
    *)
      echo "Unknown --only value: ${only}" >&2
      exit 1
      ;;
  esac
fi

should_run() {
  [[ -z "${only}" || "${only}" == "$1" ]]
}

mkdir -p "${BUILD_DIR}"

if should_run mummergpu; then
  ref="${ROOT_DIR}/data/mummergpu/NC_003997.fna"
  out="${ROOT_DIR}/data/mummergpu/NC_003997_q100bp.fna"
  if [[ -f "${out}" && ${force} -eq 0 ]]; then
    echo "mummergpu: ${out} already exists; skipping."
  else
    echo "mummergpu: generating ${out}"
    tmp="${out}.tmp"
    python3 "${ROOT_DIR}/data/mummergpu/genreads.py" "${ref}" 100 2500000 --seed 0 > "${tmp}"
    mv "${tmp}" "${out}"
  fi
fi

if should_run kmeans; then
  out="${ROOT_DIR}/data/kmeans/819200.txt"
  if [[ -f "${out}" && ${force} -eq 0 ]]; then
    echo "kmeans: ${out} already exists; skipping."
  else
    echo "kmeans: generating ${out}"
    cxx="${CXX:-c++}"
    "${cxx}" -O2 -std=c++11 \
      "${ROOT_DIR}/data/kmeans/inpuGen/datagen.cpp" \
      -o "${BUILD_DIR}/kmeans_datagen"
    rm -f "${BUILD_DIR}/819200_34.txt"
    (
      cd "${BUILD_DIR}"
      "${BUILD_DIR}/kmeans_datagen" 819200 34
    )
    mv "${BUILD_DIR}/819200_34.txt" "${out}"
  fi
fi

if should_run leukocyte; then
  out="${ROOT_DIR}/data/leukocyte/testfile.avi"
  if [[ -f "${out}" && ${force} -eq 0 ]]; then
    echo "leukocyte: ${out} already exists; skipping."
  else
    echo "leukocyte: generating ${out}"
    cc="${CC:-cc}"
    "${cc}" -O2 -std=c99 \
      -I"${ROOT_DIR}/openmp/leukocyte/OpenMP" \
      "${ROOT_DIR}/data/leukocyte/inputGen/gen_avi.c" \
      "${ROOT_DIR}/openmp/leukocyte/OpenMP/avilib.c" \
      -o "${BUILD_DIR}/leukocyte_gen_avi"
    "${BUILD_DIR}/leukocyte_gen_avi" "${out}"
  fi
fi
