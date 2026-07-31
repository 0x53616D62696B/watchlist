# syntax=docker/dockerfile:1.7
ARG ALPINE_VERSION=3.23.2
FROM alpine:${ALPINE_VERSION} AS toolchain

ARG TARGETARCH
ARG GITVERSION_VERSION=6.5.1

RUN apk add --no-cache \
    bash build-base ca-certificates cmake curl git icu-data-full icu-libs \
    krb5-libs libgcc libssl3 libstdc++ linux-headers mesa-dev ninja pkgconf zlib \
    libx11-dev libxext-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev

RUN curl --fail --location --silent --show-error https://dot.net/v1/dotnet-install.sh --output /tmp/dotnet-install.sh \
    && bash /tmp/dotnet-install.sh --channel 8.0 --install-dir /opt/dotnet \
    && rm /tmp/dotnet-install.sh

ENV DOTNET_ROOT=/opt/dotnet
ENV PATH="/opt/dotnet:/root/.dotnet/tools:${PATH}"

RUN dotnet tool install --global GitVersion.Tool --version ${GITVERSION_VERSION} \
    && dotnet-gitversion /version

WORKDIR /workspace

FROM toolchain AS interactive
CMD ["/bin/bash"]

FROM toolchain AS verification
COPY . /workspace
RUN sed -i 's/\r$//' .docker/verify-linux.sh \
    && chmod +x .docker/verify-linux.sh \
    && .docker/verify-linux.sh
CMD ["ctest", "--test-dir", "build/linux-gcc", "--output-on-failure"]
