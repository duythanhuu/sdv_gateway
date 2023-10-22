# /********************************************************************************
# * Copyright (c) 2022 Contributors to the Eclipse Foundation
# *
# * See the NOTICE file(s) distributed with this work for additional
# * information regarding copyright ownership.
# *
# * This program and the accompanying materials are made available under the
# * terms of the Apache License 2.0 which is available at
# * http://www.apache.org/licenses/LICENSE-2.0
# *
# * SPDX-License-Identifier: Apache-2.0
# ********************************************************************************/

FROM --platform=$BUILDPLATFORM ubuntu:20.04 as builder

ENV DEBIAN_FRONTEND="noninteractive"
RUN DEBIAN_FRONTEND=noninteractive apt-get update -y && \
    apt-get install -y git \
    cmake g++ build-essential g++-aarch64-linux-gnu \
    binutils-aarch64-linux-gnu jq python3 python3-pip

RUN pip3 install conan==1.55.0

COPY . /src
WORKDIR /src

ARG TARGETPLATFORM
RUN echo "Building for ${TARGETPLATFORM}"

RUN if [ "$TARGETPLATFORM" = "linux/amd64" ]; then \
        ./build-release.sh amd64; \
    elif [ "$TARGETPLATFORM" = "linux/arm64" ]; then \
        ./build-release.sh aarch64; \
    else \
        echo "Unsupported platform: $TARGETPLATFORM"; exit 1; \
    fi
# RUN ./build-release.sh $TARGETPLATFORM

FROM --platform=$TARGETPLATFORM ubuntu:20.04 as final
ARG TARGETARCH

RUN DEBIAN_FRONTEND=noninteractive apt-get update -y && \
    apt-get install -y jq && \
    rm -rf /var/lib/apt/lists/*

WORKDIR "/app/lib"
COPY --from=builder "/src/target/*/release/install/lib/*" "/app/lib"

WORKDIR "/app/bin"
COPY --from=builder "/src/target/*/release/install/bin" "/app/bin"

RUN find /app 1>&2

CMD [ "/app/bin/someip2val-docker.sh" ]
