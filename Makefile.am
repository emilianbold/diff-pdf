
bin_PROGRAMS = diff-pdf

diff_pdf_SOURCES = diff-pdf.cpp

diff_pdf_CXXFLAGS = $(POPPLER_CFLAGS)
diff_pdf_LDADD = $(POPPLER_LIBS)

EXTRA_DIST = bootstrap README.md win32/fonts.conf win32/collect-dlls.sh

windows-dist: diff-pdf-win-$(VERSION).zip

diff-pdf-win-$(VERSION).zip: all
	rm -rf $@ windist
	$(srcdir)/win32/collect-dlls.sh windist diff-pdf.exe
	mkdir -p windist/fonts && cp -a $(srcdir)/win32/fonts.conf windist/fonts/
	(cd windist && zip -9r ../$@ .)
	rm -rf windist

.PHONY: windows-dist
