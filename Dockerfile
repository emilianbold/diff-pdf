# Multi-stage Dockerfile for diff-pdf
# Stage 1: Build diff-pdf from source
FROM ubuntu:24.04 AS builder

# Avoid interactive prompts during build
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    make \
    automake \
    g++ \
    libpoppler-glib-dev \
    poppler-utils \
    libwxgtk3.2-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Copy source code
WORKDIR /build
COPY . .

# Build diff-pdf
RUN ./bootstrap && \
    ./configure && \
    make && \
    make install DESTDIR=/install

# Stage 2: Create minimal runtime image
FROM ubuntu:24.04

# Avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install only runtime dependencies
RUN apt-get update && apt-get install -y \
    libpoppler-glib8 \
    libwxgtk3.2-1 \
    libcairo2 \
    poppler-utils \
    && rm -rf /var/lib/apt/lists/*

# Copy the built binary from builder stage
COPY --from=builder /install/usr/local/bin/diff-pdf /usr/local/bin/diff-pdf

# Set working directory for PDF operations
WORKDIR /pdfs

# Default entrypoint
ENTRYPOINT ["/usr/local/bin/diff-pdf"]

# Show help by default if no arguments provided
CMD ["--help"]
