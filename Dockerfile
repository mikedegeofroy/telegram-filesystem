# Use the CentOS 7 base image
FROM  --platform=linux/amd64 centos:7

# Install necessary tools and libraries
RUN yum -y update && \
    yum -y install gcc-c++ make && \
    yum -y groupinstall "Development Tools" && \
    yum clean all

RUN set -ex \
    && for key in C6C265324BBEBDC350B513D02D2CEF1034921684; do \
    gpg --keyserver hkp://p80.pool.sks-keyservers.net:80 --recv-keys "$key" || \
    gpg --keyserver hkp://ipv4.pool.sks-keyservers.net --recv-keys "$key" || \
    gpg --keyserver hkp://pgp.mit.edu:80 --recv-keys "$key" ; \
    done

ENV CMAKE_VERSION 3.8.2

RUN set -ex \
    && curl -fsSLO --compressed https://cmake.org/files/v3.8/cmake-${CMAKE_VERSION}-Linux-x86_64.tar.gz \
    && curl -fsSLO --compressed https://cmake.org/files/v3.8/cmake-${CMAKE_VERSION}-SHA-256.txt.asc \
    && curl -fsSLO --compressed https://cmake.org/files/v3.8/cmake-${CMAKE_VERSION}-SHA-256.txt \
    && gpg --verify cmake-${CMAKE_VERSION}-SHA-256.txt.asc cmake-${CMAKE_VERSION}-SHA-256.txt \
    && grep "cmake-${CMAKE_VERSION}-Linux-x86_64.tar.gz\$" cmake-${CMAKE_VERSION}-SHA-256.txt | sha256sum -c - \
    && tar xzf cmake-${CMAKE_VERSION}-Linux-x86_64.tar.gz -C /usr/local --strip-components=1 --no-same-owner \
    && rm -rf cmake-${CMAKE_VERSION}*

# Set the working directory
WORKDIR /root

# Default command
CMD ["/bin/bash"]
