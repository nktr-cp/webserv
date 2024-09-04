# Dockerfile to build in linux environment
FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

CMD ["/bin/bash"]
