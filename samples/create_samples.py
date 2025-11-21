#!/usr/bin/env python3
"""Create sample PDFs for diff-pdf demonstration"""

from reportlab.lib.pagesizes import letter
from reportlab.pdfgen import canvas
from reportlab.lib.units import inch
from reportlab.lib import colors

def create_pdf_v1(filename):
    """Create first version of sample PDF"""
    c = canvas.Canvas(filename, pagesize=letter)
    width, height = letter

    # Title
    c.setFont("Helvetica-Bold", 24)
    c.drawString(1*inch, height - 1*inch, "Sample Document")

    # Subtitle
    c.setFont("Helvetica", 14)
    c.drawString(1*inch, height - 1.5*inch, "Version 1.0 - Initial Release")

    # Some body text
    c.setFont("Helvetica", 12)
    text_lines = [
        "This is a sample PDF document created to demonstrate",
        "the diff-pdf tool. It contains text, shapes, and colors",
        "that will be slightly different in the second version.",
        "",
        "Features:",
        "• Simple text content",
        "• Basic shapes and colors",
        "• Multiple elements for comparison"
    ]

    y = height - 2.5*inch
    for line in text_lines:
        c.drawString(1*inch, y, line)
        y -= 0.25*inch

    # Draw a blue rectangle
    c.setFillColor(colors.blue)
    c.rect(1*inch, 2*inch, 2*inch, 1*inch, fill=1)

    # Draw a red circle
    c.setFillColor(colors.red)
    c.circle(5*inch, 2.5*inch, 0.5*inch, fill=1)

    # Footer
    c.setFont("Helvetica-Oblique", 10)
    c.setFillColor(colors.black)
    c.drawString(1*inch, 0.5*inch, "Created for diff-pdf demonstration")

    c.save()
    print(f"Created {filename}")

def create_pdf_v2(filename):
    """Create second version of sample PDF with differences"""
    c = canvas.Canvas(filename, pagesize=letter)
    width, height = letter

    # Title (same)
    c.setFont("Helvetica-Bold", 24)
    c.drawString(1*inch, height - 1*inch, "Sample Document")

    # Subtitle (CHANGED)
    c.setFont("Helvetica", 14)
    c.drawString(1*inch, height - 1.5*inch, "Version 2.0 - Updated Release")

    # Some body text (CHANGED)
    c.setFont("Helvetica", 12)
    text_lines = [
        "This is a sample PDF document created to demonstrate",
        "the diff-pdf tool. It contains text, shapes, and colors",
        "that have been modified in this second version.",
        "",
        "Features:",
        "• Updated text content",
        "• Modified shapes and colors",
        "• Multiple elements for comparison"
    ]

    y = height - 2.5*inch
    for line in text_lines:
        c.drawString(1*inch, y, line)
        y -= 0.25*inch

    # Draw a green rectangle (CHANGED COLOR)
    c.setFillColor(colors.green)
    c.rect(1*inch, 2*inch, 2*inch, 1*inch, fill=1)

    # Draw a red circle (same position and color)
    c.setFillColor(colors.red)
    c.circle(5*inch, 2.5*inch, 0.5*inch, fill=1)

    # Add a new yellow square (NEW ELEMENT)
    c.setFillColor(colors.yellow)
    c.rect(4*inch, 1.5*inch, 1*inch, 1*inch, fill=1)

    # Footer (same)
    c.setFont("Helvetica-Oblique", 10)
    c.setFillColor(colors.black)
    c.drawString(1*inch, 0.5*inch, "Created for diff-pdf demonstration")

    c.save()
    print(f"Created {filename}")

if __name__ == "__main__":
    create_pdf_v1("sample_v1.pdf")
    create_pdf_v2("sample_v2.pdf")
    print("\nSample PDFs created successfully!")
    print("Differences between v1 and v2:")
    print("  - Subtitle text: 'Version 1.0' → 'Version 2.0'")
    print("  - Body text: 'slightly different' → 'modified'")
    print("  - Body text: 'Basic shapes' → 'Modified shapes'")
    print("  - Rectangle color: blue → green")
    print("  - New yellow square added in v2")
