"""Generate synthetic Thorlabs-shaped ZIPs; never uses research input files."""
import io
from pathlib import Path
import struct
import zipfile
import zlib

OUT = Path(__file__).resolve().parents[1] / 'tmp/oct-regression/fixtures'
OUT.mkdir(parents=True, exist_ok=True)


def header(nx=3, ny=2, nz=4, extra='', data_extra='', ranges=True):
    calibration = 'RangeX="0.3" RangeY="0.2" RangeZ="0.4"' if ranges else ''
    return f'''<?xml version="1.0"?><Ocity version="1.0">
<DataFiles><DataFile Type="Real" SizeX="{nx}" SizeY="{ny}" SizeZ="{nz}"
BytesPerPixel="4" {calibration} {data_extra}>data\\Intensity.data</DataFile></DataFiles>
<Image Type="Processed"><AxisOrder>ZXYT</AxisOrder><SizeReal Unit="mm">
<SizeZ>0.56</SizeZ></SizeReal><PixelSpacing><SizeZ>0.14</SizeZ></PixelSpacing>
{extra}</Image><Acquisition><RefractiveIndex>1.4</RefractiveIndex></Acquisition></Ocity>'''.encode()


def samples(nx=3, ny=2, nz=4):
    return b''.join(struct.pack('<f', -20 + 20*y + 4*x + z*0.25)
                    for y in range(ny) for x in range(nx) for z in range(nz))


def write(name, xml=None, payload=None, compression=zipfile.ZIP_STORED, **kwargs):
    with zipfile.ZipFile(OUT / name, 'w', compression=compression) as archive:
        archive.writestr('Header.xml', header() if xml is None else xml)
        archive.writestr('data/Intensity.data', samples() if payload is None else payload, **kwargs)


write('stored.oct')
write('deflate.oct', compression=zipfile.ZIP_DEFLATED)
write('unsupported-compression.oct', compression=zipfile.ZIP_BZIP2)
write('unknown-calibration.oct', xml=header(ranges=False))
write('two-dimensional.oct', xml=header(ny=1), payload=samples(ny=1))
# Cross both compressed input and uncompressed output block boundaries.
write('large-deflate.oct', xml=header(nx=257, ny=3, nz=83), payload=samples(257, 3, 83),
      compression=zipfile.ZIP_DEFLATED)
write('wrong-size.oct', payload=samples()[:-4])
write('nan.oct', payload=struct.pack('<f', float('nan')) + samples()[4:])
write('wrong-axis.oct', xml=header().replace(b'ZXYT', b'XYZT'))
write('time-series.oct', xml=header(data_extra='SizeT="2"'))
write('freeform.oct', xml=header(extra='<FreeformScanPatternIsActive>True</FreeformScanPatternIsActive>'))
write('bad-range.oct', xml=header().replace(b'RangeZ="0.4"', b'RangeZ="nan"'))
write('wrong-type.oct', xml=header().replace(b'Type="Real"', b'Type="Complex"'))
write('bad-xml.oct', xml=b'<Ocity><DataFiles>')
write('bad-version.oct', xml=header().replace(b'Ocity version="1.0"', b'Ocity version="9.0"'))
write('dtd.oct', xml=header().replace(b'<Ocity', b'<!DOCTYPE Ocity [<!ENTITY bad "expanded">]><Ocity', 1))
write('conflicting-dimensions.oct', xml=header(extra='<SizePixel><SizeX>99</SizeX></SizePixel>'))
write('overflow.oct', xml=header(nx=2147483647, ny=2147483647, nz=2147483647), payload=b'')
with zipfile.ZipFile(OUT / 'raw-only.oct', 'w') as archive:
    archive.writestr('Header.xml', b'<Ocity version="1.0"><DataFiles><DataFile Type="Raw">data\\Spectral0.data</DataFile></DataFiles></Ocity>')
    archive.writestr('data/Spectral0.data', b'raw spectra')
with zipfile.ZipFile(OUT / 'backslash.oct', 'w') as archive:
    archive.writestr('header.xml', header())
    archive.writestr('data\\Intensity.data', samples())
with zipfile.ZipFile(OUT / 'local-zip64.oct', 'w') as archive:
    archive.writestr('Header.xml', header())
    with archive.open('data/Intensity.data', 'w', force_zip64=True) as member:
        member.write(samples())
with zipfile.ZipFile(OUT / 'duplicate.oct', 'w') as archive:
    archive.writestr('Header.xml', header())
    archive.writestr('header.xml', header())
    archive.writestr('data/Intensity.data', samples())
corrupt = bytearray((OUT / 'stored.oct').read_bytes())
with zipfile.ZipFile(OUT / 'stored.oct') as archive:
    info = archive.getinfo('data/Intensity.data')
    offset = info.header_offset + 30 + len(info.filename.encode()) + len(info.extra)
corrupt[offset] ^= 1
(OUT / 'crc-error.oct').write_bytes(corrupt)
(OUT / 'truncated.oct').write_bytes(corrupt[:-20])

# Full ZIP64 central directory and EOCD, despite tiny synthetic payloads.
body, central = bytearray(), bytearray()
for name, data in [('Header.xml', header()), ('data/Intensity.data', samples())]:
    name = name.encode()
    crc, offset = zlib.crc32(data), len(body)
    extra = struct.pack('<HHQQ', 1, 16, len(data), len(data))
    body += struct.pack('<I5H3I2H', 0x04034b50, 45, 0, 0, 0, 0,
                        crc, 0xffffffff, 0xffffffff, len(name), len(extra)) + name + extra + data
    extra = struct.pack('<HHQQQ', 1, 24, len(data), len(data), offset)
    central += struct.pack('<I6H3I5H2I', 0x02014b50, 45, 45, 0, 0, 0, 0, crc,
                           0xffffffff, 0xffffffff, len(name), len(extra), 0, 0, 0, 0, 0xffffffff) + name + extra
start = len(body)
body += central
zip64 = len(body)
body += struct.pack('<IQ2H2I4Q', 0x06064b50, 44, 45, 45, 0, 0, 2, 2, len(central), start)
body += struct.pack('<IIQI', 0x07064b50, 0, zip64, 1)
body += struct.pack('<I4H2IH', 0x06054b50, 0, 0, 0xffff, 0xffff, 0xffffffff, 0xffffffff, 0)
(OUT / 'zip64.oct').write_bytes(body)

# ZIP data descriptors (bit 3) from a non-seekable writer.
class Sequential(io.BytesIO):
    def seekable(self): return False
    def seek(self, *args): raise io.UnsupportedOperation('sequential')

stream = Sequential()
with zipfile.ZipFile(stream, 'w', compression=zipfile.ZIP_DEFLATED) as archive:
    archive.writestr('Header.xml', header())
    archive.writestr('data/Intensity.data', samples())
(OUT / 'descriptor.oct').write_bytes(stream.getvalue())

# A valid 0 dB cube surrounded by low signal: must remain visible in 3D.
volume = b''.join(struct.pack('<f', 0 if 8 <= x < 24 and 8 <= y < 24 and 8 <= z < 24 else -30)
                  for y in range(32) for x in range(32) for z in range(32))
write('render.oct', xml=header(32, 32, 32), payload=volume, compression=zipfile.ZIP_DEFLATED)
print(f'Generated {len(list(OUT.glob("*.oct")))} synthetic OCT fixtures in {OUT}')
