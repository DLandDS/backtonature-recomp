FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV PATH="/opt/decomp-python/bin:/opt/decomp-tools/bin:/usr/local/bin:${PATH}"
ENV PYTHONDONTWRITEBYTECODE=1
ENV PIP_DISABLE_PIP_VERSION_CHECK=1

RUN apt-get update && apt-get install -y --no-install-recommends \
    bash \
    ca-certificates \
    curl \
    wget \
    git \
    file \
    less \
    vim-tiny \
    nano \
    xz-utils \
    bzip2 \
    unzip \
    zip \
    patch \
    diffutils \
    rsync \
    jq \
    build-essential \
    gcc \
    g++ \
    gcc-multilib \
    libc6-dev-i386 \
    make \
    cmake \
    ninja-build \
    pkg-config \
    python3 \
    python3-pip \
    python3-venv \
    binutils-mipsel-linux-gnu \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /opt/decomp-tools/bin /work

RUN python3 -m venv /opt/decomp-python \
    && /opt/decomp-python/bin/python -m pip install -U pip setuptools wheel \
    && /opt/decomp-python/bin/python -m pip install \
        "splat64[mips]" \
        spimdisasm \
        "asm-differ @ git+https://github.com/simonlindholm/asm-differ.git" \
        "m2c @ git+https://github.com/matt-kempster/m2c.git"

RUN git clone --depth=1 https://github.com/simonlindholm/decomp-permuter.git /opt/decomp-tools/decomp-permuter \
    && python3 -m venv /opt/decomp-tools/decomp-permuter/.venv \
    && /opt/decomp-tools/decomp-permuter/.venv/bin/python -m pip install -U pip setuptools wheel \
    && /opt/decomp-tools/decomp-permuter/.venv/bin/python -m pip install \
        pycparser \
        pynacl \
        toml \
        Levenshtein \
        docker \
    && printf '%s\n' \
        '#!/usr/bin/env bash' \
        'exec /opt/decomp-tools/decomp-permuter/.venv/bin/python /opt/decomp-tools/decomp-permuter/permuter.py "$@"' \
        > /opt/decomp-tools/bin/permuter \
    && chmod +x /opt/decomp-tools/bin/permuter

# Pinned, because maspsx decides where load-delay nops go and an unpinned
# clone would silently change the bytes this project exists to reproduce.
ARG MASPSX_REV=746b895f02929ecd148af7b1f4ff05b69f973878
COPY patches/maspsx-extern-sdata.patch /tmp/maspsx-extern-sdata.patch
RUN git clone https://github.com/mkst/maspsx.git /opt/decomp-tools/maspsx \
    && git -C /opt/decomp-tools/maspsx checkout --detach ${MASPSX_REV} \
    && git -C /opt/decomp-tools/maspsx apply --recount --verbose /tmp/maspsx-extern-sdata.patch \
    && python3 -m venv /opt/decomp-tools/maspsx/.venv \
    && /opt/decomp-tools/maspsx/.venv/bin/python -m pip install -U pip setuptools wheel \
    && /opt/decomp-tools/maspsx/.venv/bin/python -m pip install \
        "PyYAML>=6,<7" \
        "rabbitizer==1.12.0" \
        "spimdisasm==1.28.0" \
    && printf '%s\n' \
        '#!/usr/bin/env bash' \
        'exec /opt/decomp-tools/maspsx/.venv/bin/python /opt/decomp-tools/maspsx/maspsx.py "$@"' \
        > /opt/decomp-tools/bin/maspsx \
    && chmod +x /opt/decomp-tools/bin/maspsx

RUN git clone --depth=1 --recurse-submodules https://github.com/Lameguy64/mkpsxiso.git /opt/decomp-tools/mkpsxiso \
    && cmake -S /opt/decomp-tools/mkpsxiso -B /opt/decomp-tools/mkpsxiso/build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build /opt/decomp-tools/mkpsxiso/build --parallel \
    && find /opt/decomp-tools/mkpsxiso/build -type f -name mkpsxiso -exec install -m 0755 {} /usr/local/bin/mkpsxiso \; -quit \
    && find /opt/decomp-tools/mkpsxiso/build -type f -name dumpsxiso -exec install -m 0755 {} /usr/local/bin/dumpsxiso \; -quit \
    && test -x /usr/local/bin/mkpsxiso \
    && test -x /usr/local/bin/dumpsxiso

RUN git clone --depth=1 https://github.com/decompals/old-gcc.git /opt/decomp-tools/old-gcc \
    && printf '%s\n' \
        '#!/usr/bin/env bash' \
        'set -euo pipefail' \
        'version="${1:-}"' \
        'if [ -z "$version" ]; then echo "usage: old-gcc-build <version>" >&2; exit 2; fi' \
        'cd /opt/decomp-tools/old-gcc' \
        'VERSION="$version" make' \
        > /opt/decomp-tools/bin/old-gcc-build \
    && chmod +x /opt/decomp-tools/bin/old-gcc-build

RUN curl -L \
        -o /usr/local/bin/objdiff-cli \
        https://github.com/encounter/objdiff/releases/download/v3.8.0/objdiff-cli-linux-x86_64 \
    && echo "bc1e047126f9c6914bd1695798175234642ab9eaf45e886f841b59a4231e1a81  /usr/local/bin/objdiff-cli" | sha256sum -c - \
    && chmod +x /usr/local/bin/objdiff-cli

# wibo: a minimal Win32 loader, enough to run 1998 console-mode PE binaries.
# Phase 02 identified this game's SDK as PSY-Q 4.6 and byte-verified 147/150 of
# its objects, and that SDK is on disk with its original compiler intact. Running
# the real CC1PSX.EXE removes the guesswork about which GCC 2.x approximates it
# and with which flags -- it is the compiler that built this game.
RUN curl -L \
        -o /usr/local/bin/wibo \
        https://github.com/decompals/wibo/releases/download/1.2.0/wibo-x86_64 \
    && echo "13f86a2d618f0dbe67179d349625345eabf9b46450295cb4c904e49f6aff85af  /usr/local/bin/wibo" | sha256sum -c - \
    && chmod +x /usr/local/bin/wibo

RUN printf '%s\n' \
    '#!/usr/bin/env bash' \
    'set -euo pipefail' \
    'echo "== host/workspace =="' \
    'pwd' \
    'test -d /work && ls -la /work | sed -n "1,40p"' \
    'echo "== python decomp tools =="' \
    'python --version' \
    'python -c "import diff; print(\"asm-differ import ok\")"' \
    'python -c "import m2c; print(\"m2c import ok\")"' \
    'splat --version || true' \
    'spimdisasm --version || true' \
    'm2c --help | sed -n "1,2p"' \
    'echo "== psx/mips tools =="' \
    'mipsel-linux-gnu-as --version | sed -n "1p"' \
    'mipsel-linux-gnu-ld --version | sed -n "1p"' \
    'mipsel-linux-gnu-objdump --version | sed -n "1p"' \
    'printf ".text\nnop\n" | maspsx --passthrough --force-stdin >/tmp/maspsx-test.s' \
    'permuter -h >/tmp/permuter-help.txt' \
    'mkpsxiso --help >/tmp/mkpsxiso-help.txt || true' \
    'dumpsxiso --help >/tmp/dumpsxiso-help.txt || true' \
    'objdiff-cli --version || true' \
    'echo "toolchain verification ok"' \
    > /usr/local/bin/decomp-env-check \
    && chmod +x /usr/local/bin/decomp-env-check

WORKDIR /work
CMD ["/bin/bash"]
