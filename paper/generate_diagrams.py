"""Reproducible paper diagrams with original schematic scientific symbols.

PNG is used by the manuscript; matching SVG keeps all shapes and text editable.
No experimental images or third-party illustration assets are used here.
"""

from __future__ import annotations

import math
from pathlib import Path
import xml.etree.ElementTree as ET

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parent
FIGURES = ROOT / "figures"
BACKGROUND = "#FAFAF9"
INK = "#20272B"
CHARCOAL = "#374148"
MID_GRAY = "#707A80"
BORDER = "#697780"
PANEL = "#FFFFFF"
STEEL_BLUE = "#4E7080"
BLUE_LIGHT = "#E3ECEF"
COPPER = "#8A604A"
COPPER_LIGHT = "#EFE2D9"


def font(size: int, bold: bool = False) -> ImageFont.FreeTypeFont:
    names = (
        ["C:/Windows/Fonts/arialbd.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"]
        if bold else
        ["C:/Windows/Fonts/arial.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"]
    )
    for name in names:
        if Path(name).exists():
            return ImageFont.truetype(name, size=size)
    return ImageFont.load_default(size=size)


class Diagram:
    """Record identical drawing primitives to Pillow and a vector SVG."""

    def __init__(self, width: int, height: int, title: str):
        self.image = Image.new("RGB", (width, height), BACKGROUND)
        self.draw = ImageDraw.Draw(self.image)
        self.svg = ET.Element("svg", xmlns="http://www.w3.org/2000/svg",
                              viewBox=f"0 0 {width} {height}", width=str(width), height=str(height))
        ET.SubElement(self.svg, "title").text = title
        ET.SubElement(self.svg, "desc").text = (
            "Original schematic symbols, not measured data. "
            "Steel blue denotes image data; copper denotes derived geometry."
        )
        self.boxes = []
        self.routes = []
        self.rect((0, 0, width, height), BACKGROUND)

    def element(self, tag, **attrs):
        return ET.SubElement(self.svg, tag, {k.replace("_", "-"): str(v) for k, v in attrs.items()})

    def rect(self, xy, fill=PANEL, outline=None, width=4):
        self.draw.rectangle(xy, fill=fill, outline=outline, width=width)
        x0, y0, x1, y1 = xy
        self.element("rect", x=x0, y=y0, width=x1-x0, height=y1-y0,
                     fill=fill or "none", stroke=outline or "none", stroke_width=width)

    def ellipse(self, xy, fill=None, outline=BORDER, width=4):
        self.draw.ellipse(xy, fill=fill, outline=outline, width=width)
        x0, y0, x1, y1 = xy
        self.element("ellipse", cx=(x0+x1)/2, cy=(y0+y1)/2, rx=(x1-x0)/2, ry=(y1-y0)/2,
                     fill=fill or "none", stroke=outline or "none", stroke_width=width)

    def polygon(self, points, fill=None, outline=BORDER, width=4):
        # Pillow's default ink would otherwise fill nominally transparent shapes.
        if fill is not None:
            self.draw.polygon(points, fill=fill)
        if outline:
            self.draw.line([*points, points[0]], fill=outline, width=width, joint="curve")
        self.element("polygon", points=" ".join(f"{x},{y}" for x, y in points),
                     fill=fill or "none", stroke=outline or "none", stroke_width=width,
                     stroke_linejoin="round")

    def line(self, points, color=BORDER, width=5, dashed=False):
        if dashed:
            for (x0, y0), (x1, y1) in zip(points, points[1:]):
                length = math.hypot(x1-x0, y1-y0)
                for offset in range(0, math.ceil(length), 28):
                    t0, t1 = offset/length, min((offset+16)/length, 1)
                    self.draw.line([(x0+(x1-x0)*t0, y0+(y1-y0)*t0),
                                    (x0+(x1-x0)*t1, y0+(y1-y0)*t1)], fill=color, width=width)
        else:
            self.draw.line(points, fill=color, width=width, joint="curve")
        attrs = dict(points=" ".join(f"{x},{y}" for x, y in points), fill="none",
                     stroke=color, stroke_width=width, stroke_linejoin="round")
        if dashed:
            attrs["stroke_dasharray"] = "16 12"
        self.element("polyline", **attrs)

    def text(self, xy, value, size=38, color=INK, bold=False, anchor="mm", max_width=None):
        typeface = font(size, bold)
        if max_width is not None:
            assert self.draw.textlength(value, font=typeface) <= max_width, f"Text too wide: {value}"
        self.draw.text(xy, value, font=typeface, fill=color, anchor=anchor)
        node = self.element("text", x=xy[0], y=xy[1], fill=color, font_family="Arial, DejaVu Sans, sans-serif",
                            font_size=size, font_weight="bold" if bold else "normal",
                            text_anchor="start" if anchor == "lm" else "middle",
                            dominant_baseline="central")
        node.text = value

    def box(self, xy, title, accent, header=82, title_size=45):
        self.boxes.append(xy)
        x0, y0, x1, y1 = xy
        self.rect(xy, PANEL, BORDER, 4)
        self.rect((x0+3, y0+3, x1-3, y0+header), accent)
        lines = title.split("\n")
        for index, line in enumerate(lines):
            self.text(((x0+x1)/2, y0+header/2+(index-(len(lines)-1)/2)*48),
                      line, title_size, "#FFFFFF", True, max_width=x1-x0-22)

    def arrow(self, points, color=CHARCOAL, dashed=False):
        """Only orthogonal connectors, with no path through another module."""
        for (x0, y0), (x1, y1) in zip(points, points[1:]):
            assert (x0 == x1) != (y0 == y1), "Connector must be horizontal or vertical"
            for bx0, by0, bx1, by1 in self.boxes:
                if x0 == x1:
                    crosses = bx0 < x0 < bx1 and max(min(y0, y1), by0) < min(max(y0, y1), by1)
                else:
                    crosses = by0 < y0 < by1 and max(min(x0, x1), bx0) < min(max(x0, x1), bx1)
                assert not crosses, "Connector crosses a module"
        self.routes.append(points)
        self.line(points, color, 8, dashed)
        (x0, y0), (x1, y1) = points[-2:]
        length = math.hypot(x1-x0, y1-y0)
        assert length >= 30
        ux, uy = (x1-x0)/length, (y1-y0)/length
        self.polygon([(x1, y1), (x1-ux*25-uy*13, y1-uy*25+ux*13),
                      (x1-ux*25+uy*13, y1-uy*25-ux*13)], color, None)

    def save(self, name):
        FIGURES.mkdir(parents=True, exist_ok=True)
        self.image.save(FIGURES / f"{name}.png", dpi=(300, 300), optimize=True)
        ET.indent(self.svg)
        ET.ElementTree(self.svg).write(FIGURES / f"{name}.svg", encoding="utf-8", xml_declaration=True)
        print(f"{name}: PNG + editable SVG; {len(self.routes)} orthogonal connectors validated")


class Icon:
    """Local 300 x 200 coordinate system for reusable scientific symbols."""

    def __init__(self, diagram, cx, cy, width):
        self.d = diagram
        self.scale = width/300
        self.x, self.y = cx-width/2, cy-width/3

    def point(self, xy):
        return self.x+xy[0]*self.scale, self.y+xy[1]*self.scale

    def bounds(self, xy):
        return (*self.point(xy[:2]), *self.point(xy[2:]))

    def line(self, points, color=BORDER, width=3, dashed=False):
        self.d.line([self.point(p) for p in points], color, max(2, round(width*self.scale)), dashed)

    def polygon(self, points, fill=None, outline=BORDER, width=3):
        self.d.polygon([self.point(p) for p in points], fill, outline, max(2, round(width*self.scale)))

    def ellipse(self, xy, fill=None, outline=BORDER, width=3):
        self.d.ellipse(self.bounds(xy), fill, outline, max(2, round(width*self.scale)))

    def rect(self, xy, fill=None, outline=BORDER, width=3):
        self.d.rect(self.bounds(xy), fill, outline, max(2, round(width*self.scale)))


def mesh(icon, cx=150, cy=100, rx=75, ry=56, fill=COPPER_LIGHT, color=COPPER):
    """Triangulated ellipsoid-like front surface, drawn without source data."""
    rows = []
    for latitude in (-90, -55, -20, 20, 55, 90):
        a = math.radians(latitude)
        row = []
        for longitude in (-90, -60, -30, 0, 30, 60, 90):
            b = math.radians(longitude)
            row.append((cx+rx*math.cos(a)*math.sin(b),
                        cy+ry*math.sin(a)+ry*.15*math.cos(a)*math.cos(b)))
        rows.append(row)
    for upper, lower in zip(rows, rows[1:]):
        for n in range(len(upper)-1):
            icon.polygon([upper[n], lower[n], lower[n+1]], fill, color, 1.7)
            icon.polygon([upper[n], lower[n+1], upper[n+1]], fill, color, 1.7)


def volume(icon, overlay=False, plane=False):
    front = [(25, 65), (215, 65), (215, 180), (25, 180)]
    icon.polygon([(25, 65), (85, 20), (275, 20), (215, 65)], "#D4DADD")
    icon.polygon([(215, 65), (275, 20), (275, 135), (215, 180)], "#929CA3")
    icon.polygon(front, "#BDC5CA")
    for y in (85, 108, 131, 154):
        icon.line([(26, y), (214, y)], "#ABB5BC", 2)
        icon.line([(216, y), (274, y-45)], "#7D8A93", 2)
    # Grayscale structures hint at tissue contrast; they are not an OCT measurement.
    for x, y, rx, ry in ((74, 109, 31, 22), (160, 137, 34, 24), (175, 91, 17, 12)):
        icon.ellipse((x-rx, y-ry, x+rx, y+ry), "#E5E9EB", "#919DA5", 2)
        icon.ellipse((x-rx*.55, y-ry*.55, x+rx*.55, y+ry*.55), "#C2CBD0", None)
        if overlay:
            mesh(icon, x, y, rx, ry, fill=None)
    if plane:
        icon.polygon([(123, 66), (183, 21), (183, 136), (123, 181)], None, STEEL_BLUE, 4)
    icon.line(front+[front[0]], BORDER, 3)


def geometry(icon):
    mesh(icon, 103, 116, 63, 46)
    mesh(icon, 214, 78, 40, 31)
    mesh(icon, 230, 148, 28, 23)
    icon.line([(30, 175), (120, 175)], MID_GRAY, 2)
    icon.line([(30, 175), (30, 100)], MID_GRAY, 2)
    icon.line([(30, 175), (65, 151)], MID_GRAY, 2)


def transfer(icon):
    icon.rect((65, 32, 120, 172), BLUE_LIGHT, None)
    icon.line([(30, 25), (30, 175), (278, 175)], MID_GRAY, 3)
    icon.line([(34, 169), (77, 167), (116, 128), (164, 74), (212, 39), (271, 32)], STEEL_BLUE, 5)
    for x, y in ((77, 167), (116, 128), (164, 74), (212, 39)):
        icon.ellipse((x-5, y-5, x+5, y+5), PANEL, STEEL_BLUE, 2)


def clipping(icon):
    mesh(icon, 155, 100, 103, 70)
    icon.line([(95, 15), (270, 15), (270, 186), (95, 186), (95, 15)], STEEL_BLUE, 3, True)
    icon.line([(95, 15), (95, 186)], STEEL_BLUE, 4)


def dish(icon, laser=False):
    icon.rect((32, 101, 268, 140), BLUE_LIGHT, None)
    icon.ellipse((32, 107, 268, 176), BLUE_LIGHT, STEEL_BLUE, 3)
    icon.line([(32, 101), (32, 140)], STEEL_BLUE, 3)
    icon.line([(268, 101), (268, 140)], STEEL_BLUE, 3)
    icon.ellipse((32, 65, 268, 142), "#F2F6F7", STEEL_BLUE, 3)
    for x, y, r in ((75, 96, 13), (119, 115, 12), (175, 108, 17), (213, 91, 12)):
        icon.ellipse((x-r, y-r*.65, x+r, y+r*.65), COPPER_LIGHT, COPPER, 2)
        icon.ellipse((x-3, y-2, x+3, y+2), COPPER, None)
    if laser:
        icon.polygon([(153, 4), (197, 4), (190, 39), (160, 39)], "#CDD4D8", CHARCOAL, 3)
        icon.line([(175, 43), (175, 101)], COPPER, 4)
        icon.ellipse((152, 92, 198, 124), None, COPPER, 2)
        icon.line([(150, 108), (200, 108)], COPPER, 2)
        icon.line([(175, 88), (175, 128)], COPPER, 2)


def acquisition(icon):
    volume(icon)
    icon.rect((117, 0, 163, 25), CHARCOAL, CHARCOAL)
    icon.line([(130, 27), (130, 62)], STEEL_BLUE, 3)
    icon.line([(151, 27), (151, 62)], STEEL_BLUE, 3)


def displacement(icon):
    icon.ellipse((40, 45, 111, 116), BLUE_LIGHT, STEEL_BLUE, 3)
    mesh(icon, 197, 81, 47, 35)
    icon.line([(113, 79), (141, 79)], CHARCOAL, 3)
    icon.polygon([(145, 79), (136, 74), (136, 84)], CHARCOAL, None)
    icon.line([(148, 142), (246, 142)], COPPER, 2)
    icon.line([(148, 134), (148, 151)], COPPER, 2)
    icon.line([(246, 134), (246, 151)], COPPER, 2)
    icon.line([(35, 170), (270, 170)], MID_GRAY, 2)
    icon.line([(35, 170), (35, 27)], MID_GRAY, 2)


def sample_tube(icon):
    icon.polygon([(102, 49), (199, 49), (192, 140), (157, 183), (141, 183), (108, 140)], BLUE_LIGHT, STEEL_BLUE)
    icon.polygon([(110, 112), (190, 112), (185, 141), (155, 176), (144, 176), (116, 139)], "#B9CBD3", None)
    icon.rect((95, 30, 206, 53), "#D3DCE1", STEEL_BLUE)
    icon.line([(192, 30), (221, 5), (270, 23)], STEEL_BLUE, 4)
    icon.ellipse((147, 79, 163, 102), COPPER_LIGHT, COPPER, 2)


def spectrum(icon):
    icon.line([(29, 25), (29, 175), (275, 175)], CHARCOAL, 3)
    for x, h in ((52, 38), (76, 74), (103, 43), (127, 137), (149, 58), (181, 109), (211, 51), (246, 89)):
        icon.line([(x, 170), (x, 170-h)], STEEL_BLUE, 4)


def body(diagram, cx, first_y, lines, width, size=39, spacing=55):
    for i, line in enumerate(lines):
        diagram.text((cx, first_y+i*spacing), line, size, max_width=width-32)


def overlay_pipeline():
    d = Diagram(2400, 1260, "OCTview3R volume and PolyData rendering architecture")
    d.box((40, 70, 550, 575), "Volumetric image", STEEL_BLUE)
    d.box((40, 685, 550, 1190), "Segmented geometry", COPPER, title_size=43)
    d.box((675, 70, 1195, 575), "Volume pipeline", STEEL_BLUE)
    d.box((675, 685, 1195, 1190), "PolyData pipeline", COPPER)
    d.box((1345, 300, 1815, 975), "Shared VTK\nrenderer", CHARCOAL, header=125)
    d.box((1940, 300, 2360, 975), "Interactive\noverlay", STEEL_BLUE, header=125)

    volume(Icon(d, 295, 280, 325))
    geometry(Icon(d, 295, 895, 325))
    transfer(Icon(d, 935, 250, 265))
    clipping(Icon(d, 935, 865, 265))
    volume(Icon(d, 1580, 585, 320), overlay=True, plane=True)
    volume(Icon(d, 2150, 585, 310), overlay=True)

    body(d, 295, 425, ["Scalar / RGB volumes", "RAW / TIFF / JPEG", "Legacy VTK"], 510)
    body(d, 295, 1040, ["Points / meshes / surfaces", "VTK / STL / PLY", "VTP / OBJ / XYZ"], 510, 37)
    body(d, 935, 370, ["Grayscale transfer", "RGB + luminance alpha", "Threshold + window/level", "VOI crop + slice plane"], 520, 37, 53)
    body(d, 935, 1008, ["Representation + normals", "Box clipping", "Colour + gloss"], 520, 37, 56)
    body(d, 1580, 755, ["Common coordinates", "Per-dataset transforms", "Opacity + colour", "Parent-linked plane"], 470, 36, 54)
    body(d, 2150, 775, ["Segmentation QC", "Alignment checks", "Publication views"], 420, 38, 58)

    d.arrow([(550, 320), (675, 320)], STEEL_BLUE)
    d.arrow([(550, 935), (675, 935)], COPPER)
    d.arrow([(1195, 320), (1260, 320), (1260, 510), (1345, 510)], STEEL_BLUE)
    d.arrow([(1195, 935), (1295, 935), (1295, 810), (1345, 810)], COPPER)
    d.arrow([(1815, 635), (1940, 635)], CHARCOAL)
    d.text((930, 627), "TWO PIPELINES / ONE SPATIAL SCENE", 30, MID_GRAY)
    d.save("overlay-pipeline")


def organoid_workflow():
    d = Diagram(2400, 1270, "Organoid OCT, segmentation and targeted sampling workflow")
    boxes = [(50, 70, 540, 590), (650, 70, 1140, 590),
             (1250, 70, 1740, 590), (1850, 70, 2340, 590)]
    for box, title, color in zip(boxes, ["Organoid culture", "3D OCT", "Segmentation", "OCTview3R overlay"],
                                 [STEEL_BLUE, STEEL_BLUE, COPPER, CHARCOAL]):
        d.box(box, title, color, title_size=43)
    d.box((220, 900, 1120, 1220), "Spatial characterization", CHARCOAL)
    d.box((1340, 900, 2340, 1220), "Targeted molecular workflow", STEEL_BLUE)

    dish(Icon(d, 295, 298, 365))
    acquisition(Icon(d, 895, 308, 330))
    geometry(Icon(d, 1495, 300, 330))
    volume(Icon(d, 2095, 308, 330), overlay=True)
    body(d, 295, 472, ["Organoids embedded", "in Matrigel"], 490, 39)
    body(d, 895, 472, ["Volumetric acquisition", "Repeated time points"], 490, 39)
    body(d, 1495, 472, ["Individual organoids", "Size + 3D position"], 490, 39)
    body(d, 2095, 472, ["OCT + PolyData", "Visual inspection"], 490, 39)

    displacement(Icon(d, 385, 1080, 265))
    body(d, 795, 1035, ["Size and position", "Motion of neighbouring", "organoids"], 550, 38, 58)
    dish(Icon(d, 1515, 1070, 192), laser=True)
    sample_tube(Icon(d, 1825, 1068, 190))
    spectrum(Icon(d, 2170, 1068, 198))
    # Within this module, three explicit steps replace the old text arrows.
    d.line([(1625, 1065), (1680, 1065)], MID_GRAY, 5)
    d.polygon([(1686, 1065), (1673, 1058), (1673, 1072)], MID_GRAY, None)
    d.line([(1950, 1065), (2010, 1065)], MID_GRAY, 5)
    d.polygon([(2016, 1065), (2003, 1058), (2003, 1072)], MID_GRAY, None)
    for x, label in ((1515, "Select + ablate"), (1825, "Sampling"), (2170, "Proteomics")):
        d.text((x, 1167), label, 34, max_width=280)

    for first, second, color in zip(boxes, boxes[1:], [STEEL_BLUE, STEEL_BLUE, COPPER]):
        d.arrow([(first[2], 320), (second[0], 320)], color)
    d.arrow([(895, 590), (895, 680), (475, 680), (475, 900)], STEEL_BLUE, True)
    d.arrow([(1495, 590), (1495, 785), (955, 785), (955, 900)], COPPER)
    d.arrow([(2095, 590), (2095, 900)], CHARCOAL)
    d.text((675, 639), "Repeated acquisitions", 34, STEEL_BLUE)
    d.text((1210, 744), "Derived geometry", 34, COPPER)
    d.text((2140, 744), "Target context", 34, CHARCOAL, anchor="lm")
    d.save("organoid-study-workflow")


if __name__ == "__main__":
    overlay_pipeline()
    organoid_workflow()
