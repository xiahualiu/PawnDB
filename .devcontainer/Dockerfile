# syntax=docker/dockerfile:1.7
FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y curl \
    && curl -fsSL https://deb.nodesource.com/setup_22.x | bash - \
    && apt-get install -y nodejs \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        bash-completion \
        build-essential \
        ca-certificates \
        ccache \
        clang-20 \
        clangd-20 \
        clang-format-20 \
        libclang-rt-20-dev \
        cmake \
        curl \
        doxygen \
        gcovr \
        git \
        gnupg \
        graphviz \
        mscgen \
        ninja-build \
        pkg-config \
        pre-commit \
        llvm-20 \
        python3 \
        sudo \
    && mkdir -p -m 755 /etc/apt/keyrings \
    && curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg | dd of=/etc/apt/keyrings/githubcli-archive-keyring.gpg \
    && chmod go+r /etc/apt/keyrings/githubcli-archive-keyring.gpg \
    && echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" > /etc/apt/sources.list.d/github-cli.list \
    && apt-get update \
    && apt-get install -y --no-install-recommends gh \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /workspaces \
    && chown -R ubuntu:ubuntu /workspaces \
    && echo 'ubuntu ALL=(ALL) NOPASSWD:ALL' > /etc/sudoers.d/ubuntu \
    && chmod 0440 /etc/sudoers.d/ubuntu

USER ubuntu

RUN --mount=type=secret,id=gh_token,required=true,uid=1000,gid=1000,mode=0400 \
    test -s /run/secrets/gh_token \
    || (echo "Error: required secret 'gh_token' is missing or empty at /run/secrets/gh_token" >&2; exit 1) \
    && gh auth login --with-token < /run/secrets/gh_token \
    && gh auth status

WORKDIR /workspaces
