FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    gdb \
    git \
    python3 \
    python3-pip \
	python3-venv \
	&& apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# error: externally-managed-environment
# RUN pip3 install --no-cache-dir requests

RUN python3 -m venv /opt/venv && \
    /opt/venv/bin/pip install --no-cache-dir requests

ENV PATH="/opt/venv/bin:$PATH"


WORKDIR /workspace
