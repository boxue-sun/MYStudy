#!/bin/bash
# 定义容器名称变量
CONTAINER_NAME=airos2.0_dev_root
# 定义主机目录变量
HOST_DIR=/work/airos2.0

# 检查是否以 root 用户执行
if [ "$(id -u)" -ne 0 ]; then
    echo "This script must be run as root."
    exit 1
fi
create_container=false
# 检查是否提供了文件名参数
if [ -z "$1" ]; then
    echo "Usage: $0 filename.tar.gz [cc]"
    exit
fi

# 检查第二个参数是否为 "cc"
if [ "$2" == "cc" ]; then
    create_container=true
fi

# 获取输入文件的文件名
FILE=$1
if [ "$create_container" = false ]; then
    echo "Container creation is not allowed. Exiting."

  # 检查文件是否存在
  if [ ! -f "$FILE" ]; then
      echo "File $FILE does not exist."
      exit 1
  fi

  # 创建一个临时目录来解压文件
  TEMP_DIR=$(mktemp -d ./temp.XXXXXX)
  if [ ! -d "$TEMP_DIR" ]; then
      echo "Failed to create temporary directory."
      exit 1
  fi

  # 解压文件到临时目录，并显示进度
  echo "Extracting files..."
  tar --checkpoint=.1000 --checkpoint-action=echo="Extracting: %T" -xzvf "$FILE" -C "$TEMP_DIR"

  # 检查解压是否成功
  if [ $? -eq 0 ]; then
      echo "File $FILE successfully extracted."

      # 查找解压后的 .tar 文件
      TAR_FILE=$(find "$TEMP_DIR" -name "*.tar")
      if [ -z "$TAR_FILE" ]; then
          echo "No .tar file found after extraction."
          rm -rf "$TEMP_DIR"
          exit 1
      fi

      # 计算解压后的文件总大小
      TOTAL_SIZE=$(du -sh "$TEMP_DIR" | cut -f1)
      echo "Total size of extracted files: $TOTAL_SIZE"

      # 加载 Docker 镜像
      echo "Loading Docker image from $TAR_FILE..."
      docker load -i "$TAR_FILE"

      if [ $? -eq 0 ]; then
          echo "Docker image successfully loaded."
      else
          echo "Failed to load Docker image from $TAR_FILE."
      fi

      # 删除临时目录
      rm -rf "$TEMP_DIR"
  else
      echo "Failed to extract $FILE."
      rm -rf "$TEMP_DIR"
      exit 1
  fi
fi

############################################## 创建容器
# 镜像名称
IMAGE_NAME="cictci/airos:$(basename "$FILE" .tar.gz)"
echo "Image name $IMAGE_NAME"



# 检查容器是否存在，并根据需要删除
if [ "$(docker ps -aq -f name=$CONTAINER_NAME)" ]; then
  echo "Container $CONTAINER_NAME already exists. Removing it..."
  docker rm -f $CONTAINER_NAME
else
  echo "Container $CONTAINER_NAME does not exist."
fi

echo "Start creat container $CONTAINER_NAME... "

# 使用变量启动容器
docker run --gpus all --runtime=nvidia -itd --restart unless-stopped \
--name $CONTAINER_NAME --net host \
-v ${HOST_DIR}:/home/airos \
-v /media:/media \
-v /tmp/.X11-unix:/tmp/.X11-unix:rw \
-v /etc/localtime:/etc/localtime:ro \
-v /usr/src:/usr/src:ro \
-v /lib/modules:/lib/modules:ro \
--shm-size 2G \
-e USER=root \
-e DOCKER_USER=root \
-e DOCKER_USER_ID=0 \
-e DOCKER_GRP=root \
-e DOCKER_GRP_ID=0 \
-e DOCKER_IMG=$IMAGE_NAME \
-e DISPLAY=:0 \
-e USE_GPU_HOST=1 \
-e NVIDIA_VISIBLE_DEVICES=all \
-e NVIDIA_DRIVER_CAPABILITIES=compute,video,graphics,utility \
--add-host in-docker:127.0.0.1 \
--add-host lcfc-desktop:127.0.0.1 \
--hostname in-docker \
-w /home/airos \
--privileged $IMAGE_NAME
