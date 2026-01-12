# syntax=docker/dockerfile:1
# Build with: docker build -t alpine_synology -f ./.docker/alpine_synology.Dockerfile ./.docker/
# Run and kill after with: docker run --rm alpine_synology
# Push with (need to retag or create with exactly same tag name): 
    # docker push ghcr.io/0x53616d62696b/watchlist/alpine_synology_test:0.0.1-dev1
# Apply PAT to ghcr.io docker registry:
    # echo "{PAT}" | docker login ghcr.io -u 0x53616d62696b --password-stdin
# PAT necesarry scope for github registry: classic PAT with write:packages, read:packages, delete:packages

FROM alpine:3.23.2

ARG TARGETPLATFORM
RUN echo "Building for ${TARGETPLATFORM:-unknown}"

CMD ["sh", "-c", "echo Hello from Synology NAS"]
