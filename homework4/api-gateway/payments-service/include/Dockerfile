FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libpqxx-dev \
    libssl-dev \
    nlohmann-json3-dev \
    pkg-config \
    wget \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /tmp
RUN wget https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.12.4.tar.gz \
    && tar -xzf v0.12.4.tar.gz \
    && cp cpp-httplib-0.12.4/httplib.h /usr/local/include/

WORKDIR /app
COPY . .

RUN mkdir build && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release \
    && make -j$(nproc)

FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libpqxx-6.4 \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/payments-service /app/payments-service

EXPOSE 8080

CMD ["/app/payments-service"]