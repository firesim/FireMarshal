#!/usr/bin/env bash

set -ex

CUR_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

REQS_DIR="$CUR_DIR/../conda-reqs"
if [ ! -d "$REQS_DIR" ]; then
  echo "$REQS_DIR does not exist, something went wrong"
  exit 1
fi

REQS_FILE=$REQS_DIR/conda-reqs.yaml

if ! conda-lock --version | grep $(grep "conda-lock" $REQS_FILE | sed 's/^ \+-.*=//'); then
  echo "Invalid conda-lock version, make sure you're calling this script with the conda-lock version found in $REQS_FILE"
  exit 1
fi

# note: lock file must end in .conda-lock.yml - see https://github.com/conda-incubator/conda-lock/issues/154
LOCKFILE=$REQS_DIR/conda-requirements-linux-64.conda-lock.yml
rm -rf $LOCKFILE

conda-lock \
  --no-mamba \
  --no-micromamba \
  -f "$REQS_DIR/conda-reqs.yaml" \
  -f "$REQS_DIR/riscv-tools.yaml" \
  -p linux-64 \
  --lockfile $LOCKFILE
