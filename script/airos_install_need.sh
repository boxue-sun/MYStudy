#! /usr/bin/env bash
OUT_DIR="/home/airos/os"

# 3rd
for lib in $(ls "/opt/local")
do
    if [ -e "/opt/local/${lib}" ]; then
        export LD_LIBRARY_PATH=/opt/local/${lib}/lib:$LD_LIBRARY_PATH
    fi
done

install_apt_package()
{
    PACKAGE_NAME=$1  # 需要检查和安装的包名
    PACKAGE_DIR=$2  # 包含 .deb 文件的目录

    # 检查包是否已安装
    if ! dpkg-query -W -f='${Status}' "$PACKAGE_NAME" 2>/dev/null | grep -q "^install"; then
        echo "$PACKAGE_NAME is not installed. Attempting to install it via apt-get..."
        ls
        # 检查 need_package 目录是否存在
        if [ -d "$PACKAGE_DIR" ]; then
            # 这里假设是 deb 文件
            dpkg -i "$PACKAGE_DIR/$PACKAGE_NAME"*.deb
            # 处理依赖关系
            apt-get install -f
        else
            echo "$PACKAGE_DIR directory does not exist."
            # 尝试通过 apt-get 安装
            if ! apt-get install -y --no-install-recommends "$PACKAGE_NAME"; then
                echo "apt-get installation failed. Attempting to install from the $PACKAGE_DIR directory..."
            else
                echo "$PACKAGE_NAME installed successfully via apt-get."
            fi
        fi
    fi
}
PACKAGE_APT_NAME="net-tools"
install_apt_package "${PACKAGE_APT_NAME}" "${PACKAGE_APT_DIR}"

PACKAGE_APT_NAME="libgeographic-dev"
install_apt_package "${PACKAGE_APT_NAME}" "${PACKAGE_APT_DIR}"

PACKAGE_APT_NAME="libsqlite3-dev"
install_apt_package "${PACKAGE_APT_NAME}" "${PACKAGE_APT_DIR}"
PACKAGE_APT_NAME="sqlite3"
install_apt_package "${PACKAGE_APT_NAME}" "${PACKAGE_APT_DIR}"
PACKAGE_APT_NAME="libssh-dev"
PACKAGE_APT_DIR="/home/airos/need_package"
install_apt_package "${PACKAGE_APT_NAME}" "${PACKAGE_APT_DIR}"

install_python_package()
{
    PACKAGE_NAME="$1"  # 需要检查和安装的 Python 包名
    PACKAGE_DIR="$2"   # 包含 .whl 文件的目录

    # 检查 Python 包是否已安装
    if ! pip show "$PACKAGE_NAME" > /dev/null 2>&1; then
        echo "$PACKAGE_NAME is not installed. Attempting to install it via pip..."
        ls
        # 检查 need_package 目录是否存在
        if [ -d "$PACKAGE_DIR" ]; then
            # 进入 need_package 目录
            cd "$PACKAGE_DIR" || { echo "Failed to enter $PACKAGE_DIR directory."; exit 1; }

            # 使用 pip 安装 whl 文件
            if ls *.whl 1> /dev/null 2>&1; then
                echo "Installing $PACKAGE_NAME from wheel file..."
#                pip install --no-index --find-links=$PACKAGE_DIR *.whl
                 pip install --no-index --find-links=./ paramiko pyftpdlib pydatahub
            else
                echo "No wheel file found in $PACKAGE_DIR directory."
            fi
            cd - # 返回原始目录
        else
            echo "$PACKAGE_DIR directory does not exist."
            # 尝试通过 pip 安装ls
            if ! pip install "$PACKAGE_NAME"; then
                echo "pip installation failed. Attempting to install from the $PACKAGE_DIR directory..."
            else
                echo "$PACKAGE_NAME installed successfully via pip."
            fi
        fi

    fi
}

# 调用函数，传递参数
PACKAGE_PYTHON_NAME="paramiko"
PACKAGE_PYTHON_DIR="/home/airos/need_package"
install_python_package "${PACKAGE_PYTHON_NAME}" "${PACKAGE_PYTHON_DIR}"

PACKAGE_PYTHON_NAME="pyftpdlib"
install_python_package "${PACKAGE_PYTHON_NAME}" "${PACKAGE_PYTHON_DIR}"

PACKAGE_PYTHON_NAME="pydatahub"
install_python_package "${PACKAGE_PYTHON_NAME}" "${PACKAGE_PYTHON_DIR}"