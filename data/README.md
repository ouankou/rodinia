Rodinia data

This directory vendors input datasets from:
https://github.com/passlab/NeoRodinia-Old/tree/main/data

Most inputs are checked in. Large inputs that exceed GitHub size limits are
generated locally (gitignored):
  - mummergpu/NC_003997_q100bp.fna: 2,500,000 reads (100 bp) from NC_003997.fna
  - leukocyte/testfile.avi: 640x480, 30 fps, 600-frame synthetic video
  - kmeans/819200.txt: 819,200 objects with 34 features

Generate them with:
  scripts/generate_data.sh
  scripts/generate_data.sh --only <mummergpu|leukocyte|kmeans>
  scripts/generate_data.sh --force

Generator requirements: python3, a C compiler, and a C++ compiler.

The synthetic replacements are not bit-identical to published datasets, but
they preserve the workload sizes and formats so performance testing remains
meaningful.

Local generation (optional for smaller datasets):
  - bfs: data/bfs/inputGen (build graphgen and run for desired size; gen_dataset.sh includes graph1MW_6)
  - kmeans: data/kmeans/inpuGen (build datagen and run for desired size)
  - mummergpu: python3 data/mummergpu/genreads.py data/mummergpu/NC_003997.fna 100 <reads> > data/mummergpu/NC_003997_q100bp.fna
