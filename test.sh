#!/bin/bash

# 检查传入参数的数量
if [[ $# -ne 2 ]]; then
    echo "[ERROR] Incorrect number of arguments."
    echo "[INFO] Usage: $0 <TAR_FILE> <YAML_FILE>"
    exit 1
fi

# 传入的 TAR 和 YAML 文件
TAR_FILE="$1"
YAML_FILE="$2"

# 设置临时目录
TEMP_DIR="temp/$(basename $TAR_FILE .tar.gz)_temp"

# 打印解压命令
echo "[INFO] Executing: tar -xvzf $TAR_FILE -C $TEMP_DIR"
# 解压文件到临时目录
echo "[INFO] Extracting $TAR_FILE to $TEMP_DIR..."
mkdir -p $TEMP_DIR
tar -xvzf $TAR_FILE -C $TEMP_DIR

# 查找解压后的 version.txt 文件
VERSION_FILE=$(find $TEMP_DIR -name "version.txt" | head -n 1)

# 如果找不到 version.txt，则退出
if [[ -z "$VERSION_FILE" ]]; then
    echo "[ERROR] version.txt not found in the extracted files."
    exit 1
fi

echo "[INFO] Found version.txt at $VERSION_FILE"

# 读取 version.txt 中的内容
TYPE=$(jq -r '.type' $VERSION_FILE)
NAME=$(jq -r '.name' $VERSION_FILE)

echo "[INFO] Parsed version.txt: type=$TYPE, name=$NAME"

# 获取模块目录路径
MODULE_DIR=$(dirname "$VERSION_FILE")
echo "[INFO] MODULE_DIR is $MODULE_DIR"

# 打印安装命令
echo "[INFO] Executing: airospkg install -file $TAR_FILE"
# 安装文件
airospkg install -file $TAR_FILE

# 打印运行命令并让其在后台运行
echo "[INFO] Executing in background: nohup airospkg run -type $TYPE -name $NAME > temp/$(basename $TAR_FILE .tar.gz).log 2>&1 &"
# 运行命令（在后台执行）
nohup airospkg run -type $TYPE -name $NAME > temp/$(basename $TAR_FILE .tar.gz).log 2>&1 &

# 打印完成提示
echo "[INFO] Command execution started in background."

# 清理临时解压的文件
echo "[INFO] Executing: rm -rf $TEMP_DIR"
rm -rf $TEMP_DIR
echo "[INFO] Temporary files cleaned up."

# 可以选择等待或进一步的调试信息输出
echo "[INFO] Execution completed. Please check logs for further details."
