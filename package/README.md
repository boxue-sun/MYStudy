# Make SDK IMG

```bash
# 创建运行airos docker
  bash docker/scripts/docker_start.sh
# 进入docker
  bash docker/scripts/docker_into.sh
# 编译airos
  bash build_airos.sh release
# 产出sdk
  bash package/make_sdk.sh
# 产出package
  cd package
  bash make_pkg.sh
# 退出docker
  exit
# 创建airos sdk镜像
  bash package/make_sdk_img.sh
# 安装cudaruntime
  ...
# 创建airos sdk容器
  docker run --gpus all -itd --name "airos-sdk-dev" airos-sdk bash
# 进入airos sdk容器
  docker exec -it airos-sdk-dev bash
```