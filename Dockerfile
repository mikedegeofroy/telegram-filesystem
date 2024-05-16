# Use the Ubuntu 20.04 base image
FROM ubuntu:20.04

# Enable all repositories and add the PPA for a newer GCC version
RUN sed -i 's/# deb/deb/g' /etc/apt/sources.list && \
    apt-get update && \
    apt-get install -y software-properties-common && \
    add-apt-repository ppa:ubuntu-toolchain-r/test && \
    apt-get update && \
    apt-get install -y gcc-10 cmake g++-10 wget make pkg-config libfuse-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Update alternatives to use gcc-10 and g++-10
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 60 --slave /usr/bin/g++ g++ /usr/bin/g++-10 && \
    update-alternatives --set gcc /usr/bin/gcc-10

# Set the working directory
WORKDIR /root

# Default command
CMD ["/bin/bash"]
