#!/bin/bash

LOG_PATH="/home/airos/log/airos_package_update.log"

return_info() {
  local DATE_N=$(date "+%Y-%m-%d %H:%M:%S")
  [[ $# -eq 0 ]] && return
  echo -e "\033[1;32m [INFO]--- ${DATE_N} $* \033[0m" >> "$LOG_PATH"
}

# Check if the correct number of arguments is passed
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <compressed file path>"
    exit 1
fi

# Define the compressed file path
TAR_FILE="$1"

# Check if the compressed file exists
if [ ! -f "$TAR_FILE" ]; then
    return_info "Compressed file $TAR_FILE does not exist"
    echo "Compressed file $TAR_FILE does not exist"
    exit 1
fi

VERSION_FILE_PATH="/home/airos/os/.airos_version"
return_info "old version: $(cat $VERSION_FILE_PATH)"
echo "old version: $(cat $VERSION_FILE_PATH)"

# Environment variables
source /home/airos/os/setup.bash
airos_launch stop >> $LOG_PATH

# Extract and forcefully overwrite files to the current directory
return_info "Extracting $TAR_FILE to the current directory and forcefully overwriting..."
echo "Extracting $TAR_FILE to the current directory and forcefully overwriting..."
tar --overwrite -xzf "$TAR_FILE"

airos_launch all  >> $LOG_PATH

return_info "update finshed! version: $(cat $VERSION_FILE_PATH)"
echo "update finshed! version: $(cat $VERSION_FILE_PATH)"
