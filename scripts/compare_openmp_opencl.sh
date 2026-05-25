#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OMP_DIR="${ROOT_DIR}/build-omp-clang/run_outputs/omp"
OCL_DIR="${ROOT_DIR}/build-opencl/run_outputs/opencl"
OMP_ALIGNED_DIR="${ROOT_DIR}/build-omp-clang/run_outputs/omp_aligned"
OCL_ALIGNED_DIR="${ROOT_DIR}/build-opencl/run_outputs/opencl_aligned"
OMP_BPTREE_DIR="${ROOT_DIR}/build-omp-clang/run_outputs/omp_bptree_only"
OCL_BPTREE_DIR="${ROOT_DIR}/build-opencl/run_outputs/opencl_bptree_only"
OMP_BACKPROP_DIR="${ROOT_DIR}/build-omp-clang/run_outputs/omp_backprop_only"
OCL_BACKPROP_DIR="${ROOT_DIR}/build-opencl/run_outputs/opencl_backprop_only"
OMP_BFS_DIR="${ROOT_DIR}/build-omp-clang/run_outputs/omp_bfs_only"
OCL_BFS_DIR="${ROOT_DIR}/build-opencl/run_outputs/opencl_bfs_only"
OMP_HEARTWALL_DIR="${ROOT_DIR}/build-omp-clang/run_outputs/omp_heartwall_only"
OCL_HEARTWALL_DIR="${ROOT_DIR}/build-opencl/run_outputs/opencl_heartwall_only"
OMP_KMEANS_DIR="${ROOT_DIR}/build-omp-clang/run_outputs/omp_kmeans_only"
OCL_KMEANS_DIR="${ROOT_DIR}/build-opencl/run_outputs/opencl_kmeans_only"
OMP_LAVAMD_DIR="${ROOT_DIR}/build-omp-clang/run_outputs/omp_lavamd_only"
OCL_LAVAMD_DIR="${ROOT_DIR}/build-opencl/run_outputs/opencl_lavamd_only"
OMP_LUD_DIR="${ROOT_DIR}/build-omp-clang/run_outputs/omp_lud_only"
OCL_LUD_DIR="${ROOT_DIR}/build-opencl/run_outputs/opencl_lud_only"
OMP_MYO_DIR="${ROOT_DIR}/build-omp-clang/run_outputs/omp_myocyte_only"
OCL_MYO_DIR="${ROOT_DIR}/build-opencl/run_outputs/opencl_myocyte_only"
OMP_NN_DIR="${ROOT_DIR}/build-omp-clang/run_outputs/omp_nn_only"
OCL_NN_DIR="${ROOT_DIR}/build-opencl/run_outputs/opencl_nn_only"
OMP_NW_DIR="${ROOT_DIR}/build-omp-clang/run_outputs/omp_nw_only"
OCL_NW_DIR="${ROOT_DIR}/build-opencl/run_outputs/opencl_nw_only"
OMP_PARTICLE_DIR="${ROOT_DIR}/build-omp-clang/run_outputs/omp_particlefilter_only"
OCL_PARTICLE_DIR="${ROOT_DIR}/build-opencl/run_outputs/opencl_particlefilter_only"
OMP_STREAMCLUSTER_DIR="${ROOT_DIR}/build-omp-clang/run_outputs/omp_streamcluster_only"
OCL_STREAMCLUSTER_DIR="${ROOT_DIR}/build-opencl/run_outputs/opencl_streamcluster_only"

usage() {
  cat <<'EOF'
Usage: scripts/compare_openmp_opencl.sh [options]

Options:
  --omp-dir <path>          OpenMP output directory (default: build-omp-clang/run_outputs/omp)
  --opencl-dir <path>       OpenCL output directory (default: build-opencl/run_outputs/opencl)
  --aligned-omp-dir <path>  Aligned OpenMP outputs (default: build-omp-clang/run_outputs/omp_aligned)
  --aligned-opencl-dir <path> Aligned OpenCL outputs (default: build-opencl/run_outputs/opencl_aligned)
  --bptree-omp-dir <path>   OpenMP b+tree-only outputs (default: build-omp-clang/run_outputs/omp_bptree_only)
  --bptree-opencl-dir <path> OpenCL b+tree-only outputs (default: build-opencl/run_outputs/opencl_bptree_only)
  -h, --help                Show this help
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --omp-dir)
      OMP_DIR="$2"
      shift
      ;;
    --opencl-dir)
      OCL_DIR="$2"
      shift
      ;;
    --aligned-omp-dir)
      OMP_ALIGNED_DIR="$2"
      shift
      ;;
    --aligned-opencl-dir)
      OCL_ALIGNED_DIR="$2"
      shift
      ;;
    --bptree-omp-dir)
      OMP_BPTREE_DIR="$2"
      shift
      ;;
    --bptree-opencl-dir)
      OCL_BPTREE_DIR="$2"
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

print_line() {
  printf "%-18s %-8s %s\n" "$1" "$2" "$3"
}

require_files() {
  local name="$1"
  local f1="$2"
  local f2="$3"
  if [[ ! -f "$f1" ]]; then
    print_line "$name" "SKIP" "missing: $f1"
    return 1
  fi
  if [[ ! -f "$f2" ]]; then
    print_line "$name" "SKIP" "missing: $f2"
    return 1
  fi
  return 0
}

compare_exact() {
  local name="$1"
  local f1="$2"
  local f2="$3"
  require_files "$name" "$f1" "$f2" || return 0
  if cmp -s "$f1" "$f2"; then
    print_line "$name" "OK" "exact match"
  else
    local s1 s2
    s1=$(wc -c < "$f1")
    s2=$(wc -c < "$f2")
    print_line "$name" "MISMATCH" "size $s1 vs $s2"
  fi
}

compare_float_column() {
  local name="$1"
  local f1="$2"
  local f2="$3"
  local col="$4"
  local skip="$5"
  local abs_tol="$6"
  local rel_tol="$7"
  require_files "$name" "$f1" "$f2" || return 0
  local result
  result=$(awk -v col="$col" -v skip="$skip" -v abs_tol="$abs_tol" -v rel_tol="$rel_tol" '
    FNR <= skip { next }
    NR == FNR { a[++n] = $col; next }
    {
      if (++m > n) { missing = 1; next }
      ref = a[m];
      val = $col;
      diff = val - ref;
      if (diff < 0) diff = -diff;
      if (diff > max_abs) max_abs = diff;
      denom = ref; if (denom < 0) denom = -denom;
      if (denom < 1e-12) denom = 1e-12;
      rel = diff / denom;
      if (rel > max_rel) max_rel = rel;
      if (diff > abs_tol || rel > rel_tol) count++;
    }
    END {
      if (missing || m != n) {
        print "MISMATCH line_count";
        exit 0;
      }
      status = (count > 0) ? "MISMATCH" : "OK";
      printf "%s max_abs=%g max_rel=%g count_gt_tol=%d", status, max_abs, max_rel, count;
    }' "$f1" "$f2")
  if [[ "$result" == MISMATCH* ]]; then
    print_line "$name" "MISMATCH" "$result"
  else
    print_line "$name" "OK" "$result"
  fi
}

compare_float_triple() {
  local name="$1"
  local f1="$2"
  local f2="$3"
  local skip="$4"
  local abs_tol="$5"
  local rel_tol="$6"
  require_files "$name" "$f1" "$f2" || return 0
  local result
  result=$(awk -v skip="$skip" -v abs_tol="$abs_tol" -v rel_tol="$rel_tol" '
    FNR <= skip { next }
    NR == FNR { a1[++n]=$1; a2[n]=$2; a3[n]=$3; next }
    {
      if (++m > n) { missing = 1; next }
      vals[1]=$1; vals[2]=$2; vals[3]=$3;
      refs[1]=a1[m]; refs[2]=a2[m]; refs[3]=a3[m];
      for (i=1; i<=3; i++) {
        diff = vals[i] - refs[i];
        if (diff < 0) diff = -diff;
        if (diff > max_abs) max_abs = diff;
        denom = refs[i]; if (denom < 0) denom = -denom;
        if (denom < 1e-12) denom = 1e-12;
        rel = diff / denom;
        if (rel > max_rel) max_rel = rel;
        if (diff > abs_tol || rel > rel_tol) count++;
      }
    }
    END {
      if (missing || m != n) {
        print "MISMATCH line_count";
        exit 0;
      }
      status = (count > 0) ? "MISMATCH" : "OK";
      printf "%s max_abs=%g max_rel=%g count_gt_tol=%d", status, max_abs, max_rel, count;
    }' "$f1" "$f2")
  if [[ "$result" == MISMATCH* ]]; then
    print_line "$name" "MISMATCH" "$result"
  else
    print_line "$name" "OK" "$result"
  fi
}

compare_numeric_lines() {
  local name="$1"
  local f1="$2"
  local f2="$3"
  require_files "$name" "$f1" "$f2" || return 0
  local result
  result=$(awk '
    NR==FNR {
      if ($0 ~ /^[0-9-]/) { a[++n] = $0 }
      next
    }
    {
      if ($0 ~ /^[0-9-]/) {
        m++;
        if ($0 != a[m]) diff = 1;
      }
    }
    END {
      if (diff || m != n) {
        print "MISMATCH";
      } else {
        print "OK exact numeric lines";
      }
    }' "$f1" "$f2")
  if [[ "$result" == OK* ]]; then
    print_line "$name" "OK" "$result"
  else
    print_line "$name" "MISMATCH" "$result"
  fi
}

compare_float_list() {
  local name="$1"
  local f1="$2"
  local f2="$3"
  local skip="$4"
  local abs_tol="$5"
  local rel_tol="$6"
  require_files "$name" "$f1" "$f2" || return 0
  local result
  result=$(awk -v skip="$skip" -v abs_tol="$abs_tol" -v rel_tol="$rel_tol" '
    FNR <= skip { next }
    NR == FNR {
      for (i = 1; i <= NF; i++) a[++n] = $i;
      next
    }
    {
      for (i = 1; i <= NF; i++) {
        if (++m > n) { missing = 1; break }
        ref = a[m];
        val = $i;
        diff = val - ref;
        if (diff < 0) diff = -diff;
        if (diff > max_abs) max_abs = diff;
        denom = ref; if (denom < 0) denom = -denom;
        if (denom < 1e-12) denom = 1e-12;
        rel = diff / denom;
        if (rel > max_rel) max_rel = rel;
        if (diff > abs_tol || rel > rel_tol) count++;
      }
    }
    END {
      if (missing || m != n) {
        print "MISMATCH value_count";
        exit 0;
      }
      status = (count > 0) ? "MISMATCH" : "OK";
      printf "%s max_abs=%g max_rel=%g count_gt_tol=%d", status, max_abs, max_rel, count;
    }' "$f1" "$f2")
  if [[ "$result" == MISMATCH* ]]; then
    print_line "$name" "MISMATCH" "$result"
  else
    print_line "$name" "OK" "$result"
  fi
}

compare_float_list_csv() {
  local name="$1"
  local f1="$2"
  local f2="$3"
  local skip="$4"
  local abs_tol="$5"
  local rel_tol="$6"
  require_files "$name" "$f1" "$f2" || return 0
  local result
  result=$(awk -F '[, ]+' -v skip="$skip" -v abs_tol="$abs_tol" -v rel_tol="$rel_tol" '
    FNR <= skip { next }
    NR == FNR {
      for (i = 1; i <= NF; i++) a[++n] = $i;
      next
    }
    {
      for (i = 1; i <= NF; i++) {
        if (++m > n) { missing = 1; break }
        ref = a[m];
        val = $i;
        diff = val - ref;
        if (diff < 0) diff = -diff;
        if (diff > max_abs) max_abs = diff;
        denom = ref; if (denom < 0) denom = -denom;
        if (denom < 1e-12) denom = 1e-12;
        rel = diff / denom;
        if (rel > max_rel) max_rel = rel;
        if (diff > abs_tol || rel > rel_tol) count++;
      }
    }
    END {
      if (missing || m != n) {
        print "MISMATCH value_count";
        exit 0;
      }
      status = (count > 0) ? "MISMATCH" : "OK";
      printf "%s max_abs=%g max_rel=%g count_gt_tol=%d", status, max_abs, max_rel, count;
    }' "$f1" "$f2")
  if [[ "$result" == MISMATCH* ]]; then
    print_line "$name" "MISMATCH" "$result"
  else
    print_line "$name" "OK" "$result"
  fi
}

compare_backprop_errors() {
  local name="$1"
  local f1="$2"
  local f2="$3"
  local abs_tol="$4"
  local rel_tol="$5"
  require_files "$name" "$f1" "$f2" || return 0
  local result
  result=$(awk -v abs_tol="$abs_tol" -v rel_tol="$rel_tol" '
    FNR == 1 { file++ }
    /Output error:/ { if (file == 1) out1 = $3; else out2 = $3 }
    /Hidden error:/ { if (file == 1) hid1 = $3; else hid2 = $3 }
    END {
      if (out1 == "" || out2 == "" || hid1 == "" || hid2 == "") {
        print "MISMATCH missing_fields";
        exit 0;
      }
      count = 0;
      max_abs = 0;
      max_rel = 0;
      diff = out2 - out1; if (diff < 0) diff = -diff;
      if (diff > max_abs) max_abs = diff;
      denom = out1; if (denom < 0) denom = -denom; if (denom < 1e-12) denom = 1e-12;
      rel = diff / denom; if (rel > max_rel) max_rel = rel;
      if (diff > abs_tol || rel > rel_tol) count++;
      diff = hid2 - hid1; if (diff < 0) diff = -diff;
      if (diff > max_abs) max_abs = diff;
      denom = hid1; if (denom < 0) denom = -denom; if (denom < 1e-12) denom = 1e-12;
      rel = diff / denom; if (rel > max_rel) max_rel = rel;
      if (diff > abs_tol || rel > rel_tol) count++;
      status = (count > 0) ? "MISMATCH" : "OK";
      printf "%s max_abs=%g max_rel=%g count_gt_tol=%d", status, max_abs, max_rel, count;
    }' "$f1" "$f2")
  if [[ "$result" == MISMATCH* ]]; then
    print_line "$name" "MISMATCH" "$result"
  else
    print_line "$name" "OK" "$result"
  fi
}

compare_kmeans_checksum() {
  local name="$1"
  local f1="$2"
  local f2="$3"
  local abs_tol="$4"
  local rel_tol="$5"
  require_files "$name" "$f1" "$f2" || return 0
  local result
  result=$(awk -v abs_tol="$abs_tol" -v rel_tol="$rel_tol" '
    FNR == 1 { file++ }
    /Cluster centers checksum:/ { if (file == 1) v1 = $4; else v2 = $4 }
    END {
      if (v1 == "" || v2 == "") {
        print "MISMATCH missing_fields";
        exit 0;
      }
      diff = v2 - v1; if (diff < 0) diff = -diff;
      denom = v1; if (denom < 0) denom = -denom; if (denom < 1e-12) denom = 1e-12;
      rel = diff / denom;
      status = (diff > abs_tol || rel > rel_tol) ? "MISMATCH" : "OK";
      printf "%s abs=%g rel=%g", status, diff, rel;
    }' "$f1" "$f2")
  if [[ "$result" == MISMATCH* ]]; then
    print_line "$name" "MISMATCH" "$result"
  else
    print_line "$name" "OK" "$result"
  fi
}

compare_bfs_cost() {
  local name="$1"
  local f1="$2"
  local f2="$3"
  require_files "$name" "$f1" "$f2" || return 0
  local result
  result=$(awk '
    NR==FNR {
      if (match($0, /cost:[[:space:]]*(-?[0-9]+)/, m)) a[++n] = m[1];
      next
    }
    {
      if (match($0, /cost:[[:space:]]*(-?[0-9]+)/, m)) {
        if (++m2 > n) { missing = 1; next }
        if (m[1] != a[m2]) diff = 1;
      }
    }
    END {
      if (missing || m2 != n || diff) {
        print "MISMATCH";
      } else {
        print "OK exact costs";
      }
    }' "$f1" "$f2")
  if [[ "$result" == OK* ]]; then
    print_line "$name" "OK" "$result"
  else
    print_line "$name" "MISMATCH" "$result"
  fi
}

compare_heartwall_values() {
  local name="$1"
  local f1="$2"
  local f2="$3"
  require_files "$name" "$f1" "$f2" || return 0
  local result
  result=$(awk '
    NR==FNR {
      while (match($0, /-?[0-9]+/, m)) {
        a[++n] = m[0];
        $0 = substr($0, RSTART + RLENGTH);
      }
      next
    }
    {
      while (match($0, /-?[0-9]+/, m)) {
        if (++m2 > n) { missing = 1; break }
        diff = m[0] - a[m2];
        if (diff < 0) diff = -diff;
        if (diff > max) max = diff;
        if (diff != 0) count++;
        $0 = substr($0, RSTART + RLENGTH);
      }
    }
    END {
      if (missing || m2 != n) {
        print "MISMATCH value_count";
        exit 0;
      }
      if (count == 0) {
        print "OK exact integers";
      } else if (count <= 1 && max <= 1) {
        printf "OK within_tol max_diff=%d count_diff=%d", max, count;
      } else {
        printf "MISMATCH max_diff=%d count_diff=%d", max, count;
      }
    }' "$f1" "$f2")
  if [[ "$result" == MISMATCH* ]]; then
    print_line "$name" "MISMATCH" "$result"
  else
    print_line "$name" "OK" "$result"
  fi
}

compare_myocyte_values() {
  local name="$1"
  local f1="$2"
  local f2="$3"
  local abs_tol="$4"
  local rel_tol="$5"
  require_files "$name" "$f1" "$f2" || return 0
  local result
  result=$(awk -v abs_tol="$abs_tol" -v rel_tol="$rel_tol" '
    NR==FNR {
      if (index($0, "y[") > 0) {
        split($0, parts, "=");
        v = parts[2] + 0;
        a[++n] = v;
      }
      next
    }
    {
      if (index($0, "y[") > 0) {
        if (++m > n) { missing = 1; next }
        v = $0;
        split(v, parts, "=");
        val = parts[2] + 0;
        ref = a[m];
        diff = val - ref;
        if (diff < 0) diff = -diff;
        if (diff > max_abs) max_abs = diff;
        denom = ref; if (denom < 0) denom = -denom;
        if (denom < 1e-12) denom = 1e-12;
        rel = diff / denom;
        if (rel > max_rel) max_rel = rel;
        if (diff > abs_tol || rel > rel_tol) count++;
      }
    }
    END {
      if (missing || m != n) {
        print "MISMATCH value_count";
        exit 0;
      }
      status = (count > 0) ? "MISMATCH" : "OK";
      printf "%s max_abs=%g max_rel=%g count_gt_tol=%d", status, max_abs, max_rel, count;
    }' "$f1" "$f2")
  if [[ "$result" == MISMATCH* ]]; then
    print_line "$name" "MISMATCH" "$result"
  else
    print_line "$name" "OK" "$result"
  fi
}

compare_nn_neighbors() {
  local name="$1"
  local f1="$2"
  local f2="$3"
  local abs_tol="$4"
  local rel_tol="$5"
  require_files "$name" "$f1" "$f2" || return 0
  local result
  result=$(awk -v abs_tol="$abs_tol" -v rel_tol="$rel_tol" '
    NR==FNR {
      if (match($0, /(.*)-->[^0-9-]*([0-9eE.+-]+)/, m)) {
        name = m[1];
        gsub(/^[[:space:]]+|[[:space:]]+$/, "", name);
        dist = m[2] + 0;
        vals[name] = dist;
        n++;
      }
      next
    }
    {
      if (match($0, /(.*)-->[^0-9-]*([0-9eE.+-]+)/, m)) {
        name = m[1];
        gsub(/^[[:space:]]+|[[:space:]]+$/, "", name);
        if (!(name in vals)) { missing = 1; next }
        dist = m[2] + 0;
        ref = vals[name];
        diff = dist - ref;
        if (diff < 0) diff = -diff;
        if (diff > max_abs) max_abs = diff;
        denom = ref; if (denom < 0) denom = -denom;
        if (denom < 1e-12) denom = 1e-12;
        rel = diff / denom;
        if (rel > max_rel) max_rel = rel;
        if (diff > abs_tol || rel > rel_tol) count++;
        m2++;
      }
    }
    END {
      if (missing || m2 != n) {
        print "MISMATCH neighbor_set";
        exit 0;
      }
      status = (count > 0) ? "MISMATCH" : "OK";
      printf "%s max_abs=%g max_rel=%g count_gt_tol=%d", status, max_abs, max_rel, count;
    }' "$f1" "$f2")
  if [[ "$result" == MISMATCH* ]]; then
    print_line "$name" "MISMATCH" "$result"
  else
    print_line "$name" "OK" "$result"
  fi
}

compare_particlefilter() {
  local name="$1"
  local f1="$2"
  local f2="$3"
  local abs_tol="$4"
  local rel_tol="$5"
  require_files "$name" "$f1" "$f2" || return 0
  local result
  result=$(awk -v abs_tol="$abs_tol" -v rel_tol="$rel_tol" '
    FNR == 1 { file++ }
    {
      if ($1 == "XE:") {
        if (file == 1) xe1 = $2; else xe2 = $2;
      } else if ($1 == "YE:") {
        if (file == 1) ye1 = $2; else ye2 = $2;
        if (file == 1) after_ye1 = 1; else after_ye2 = 1;
      } else if ($1 == "distance:") {
        if (file == 1) dist1 = $2; else dist2 = $2;
      } else if ($0 ~ /^[0-9eE.+-]+$/) {
        if (file == 1 && after_ye1) {
          dist1 = $1;
          after_ye1 = 0;
        }
        if (file == 2 && after_ye2) {
          dist2 = $1;
          after_ye2 = 0;
        }
      }
    }
    END {
      if (xe1 == "" || xe2 == "" || ye1 == "" || ye2 == "" || dist1 == "" || dist2 == "") {
        print "MISMATCH missing_fields";
        exit 0;
      }
      count = 0;
      max_abs = 0;
      max_rel = 0;
      diff = xe2 - xe1; if (diff < 0) diff = -diff;
      if (diff > max_abs) max_abs = diff;
      denom = xe1; if (denom < 0) denom = -denom; if (denom < 1e-12) denom = 1e-12;
      rel = diff / denom; if (rel > max_rel) max_rel = rel;
      if (diff > abs_tol || rel > rel_tol) count++;
      diff = ye2 - ye1; if (diff < 0) diff = -diff;
      if (diff > max_abs) max_abs = diff;
      denom = ye1; if (denom < 0) denom = -denom; if (denom < 1e-12) denom = 1e-12;
      rel = diff / denom; if (rel > max_rel) max_rel = rel;
      if (diff > abs_tol || rel > rel_tol) count++;
      diff = dist2 - dist1; if (diff < 0) diff = -diff;
      if (diff > max_abs) max_abs = diff;
      denom = dist1; if (denom < 0) denom = -denom; if (denom < 1e-12) denom = 1e-12;
      rel = diff / denom; if (rel > max_rel) max_rel = rel;
      if (diff > abs_tol || rel > rel_tol) count++;
      status = (count > 0) ? "MISMATCH" : "OK";
      printf "%s max_abs=%g max_rel=%g count_gt_tol=%d", status, max_abs, max_rel, count;
    }' "$f1" "$f2")
  if [[ "$result" == MISMATCH* ]]; then
    print_line "$name" "MISMATCH" "$result"
  else
    print_line "$name" "OK" "$result"
  fi
}

compare_lud_verify() {
  local name="$1"
  local f1="$2"
  local f2="$3"
  require_files "$name" "$f1" "$f2" || return 0
  local result
  result=$(awk '
    FNR == 1 { file++ }
    /dismatch/ { if (file == 1) bad1 = 1; else bad2 = 1 }
    END {
      if (bad1 || bad2) {
        print "MISMATCH verification_errors";
      } else {
        print "OK no mismatches";
      }
    }' "$f1" "$f2")
  if [[ "$result" == MISMATCH* ]]; then
    print_line "$name" "MISMATCH" "$result"
  else
    print_line "$name" "OK" "$result"
  fi
}

compare_streamcluster_centers() {
  local name="$1"
  local f1="$2"
  local f2="$3"
  local expected_weight="$4"
  local kmin="$5"
  local kmax="$6"
  local dim="$7"
  require_files "$name" "$f1" "$f2" || return 0
  local result
  result=$(awk -v expected_weight="$expected_weight" -v kmin="$kmin" -v kmax="$kmax" -v dim="$dim" '
    FNR == 1 { file++ }
    {
      line = (FNR - 1) % 4;
      if (line == 0) {
        if ($0 !~ /^[0-9]+$/) bad[file] = 1;
        centers[file]++;
      } else if (line == 1) {
        if ($0 !~ /^-?[0-9]+(\.[0-9]+)?$/) bad[file] = 1;
        weight[file] += $1;
      } else if (line == 2) {
        if (NF != dim) bad[file] = 1;
        for (i = 1; i <= NF; i++) {
          if ($i !~ /^-?[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)?$/) bad[file] = 1;
        }
      } else if ($0 != "") {
        bad[file] = 1;
      }
    }
    END {
      for (i = 1; i <= 2; i++) {
        if (centers[i] < kmin || centers[i] > kmax) bad[i] = 1;
        if (weight[i] != expected_weight) bad[i] = 1;
      }
      if (bad[1] || bad[2]) {
        printf "MISMATCH centers=%d/%d weight=%g/%g", centers[1], centers[2], weight[1], weight[2];
        exit 0;
      }
      printf "OK structural centers=%d/%d weight=%g/%g", centers[1], centers[2], weight[1], weight[2];
    }' "$f1" "$f2")
  if [[ "$result" == MISMATCH* ]]; then
    print_line "$name" "MISMATCH" "$result"
  else
    print_line "$name" "OK" "$result"
  fi
}

compare_pgm() {
  local name="$1"
  local f1="$2"
  local f2="$3"
  local abs_tol="$4"
  require_files "$name" "$f1" "$f2" || return 0
  local result
  result=$(awk -v abs_tol="$abs_tol" '
    FNR <= 3 { next }
    NR == FNR {
      for (i = 1; i <= NF; i++) a[++n] = $i;
      next
    }
    {
      for (i = 1; i <= NF; i++) {
        if (++m > n) { missing = 1; break }
        diff = $i - a[m];
        if (diff < 0) diff = -diff;
        if (diff > max_abs) max_abs = diff;
        if (diff > abs_tol) count++;
      }
    }
    END {
      if (missing || m != n) {
        print "MISMATCH value_count";
        exit 0;
      }
      status = (count > 0) ? "MISMATCH" : "OK";
      printf "%s max_abs=%g count_gt_tol=%d", status, max_abs, count;
    }' "$f1" "$f2")
  if [[ "$result" == MISMATCH* ]]; then
    print_line "$name" "MISMATCH" "$result"
  else
    print_line "$name" "OK" "$result"
  fi
}

compare_leukocyte() {
  local name="$1"
  local f1="$2"
  local f2="$3"
  require_files "$name" "$f1" "$f2" || return 0
  local result
  result=$(awk '
    FNR == 1 { file++ }
    {
      gsub(/\r/, "");
      if ($1 == "Cells" && $2 == "detected:") {
        if (file == 1) cells1 = $3; else cells2 = $3;
      }
      if ($1 == "Tracking" && $2 == "cells" && $3 == "across") {
        if (file == 1) frames1 = $4; else frames2 = $4;
      }
    }
    END {
      if (cells1 == "" || cells2 == "" || frames1 == "" || frames2 == "") {
        print "MISMATCH missing_fields";
        exit 0;
      }
      if (cells1 != cells2 || frames1 != frames2) {
        printf "MISMATCH cells=%s/%s frames=%s/%s", cells1, cells2, frames1, frames2;
      } else {
        printf "OK cells=%s frames=%s", cells1, frames1;
      }
    }' "$f1" "$f2")
  if [[ "$result" == MISMATCH* ]]; then
    print_line "$name" "MISMATCH" "$result"
  else
    print_line "$name" "OK" "$result"
  fi
}

echo "Benchmark           Status   Details"
echo "------------------------------------"

compare_backprop_errors "backprop" \
  "${OMP_BACKPROP_DIR}/backprop.log" \
  "${OCL_BACKPROP_DIR}/backprop.log" \
  1e-6 1e-6

compare_bfs_cost "bfs" \
  "${OMP_BFS_DIR}/result.txt" \
  "${OCL_BFS_DIR}/result.txt"

compare_exact "b+tree" \
  "${OMP_BPTREE_DIR}/output.txt" \
  "${OCL_BPTREE_DIR}/output.txt"

compare_float_column "cfd:density" \
  "${OMP_ALIGNED_DIR}/cfd/density" \
  "${OCL_ALIGNED_DIR}/cfd/density" \
  1 1 1e-4 1e-4
compare_float_triple "cfd:momentum" \
  "${OMP_ALIGNED_DIR}/cfd/momentum" \
  "${OCL_ALIGNED_DIR}/cfd/momentum" \
  1 1e-4 1e-4
compare_float_column "cfd:densityE" \
  "${OMP_ALIGNED_DIR}/cfd/density_energy" \
  "${OCL_ALIGNED_DIR}/cfd/density_energy" \
  1 1 1e-4 1e-4

compare_heartwall_values "heartwall" \
  "${OMP_HEARTWALL_DIR}/result.txt" \
  "${OCL_HEARTWALL_DIR}/result.txt"

compare_float_column "hotspot" \
  "${OMP_ALIGNED_DIR}/hotspot/hotspot.out" \
  "${OCL_ALIGNED_DIR}/hotspot/hotspot.out" \
  2 0 1e-3 1e-3

compare_float_column "hotspot3d" \
  "${OMP_DIR}/hotspot3d.out" \
  "${OCL_DIR}/hotspot3d.out" \
  2 0 1e-3 1e-3

compare_kmeans_checksum "kmeans" \
  "${OMP_KMEANS_DIR}/kmeans.log" \
  "${OCL_KMEANS_DIR}/kmeans.log" \
  1e-3 1e-3

compare_float_list_csv "lavaMD" \
  "${OMP_LAVAMD_DIR}/result.txt" \
  "${OCL_LAVAMD_DIR}/result.txt" \
  0 1e-4 1e-4

compare_numeric_lines "pathfinder" \
  "${OMP_DIR}/pathfinder.out" \
  "${OCL_DIR}/pathfinder.txt"

compare_leukocyte "leukocyte" \
  "${OMP_ALIGNED_DIR}/leukocyte/run.log" \
  "${OCL_ALIGNED_DIR}/leukocyte/run.log"

compare_lud_verify "lud" \
  "${OMP_LUD_DIR}/lud.log" \
  "${OCL_LUD_DIR}/lud.log"

compare_myocyte_values "myocyte" \
  "${OMP_MYO_DIR}/output.txt" \
  "${OCL_MYO_DIR}/output.txt" \
  5e-4 5e-4

compare_nn_neighbors "nn" \
  "${OMP_NN_DIR}/nn.log" \
  "${OCL_NN_DIR}/nn.log" \
  1e-4 1e-4

compare_numeric_lines "nw" \
  "${OMP_NW_DIR}/result.txt" \
  "${OCL_NW_DIR}/result.txt"

compare_particlefilter "particlefilter" \
  "${OMP_PARTICLE_DIR}/particlefilter.log" \
  "${OCL_PARTICLE_DIR}/particlefilter_naive_output.txt" \
  1e-2 5e-3

compare_pgm "srad" \
  "${OMP_DIR}/image_out.pgm" \
  "${OCL_DIR}/image_out.pgm" \
  1

compare_streamcluster_centers "streamcluster" \
  "${OMP_STREAMCLUSTER_DIR}/streamcluster.txt" \
  "${OCL_STREAMCLUSTER_DIR}/streamcluster.txt" \
  65536 10 20 256
