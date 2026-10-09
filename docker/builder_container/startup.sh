#!/bin/bash
# start.sh

# 定义版本文件路径
AIROS_VERSION_FILE="/home/airos/os/.airos_version"
BACKUP_DIR="/home/airos_bak"

# 检查版本文件是否存在
if [ -e "$AIROS_VERSION_FILE" ]; then
    echo "The file $AIROS_VERSION_FILE exists."
else
    echo "The file $AIROS_VERSION_FILE does not exist."

    # 自动识别备份目录中的唯一文件
    BACKUP_FILE=$(ls -1 "$BACKUP_DIR"/*.tar.gz 2>/dev/null | head -n 1)

    if [ -n "$BACKUP_FILE" ]; then
        echo "Extracting backup archive $BACKUP_FILE..."
        tar -xzf "$BACKUP_FILE" -C /home/airos/
    else
        echo "No backup archive found in $BACKUP_DIR."
    fi
fi

# 调用开机脚本文件
bash /home/airos/script/airos_auto_start.sh
