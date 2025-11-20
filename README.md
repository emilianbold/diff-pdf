*Note: this repository is provided **as-is** and the code is not being actively
developed. If you wish to improve it, that's greatly appreciated: please make
the changes and submit a pull request, I'll gladly merge it or help you out
with finishing it. However, please do not expect any kind of support, including
implementation of feature requests or fixes. If you're not a developer and/or
willing to get your hands dirty, this tool is probably not for you.*

[![Build](https://github.com/vslavik/diff-pdf/actions/workflows/build.yml/badge.svg)](https://github.com/vslavik/diff-pdf/actions/workflows/build.yml)

## Usage

diff-pdf is a command-line tool for visually comparing two PDFs.

It takes two PDF files as arguments. By default, its only output is its return
code, which is 0 if there are no differences and 1 if the two PDFs differ. If
given the `--output-diff` option, it produces a PDF file with visually
highlighted differences:

```
$ diff-pdf --output-diff=diff.pdf a.pdf b.pdf
```

See the output of `$ diff-pdf --help` for complete list of options.


## Obtaining the binaries

Precompiled Linux binaries are available as part of
[the latest release](https://github.com/vslavik/diff-pdf/releases/latest/)
as a tar.gz archive.

On Fedora and CentOS 8:
```
$ sudo dnf install diff-pdf
```

Precompiled version for openSUSE can be downloaded from the
[openSUSE build service](http://software.opensuse.org).

### Using Docker

If you have Docker installed, you can use diff-pdf without installing any dependencies locally:

```
$ ./diff-pdf.sh --output-diff=diff.pdf a.pdf b.pdf
```

The wrapper script will automatically build the Docker image if needed. Alternatively, you can use Docker directly:

```
$ docker build -t diff-pdf:latest .
$ docker run --rm -v "$(pwd):/pdfs" -w /pdfs diff-pdf:latest --output-diff=diff.pdf a.pdf b.pdf
```

Pre-built Docker images are available from GitHub Container Registry:
```
$ docker pull ghcr.io/emilianbold/diff-pdf:latest
```


## Compiling from sources

The build system uses Automake and is designed for Linux environments.
Compilation is done in the usual way:

```
$ ./bootstrap
$ ./configure
$ make
$ make install
```

(Note that the first step, running the `./bootstrap` script, is only required
when building sources checked from version control system, i.e. when `configure`
and `Makefile.in` files are missing.)

As for dependencies, diff-pdf requires the following libraries:

- Cairo >= 1.4
- Poppler >= 0.10
- GLib >= 2.36

### Ubuntu 24.04 / Debian 12 or newer:

```
$ sudo apt-get install make automake g++
$ sudo apt-get install libpoppler-glib-dev libcairo2-dev pkg-config
```

### CentOS / Fedora:

```
$ sudo yum groupinstall "Development Tools"
$ sudo yum install poppler-glib-devel cairo-devel
```


## Installing

On Linux, the usual `make install` is sufficient.
