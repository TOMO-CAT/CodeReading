FROM ubuntu:22.04

# apt install
RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list
RUN apt-get clean && apt-get update && apt-get install -y --fix-missing \
    build-essential git curl wget sudo unzip

# install xmake
RUN wget https://xmake.io/shget.text -O - | bash && /bin/bash -c "source ~/.xmake/profile"
ENV  XMAKE_ROOT=y

# work dir
WORKDIR /Coost
