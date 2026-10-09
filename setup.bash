#! /usr/bin/env bash
OUT_DIR="/home/airos/os"

# 3rd
for lib in $(ls "/opt/local")
do
    if [ -e "/opt/local/${lib}" ]; then
        export LD_LIBRARY_PATH=/opt/local/${lib}/lib:$LD_LIBRARY_PATH
    fi
done
# 检查 libgeographic-dev 是否安装
if ! dpkg-query -W -f='${Status}' libgeographic-dev 2>/dev/null | grep -q "^install"; then
    echo "libgeographic-dev is not installed. Installing it..."
    sudo apt-get install -y --no-install-recommends libgeographic-dev
fi
if ! dpkg-query -W -f='${Status}' libsqlite3-dev 2>/dev/null | grep -q "^install"; then
    echo "sqlite3 is not installed. Installing it..."
    sudo apt-get install -y --no-install-recommends libsqlite3-dev
fi
# cyberRT
source /opt/local/cyber-rt/setup.bash
export PYTHONPATH="/usr/local/lib/python2.7/dist-packages/protobuf-3.14.0-py2.7.egg:${PYTHONPATH}"

export GLOG_log_dir="/home/airos/log"
mkdir -p ${GLOG_log_dir}
export LANG=en_US.UTF-8
export LC_ALL=en_US.UTF-8
export GLOG_alsologtostderr=1
export GLOG_colorlogtostderr=1
export GLOG_minloglevel=0
export GLOG_max_log_size=120

export LD_LIBRARY_PATH=${OUT_DIR}/3rd:$LD_LIBRARY_PATH

# airservice
export LD_LIBRARY_PATH=${OUT_DIR}/lib:$LD_LIBRARY_PATH

#cuda
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/cuda-11.4/compat

# package
pkg_dir=(modules app)
for pkg in "${pkg_dir[@]}"; do
  for sub_dir in "${pkg}"/lib/*; do
    if [ -d "$sub_dir" ]; then
      export LD_LIBRARY_PATH="$sub_dir:$LD_LIBRARY_PATH"
      if [ -d "${sub_dir}/lib" ]; then
        export LD_LIBRARY_PATH="${sub_dir}/lib:$LD_LIBRARY_PATH"
      fi
    fi
  done
done

for all_device in ${OUT_DIR}/device/lib/*; do
  for device in "${all_device[@]}"; do
    for pkg in ${device}/*; do
      export LD_LIBRARY_PATH="$pkg:$LD_LIBRARY_PATH"
      if [ -d "${pkg}/lib" ]; then
        export LD_LIBRARY_PATH="${pkg}/lib:$LD_LIBRARY_PATH"
      fi
    done
  done
done

export PATH="/home/airos/script/":$PATH
export PATH="/opt/airos/package/bin":$PATH

# env
export PARAM_DIR="/home/airos/param/"
