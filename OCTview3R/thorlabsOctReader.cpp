#include "thorlabsOctReader.h"

#include <QByteArray>
#include <QFile>
#include <QMap>
#include <QStringList>
#include <QXmlStreamReader>
#include <QtEndian>
#include <vtkImageData.h>
#include <vtk_zlib.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <new>
#include <stdexcept>
#include <vector>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace
{
struct ImportError { QString message; };
void check(bool valid, const QString& message)
{
    if (!valid) throw ImportError{message};
}
quint16 u16(const char* p) { return qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(p)); }
quint32 u32(const char* p) { return qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(p)); }
quint64 u64(const char* p) { return qFromLittleEndian<quint64>(reinterpret_cast<const uchar*>(p)); }
quint64 multiply(quint64 a, quint64 b)
{
    check(!b || a <= std::numeric_limits<quint64>::max() / b, "OCT dimensions overflow the supported size.");
    return a * b;
}
QString normalized(QString name)
{
    name.replace('\\', '/');
    return name.toLower();
}
QByteArray exact(QFile& file, qint64 count)
{
    check(count >= 0 && count <= 64 * 1024 * 1024, "OCT archive field is too large.");
    auto bytes = file.read(count);
    check(bytes.size() == count, "OCT archive is truncated or unreadable.");
    return bytes;
}

struct Entry
{
    quint64 packed = 0, unpacked = 0, offset = 0;
    quint32 crc = 0;
    quint16 method = 0, flags = 0;
};

// Small read-only ZIP subset, using VTK's existing zlib. Read the central
// directory (including ZIP64) rather than searching for signatures in payloads.
// No archive member is ever used as a filesystem destination.
class Archive
{
public:
    explicit Archive(const QString& path) : file(path)
    {
        check(file.open(QIODevice::ReadOnly), "Cannot open OCT file for reading.");
        check(file.size() >= 22, "Not a supported ZIP-based Thorlabs OCT file.");
        const qint64 tailStart = std::max<qint64>(0, file.size() - 65557);
        file.seek(tailStart);
        const auto tail = exact(file, file.size() - tailStart);
        int end = -1;
        for (int i = tail.size() - 22; i >= 0; --i)
            if (u32(tail.constData() + i) == 0x06054b50 &&
                i + 22 + u16(tail.constData() + i + 20) == tail.size()) { end = i; break; }
        check(end >= 0, "Not a supported ZIP-based Thorlabs OCT file (missing ZIP directory).");
        const char* e = tail.constData() + end;
        check(u16(e + 4) == 0 && u16(e + 6) == 0 && u16(e + 8) == u16(e + 10),
            "Multi-disk OCT archives are not supported.");
        quint64 count = u16(e + 10), directoryBytes = u32(e + 12), directory = u32(e + 16);
        const quint64 endOffset = tailStart + end;
        if (count == 0xffff || directoryBytes == 0xffffffff || directory == 0xffffffff) {
            check(endOffset >= 20 && file.seek(endOffset - 20), "Invalid ZIP64 locator.");
            const auto locator = exact(file, 20);
            check(u32(locator.constData()) == 0x07064b50 && u32(locator.constData() + 4) == 0 &&
                u32(locator.constData() + 16) == 1, "Invalid or multi-disk ZIP64 OCT archive.");
            const quint64 offset = u64(locator.constData() + 8);
            check(offset < endOffset - 20 && file.seek(offset), "Invalid ZIP64 directory offset.");
            const auto record = exact(file, 56);
            const char* r = record.constData();
            check(u32(r) == 0x06064b50 && u64(r + 4) >= 44 && u32(r + 16) == 0 &&
                u32(r + 20) == 0 && u64(r + 24) == u64(r + 32), "Invalid ZIP64 directory.");
            count = u64(r + 32); directoryBytes = u64(r + 40); directory = u64(r + 48);
        }
        check(count > 0 && count <= 50000 && directoryBytes <= 64 * 1024 * 1024 &&
            directory <= endOffset && directoryBytes <= endOffset - directory,
            "OCT ZIP directory exceeds supported bounds.");
        check(file.seek(directory), "Cannot seek to OCT ZIP directory.");
        const auto central = exact(file, directoryBytes);
        quint64 position = 0;
        for (quint64 i = 0; i < count; ++i) {
            check(position + 46 <= directoryBytes, "Truncated OCT ZIP entry.");
            const char* p = central.constData() + position;
            check(u32(p) == 0x02014b50, "Invalid OCT ZIP entry.");
            const int nameSize = u16(p + 28), extraSize = u16(p + 30), commentSize = u16(p + 32);
            check(position + 46 + nameSize + extraSize + commentSize <= directoryBytes,
                "Truncated OCT ZIP entry name/metadata.");
            Entry entry;
            entry.flags = u16(p + 8); entry.method = u16(p + 10); entry.crc = u32(p + 16);
            entry.packed = u32(p + 20); entry.unpacked = u32(p + 24); entry.offset = u32(p + 42);
            check(u16(p + 34) == 0, "Multi-disk OCT entries are unsupported.");
            bool need64 = entry.packed == 0xffffffff || entry.unpacked == 0xffffffff || entry.offset == 0xffffffff;
            const char* extra = p + 46 + nameSize;
            for (int j = 0; j + 4 <= extraSize;) {
                const int size = u16(extra + j + 2);
                check(j + 4 + size <= extraSize, "Invalid ZIP extra field.");
                if (u16(extra + j) == 1 && need64) {
                    int cursor = j + 4;
                    const auto take64 = [&]() {
                        check(cursor + 8 <= j + 4 + size, "Incomplete ZIP64 entry.");
                        const auto value = u64(extra + cursor); cursor += 8; return value;
                    };
                    if (entry.unpacked == 0xffffffff) entry.unpacked = take64();
                    if (entry.packed == 0xffffffff) entry.packed = take64();
                    if (entry.offset == 0xffffffff) entry.offset = take64();
                    need64 = false;
                }
                j += 4 + size;
            }
            check(!need64, "Missing ZIP64 entry sizes.");
            const QString name = normalized(QString::fromUtf8(p + 46, nameSize));
            check(!entries.contains(name), "Ambiguous duplicate names in OCT archive.");
            entries.insert(name, entry);
            position += 46 + nameSize + extraSize + commentSize;
        }
        check(position == directoryBytes, "Unexpected trailing OCT ZIP directory data.");
        dataEnd = directory;
    }

    Entry entry(const QString& name) const
    {
        check(entries.contains(normalized(name)), "OCT archive is missing " + name + ".");
        const auto e = entries.value(normalized(name));
        check(!(e.flags & (1 | 0x40 | 0x2000)), "Encrypted OCT archives are not supported.");
        check(e.method == 0 || e.method == 8, "Unsupported OCT ZIP compression (use Stored or Deflate).");
        return e;
    }

    void stream(const QString& name, quint64 expected,
        const std::function<void(const char*, int)>& consume)
    {
        const Entry e = entry(name);
        check(e.unpacked == expected, "OCT data size does not match its header dimensions.");
        check(e.offset <= dataEnd && dataEnd - e.offset >= 30 && file.seek(e.offset), "Invalid OCT member offset.");
        const auto local = exact(file, 30);
        const char* h = local.constData();
        check(u32(h) == 0x04034b50 && u16(h + 6) == e.flags && u16(h + 8) == e.method,
            "Inconsistent OCT ZIP local header.");
        const auto localName = exact(file, u16(h + 26));
        check(normalized(QString::fromUtf8(localName)) == normalized(name), "Inconsistent OCT ZIP member name.");
        const quint64 start = e.offset + 30 + u16(h + 26) + u16(h + 28);
        check(start <= dataEnd && e.packed <= dataEnd - start && file.seek(start), "Truncated OCT member payload.");
        struct Inflater {
            z_stream z{}; bool active = false;
            ~Inflater() { if (active) inflateEnd(&z); }
        } inflater;
        if (e.method == 8) {
            check(inflateInit2(&inflater.z, -MAX_WBITS) == Z_OK, "Cannot initialize OCT decompression.");
            inflater.active = true;
        } else check(e.packed == e.unpacked, "Inconsistent uncompressed OCT ZIP sizes.");
        quint64 packed = 0, unpacked = 0;
        uLong checksum = crc32(0, Z_NULL, 0);
        int status = Z_OK;
        std::vector<char> buffer(64 * 1024);
        const auto deliver = [&](const char* bytes, int size) {
            check(static_cast<quint64>(size) <= expected - unpacked, "OCT decompression exceeds the declared size.");
            checksum = crc32(checksum, reinterpret_cast<const Bytef*>(bytes), size);
            consume(bytes, size); unpacked += size;
        };
        while (packed < e.packed) {
            auto input = exact(file, std::min<quint64>(64 * 1024, e.packed - packed));
            packed += input.size();
            if (e.method == 0) { deliver(input.constData(), input.size()); continue; }
            check(status != Z_STREAM_END, "Unexpected data after OCT compressed stream.");
            inflater.z.next_in = reinterpret_cast<Bytef*>(input.data());
            inflater.z.avail_in = input.size();
            do {
                inflater.z.next_out = reinterpret_cast<Bytef*>(buffer.data());
                inflater.z.avail_out = static_cast<uInt>(buffer.size());
                const auto before = inflater.z.avail_in;
                status = inflate(&inflater.z, Z_NO_FLUSH);
                const int produced = static_cast<int>(buffer.size()) - inflater.z.avail_out;
                if (status == Z_BUF_ERROR && before == 0 && produced == 0) break;
                check(status == Z_OK || status == Z_STREAM_END, "Corrupt OCT Deflate stream.");
                deliver(buffer.data(), produced);
                check(status == Z_STREAM_END || before != inflater.z.avail_in || produced,
                    "OCT decompressor made no progress.");
            } while (status != Z_STREAM_END && (inflater.z.avail_in || inflater.z.avail_out == 0));
            check(status != Z_STREAM_END || (inflater.z.avail_in == 0 && packed == e.packed),
                "Unexpected trailing OCT compressed data.");
        }
        check(e.method == 0 || status == Z_STREAM_END, "Incomplete OCT Deflate stream.");
        check(unpacked == expected && checksum == e.crc, "OCT data is incomplete or fails its ZIP CRC checksum.");
    }
private:
    QFile file;
    QMap<QString, Entry> entries;
    quint64 dataEnd = 0;
};

struct Description
{
    ThorlabsOct::Metadata metadata;
    QString intensity;
    quint64 bytes = 0;
};

Description describe(Archive& archive)
{
    const auto header = archive.entry("Header.xml");
    check(header.unpacked > 0 && header.unpacked <= 2 * 1024 * 1024, "OCT Header.xml is empty or too large.");
    QByteArray xml;
    archive.stream("Header.xml", header.unpacked, [&](const char* p, int n) { xml.append(p, n); });
    QXmlStreamReader reader(xml);
    QStringList path;
    QMap<QString, QString> fields, attributes;
    Description result;
    bool rootSeen = false;
    while (!reader.atEnd()) {
        const auto token = reader.readNext();
        check(token != QXmlStreamReader::DTD && token != QXmlStreamReader::EntityReference,
            "DTD/entity declarations are not supported in OCT headers.");
        if (reader.isStartElement()) {
            path.append(reader.name().toString());
            if (path.size() == 1) {
                check(reader.name() == "Ocity" && reader.attributes().value("version") == "1.0",
                    "Unsupported Thorlabs OCT container version (expected Ocity 1.0).");
                rootSeen = true;
            }
            const QString key = path.join('/');
            if (key == "Ocity/DataFiles/DataFile") {
                const auto attrs = reader.attributes();
                const auto name = reader.readElementText().trimmed();
                if (normalized(name) == "data/intensity.data") {
                    check(result.intensity.isEmpty(), "Multiple OCT intensity declarations are ambiguous.");
                    result.intensity = name;
                    for (const auto& a : attrs) attributes.insert(a.name().toString(), a.value().toString());
                }
                path.removeLast();
            } else if (key == "Ocity/Image/SizeReal") {
                fields.insert(key + "/Unit", reader.attributes().value("Unit").toString());
            }
        } else if (reader.isEndElement()) {
            path.removeLast();
        } else if (reader.isCharacters() && !reader.isWhitespace()) {
            fields[path.join('/')] += reader.text().toString();
        }
    }
    check(rootSeen && !reader.hasError(), "Invalid OCT Header.xml: " + reader.errorString());
    check(!result.intensity.isEmpty(),
        "This OCT file has no reconstructed Intensity.data. In ThorImageOCT, save with 'Store Processed Data' enabled. Raw spectra and complex/PS data are not reconstructed by OCTview3R.");
    check(attributes.value("Type") == "Real" && attributes.value("BytesPerPixel") == "4",
        "Only reconstructed float32 OCT intensity data is supported.");
    const auto order = fields.value("Ocity/Image/AxisOrder").trimmed().toUpper();
    check(order.isEmpty() || order == "ZXY" || order == "ZXYT", "Unsupported OCT axis order (expected ZXY or ZXYT).");
    check(fields.value("Ocity/Image/FreeformScanPatternIsActive").trimmed().compare("true", Qt::CaseInsensitive) != 0,
        "Freeform OCT scans are not a regular volume. Export a Cartesian image volume first.");
    for (const auto& time : {attributes.value("SizeT"), fields.value("Ocity/Image/SizePixel/SizeT")})
        check(time.trimmed().isEmpty() || time.trimmed() == "1", "OCT time series are unsupported; save one volume first.");
    const QString unit = fields.value("Ocity/Image/SizeReal/Unit").trimmed();
    check(unit.isEmpty() || unit.compare("mm", Qt::CaseInsensitive) == 0, "Unsupported OCT length unit (expected mm).");
    const char* axes[] = {"X", "Y", "Z"};
    quint64 points = 1;
    for (int axis = 0; axis < 3; ++axis) {
        const QString suffix = axes[axis];
        bool ok = false;
        const QString dimension = attributes.value("Size" + suffix, axis == 2 ? "" : "1");
        const int size = dimension.toInt(&ok);
        check(ok && size >= 1, "Missing or invalid OCT dimension Size" + suffix + ".");
        const QString imageSize = fields.value("Ocity/Image/SizePixel/Size" + suffix).trimmed();
        if (!imageSize.isEmpty()) check(imageSize.toInt(&ok) == size && ok, "Conflicting OCT dimensions in header.");
        result.metadata.dimensions[axis] = size;
        points = multiply(points, size);
        const QString range = attributes.value("Range" + suffix);
        if (!range.isEmpty()) {
            const double length = range.toDouble(&ok);
            check(ok && std::isfinite(length) && length > 0, "Invalid OCT physical range for " + suffix + ".");
            // Use the range attached to THIS intensity object. Image/SizeReal
            // and PixelSpacing can describe optical depth before refractive-index
            // correction. RangeZ is already corrected; never divide it again.
            result.metadata.spacing[axis] = length / size;
            result.metadata.calibratedAxes |= 1u << axis;
        }
    }
    result.bytes = multiply(points, 4);
    check(points <= static_cast<quint64>(std::numeric_limits<vtkIdType>::max()), "OCT volume is too large for VTK.");
    check(archive.entry(result.intensity).unpacked == result.bytes, "OCT intensity byte count disagrees with its dimensions.");
    return result;
}

void checkMemory(quint64 bytes, quint64 frameBytes, quint64 budget)
{
    const quint64 renderWorkingSet = multiply(bytes, 4);
    constexpr quint64 reserve = 512ULL * 1024ULL * 1024ULL;
    check(renderWorkingSet <= std::numeric_limits<quint64>::max() - frameBytes - reserve,
        "OCT working-memory estimate overflow.");
#ifdef Q_OS_WIN
    if (!budget) {
        MEMORYSTATUSEX memory{}; memory.dwLength = sizeof(memory);
        check(GlobalMemoryStatusEx(&memory), "Cannot determine available RAM for OCT import.");
        budget = std::min<quint64>(memory.ullAvailPhys, memory.ullAvailVirtual);
    }
#endif
    const quint64 required = renderWorkingSet + frameBytes + reserve;
    check(!budget || required <= budget,
        QString("Insufficient memory for OCT import/rendering: approximately %1 GiB required, %2 GiB available. Close other datasets/applications or crop/export a smaller volume in ThorImageOCT.")
        .arg(required / 1073741824.0, 0, 'f', 2).arg(budget / 1073741824.0, 0, 'f', 2));
}
}

bool ThorlabsOct::inspect(const QString& path, Metadata& metadata, QString& error)
{
    metadata = {}; error.clear();
    try { Archive archive(path); metadata = describe(archive).metadata; return true; }
    catch (const ImportError& failure) { error = failure.message; }
    catch (const std::bad_alloc&) { error = "Insufficient memory to inspect OCT metadata."; }
    return false;
}

bool ThorlabsOct::read(const QString& path, vtkSmartPointer<vtkImageData>& output,
    Metadata& metadata, QString& error, const std::function<void(int)>& progress, quint64 memoryBudget)
{
    output = nullptr; metadata = {}; error.clear();
    try {
        if (progress) progress(0);
        Archive archive(path);
        const auto description = describe(archive);
        const auto& m = description.metadata;
        const int nx = m.dimensions[0], ny = m.dimensions[1], nz = m.dimensions[2];
        const quint64 frameBytes = multiply(multiply(nx, nz), 4);
        check(frameBytes <= std::numeric_limits<int>::max(), "A single OCT B-scan is too large.");
        checkMemory(description.bytes, frameBytes, memoryBudget);
        auto image = vtkSmartPointer<vtkImageData>::New();
        image->SetDimensions(nx, ny, nz);
        image->SetSpacing(m.spacing[0], m.spacing[1], m.spacing[2]);
        image->SetOrigin(0.0, 0.0, 0.0);
        image->AllocateScalars(VTK_FLOAT, 1);
        auto* values = static_cast<float*>(image->GetScalarPointer());
        check(values != nullptr, "Could not allocate the OCT volume.");
        QByteArray frame(static_cast<int>(frameBytes), Qt::Uninitialized);
        int filled = 0, y = 0, lastProgress = -1;
        archive.stream(description.intensity, description.bytes, [&](const char* bytes, int length) {
            while (length) {
                const int take = std::min(length, frame.size() - filled);
                std::memcpy(frame.data() + filled, bytes, take);
                filled += take; bytes += take; length -= take;
                if (filled != frame.size()) continue;
                check(y < ny, "Unexpected extra B-scan in OCT file.");
                for (int z = 0; z < nz; ++z) {
                    float* row = values + (static_cast<vtkIdType>(z) * ny + y) * nx;
                    for (int x = 0; x < nx; ++x) {
                        const quint32 bits = u32(frame.constData() + (static_cast<qint64>(x) * nz + z) * 4);
                        float value; std::memcpy(&value, &bits, 4);
                        if (!std::isfinite(value) || std::abs(value) > 1000000.0f)
                            throw ImportError{"OCT intensity contains non-finite or out-of-range dB values."};
                        row[x] = value; // Preserve signed float32 dB; no log or quantization.
                    }
                }
                filled = 0; ++y;
                const int percent = static_cast<int>(5 + 94LL * y / ny);
                if (progress && percent != lastProgress) { progress(percent); lastProgress = percent; }
            }
        });
        check(filled == 0 && y == ny, "Incomplete OCT volume.");
        image->Modified();
        output = image; metadata = m;
        if (progress) progress(100);
        return true;
    } catch (const ImportError& failure) { error = failure.message; }
    catch (const std::bad_alloc&) { error = "Insufficient memory to load the OCT volume. Close other datasets or export a smaller volume."; }
    return false;
}
