"""Generate publication-sized block diagrams used by the JOSS draft."""

from __future__ import annotations

from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parent
FIGURES = ROOT / "figures"

NAVY = "#17365D"
BLUE = "#2F75B5"
BLUE_LIGHT = "#EAF3FA"
RED = "#C43C3C"
RED_LIGHT = "#FBECEC"
TEAL = "#23877A"
TEAL_LIGHT = "#E7F5F2"
GOLD = "#A66B00"
GOLD_LIGHT = "#FFF4D6"
INK = "#1F2933"
MUTED = "#5B6573"
BORDER = "#95A1B2"
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


FONT_TITLE = font(52, bold=True)
FONT_NODE = font(38, bold=True)
FONT_NODE_SMALL = font(33, bold=True)
FONT_BODY = font(31)
FONT_SMALL = font(27)
FONT_LANE = font(30, bold=True)


def rounded_box(
    draw: ImageDraw.ImageDraw,
    xy: tuple[int, int, int, int],
    fill: str,
    outline: str,
    title: str,
    lines: list[str],
    title_color: str = INK,
    title_font: ImageFont.FreeTypeFont = FONT_NODE,
) -> None:
    draw.rounded_rectangle(xy, radius=28, fill=fill, outline=outline, width=5)
    x0, y0, x1, _ = xy
    draw.text(((x0 + x1) / 2, y0 + 44), title, font=title_font, fill=title_color, anchor="mm")
    y = y0 + 108
    for line in lines:
        draw.text(((x0 + x1) / 2, y), line, font=FONT_BODY, fill=INK, anchor="mm")
        y += 48


def arrow(
    draw: ImageDraw.ImageDraw,
    start: tuple[int, int],
    end: tuple[int, int],
    color: str,
    width: int = 12,
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
    head = 32
    wing = 19
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
    image = Image.new("RGB", (2400, 1120), WHITE)
    draw = ImageDraw.Draw(image)

    rounded_box(
        draw,
        (70, 105, 530, 425),
        BLUE_LIGHT,
        BLUE,
        "Volumetric OCT",
        ["RAW, TIFF, JPEG stack", "legacy VTK image data"],
    )
    rounded_box(
        draw,
        (680, 105, 1190, 425),
        BLUE_LIGHT,
        BLUE,
        "Volume pipeline",
        ["threshold + transfer function", "VOI crop + image plane"],
    )
    rounded_box(
        draw,
        (70, 690, 530, 1010),
        RED_LIGHT,
        RED,
        "Segmented geometry",
        ["points, meshes, surfaces", "VTK, STL, PLY, VTP, OBJ, XYZ"],
    )
    rounded_box(
        draw,
        (680, 690, 1190, 1010),
        RED_LIGHT,
        RED,
        "PolyData pipeline",
        ["representation + normals", "clip box + colour + gloss"],
    )
    rounded_box(
        draw,
        (1370, 275, 1880, 840),
        TEAL_LIGHT,
        TEAL,
        "Shared VTK renderer",
        [
            "common 3D coordinates",
            "rotation / shift / scale",
            "opacity and colour",
            "co-moving slice plane",
        ],
    )
    rounded_box(
        draw,
        (1960, 390, 2360, 730),
        GOLD_LIGHT,
        GOLD,
        "Interactive overlay",
        ["segmentation QC", "alignment checks", "publication views"],
        title_font=FONT_NODE_SMALL,
    )

    arrow(draw, (530, 265), (680, 265), BLUE)
    arrow(draw, (1190, 265), (1370, 410), BLUE)
    arrow(draw, (530, 850), (680, 850), RED)
    arrow(draw, (1190, 850), (1370, 705), RED)
    arrow(draw, (1880, 558), (1960, 558), TEAL)

    draw.text((935, 500), "independent inputs", font=FONT_SMALL, fill=MUTED, anchor="mm")
    draw.line((735, 548, 1135, 548), fill=BORDER, width=3)
    draw.text((935, 598), "one spatial scene", font=FONT_SMALL, fill=TEAL, anchor="mm")

    save(image, "overlay-pipeline.png")


def organoid_workflow() -> None:
    image = Image.new("RGB", (2400, 1220), WHITE)
    draw = ImageDraw.Draw(image)

    draw.text((65, 108), "Imaging and geometry", font=FONT_LANE, fill=NAVY, anchor="lm")
    draw.line((65, 146, 2335, 146), fill=NAVY, width=4)

    top_y0, top_y1 = 215, 510
    rounded_box(draw, (65, top_y0, 465, top_y1), BLUE_LIGHT, BLUE, "Organoid culture", ["organoids embedded", "in Matrigel"])
    rounded_box(draw, (570, top_y0, 970, top_y1), BLUE_LIGHT, BLUE, "3D OCT", ["volumetric acquisition", "repeated time points"])
    rounded_box(draw, (1075, top_y0, 1535, top_y1), RED_LIGHT, RED, "Segmentation", ["individual organoids", "size + 3D position"])
    rounded_box(draw, (1660, top_y0, 2160, top_y1), TEAL_LIGHT, TEAL, "OCTview3R overlay", ["OCT intensity + PolyData", "visual validation"])

    arrow(draw, (465, 363), (570, 363), BLUE)
    arrow(draw, (970, 363), (1075, 363), BLUE)
    arrow(draw, (1535, 363), (1660, 363), RED)

    draw.text((65, 660), "Analysis and intervention", font=FONT_LANE, fill=NAVY, anchor="lm")
    draw.line((65, 698, 2335, 698), fill=NAVY, width=4)

    rounded_box(
        draw,
        (250, 775, 1030, 1110),
        GOLD_LIGHT,
        GOLD,
        "Spatial characterization",
        ["organoid size and position", "motion of neighbouring organoids"],
    )
    rounded_box(
        draw,
        (1325, 775, 2245, 1110),
        TEAL_LIGHT,
        TEAL,
        "Targeted molecular workflow",
        ["select organoid -> laser ablation", "targeted sampling -> proteomics"],
    )

    arrow(draw, (1305, 510), (780, 775), RED)
    arrow(draw, (1910, 510), (1795, 775), TEAL)
    arrow(draw, (770, 510), (590, 775), BLUE, dashed=True)

    draw.text((850, 640), "time-resolved displacement", font=FONT_SMALL, fill=BLUE, anchor="mm")
    draw.text((1715, 635), "verified target context", font=FONT_SMALL, fill=TEAL, anchor="mm")

    save(image, "organoid-study-workflow.png")


if __name__ == "__main__":
    overlay_pipeline()
    organoid_workflow()
