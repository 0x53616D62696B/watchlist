# syntax=docker/dockerfile:1

FROM alpine:3.23.2

ARG TARGETPLATFORM
RUN echo "Building for ${TARGETPLATFORM:-unknown}"

CMD ["sh", "-c", "echo Hello from Synology NAS"]
