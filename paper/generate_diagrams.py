"""Generate publication-sized block diagrams used by the JOSS draft."""

from __future__ import annotations

from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parent
FIGURES = ROOT / "figures"

BACKGROUND = "#FAFAF9"
INK = "#171A1D"
CHARCOAL = "#343A40"
MID_GRAY = "#6B737B"
BORDER = "#59616A"
PANEL = "#F1F3F4"
PANEL_ALT = "#E7EAEC"
GRID = "#D4D8DC"
STEEL_BLUE = "#4E7080"
COPPER = "#8A604A"
WHITE = "#FFFFFF"


def font(size: int, bold: bool = False) -> ImageFont.FreeTypeFont:
    names = (
        ["C:/Windows/Fonts/arialbd.ttf", "C:/Windows/Fonts/segoeuib.ttf"]
        if bold
        else ["C:/Windows/Fonts/arial.ttf", "C:/Windows/Fonts/segoeui.ttf"]
    )
    for name in names:
        path = Path(name)
        if path.exists():
            return ImageFont.truetype(str(path), size=size)
    return ImageFont.load_default()


FONT_NODE = font(44, bold=True)
FONT_NODE_SMALL = font(39, bold=True)
FONT_BODY = font(35)
FONT_SMALL = font(31)
FONT_LANE = font(36, bold=True)


def technical_box(
    draw: ImageDraw.ImageDraw,
    xy: tuple[int, int, int, int],
    header_fill: str,
    title: str,
    lines: list[str],
    title_font: ImageFont.FreeTypeFont = FONT_NODE,
    body_fill: str = PANEL,
    body_font: ImageFont.FreeTypeFont = FONT_BODY,
) -> None:
    """Draw a restrained engineering-style module with a distinct header."""
    x0, y0, x1, y1 = xy
    header_height = 88
    draw.rectangle(xy, fill=body_fill, outline=BORDER, width=5)
    draw.rectangle(
        (x0 + 5, y0 + 5, x1 - 5, y0 + header_height),
        fill=header_fill,
    )
    draw.line(
        (x0 + 5, y0 + header_height, x1 - 5, y0 + header_height),
        fill=BORDER,
        width=4,
    )
    draw.text(
        ((x0 + x1) / 2, y0 + header_height / 2 + 1),
        title,
        font=title_font,
        fill=WHITE,
        anchor="mm",
    )

    line_spacing = 54
    content_center = y0 + header_height + (y1 - y0 - header_height) / 2
    first_y = content_center - (len(lines) - 1) * line_spacing / 2
    for index, line in enumerate(lines):
        draw.text(
            ((x0 + x1) / 2, first_y + index * line_spacing),
            line,
            font=body_font,
            fill=INK,
            anchor="mm",
        )


def arrow(
    draw: ImageDraw.ImageDraw,
    start: tuple[int, int],
    end: tuple[int, int],
    color: str,
    width: int = 9,
    dashed: bool = False,
) -> None:
    x0, y0 = start
    x1, y1 = end
    if dashed:
        segments = 12
        for index in range(segments):
            if index % 2:
                continue
            t0 = index / segments
            t1 = min((index + 1) / segments, 0.92)
            draw.line(
                (x0 + (x1 - x0) * t0, y0 + (y1 - y0) * t0,
                 x0 + (x1 - x0) * t1, y0 + (y1 - y0) * t1),
                fill=color,
                width=width,
            )
    else:
        draw.line((x0, y0, x1, y1), fill=color, width=width)
    direction_x = x1 - x0
    direction_y = y1 - y0
    length = max((direction_x**2 + direction_y**2) ** 0.5, 1.0)
    ux, uy = direction_x / length, direction_y / length
    px, py = -uy, ux
    head = 28
    wing = 16
    base_x = x1 - ux * head
    base_y = y1 - uy * head
    draw.polygon(
        [
            (x1, y1),
            (base_x + px * wing, base_y + py * wing),
            (base_x - px * wing, base_y - py * wing),
        ],
        fill=color,
    )


def save(image: Image.Image, name: str) -> None:
    FIGURES.mkdir(parents=True, exist_ok=True)
    image.save(FIGURES / name, dpi=(300, 300), optimize=True)


def overlay_pipeline() -> None:
    image = Image.new("RGB", (2400, 1120), BACKGROUND)
    draw = ImageDraw.Draw(image)

    technical_box(
        draw,
        (70, 105, 530, 425),
        STEEL_BLUE,
        "Volumetric image",
        ["RAW, TIFF, JPEG stack", "scalar or RGB; legacy VTK"],
    )
    technical_box(
        draw,
        (680, 105, 1190, 425),
        STEEL_BLUE,
        "Volume pipeline",
        [
            "grayscale transfer",
            "RGB + luminance alpha",
            "threshold + window / level",
            "VOI crop + image plane",
        ],
    )
    technical_box(
        draw,
        (70, 690, 530, 1010),
        COPPER,
        "Segmented geometry",
        ["points, meshes, surfaces", "VTK / STL / PLY", "VTP / OBJ / XYZ"],
    )
    technical_box(
        draw,
        (680, 690, 1190, 1010),
        COPPER,
        "PolyData pipeline",
        ["representation + normals", "clip box + colour + gloss"],
    )
    technical_box(
        draw,
        (1370, 275, 1880, 840),
        CHARCOAL,
        "Shared VTK renderer",
        [
            "common 3D coordinates",
            "per-dataset transforms",
            "opacity and colour",
            "co-moving slice plane",
        ],
    )
    technical_box(
        draw,
        (1960, 390, 2360, 730),
        STEEL_BLUE,
        "Interactive overlay",
        ["segmentation QC", "alignment checks", "publication views"],
        title_font=FONT_NODE_SMALL,
    )

    arrow(draw, (530, 265), (680, 265), STEEL_BLUE)
    arrow(draw, (1190, 265), (1370, 410), STEEL_BLUE)
    arrow(draw, (530, 850), (680, 850), COPPER)
    arrow(draw, (1190, 850), (1370, 705), COPPER)
    arrow(draw, (1880, 558), (1960, 558), CHARCOAL)

    draw.text((935, 500), "TWO INPUT PIPELINES", font=FONT_SMALL, fill=MID_GRAY, anchor="mm")
    draw.line((735, 548, 1135, 548), fill=GRID, width=4)
    draw.text((935, 598), "ONE SPATIAL SCENE", font=FONT_SMALL, fill=CHARCOAL, anchor="mm")

    save(image, "overlay-pipeline.png")


def organoid_workflow() -> None:
    image = Image.new("RGB", (2400, 1220), BACKGROUND)
    draw = ImageDraw.Draw(image)

    draw.text((65, 108), "IMAGING AND GEOMETRY", font=FONT_LANE, fill=CHARCOAL, anchor="lm")
    draw.line((65, 146, 2335, 146), fill=BORDER, width=4)

    top_y0, top_y1 = 215, 510
    technical_box(draw, (65, top_y0, 465, top_y1), STEEL_BLUE, "Organoid culture", ["organoids embedded", "in Matrigel"])
    technical_box(draw, (570, top_y0, 970, top_y1), STEEL_BLUE, "3D OCT", ["volumetric acquisition", "repeated time points"])
    technical_box(draw, (1075, top_y0, 1535, top_y1), COPPER, "Segmentation", ["individual organoids", "size + 3D position"])
    technical_box(draw, (1660, top_y0, 2160, top_y1), CHARCOAL, "OCTview3R overlay", ["OCT intensity + PolyData", "visual inspection"])

    arrow(draw, (465, 363), (570, 363), STEEL_BLUE)
    arrow(draw, (970, 363), (1075, 363), STEEL_BLUE)
    arrow(draw, (1535, 363), (1660, 363), COPPER)

    draw.text((65, 660), "ANALYSIS AND INTERVENTION", font=FONT_LANE, fill=CHARCOAL, anchor="lm")
    draw.line((65, 698, 2335, 698), fill=BORDER, width=4)

    technical_box(
        draw,
        (250, 775, 1030, 1110),
        CHARCOAL,
        "Spatial characterization",
        ["organoid size and position", "motion of neighbouring organoids"],
    )
    technical_box(
        draw,
        (1325, 775, 2245, 1110),
        STEEL_BLUE,
        "Targeted molecular workflow",
        ["select organoid -> laser ablation", "targeted sampling -> proteomics"],
    )

    arrow(draw, (1305, 510), (780, 775), COPPER)
    arrow(draw, (1910, 510), (1795, 775), CHARCOAL)
    arrow(draw, (770, 510), (590, 775), STEEL_BLUE, dashed=True)

    draw.text((355, 590), "REPEATED ACQUISITIONS", font=FONT_SMALL, fill=STEEL_BLUE, anchor="mm")
    draw.text((1715, 600), "TARGET CONTEXT", font=FONT_SMALL, fill=CHARCOAL, anchor="mm")

    save(image, "organoid-study-workflow.png")


if __name__ == "__main__":
    overlay_pipeline()
    organoid_workflow()
