FROM ubuntu:latest

LABEL maintainer="dmitry@kernelgen.org"

ENV DEBIAN_FRONTEND noninteractive
ENV LC_ALL C.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US.UTF-8

RUN apt-get update && \
    apt-get -y --no-install-recommends install \
        build-essential \
        git \
        cmake \
        ninja-build \
        qt6-base-dev \
        libqscintilla2-qt6-dev && \
    apt-get clean

COPY . /root/
WORKDIR /root

RUN mkdir build && cd build && \
    cmake -GNinja -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . -j $(nproc) && \
    cpack

