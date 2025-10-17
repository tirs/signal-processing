FROM ubuntu:22.04

LABEL maintainer="Satellite Signal Processing Project"
LABEL description="Production-ready satellite signal processing framework"

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    python3 \
    python3-pip \
    python3-matplotlib \
    libopenblas-dev \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /app
WORKDIR /app

COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build --config Release

RUN python3 -m pip install matplotlib numpy scipy

ENV PATH="/app/build:${PATH}"

EXPOSE 8080

VOLUME ["/data", "/output"]

RUN mkdir -p /data /output

RUN chmod +x ./build/satellite_signal_demo || true
RUN chmod +x ./build/benchmark_suite || true
RUN chmod +x ./build/unit_tests || true

CMD ["satellite_signal_demo", "--help"]