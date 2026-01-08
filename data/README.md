Rodinia data

This repository does not vendor benchmark input datasets. Download the full
Rodinia data bundle on demand:

  scripts/generate_data.sh

By default the script fetches the data directory from:
  https://github.com/passlab/NeoRodinia-Old/tree/main/data

Override the source with:
  RODINIA_DATA_URL=<tarball-url> scripts/generate_data.sh
  scripts/generate_data.sh --url <tarball-url>

Requirements: curl or wget, plus tar.

Downloaded files are ignored by git.
