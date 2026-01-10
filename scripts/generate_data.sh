#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DATA_DIR="${ROOT_DIR}/data"
DATA_URL_DEFAULT="https://github.com/passlab/NeoRodinia-Old/archive/refs/heads/main.tar.gz"
DATA_URL="${RODINIA_DATA_URL:-${DATA_URL_DEFAULT}}"
force=0

usage() {
  cat <<'USAGE'
Usage: scripts/generate_data.sh [--force] [--url <tarball-url>]

Downloads the full Rodinia input data bundle into ./data.
USAGE
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --force)
      force=1
      ;;
    --url)
      shift
      DATA_URL="${1:-}"
      if [[ -z "${DATA_URL}" ]]; then
        echo "--url requires a value" >&2
        exit 1
      fi
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

if [[ -f "${DATA_DIR}/.downloaded" && ${force} -eq 0 ]]; then
  echo "Rodinia data already present; skipping. Use --force to re-download."
  exit 0
fi

if ! command -v tar >/dev/null 2>&1; then
  echo "Missing dependency: tar" >&2
  exit 1
fi

if command -v curl >/dev/null 2>&1; then
  download_cmd="curl"
elif command -v wget >/dev/null 2>&1; then
  download_cmd="wget"
else
  echo "Missing download tool: install curl or wget." >&2
  exit 1
fi

tmp_dir="$(mktemp -d)"
cleanup() {
  rm -rf "${tmp_dir}"
}
trap cleanup EXIT

archive="${tmp_dir}/data.tar.gz"
if [[ "${download_cmd}" == "curl" ]]; then
  curl -fL "${DATA_URL}" -o "${archive}"
else
  wget -O "${archive}" "${DATA_URL}"
fi

tar -xzf "${archive}" -C "${tmp_dir}"

repo_dir=""
for entry in "${tmp_dir}"/*; do
  if [[ -d "${entry}" ]]; then
    repo_dir="${entry}"
    break
  fi
done

if [[ -z "${repo_dir}" || ! -d "${repo_dir}/data" ]]; then
  echo "Downloaded archive does not contain a data/ directory." >&2
  exit 1
fi

mkdir -p "${DATA_DIR}"

readme_backup=""
gitignore_backup=""
if [[ -f "${DATA_DIR}/README.md" ]]; then
  readme_backup="${tmp_dir}/README.md"
  cp "${DATA_DIR}/README.md" "${readme_backup}"
fi
if [[ -f "${DATA_DIR}/.gitignore" ]]; then
  gitignore_backup="${tmp_dir}/.gitignore"
  cp "${DATA_DIR}/.gitignore" "${gitignore_backup}"
fi

find "${DATA_DIR}" -mindepth 1 -maxdepth 1 \
  ! -name 'README.md' ! -name '.gitignore' -exec rm -rf {} +

cp -R "${repo_dir}/data"/. "${DATA_DIR}/"

if [[ -n "${readme_backup}" ]]; then
  cp "${readme_backup}" "${DATA_DIR}/README.md"
fi
if [[ -n "${gitignore_backup}" ]]; then
  cp "${gitignore_backup}" "${DATA_DIR}/.gitignore"
fi

apply_text_fix() {
  local file="$1"
  local from="$2"
  local to="$3"
  local tmp_file=""

  if [[ ! -f "${file}" ]]; then
    return 0
  fi

  tmp_file="$(mktemp)"
  sed "s/${from}/${to}/g" "${file}" > "${tmp_file}"
  mv "${tmp_file}" "${file}"
}

apply_text_fix "${DATA_DIR}/hotspot/inputGen/hotspotver.cpp" "Univeristy" "University"
apply_text_fix "${DATA_DIR}/hotspot/inputGen/hotspotex.cpp" "Univeristy" "University"
apply_text_fix "${DATA_DIR}/hotspot/inputGen/README" "prexisting" "preexisting"
apply_text_fix "${DATA_DIR}/bfs/inputGen/graphgen.cpp" "Univeristy" "University"
apply_text_fix "${DATA_DIR}/bfs/inputGen/graphgen.cpp" "are there may be" "and there may be"
apply_text_fix "${DATA_DIR}/bfs/inputGen/graphgen.cpp" "Parse command lined" "Parse command line"

touch "${DATA_DIR}/.downloaded"

echo "Rodinia data downloaded into ${DATA_DIR}"
