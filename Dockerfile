FROM alpine:latest as builder

RUN apk update && \
    apk add  \
        gcc \
        g++ \
        ccache \
        musl-dev \
        linux-headers \
        libgmpxx \
        cmake \
        make \
        git \
        perl \
        python3 \
        py3-pip \
        py3-setuptools && \
    pip3 install --user dataclasses_json Jinja2 importlib_resources pluginbase gitpython

ADD . /koinos-services
WORKDIR /koinos-services

ENV CC=/usr/lib/ccache/bin/gcc
ENV CXX=/usr/lib/ccache/bin/g++

RUN mkdir -p /koinos-services/.ccache && \
    ln -s /koinos-services/.ccache $HOME/.ccache && \
    git submodule update --init --recursive && \
    cmake -DCMAKE_BUILD_TYPE=Release . && \
    cmake --build . --config Release --parallel

FROM alpine:latest
RUN apk update && \
    apk add \
        musl \
        libstdc++
COPY --from=builder /koinos-services/programs/koinos_services/koinos_services /usr/local/bin
ENTRYPOINT [ "/usr/local/bin/koinos_services" ]
