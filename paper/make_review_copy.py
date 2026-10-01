"""Remove only Inara's large DRAFT watermark from an author-review PDF.

The original journal output must be retained separately. This does not change
submission status, publication metadata, or DOI placeholders. Fail closed when
the template changes instead of deleting arbitrary text or painting over pages.
"""

import argparse
import hashlib
import io
import re
from pathlib import Path

from pypdf import PdfReader, PdfWriter
from pypdf.generic import ArrayObject, ByteStringObject, TextStringObject


def normalized(text):
    return re.sub(r"\s+", "", text)


def preserve_string_bytes(value):
    # Inara uses custom CID fonts. Re-encoding an auto-decoded PDF string can
    # discard two-byte glyph codes, even though its Python text looks valid.
    if isinstance(value, TextStringObject):
        return ByteStringObject(value.original_bytes)
    if isinstance(value, ArrayObject):
        return ArrayObject([preserve_string_bytes(item) for item in value])
    return value


def page_signature(page):
    """Check the non-text content as well as extracted text after rewriting."""
    images = sorted(hashlib.sha256(image.data).hexdigest() for image in page.images)
    links = []
    for ref in page.get("/Annots", []):
        annotation = ref.get_object()
        action = annotation.get("/A", {})
        links.append((str(annotation.get("/Subtype")), str(annotation.get("/Rect")),
                      str(action.get("/URI", ""))))
    return list(page.mediabox), images, links


def make_review_copy(source, destination):
    source, destination = Path(source).resolve(), Path(destination).resolve()
    if source == destination:
        raise ValueError("Keep the original PDF: input and output must differ")
    reader = PdfReader(source)
    if reader.is_encrypted:
        raise ValueError("Encrypted PDF is not supported")
    writer = PdfWriter(clone_from=reader)
    expected_text = []
    signatures = []

    for number, page in enumerate(writer.pages, start=1):
        large_text = []

        def visit(text, cm, tm, font, size):
            if size > 100 and text.strip():
                large_text.append(text.strip())

        before = page.extract_text(visitor_text=visit)
        if large_text != ["DRAFT"]:
            raise ValueError(f"Page {number}: expected exactly one large DRAFT watermark")
        contents = page.get_contents()
        size = 0
        stack = []
        remove = []
        for index, (operands, operator) in enumerate(contents.operations):
            if operator == b"q":
                stack.append(size)
            elif operator == b"Q":
                size = stack.pop()
            elif operator == b"Tf":
                size = float(operands[1])
            elif operator in (b"Tj", b"TJ", b"'", b'"') and size > 100:
                remove.append(index)
        if len(remove) != 1:
            raise ValueError(f"Page {number}: unexpected watermark drawing structure")
        signatures.append(page_signature(page))
        expected_text.append(normalized(before.replace("DRAFT", "", 1)))
        # Remove the text drawing instruction, keeping graphics state and all
        # other drawing operations intact. No page rasterization or redaction.
        del contents.operations[remove[0]]
        contents.operations = [([preserve_string_bytes(item) for item in operands], operator)
                               for operands, operator in contents.operations]
        page.replace_contents(contents)
        if normalized(page.extract_text()) != expected_text[-1]:
            raise ValueError(f"Page {number}: manuscript text changed")

    buffer = io.BytesIO()
    writer.write(buffer)
    buffer.seek(0)
    result = PdfReader(buffer)
    if len(result.pages) != len(reader.pages):
        raise ValueError("Page count changed")
    for index, page in enumerate(result.pages):
        if normalized(page.extract_text()) != expected_text[index]:
            raise ValueError(f"Page {index + 1}: text did not survive serialization")
        if page_signature(page) != signatures[index]:
            raise ValueError(f"Page {index + 1}: images, links or page geometry changed")
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(buffer.getvalue())
    print(f"PASS: removed {len(result.pages)} large watermarks; text, images, links and page geometry preserved")
    print(destination)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("destination", type=Path)
    args = parser.parse_args()
    make_review_copy(args.source, args.destination)
