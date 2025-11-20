# diff-pdf

[![Build](https://github.com/vslavik/diff-pdf/actions/workflows/build.yml/badge.svg)](https://github.com/vslavik/diff-pdf/actions/workflows/build.yml)

A command-line tool for visually comparing two PDF files on Linux.

## Overview

diff-pdf is a simple but powerful tool that compares two PDF files visually. It rasterizes the pages and compares them pixel-by-pixel, making it ideal for catching visual differences that may not be apparent from text-based diff tools.

By default, it returns exit code 0 if the PDFs are identical and 1 if they differ. It can also generate a visual diff PDF showing the differences highlighted.

## Usage

### Basic comparison

```bash
# Compare two PDFs (exit code indicates if they differ)
diff-pdf file1.pdf file2.pdf

# Generate a visual diff PDF
diff-pdf --output-diff=differences.pdf file1.pdf file2.pdf

# Verbose output showing which pages differ
diff-pdf --verbose file1.pdf file2.pdf
```

### Advanced options

```bash
# Skip identical pages in the output
diff-pdf --skip-identical --output-diff=diff.pdf file1.pdf file2.pdf

# Mark differences with visual indicators
diff-pdf --mark-differences --output-diff=diff.pdf file1.pdf file2.pdf

# Grayscale mode (differences shown in color)
diff-pdf --grayscale --output-diff=diff.pdf file1.pdf file2.pdf

# Allow tolerance for minor differences
diff-pdf --channel-tolerance=10 file1.pdf file2.pdf

# Set DPI for rendering (default: 300)
diff-pdf --dpi=150 file1.pdf file2.pdf
```

For complete options, run:
```bash
diff-pdf --help
```

## Installation

### Precompiled binaries

Download precompiled Linux binaries from [the latest release](https://github.com/vslavik/diff-pdf/releases/latest/).

```bash
# Download and extract
wget https://github.com/emilianbold/diff-pdf/releases/latest/download/diff-pdf-linux-x86_64.tar.gz
tar xzf diff-pdf-linux-x86_64.tar.gz
sudo install -m 755 diff-pdf /usr/local/bin/
```

### Distribution packages

**Fedora / CentOS:**
```bash
sudo dnf install diff-pdf
```

**openSUSE:**
Available from the [openSUSE Build Service](http://software.opensuse.org).

### Docker

Use diff-pdf without installing dependencies:

```bash
# Using the wrapper script
./diff-pdf.sh --output-diff=diff.pdf file1.pdf file2.pdf

# Or directly with Docker
docker run --rm -v "$(pwd):/pdfs" -w /pdfs \
  ghcr.io/emilianbold/diff-pdf:latest \
  --output-diff=diff.pdf file1.pdf file2.pdf
```

## Building from source

### Dependencies

diff-pdf requires:
- Cairo >= 1.4
- Poppler >= 0.10
- GLib >= 2.36
- Standard build tools (make, automake, g++)

### Ubuntu / Debian

```bash
# Install dependencies
sudo apt-get install make automake g++ \
  libpoppler-glib-dev libcairo2-dev pkg-config

# Build
./bootstrap
./configure
make
sudo make install
```

### Fedora / CentOS

```bash
# Install dependencies
sudo yum groupinstall "Development Tools"
sudo yum install poppler-glib-devel cairo-devel

# Build
./bootstrap
./configure
make
sudo make install
```

**Note:** The `./bootstrap` step is only needed when building from a git checkout. Release tarballs already include the configure script.

## Technical details

- Rasterizes PDF pages using Poppler at configurable DPI (default 300)
- Compares images pixel-by-pixel with optional tolerance
- Generates visual diffs by combining channels from both PDFs
- Written in C++ using Cairo, Poppler, and GLib
- Command-line only (no GUI)
- Linux-focused but potentially portable to other Unix-like systems

## License

GPLv2 - see COPYING file for details.

## Contributing

Contributions are welcome! Please submit pull requests or open issues on GitHub.
