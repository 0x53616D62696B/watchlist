# syntax=docker/dockerfile:1
# Build with: docker build -t alpine_synology -f ./.docker/alpine_synology.Dockerfile ./.docker/
# Run and kill after with: docker run --rm alpine_synology

FROM alpine:3.23.2

ARG TARGETPLATFORM
RUN echo "Building for ${TARGETPLATFORM:-unknown}"

CMD ["sh", "-c", "echo Hello from Synology NAS"]
