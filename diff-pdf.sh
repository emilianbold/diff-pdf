#!/bin/bash
# Wrapper script for running diff-pdf in Docker
# Usage: ./diff-pdf.sh [diff-pdf options] file1.pdf file2.pdf

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo -e "${RED}Error: Docker is not installed or not in PATH${NC}"
    exit 1
fi

# Check if image exists, if not build it
if ! docker image inspect diff-pdf:latest &> /dev/null; then
    echo -e "${YELLOW}diff-pdf Docker image not found. Building it now...${NC}"
    docker build -t diff-pdf:latest "$(dirname "$0")"
fi

# Get the absolute path of the current directory
WORKDIR="$(pwd)"

# Run diff-pdf in Docker with mounted volume
# Pass all arguments directly to diff-pdf
docker run --rm \
    -v "$WORKDIR:/pdfs" \
    -w /pdfs \
    diff-pdf:latest \
    "$@"

# Capture and return the exit code
exit $?
