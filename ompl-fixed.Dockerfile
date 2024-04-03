FROM ubuntu:jammy as builder
# avoid interactive configuration dialog from tzdata, which gets pulled in
# as a dependency
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        castxml \
        cmake \
        libboost-filesystem-dev \
        libboost-numpy-dev \
        libboost-program-options-dev \
        libboost-python-dev \
        libboost-serialization-dev \
        libboost-system-dev \
        libboost-test-dev \
        libeigen3-dev \
        libexpat1 \
        libflann-dev \
        libtriangle-dev \
        ninja-build \
        pkg-config \
        python3-dev \
        python3-numpy \
        python3-pip \
        pypy3 \
        wget \
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/{apt,dpkg,cache,log} /tmp/* /var/tmp/*
RUN pip3 install pygccxml pyplusplus
COPY . /ompl
WORKDIR /ompl
RUN mkdir -p build/Release \
    && cd build/Release \
    && NPROC=$(nproc) \
    && HALF_NPROC=$((NPROC / 2)) \
    && cmake ../.. \
    && make -j $HALF_NPROC update_bindings \
    && make -j $HALF_NPROC
# RUN cmake \
#         -G Ninja \
#         -B build \
#         -DPYTHON_EXEC=/usr/bin/python3 \
#         -DOMPL_REGISTRATION=OFF \
#         -DCMAKE_INSTALL_PREFIX=/usr \
#     && cmake --build build -t update_bindings \
#     && NPROC=$(nproc) \
#     && HALF_NPROC=$((NPROC / 2)) \
#     && cmake --build build -- -j $HALF_NPROC \
#     && cmake --install build \
#     && cd tests/cmake_export \
#     && cmake -B build -DCMAKE_INSTALL_PREFIX=../../install \
#     && cmake --build build

# FROM builder
# ENV DEBIAN_FRONTEND=noninteractive
# RUN apt-get update \
#     && apt-get install -y --no-install-recommends \
#         build-essential \
#         cmake \
#         libboost-filesystem-dev \
#         libboost-numpy-dev \
#         libboost-program-options-dev \
#         libboost-python-dev \
#         libboost-serialization-dev \
#         libboost-system-dev \
#         libeigen3-dev \
#         libflann-dev \
#         libmongoc-dev \
#         libtriangle-dev \
#         ninja-build \
#         pkg-config \
#         python3-dev \
#         python3-numpy \
#         python3-pip \
#         wget \
#     && apt-get autoremove -y \
#     && apt-get clean -y \
#     && rm -rf /var/lib/apt/lists/{apt,dpkg,cache,log} /tmp/* /var/tmp/*
# ENV DEBIAN_FRONTEND=
#
# COPY --from=ompl-builder /usr /usr
