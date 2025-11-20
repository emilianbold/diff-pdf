/*
 * This file is part of diff-pdf.
 *
 * Copyright (C) 2009 TT-Solutions.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <algorithm>
#include <string>
#include <vector>

#include <glib.h>
#include <poppler.h>
#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>

// ------------------------------------------------------------------------
// Helper structures (replacing wx types)
// ------------------------------------------------------------------------

struct Rect
{
    int x, y, width, height;

    Rect() : x(0), y(0), width(0), height(0) {}
    Rect(int x_, int y_, int w_, int h_) : x(x_), y(y_), width(w_), height(h_) {}

    void Offset(int dx, int dy) { x += dx; y += dy; }

    void Union(const Rect& r)
    {
        if (r.width == 0 || r.height == 0)
            return;
        if (width == 0 || height == 0)
        {
            *this = r;
            return;
        }

        int right = std::max(x + width, r.x + r.width);
        int bottom = std::max(y + height, r.y + r.height);
        x = std::min(x, r.x);
        y = std::min(y, r.y);
        width = right - x;
        height = bottom - y;
    }

    bool operator!=(const Rect& r) const
    {
        return x != r.x || y != r.y || width != r.width || height != r.height;
    }
};

// ------------------------------------------------------------------------
// PDF rendering functions
// ------------------------------------------------------------------------

bool g_verbose = false;
bool g_skip_identical = false;
bool g_mark_differences = false;
long g_channel_tolerance = 0;
long g_per_page_pixel_tolerance = 0;
bool g_grayscale = false;
// Resolution to use for rasterization, in DPI
#define DEFAULT_RESOLUTION 300
long g_resolution = DEFAULT_RESOLUTION;

inline unsigned char to_grayscale(unsigned char r, unsigned char g, unsigned char b)
{
    return (unsigned char)(0.2126 * r + 0.7152 * g + 0.0722 * b);
}

cairo_surface_t *render_page(PopplerPage *page)
{
    double w, h;
    poppler_page_get_size(page, &w, &h);

    const int w_px = int((int)g_resolution * w / 72.0);
    const int h_px = int((int)g_resolution * h / 72.0);

    cairo_surface_t *surface =
        cairo_image_surface_create(CAIRO_FORMAT_RGB24, w_px, h_px);

    cairo_t *cr = cairo_create(surface);

    // clear the surface to white background:
    cairo_save(cr);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_rectangle(cr, 0, 0, w_px, h_px);
    cairo_fill(cr);
    cairo_restore(cr);

    // Scale so that PDF output covers the whole surface. Image surface is
    // created with transformation set up so that 1 coordinate unit is 1 pixel;
    // Poppler assumes 1 unit = 1 point.
    cairo_scale(cr, (int)g_resolution / 72.0, (int)g_resolution / 72.0);

    poppler_page_render(page, cr);

    cairo_show_page(cr);

    cairo_destroy(cr);

    return surface;
}


// Creates image of differences between s1 and s2. If the offset is specified,
// then s2 is displaced by it.
cairo_surface_t *diff_images(int page, cairo_surface_t *s1, cairo_surface_t *s2,
                             int offset_x = 0, int offset_y = 0)
{
    assert( s1 || s2 );

    long pixel_diff_count = 0;
    Rect r1, r2;

    if ( s1 )
    {
        r1 = Rect(0, 0,
                  cairo_image_surface_get_width(s1),
                  cairo_image_surface_get_height(s1));
    }
    if ( s2 )
    {
        r2 = Rect(offset_x, offset_y,
                  cairo_image_surface_get_width(s2),
                  cairo_image_surface_get_height(s2));
    }

    // compute union rectangle starting at [0,0] position
    Rect rdiff(r1);
    rdiff.Union(r2);
    r1.Offset(-rdiff.x, -rdiff.y);
    r2.Offset(-rdiff.x, -rdiff.y);
    rdiff.Offset(-rdiff.x, -rdiff.y);

    bool changes = false;

    cairo_surface_t *diff =
        cairo_image_surface_create(CAIRO_FORMAT_RGB24, rdiff.width, rdiff.height);

    // clear the surface to white background if the merged images don't fully
    // overlap:
    if ( r1 != r2 )
    {
        changes = true;

        cairo_t *cr = cairo_create(diff);
        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_rectangle(cr, 0, 0, rdiff.width, rdiff.height);
        cairo_fill(cr);
        cairo_destroy(cr);
    }

    const int stride1 = s1 ? cairo_image_surface_get_stride(s1) : 0;
    const int stride2 = s2 ? cairo_image_surface_get_stride(s2) : 0;
    const int stridediff = cairo_image_surface_get_stride(diff);

    const unsigned char *data1 = s1 ? cairo_image_surface_get_data(s1) : NULL;
    const unsigned char *data2 = s2 ? cairo_image_surface_get_data(s2) : NULL;
    unsigned char *datadiff = cairo_image_surface_get_data(diff);

    // we visualize the differences by taking one channel from s1
    // and the other two channels from s2:

    // first, copy s1 over:
    if ( s1 )
    {
        unsigned char *out = datadiff + r1.y * stridediff + r1.x * 4;
        for ( int y = 0;
              y < r1.height;
              y++, data1 += stride1, out += stridediff )
        {
            memcpy(out, data1, r1.width * 4);
        }
    }

    // then, copy B channel from s2 over it; also compare the two versions
    // to see if there are any differences:
    if ( s2 )
    {
        unsigned char *out = datadiff + r2.y * stridediff + r2.x * 4;
        for ( int y = 0;
              y < r2.height;
              y++, data2 += stride2, out += stridediff )
        {
            bool linediff = false;

            for ( int x = 0; x < r2.width * 4; x += 4 )
            {
                unsigned char cr1 = *(out + x + 0);
                unsigned char cg1 = *(out + x + 1);
                unsigned char cb1 = *(out + x + 2);

                unsigned char cr2 = *(data2 + x + 0);
                unsigned char cg2 = *(data2 + x + 1);
                unsigned char cb2 = *(data2 + x + 2);

                if ( cr1 > (cr2+g_channel_tolerance) || cr1 < (cr2-g_channel_tolerance)
                  || cg1 > (cg2+g_channel_tolerance) || cg1 < (cg2-g_channel_tolerance)
                  || cb1 > (cb2+g_channel_tolerance) || cb1 < (cb2-g_channel_tolerance)
                   )
                {
                    pixel_diff_count++;
                    changes = true;
                    linediff = true;
                }

                if (g_grayscale)
                {
                    // convert both images to grayscale, use blue for s1, red for s2
                    unsigned char gray1 = to_grayscale(cr1, cg1, cb1);
                    unsigned char gray2 = to_grayscale(cr2, cg2, cb2);
                    *(out + x + 0) = gray2;
                    *(out + x + 1) = (gray1 + gray2) / 2;
                    *(out + x + 2) = gray1;
                }
                else
                {
                    // change the B channel to be from s2; RG will be s1
                    *(out + x + 2) = cb2;
                }
            }

            if (g_mark_differences && linediff)
            {
                for (int x = 0; x < (10 < r2.width ? 10 : r2.width) * 4; x+=4)
                {
                   *(out + x + 0) = 0;
                   *(out + x + 1) = 0;
                   *(out + x + 2) = 255;
                }
            }
        }
    }

    if ( g_verbose )
        printf("page %d has %ld pixels that differ\n", page, pixel_diff_count);

    // If we specified a tolerance, then return if we have exceeded that for this page
    if ( g_per_page_pixel_tolerance == 0 ? changes : pixel_diff_count > g_per_page_pixel_tolerance)
    {
        return diff;
    }
    else
    {
        cairo_surface_destroy(diff);
        return NULL;
    }
}


// Compares given two pages. If cr_out is not NULL, then the diff image (either
// differences or unmodified page, if there are no diffs) is drawn to it.
bool page_compare(int page, cairo_t *cr_out,
                  PopplerPage *page1, PopplerPage *page2)
{
    cairo_surface_t *img1 = page1 ? render_page(page1) : NULL;
    cairo_surface_t *img2 = page2 ? render_page(page2) : NULL;

    cairo_surface_t *diff = diff_images(page, img1, img2, 0, 0);
    const bool has_diff = (diff != NULL);

    if ( cr_out )
    {
        if ( diff )
        {
            // render the difference as high-resolution bitmap

            cairo_save(cr_out);
            cairo_scale(cr_out, 72.0 / g_resolution, 72.0 / g_resolution);

            cairo_set_source_surface(cr_out, diff ? diff : img1, 0, 0);
            cairo_paint(cr_out);

            cairo_restore(cr_out);
        }
        else
        {
            // save space (as well as improve rendering quality) in diff pdf
            // by writing unchanged pages in their original form rather than
            // a rasterized one

            if (!g_skip_identical)
               poppler_page_render(page1, cr_out);
        }

        if (diff || !g_skip_identical)
            cairo_show_page(cr_out);
    }

    if ( diff )
        cairo_surface_destroy(diff);

    if ( img1 )
        cairo_surface_destroy(img1);
    if ( img2 )
        cairo_surface_destroy(img2);

    return !has_diff;
}


// Compares two documents, writing diff PDF into file named 'pdf_output' if
// not NULL. if 'differences' is not NULL, puts a map of which pages differ
// into it.
bool doc_compare(PopplerDocument *doc1, PopplerDocument *doc2,
                 const char *pdf_output,
                 std::vector<bool> *differences)
{
    int pages_differ = 0;

    cairo_surface_t *surface_out = NULL;
    cairo_t *cr_out = NULL;

    if ( pdf_output )
    {
        double w, h;
        poppler_page_get_size(poppler_document_get_page(doc1, 0), &w, &h);
        surface_out = cairo_pdf_surface_create(pdf_output, w, h);
        cr_out = cairo_create(surface_out);
    }

    int pages1 = poppler_document_get_n_pages(doc1);
    int pages2 = poppler_document_get_n_pages(doc2);
    int pages_total = pages1 > pages2 ? pages1 : pages2;

    if ( pages1 != pages2 )
    {
        if ( g_verbose )
            printf("pages count differs: %d vs %d\n", pages1, pages2);
    }

    for ( int page = 0; page < pages_total; page++ )
    {
        if ( pdf_output && page != 0 )
        {
            double w, h;
            poppler_page_get_size(poppler_document_get_page(doc1, page), &w, &h);
            cairo_pdf_surface_set_size(surface_out, w, h);
        }

        PopplerPage *page1 = page < pages1
                             ? poppler_document_get_page(doc1, page)
                             : NULL;
        PopplerPage *page2 = page < pages2
                             ? poppler_document_get_page(doc2, page)
                             : NULL;

        bool page_same = page_compare(page, cr_out, page1, page2);

        if ( differences )
            differences->push_back(!page_same);

        if ( !page_same )
        {
	    pages_differ ++;

            if ( g_verbose )
                printf("page %d differs\n", page);

            // If we don't need to output all different pages in any
            // form (including verbose report of differing pages!), then
            // we can stop comparing the PDFs as soon as we find the first
            // difference.
            if ( !g_verbose && !pdf_output && !differences )
                break;
        }
    }

    if ( pdf_output )
    {
        cairo_destroy(cr_out);
        cairo_surface_destroy(surface_out);
    }

    if (g_verbose)
        printf("%d of %d pages differ.\n", pages_differ, pages_total);

    // are doc1 and doc1 the same?
    return (pages_differ == 0) && (pages1 == pages2);
}


// ------------------------------------------------------------------------
// main()
// ------------------------------------------------------------------------

static void print_usage()
{
    printf("Usage: diff-pdf [options] file1.pdf file2.pdf\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help                       show this help message\n");
    printf("  -v, --verbose                    be verbose\n");
    printf("  -s, --skip-identical             only output pages with differences\n");
    printf("  -m, --mark-differences           additionally mark differences on left side\n");
    printf("  -g, --grayscale                  only differences will be in color\n");
    printf("  --output-diff=FILE               output differences to given PDF file\n");
    printf("  --channel-tolerance=N            channel tolerance (0-255, default: 0)\n");
    printf("  --per-page-pixel-tolerance=N     per-page pixel tolerance (default: 0)\n");
    printf("  --dpi=N                          rasterization resolution (default: %d)\n", DEFAULT_RESOLUTION);
    printf("\n");
}

// Convert file path to URI format expected by Poppler
static std::string file_to_uri(const char *filename)
{
    char *absolute_path = realpath(filename, NULL);
    if (!absolute_path)
    {
        // If realpath fails, use the filename as-is and hope for the best
        absolute_path = strdup(filename);
    }

    std::string uri = "file://";
    uri += absolute_path;
    free(absolute_path);
    return uri;
}

int main(int argc, char *argv[])
{
    const char *output_diff = NULL;

    static struct option long_options[] = {
        {"help",                       no_argument,       0, 'h'},
        {"verbose",                    no_argument,       0, 'v'},
        {"skip-identical",             no_argument,       0, 's'},
        {"mark-differences",           no_argument,       0, 'm'},
        {"grayscale",                  no_argument,       0, 'g'},
        {"output-diff",                required_argument, 0, 'o'},
        {"channel-tolerance",          required_argument, 0, 'c'},
        {"per-page-pixel-tolerance",   required_argument, 0, 'p'},
        {"dpi",                        required_argument, 0, 'd'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "hvsmg", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
            case 'h':
                print_usage();
                return 0;

            case 'v':
                g_verbose = true;
                break;

            case 's':
                g_skip_identical = true;
                break;

            case 'm':
                g_mark_differences = true;
                break;

            case 'g':
                g_grayscale = true;
                break;

            case 'o':
                output_diff = optarg;
                break;

            case 'c':
                g_channel_tolerance = atol(optarg);
                if (g_channel_tolerance < 0 || g_channel_tolerance > 255)
                {
                    fprintf(stderr, "Invalid channel-tolerance: %ld. Valid range is 0(default, exact matching)-255\n", g_channel_tolerance);
                    return 2;
                }
                break;

            case 'p':
                g_per_page_pixel_tolerance = atol(optarg);
                if (g_per_page_pixel_tolerance < 0)
                {
                    fprintf(stderr, "Invalid per-page-pixel-tolerance: %ld. Must be 0 or more\n", g_per_page_pixel_tolerance);
                    return 2;
                }
                break;

            case 'd':
                g_resolution = atol(optarg);
                if (g_resolution < 1 || g_resolution > 2400)
                {
                    fprintf(stderr, "Invalid dpi: %ld. Valid range is 1-2400 (default: %d)\n", g_resolution, DEFAULT_RESOLUTION);
                    return 2;
                }
                break;

            default:
                print_usage();
                return 2;
        }
    }

    // Check for required arguments
    if (optind + 2 != argc)
    {
        fprintf(stderr, "Error: Two PDF files required\n\n");
        print_usage();
        return 2;
    }

    const char *file1 = argv[optind];
    const char *file2 = argv[optind + 1];

    std::string url1 = file_to_uri(file1);
    std::string url2 = file_to_uri(file2);

    GError *err = NULL;

    PopplerDocument *doc1 = poppler_document_new_from_file(url1.c_str(), NULL, &err);
    if ( !doc1 )
    {
        fprintf(stderr, "Error opening %s: %s\n", file1, err->message);
        g_error_free(err);
        return 3;
    }

    PopplerDocument *doc2 = poppler_document_new_from_file(url2.c_str(), NULL, &err);
    if ( !doc2 )
    {
        fprintf(stderr, "Error opening %s: %s\n", file2, err->message);
        g_error_free(err);
        return 3;
    }

    int retval = doc_compare(doc1, doc2, output_diff, NULL) ? 0 : 1;

    g_object_unref(doc1);
    g_object_unref(doc2);

    // MinGW doesn't reliably flush streams on exit, so flush them explicitly:
    fflush(stdout);
    fflush(stderr);

    return retval;
}
