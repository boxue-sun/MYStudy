FROM iregistry.baidu-int.com/zhiluos/airos:dev-amd64-20231208

RUN mkdir -p /home/airos
RUN mkdir -p /opt/airos
COPY airos /home/airos
COPY sdk /opt/airos/sdk
COPY package /opt/airos/package

ENV PATH "/opt/airos/package/bin:${PATH}"
