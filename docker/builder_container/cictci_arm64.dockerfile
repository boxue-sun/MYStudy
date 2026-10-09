# -----------------------------------------------------------------------------
# Dockerfile for Building Cictci Airos Image
# -----------------------------------------------------------------------------
# Author: Chang Xuhui
# Date: 2024-07-08
# Version: 1.0
# Description: 基于百度镜像, 构建airos镜像
#              构建命令:
# docker build -f cictci_arm64.dockerfile -t cictci/airos:dev-arm64-20240708 .
# -----------------------------------------------------------------------------
# Changes:
# -----------------------------------------------------------------------------

# 基于百度提供的镜像
FROM registry.baidubce.com/zhiluos/airos:dev-arm64-20240625

# 更新包列表并安装服务
RUN apt update && \
    apt install -y --no-install-recommends \
    openssh-server libsqlite3-dev libgeographic-dev \
    tcpdump net-tools cron logrotate mosquitto  sshpass libssh-dev sqlite3

# 安装 paramiko 库
RUN pip3 install paramiko pyftpdlib pydatahub -i https://repo.huaweicloud.com/repository/pypi/simple/

# 创建目录以便于 SSH 服务的运行
RUN mkdir /var/run/sshd && \
    chmod 700 /var/run/sshd && \
    chown root:root /var/run/sshd

# 设置 root 密码
RUN echo 'root:cictci@2' | chpasswd

# 配置 SSH 服务
RUN sed -i 's/#Port 22/Port 2222/' /etc/ssh/sshd_config
RUN sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config
RUN sed -i 's/#PubkeyAuthentication yes/PubkeyAuthentication no/' /etc/ssh/sshd_config

# 拷贝程序到容器中
# COPY airos-v3.1-v1.5.0-deqing-release-202411071015.tar.gz /home/airos_bak/
# RUN tar -xzf /home/airos/airos-v1.4.6-release-202408071057.tar.gz -C /home/airos/ && \
#     rm /home/airos/airos-v1.4.6-release-202408071057.tar.gz

# 复制并解压 asn-wrapper.tar.gz 文件
COPY asn-wrapper.tar.gz /opt/local/
RUN rm -rf /opt/local/asn-wrapper && \
    tar -xzf /opt/local/asn-wrapper.tar.gz -C /opt/local/ && \
    rm /opt/local/asn-wrapper.tar.gz

# 复制并解压 airos_package.tar.gz 文件 (包管理相关功能)
COPY airos_package.tar.gz /opt/
RUN rm -rf /opt/airos && \
    tar -xzf /opt/airos_package.tar.gz -C /opt/ && \
    rm /opt/airos_package.tar.gz

# 修改 .bashrc 文件
RUN echo 'source /home/airos/os/setup.bash' >> /root/.bashrc && \
    echo 'cd /home/airos' >> /root/.bashrc

COPY startup.sh /opt/local/start/
RUN chmod +x /opt/local/start/startup.sh

# 设置开机启动脚本
CMD ["/opt/local/start/startup.sh"]
# CMD ["/bin/bash"]
