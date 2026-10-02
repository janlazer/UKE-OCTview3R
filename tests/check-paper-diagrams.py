"""Check diagram routing and PNG/SVG generation; no scientific validation."""

import importlib.util
from pathlib import Path
import tempfile
import unittest
import xml.etree.ElementTree as ET

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("diagrams", ROOT / "paper/generate_diagrams.py")
diagrams = importlib.util.module_from_spec(spec)
spec.loader.exec_module(diagrams)


class PaperDiagrams(unittest.TestCase):
    def test_diagonal_connector_is_rejected(self):
        d = diagrams.Diagram(400, 300, "test")
        with self.assertRaisesRegex(AssertionError, "horizontal or vertical"):
            d.arrow([(30, 30), (200, 180)])

    def test_connector_cannot_cross_a_module(self):
        d = diagrams.Diagram(400, 300, "test")
        d.box((100, 80, 300, 230), "Test", diagrams.CHARCOAL)
        with self.assertRaisesRegex(AssertionError, "crosses a module"):
            d.arrow([(30, 150), (350, 150)])

    def test_unfilled_polygon_preserves_background(self):
        d = diagrams.Diagram(100, 100, "test")
        before = d.image.getpixel((50, 40))
        d.polygon([(10, 10), (90, 10), (50, 90)], fill=None)
        self.assertEqual(d.image.getpixel((50, 40)), before)

    def test_both_figures_export_editable_vectors_and_png(self):
        original = diagrams.FIGURES
        with tempfile.TemporaryDirectory(prefix="octview3r-diagrams-") as folder:
            try:
                diagrams.FIGURES = Path(folder)
                diagrams.overlay_pipeline()
                diagrams.organoid_workflow()
                for name, height in (("overlay-pipeline", 1260), ("organoid-study-workflow", 1270)):
                    with Image.open(Path(folder) / f"{name}.png") as image:
                        self.assertEqual(image.size, (2400, height))
                    svg = ET.parse(Path(folder) / f"{name}.svg").getroot()
                    self.assertEqual(svg.attrib["viewBox"], f"0 0 2400 {height}")
                    self.assertGreater(len(svg.findall("{*}text")), 15)
                    self.assertFalse(svg.findall("{*}image"), "Symbols must remain editable vectors")
            finally:
                diagrams.FIGURES = original


if __name__ == "__main__":
    unittest.main()
