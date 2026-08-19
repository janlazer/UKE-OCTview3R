"""Render a readable review PDF without the official JOSS/Inara toolchain.

The generated PDF is a local review artifact. JOSS should render the final
publication proof from paper.md with Inara.
"""

from __future__ import annotations

import html
import re
from pathlib import Path

from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER, TA_LEFT
from reportlab.lib.pagesizes import letter
from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
from reportlab.lib.units import inch
from reportlab.platypus import (
    Image,
    KeepTogether,
    PageBreak,
    Paragraph,
    SimpleDocTemplate,
    Spacer,
)


ROOT = Path(__file__).resolve().parent
PAPER_PATH = ROOT / "paper.md"
BIB_PATH = ROOT / "paper.bib"
OUTPUT_PATH = ROOT.parent / "output" / "pdf" / "octview3r-joss-draft.pdf"


def parse_bibliography(text: str) -> dict[str, dict[str, str]]:
    entries: dict[str, dict[str, str]] = {}
    current: dict[str, str] | None = None
    current_key = ""

    for raw_line in text.splitlines():
        line = raw_line.strip()
        start = re.match(r"@(\w+)\{([^,]+),", line)
        if start:
            current_key = start.group(2)
            current = {"_entry_type": start.group(1).lower()}
            entries[current_key] = current
            continue
        if current is None or line == "}":
            if line == "}":
                current = None
            continue
        field = re.match(r"(\w+)\s*=\s*\{(.*)\},?$", line)
        if field:
            current[field.group(1).lower()] = field.group(2).rstrip(",")

    return entries


def author_year(entry: dict[str, str]) -> str:
    authors = entry.get("author", "Unknown").split(" and ")
    first = authors[0].split(",", 1)[0]
    author_text = first if len(authors) == 1 else f"{first} et al."
    year = entry.get("year", entry.get("date", "n.d.")[:4])
    return f"{author_text}, {year}"


def replace_citations(text: str, entries: dict[str, dict[str, str]]) -> str:
    def replacement(match: re.Match[str]) -> str:
        parts = []
        for item in match.group(1).split(";"):
            item = item.strip().lstrip("@")
            key, separator, locator = item.partition(",")
            citation = author_year(entries.get(key.strip(), {}))
            if separator:
                citation += f",{locator}"
            parts.append(citation)
        return f"({'; '.join(parts)})"

    return re.sub(r"\[@([^\]]+)\]", replacement, text)


def inline_markup(text: str, entries: dict[str, dict[str, str]]) -> str:
    text = replace_citations(text, entries)
    text = html.escape(text, quote=False)
    text = re.sub(r"`([^`]+)`", r'<font name="Courier">\1</font>', text)
    return text


def format_authors(value: str) -> str:
    names = []
    for author in value.split(" and "):
        if "," in author:
            surname, given = (part.strip() for part in author.split(",", 1))
            names.append(f"{given} {surname}")
        else:
            names.append(author)
    return "; ".join(names)


def format_reference(entry: dict[str, str]) -> str:
    authors = format_authors(entry.get("author", "Unknown author"))
    year = entry.get("year", entry.get("date", "n.d.")[:4])
    title = entry.get("title", "Untitled")
    entry_type = entry.get("_entry_type", "")
    pieces = [f"{authors} ({year}). {title}."]

    if entry_type == "article":
        journal = entry.get("journal", "")
        volume = entry.get("volume", "")
        number = entry.get("number", "")
        pages = entry.get("pages", "")
        journal_part = journal
        if volume:
            journal_part += f", {volume}"
        if number:
            journal_part += f"({number})"
        if pages:
            journal_part += f", {pages}"
        pieces.append(journal_part + ".")
    elif entry_type == "inproceedings":
        pieces.append(
            f"In {entry.get('booktitle', '')}, {entry.get('pages', '')}. "
            f"{entry.get('publisher', '')}."
        )
    elif entry_type == "incollection":
        pieces.append(
            f"In {entry.get('booktitle', '')}, {entry.get('pages', '')}. "
            f"{entry.get('publisher', '')}."
        )
    elif entry_type == "book":
        pieces.append(f"{entry.get('edition', '')}th edition. {entry.get('publisher', '')}.")
    elif entry_type in {"thesis", "phdthesis"}:
        kind = entry.get(
            "type",
            "Doctoral dissertation" if entry_type == "phdthesis" else "Thesis",
        )
        institution = entry.get("institution", entry.get("school", ""))
        pieces.append(f"{kind}, {institution}.")

    doi = entry.get("doi")
    if doi:
        pieces.append(f'<link href="https://doi.org/{doi}">https://doi.org/{doi}</link>.')
    elif entry.get("isbn"):
        pieces.append(f"ISBN {entry['isbn']}.")
    return " ".join(part for part in pieces if part and part != ".")


def footer(canvas, document) -> None:
    canvas.saveState()
    canvas.setStrokeColor(colors.HexColor("#d1d5db"))
    canvas.line(0.72 * inch, 0.52 * inch, 7.78 * inch, 0.52 * inch)
    canvas.setFont("Helvetica", 7.5)
    canvas.setFillColor(colors.HexColor("#6b7280"))
    canvas.drawString(0.72 * inch, 0.34 * inch, "OCTview3R JOSS manuscript review draft - not an official JOSS proof")
    canvas.drawRightString(7.78 * inch, 0.34 * inch, f"Page {document.page}")
    canvas.restoreState()


def build_pdf() -> None:
    source = PAPER_PATH.read_text(encoding="utf-8")
    bibliography = parse_bibliography(BIB_PATH.read_text(encoding="utf-8"))
    _, _, body = source.partition("---\n")
    _, _, body = body.partition("---\n")
    body = body.replace("# References\n", "")

    image_pattern = re.compile(r"!\[(.*?)\]\((.*?)\)\{[^}]*\}", re.DOTALL)
    figures: list[tuple[str, Path]] = []

    def replace_figure(match: re.Match[str]) -> str:
        index = len(figures)
        caption = " ".join(match.group(1).split())
        figures.append((caption, ROOT / match.group(2)))
        return f"\n\n[[FIGURE:{index}]]\n\n"

    body = image_pattern.sub(replace_figure, body)

    styles = getSampleStyleSheet()
    styles.add(
        ParagraphStyle(
            name="PaperTitle",
            parent=styles["Title"],
            fontName="Helvetica-Bold",
            fontSize=19,
            leading=22,
            textColor=colors.HexColor("#111827"),
            alignment=TA_CENTER,
            spaceAfter=10,
        )
    )
    styles.add(
        ParagraphStyle(
            name="Authors",
            parent=styles["Normal"],
            fontName="Helvetica-Bold",
            fontSize=10.5,
            leading=14,
            alignment=TA_CENTER,
            textColor=colors.HexColor("#1f2937"),
        )
    )
    styles.add(
        ParagraphStyle(
            name="Affiliations",
            parent=styles["Normal"],
            fontSize=8.5,
            leading=11,
            alignment=TA_CENTER,
            textColor=colors.HexColor("#4b5563"),
            spaceAfter=14,
        )
    )
    styles.add(
        ParagraphStyle(
            name="SectionHeading",
            parent=styles["Heading1"],
            fontName="Helvetica-Bold",
            fontSize=12.5,
            leading=15,
            textColor=colors.HexColor("#17365d"),
            spaceBefore=10,
            spaceAfter=5,
        )
    )
    styles.add(
        ParagraphStyle(
            name="BodyTextReview",
            parent=styles["BodyText"],
            fontName="Helvetica",
            fontSize=9.25,
            leading=12.2,
            alignment=TA_LEFT,
            textColor=colors.HexColor("#202124"),
            spaceAfter=6,
        )
    )
    styles.add(
        ParagraphStyle(
            name="CaptionReview",
            parent=styles["BodyText"],
            fontName="Helvetica-Oblique",
            fontSize=8,
            leading=10,
            textColor=colors.HexColor("#4b5563"),
            spaceBefore=4,
            spaceAfter=8,
        )
    )
    styles.add(
        ParagraphStyle(
            name="ReferenceReview",
            parent=styles["BodyText"],
            fontSize=8,
            leading=10.2,
            leftIndent=12,
            firstLineIndent=-12,
            spaceAfter=5,
        )
    )
    styles.add(
        ParagraphStyle(
            name="DraftNotice",
            parent=styles["BodyText"],
            fontSize=8.5,
            leading=11,
            borderColor=colors.HexColor("#93c5fd"),
            borderWidth=0.7,
            borderPadding=7,
            backColor=colors.HexColor("#eff6ff"),
            textColor=colors.HexColor("#1e3a8a"),
            spaceAfter=12,
        )
    )

    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    document = SimpleDocTemplate(
        str(OUTPUT_PATH),
        pagesize=letter,
        leftMargin=0.72 * inch,
        rightMargin=0.72 * inch,
        topMargin=0.62 * inch,
        bottomMargin=0.68 * inch,
        title="OCTview3R JOSS manuscript draft",
        author="Jan Hahn, Giovanno Möbes, Tammo Ripken",
        subject="Review rendering of the OCTview3R JOSS manuscript draft",
    )

    story = [
        Paragraph(
            "OCTview3R: Interactive visualization of volumetric OCT and segmented three-dimensional data",
            styles["PaperTitle"],
        ),
        Paragraph("Jan Hahn<super>1,2</super>, Giovanno Möbes<super>2</super>, and Tammo Ripken<super>2</super>", styles["Authors"]),
        Paragraph(
            "<super>1</super> University Medical Center Hamburg-Eppendorf (UKE), Hamburg, Germany<br/>"
            "<super>2</super> Laser Zentrum Hannover e.V. (LZH), Hannover, Germany<br/>"
            "Draft dated 19 August 2026",
            styles["Affiliations"],
        ),
        Paragraph(
            "Review rendering generated without the official JOSS/Inara toolchain. "
            "The authoritative manuscript source is <font name='Courier'>paper.md</font>.",
            styles["DraftNotice"],
        ),
    ]

    for block in re.split(r"\n\s*\n", body.strip()):
        block = block.strip()
        if not block or block.startswith("<!--"):
            continue
        figure_marker = re.fullmatch(r"\[\[FIGURE:(\d+)\]\]", block)
        if figure_marker:
            figure_index = int(figure_marker.group(1))
            caption, image_path = figures[figure_index]
            figure = Image(str(image_path))
            max_width = 7.0 * inch
            aspect_ratio = figure.imageWidth / figure.imageHeight
            max_height = (6.4 if aspect_ratio < 1.0 else 4.15) * inch
            scale = min(max_width / figure.imageWidth, max_height / figure.imageHeight)
            figure.drawWidth = figure.imageWidth * scale
            figure.drawHeight = figure.imageHeight * scale
            story.append(
                KeepTogether(
                    [
                        figure,
                        Paragraph(
                            f"Figure {figure_index + 1}: {inline_markup(caption, bibliography)}",
                            styles["CaptionReview"],
                        ),
                    ]
                )
            )
            continue
        if block.startswith("# "):
            story.append(Paragraph(html.escape(block[2:].strip()), styles["SectionHeading"]))
            continue
        paragraph = " ".join(line.strip() for line in block.splitlines())
        story.append(Paragraph(inline_markup(paragraph, bibliography), styles["BodyTextReview"]))

    story.extend([PageBreak(), Paragraph("References", styles["SectionHeading"])])
    for entry in bibliography.values():
        story.append(
            Paragraph(
                format_reference(entry),
                styles["ReferenceReview"],
            )
        )

    document.build(story, onFirstPage=footer, onLaterPages=footer)
    print(OUTPUT_PATH)


if __name__ == "__main__":
    build_pdf()
