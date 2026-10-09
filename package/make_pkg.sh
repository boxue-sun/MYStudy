#! /usr/bin/env bash
set -e

dir_script="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
echo "[INFO]dir script:${dir_script}"

cd ${dir_script}/..
bazel build package/src/...

cd ${dir_script}
if [[ -e package ]];then
    rm -rf package
fi

mkdir -p package/bin
cp ../bazel-bin/package/src/airospkg package/bin/
cp template package/ -rf
# tar -zcf package.tar.gz package/
#rm pkg -rf
