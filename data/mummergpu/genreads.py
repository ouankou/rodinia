#!/usr/bin/env python3
# encoding: utf-8
"""
genreads outputs a multi-FASTA file containing a random sampling of
read-sized subsequences of the provided reference sequence.
"""

import argparse
import random
import sys


def read_fasta(path):
    with open(path, "r", encoding="utf-8") as handle:
        header = handle.readline()
        if not header.startswith(">"):
            raise ValueError("File is not FASTA format.")
        seq = "".join(line.strip() for line in handle)
    if not seq:
        raise ValueError("Reference sequence is empty.")
    return seq


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="Generate random read-sized subsequences from a FASTA file."
    )
    parser.add_argument("reference", help="FASTA reference file")
    parser.add_argument("length", type=int, help="Length of reads")
    parser.add_argument("count", type=int, help="Number of reads")
    parser.add_argument(
        "--seed",
        type=int,
        default=0,
        help="Random seed value (default: 0)",
    )
    args = parser.parse_args(argv)

    if args.length <= 0 or args.count <= 0:
        parser.error("Read length and count must be positive.")

    random.seed(args.seed)
    try:
        seq = read_fasta(args.reference)
    except ValueError as exc:
        print(f"genreads: {exc}", file=sys.stderr)
        return 2

    if args.length > len(seq):
        print("genreads: read length exceeds reference length.", file=sys.stderr)
        return 2

    max_start = len(seq) - args.length
    for i in range(args.count):
        start = random.randint(0, max_start)
        end = start + args.length
        print(f">rid{i + 1} {start + 1}-{end}")
        print(seq[start:end])

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
