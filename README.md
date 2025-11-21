# diff-pdf

[![Build](https://github.com/emilianbold/diff-pdf/actions/workflows/build.yml/badge.svg)](https://github.com/emilianbold/diff-pdf/actions/workflows/build.yml)

A command-line tool for visually comparing two PDF files on Linux.

## Example

<img src="samples/diff-pdf-demo.png" alt="diff-pdf output showing visual differences between two PDFs" width="600">

The tool highlights differences between PDFs, making it easy to spot changes in text, colors, and layout.

## Usage

```bash
# Compare two PDFs (exit code 0 if identical, 1 if different)
diff-pdf file1.pdf file2.pdf

# Generate a visual diff PDF
diff-pdf --output-diff=diff.pdf file1.pdf file2.pdf

# Skip identical pages in output
diff-pdf --skip-identical --output-diff=diff.pdf file1.pdf file2.pdf

# Verbose mode
diff-pdf --verbose file1.pdf file2.pdf
```

See `diff-pdf --help` for all options.

## Installation

### Binary releases

Precompiled Linux binaries are available from [GitHub releases](https://github.com/emilianbold/diff-pdf/releases).

**Runtime dependencies:** The binary requires these libraries to be installed:
```bash
# Ubuntu/Debian
sudo apt-get install libpoppler-glib8 libcairo2

# Fedora/CentOS
sudo dnf install poppler-glib cairo
```

### Build from source

```bash
# Ubuntu/Debian dependencies
sudo apt-get install make automake g++ libpoppler-glib-dev libcairo2-dev pkg-config

# Build and install
./bootstrap
./configure
make
sudo make install
```

Dependencies: Cairo >= 1.4, Poppler >= 0.10, GLib >= 2.36
