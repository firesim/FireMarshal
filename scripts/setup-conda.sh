#!/usr/bin/env bash

set -ex

FDIR=$(git rev-parse --show-toplevel)

USE_UNPINNED_DEPS=false
GLOBAL_ENV_NAME=""

# getopts does not support long options, and is inflexible
while [ "$1" != "" ];
do
    case $1 in
        --conda-env-name)
            shift
            GLOBAL_ENV_NAME=${1} ;;
        -ud | --use-unpinned-deps )
            USE_UNPINNED_DEPS=true ;;
        * )
            echo "invalid option $1"
            exit 1 ;;
    esac
    shift
done

# note: lock file must end in .conda-lock.yml - see https://github.com/conda-incubator/conda-lock/issues/154
CONDA_REQS=$FDIR/conda-reqs
LOCKFILE=$CONDA_REQS/conda-requirements-linux-64.conda-lock.yml

# create conda-lock only environment to be used to generate the main environment with a valid conda-lock version
CONDA_LOCK_ENV_PATH=$FDIR/.conda-lock-env
rm -rf $CONDA_LOCK_ENV_PATH
conda create -y -p $CONDA_LOCK_ENV_PATH -c conda-forge $(grep "conda-lock" $CONDA_REQS/conda-reqs.yaml | sed 's/^ \+-//')
source $(conda info --base)/etc/profile.d/conda.sh
conda activate $CONDA_LOCK_ENV_PATH

if [ "$USE_UNPINNED_DEPS" = true ]; then
    # auto-gen the lockfiles
    $FDIR/scripts/generate-conda-lockfile.sh
fi
echo "Using lockfile for conda: $LOCKFILE"

# use conda-lock to create env
if [ -z "$GLOBAL_ENV_NAME" ] ; then
    CONDA_ENV_PATH=$FDIR/.conda-env
    CONDA_ENV_ARG="-p $CONDA_ENV_PATH"
    CONDA_ENV_NAME=$CONDA_ENV_PATH
else
    CONDA_ENV_ARG="-n $GLOBAL_ENV_NAME"
    CONDA_ENV_NAME=$GLOBAL_ENV_NAME
fi
echo "Storing main conda environment in $CONDA_ENV_NAME"

conda-lock install --conda $(which conda) $CONDA_ENV_ARG $LOCKFILE
