#!/usr/bin/env python3
import argparse
import math
import re
import sys
from pathlib import Path


NUMBER_RE = re.compile(r"[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?")
INTEGER_RE = re.compile(r"[-+]?\d+")


def read(path):
    return path.read_text(errors="replace")


def numbers(path):
    return [float(match.group(0)) for match in NUMBER_RE.finditer(read(path))]


def compare_numbers(name, lhs, rhs, abs_tol=1e-4, rel_tol=1e-5):
    if len(lhs) != len(rhs):
        raise AssertionError(f"{name}: numeric count mismatch {len(lhs)} != {len(rhs)}")
    worst = 0.0
    worst_idx = -1
    for idx, (a, b) in enumerate(zip(lhs, rhs)):
        if not math.isfinite(a) or not math.isfinite(b):
            raise AssertionError(f"{name}: value {idx} is non-finite {a} != {b}")
        diff = abs(a - b)
        tol = max(abs_tol, rel_tol * max(abs(a), abs(b), 1.0))
        if diff > tol:
            raise AssertionError(
                f"{name}: value {idx} mismatch {a} != {b} "
                f"(diff {diff}, tol {tol})"
            )
        if diff > worst:
            worst = diff
            worst_idx = idx
    return f"{name}: {len(lhs)} numeric values match (max diff {worst:g} at {worst_idx})"


def compare_numeric_file(name, openmp_dir, gpu_dir, rel_path, abs_tol=1e-4, rel_tol=1e-5):
    lhs = openmp_dir / name / rel_path
    rhs = gpu_dir / name / rel_path
    require(lhs)
    require(rhs)
    return compare_numbers(f"{name}/{rel_path}", numbers(lhs), numbers(rhs), abs_tol, rel_tol)


def compare_named_numeric_file(name, openmp_dir, gpu_dir, openmp_path, gpu_path,
                               abs_tol=1e-4, rel_tol=1e-5):
    lhs = openmp_dir / name / openmp_path
    rhs = gpu_dir / name / gpu_path
    require(lhs)
    require(rhs)
    return compare_numbers(f"{name}/{openmp_path}", numbers(lhs), numbers(rhs), abs_tol, rel_tol)


def compare_cfd_file(name, openmp_dir, gpu_dir, openmp_path, gpu_path,
                     abs_tol=1e-4, rel_tol=1e-5):
    lhs = numbers(openmp_dir / name / openmp_path)
    rhs = numbers(gpu_dir / name / gpu_path)
    if len(lhs) < 2 or len(rhs) < 2:
        raise AssertionError(f"{name}/{openmp_path}: missing CFD header")
    if lhs[0] != rhs[0]:
        raise AssertionError(f"{name}/{openmp_path}: element count mismatch {lhs[0]} != {rhs[0]}")
    return compare_numbers(f"{name}/{openmp_path}", lhs[2:], rhs[2:], abs_tol, rel_tol)


def compare_exact_file(name, openmp_dir, gpu_dir, rel_path):
    lhs = openmp_dir / name / rel_path
    rhs = gpu_dir / name / rel_path
    require(lhs)
    require(rhs)
    if lhs.read_bytes() != rhs.read_bytes():
        raise AssertionError(f"{name}/{rel_path}: files differ")
    return f"{name}/{rel_path}: exact match"


def compare_named_exact_file(name, openmp_dir, gpu_dir, openmp_path, gpu_path):
    lhs = openmp_dir / name / openmp_path
    rhs = gpu_dir / name / gpu_path
    require(lhs)
    require(rhs)
    if lhs.read_bytes() != rhs.read_bytes():
        raise AssertionError(f"{name}/{openmp_path}: files differ")
    return f"{name}/{openmp_path}: exact match"


def compare_optional_exact_file(name, openmp_dir, gpu_dir, rel_path):
    lhs = openmp_dir / name / rel_path
    rhs = gpu_dir / name / rel_path
    if not lhs.exists() and not rhs.exists():
        return None
    require(lhs)
    require(rhs)
    if lhs.read_bytes() != rhs.read_bytes():
        raise AssertionError(f"{name}/{rel_path}: files differ")
    return f"{name}/{rel_path}: exact match"


def require(path):
    if not path.is_file():
        raise AssertionError(f"missing expected file: {path}")


def log_value(name, root, pattern, cast=float):
    path = root / name / "run.log"
    require(path)
    match = re.search(pattern, read(path), re.MULTILINE)
    if not match:
        raise AssertionError(f"{name}: pattern not found in {path}: {pattern}")
    return cast(match.group(1))


def compare_log_value(name, openmp_dir, gpu_dir, pattern, abs_tol=1e-4, rel_tol=1e-5, cast=float):
    lhs = log_value(name, openmp_dir, pattern, cast)
    rhs = log_value(name, gpu_dir, pattern, cast)
    if cast is int:
        if lhs != rhs:
            raise AssertionError(f"{name}: log value mismatch {lhs} != {rhs}")
        return f"{name}: log value {lhs} matches"
    return compare_numbers(name, [lhs], [rhs], abs_tol, rel_tol)


def compare_named_log_value(label, openmp_name, gpu_name, openmp_dir, gpu_dir, pattern,
                            abs_tol=1e-4, rel_tol=1e-5, cast=float):
    lhs = log_value(openmp_name, openmp_dir, pattern, cast)
    rhs = log_value(gpu_name, gpu_dir, pattern, cast)
    if cast is int:
        if lhs != rhs:
            raise AssertionError(f"{label}: log value mismatch {lhs} != {rhs}")
        return f"{label}: log value {lhs} matches"
    return compare_numbers(label, [lhs], [rhs], abs_tol, rel_tol)


def compare_particlefilter(openmp_dir, gpu_dir):
    def trace(root):
        path = root / "particlefilter" / "run.log"
        require(path)
        values = []
        for line in read(path).splitlines():
            line = line.strip()
            if re.fullmatch(r"[-+]?\d+\.\d+", line):
                values.append(float(line))
        return values

    return compare_numbers(
        "particlefilter/distance-trace",
        trace(openmp_dir),
        trace(gpu_dir),
        abs_tol=1e-6,
        rel_tol=1e-7,
    )


def compare_nn(openmp_dir, gpu_dir):
    def nearest(root):
        path = root / "nn" / "run.log"
        require(path)
        lines = read(path).splitlines()
        try:
            start = lines.index("The 5 nearest neighbors are:")
        except ValueError as exc:
            raise AssertionError(f"nn: nearest-neighbor header missing in {path}") from exc
        return "\n".join(lines[start:start + 6])

    lhs = nearest(openmp_dir)
    rhs = nearest(gpu_dir)
    if lhs != rhs:
        raise AssertionError("nn: nearest-neighbor results differ")
    return "nn: nearest-neighbor results match"


def compare_streamcluster(openmp_dir, gpu_dir):
    return compare_numeric_file(
        "streamcluster",
        openmp_dir,
        gpu_dir,
        "streamcluster.txt",
        abs_tol=1e-4,
        rel_tol=1e-5,
    )


def compare_heartwall(openmp_dir, gpu_dir):
    lhs_path = openmp_dir / "heartwall" / "result.txt"
    rhs_path = gpu_dir / "heartwall" / "result.txt"
    require(lhs_path)
    require(rhs_path)
    lhs_lines = read(lhs_path).splitlines()
    rhs_lines = read(rhs_path).splitlines()
    if len(lhs_lines) != len(rhs_lines):
        raise AssertionError(f"heartwall/result.txt: line count mismatch {len(lhs_lines)} != {len(rhs_lines)}")
    lhs = [int(match.group(0)) for match in INTEGER_RE.finditer("\n".join(lhs_lines))]
    rhs = [int(match.group(0)) for match in INTEGER_RE.finditer("\n".join(rhs_lines))]
    if len(lhs) != len(rhs):
        raise AssertionError(f"heartwall/result.txt: numeric count mismatch {len(lhs)} != {len(rhs)}")
    if lhs[:4] != rhs[:4]:
        raise AssertionError(f"heartwall/result.txt: header mismatch {lhs[:4]} != {rhs[:4]}")
    return compare_numbers("heartwall/result.txt", lhs[4:], rhs[4:], abs_tol=20.0, rel_tol=0.0)


def compare_pathfinder(openmp_dir, gpu_dir):
    def values_without_timer(path):
        require(path)
        vals = []
        for line in read(path).splitlines():
            if line.startswith("timer:"):
                continue
            vals.extend(float(match.group(0)) for match in NUMBER_RE.finditer(line))
        return vals

    return compare_numbers(
        "pathfinder/pathfinder.out",
        values_without_timer(openmp_dir / "pathfinder" / "pathfinder.out"),
        values_without_timer(gpu_dir / "pathfinder" / "pathfinder.out"),
        0.0,
        0.0,
    )


def main():
    parser = argparse.ArgumentParser(description="Compare OpenMP and omp-gpu Rodinia outputs")
    parser.add_argument("--openmp", required=True, type=Path)
    parser.add_argument("--omp-gpu", required=True, type=Path)
    parser.add_argument("--only", action="append", default=[],
                        help="Compare only the named benchmark/check group; repeatable")
    args = parser.parse_args()

    openmp_dir = args.openmp.resolve()
    gpu_dir = args.omp_gpu.resolve()
    checks = [
        ("backprop", lambda: compare_log_value("backprop", openmp_dir, gpu_dir, r"Output error:\s+([-+0-9.eE]+)", 1e-5, 1e-5)),
        ("backprop", lambda: compare_log_value("backprop", openmp_dir, gpu_dir, r"Hidden error:\s+([-+0-9.eE]+)", 1e-5, 1e-5)),
        ("bfs", lambda: compare_exact_file("bfs", openmp_dir, gpu_dir, "result.txt")),
        ("b+tree", lambda: compare_named_exact_file("b+tree", openmp_dir, gpu_dir, "output.txt", "output.log")),
        ("cfd", lambda: compare_cfd_file("cfd", openmp_dir, gpu_dir, "density", "density.log", 1e-3, 1e-4)),
        ("cfd", lambda: compare_cfd_file("cfd", openmp_dir, gpu_dir, "density_energy", "density_energy.log", 1e-3, 1e-4)),
        ("cfd", lambda: compare_cfd_file("cfd", openmp_dir, gpu_dir, "momentum", "momentum.log", 1e-3, 1e-4)),
        ("cfd_double", lambda: compare_cfd_file("cfd_double", openmp_dir, gpu_dir, "density", "density.log", 1e-8, 1e-8)),
        ("cfd_double", lambda: compare_cfd_file("cfd_double", openmp_dir, gpu_dir, "density_energy", "density_energy.log", 1e-8, 1e-8)),
        ("cfd_double", lambda: compare_cfd_file("cfd_double", openmp_dir, gpu_dir, "momentum", "momentum.log", 1e-8, 1e-8)),
        ("cfd_pre", lambda: compare_cfd_file("cfd_pre", openmp_dir, gpu_dir, "density", "density.log", 1e-3, 1e-4)),
        ("cfd_pre", lambda: compare_cfd_file("cfd_pre", openmp_dir, gpu_dir, "density_energy", "density_energy.log", 1e-3, 1e-4)),
        ("cfd_pre", lambda: compare_cfd_file("cfd_pre", openmp_dir, gpu_dir, "momentum", "momentum.log", 1e-3, 1e-4)),
        ("cfd_pre_double", lambda: compare_cfd_file("cfd_pre_double", openmp_dir, gpu_dir, "density", "density.log", 1e-8, 1e-8)),
        ("cfd_pre_double", lambda: compare_cfd_file("cfd_pre_double", openmp_dir, gpu_dir, "density_energy", "density_energy.log", 1e-8, 1e-8)),
        ("cfd_pre_double", lambda: compare_cfd_file("cfd_pre_double", openmp_dir, gpu_dir, "momentum", "momentum.log", 1e-8, 1e-8)),
        ("heartwall", lambda: compare_heartwall(openmp_dir, gpu_dir)),
        ("hotspot", lambda: compare_numeric_file("hotspot", openmp_dir, gpu_dir, "hotspot.out", 1e-3, 5e-4)),
        ("hotspot3d", lambda: compare_numeric_file("hotspot3d", openmp_dir, gpu_dir, "hotspot3d.out", 1e-3, 5e-4)),
        ("kmeans", lambda: compare_log_value("kmeans", openmp_dir, gpu_dir, r"Cluster centers checksum:\s+([-+0-9.eE]+)", 1e-4, 1e-6)),
        ("lavaMD", lambda: compare_numeric_file("lavaMD", openmp_dir, gpu_dir, "result.txt", 1e-4, 1e-5)),
        ("leukocyte", lambda: compare_log_value("leukocyte", openmp_dir, gpu_dir, r"Cells detected:\s+(\d+)", cast=int)),
        ("leukocyte", lambda: compare_optional_exact_file("leukocyte", openmp_dir, gpu_dir, "result.txt")),
        ("lud", lambda: compare_named_log_value("lud", "lud_omp", "lud", openmp_dir, gpu_dir, r"LUD checksum:\s+([-+0-9.eE]+)", 1e-2, 1e-4)),
        ("myocyte", lambda: compare_numeric_file("myocyte", openmp_dir, gpu_dir, "output.txt", 1e-2, 1e-5)),
        ("nn", lambda: compare_nn(openmp_dir, gpu_dir)),
        ("nw", lambda: compare_named_exact_file("nw", openmp_dir, gpu_dir, "result.txt", "result.log")),
        ("particlefilter", lambda: compare_particlefilter(openmp_dir, gpu_dir)),
        ("pathfinder", lambda: compare_pathfinder(openmp_dir, gpu_dir)),
        ("srad_v1", lambda: compare_exact_file("srad_v1", openmp_dir, gpu_dir, "image_out.pgm")),
        ("srad_v2", lambda: compare_log_value("srad_v2", openmp_dir, gpu_dir, r"SRAD checksum:\s+([-+0-9.eE]+)", 1e-2, 1e-6)),
        ("streamcluster", lambda: compare_streamcluster(openmp_dir, gpu_dir)),
    ]
    if args.only:
        requested = set(args.only)
        known = {name for name, _ in checks}
        unknown = sorted(requested - known)
        if unknown:
            print(f"unknown --only benchmark(s): {', '.join(unknown)}", file=sys.stderr)
            return 2
        checks = [check for check in checks if check[0] in requested]

    failures = []
    for _, check in checks:
        try:
            result = check()
            if result:
                print(result)
        except AssertionError as exc:
            failures.append(str(exc))
            print(f"FAIL: {exc}", file=sys.stderr)

    if failures:
        print(f"{len(failures)} comparison(s) failed", file=sys.stderr)
        return 1
    print("All OpenMP vs omp-gpu comparisons passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
