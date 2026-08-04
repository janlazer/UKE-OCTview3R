#include "loading.h"

#include "opendata.h"
#include "openpoly.h"

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QString>
#include <QtGlobal>

#include <vtkAlgorithm.h>
#include <vtkBYUReader.h>
#include <vtkCallbackCommand.h>
#include <vtkCleanPolyData.h>
#include <vtkCommand.h>
#include <vtkDelaunay3D.h>
#include <vtkGeometryFilter.h>
#include <vtkImageReader.h>
#include <vtkJPEGReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyDataReader.h>
#include <vtkSimplePointsReader.h>
#include <vtkSTLReader.h>
#include <vtkStructuredPoints.h>
#include <vtkStructuredPointsReader.h>
#include <vtkTIFFReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLRectilinearGridReader.h>

#include <algorithm>
#include <cmath>

namespace
{
	struct ProgressContext
	{
		loading* worker;
		int startProgress;
		int endProgress;
	};

	bool hasUsableImage(vtkImageData* image)
	{
		return image != nullptr && image->GetNumberOfPoints() > 0;
	}

	bool hasUsablePolyData(vtkPolyData* poly)
	{
		return poly != nullptr && poly->GetNumberOfPoints() > 0;
	}

	QString tiffMetadataValue(const QString& description, const QString& key)
	{
		const QRegularExpression expression(
			QStringLiteral("(?:^|[\\r\\n])\\s*%1\\s*=\\s*([^\\r\\n]+)")
				.arg(QRegularExpression::escape(key)),
			QRegularExpression::CaseInsensitiveOption);
		const QRegularExpressionMatch match = expression.match(description);
		return match.hasMatch() ? match.captured(1).trimmed() : QString();
	}

	double millimetresPerUnit(QString unit)
	{
		unit = unit.trimmed().toLower();
		// Some ImageJ writers store the Unicode escape literally instead of
		// writing the micro sign as a UTF-8 character.
		unit.replace(QStringLiteral("\\u00b5"), QStringLiteral("u"));
		unit.replace(QChar(0x00b5), QChar('u'));
		unit.replace(QChar(0x03bc), QChar('u'));
		unit.remove(QRegularExpression(QStringLiteral("\\s+")));

		if (unit == QStringLiteral("mm") ||
			unit.startsWith(QStringLiteral("millimet")))
			return 1.0;
		if (unit == QStringLiteral("cm") ||
			unit.startsWith(QStringLiteral("centimet")))
			return 10.0;
		if (unit == QStringLiteral("m") ||
			unit == QStringLiteral("meter") ||
			unit == QStringLiteral("metre"))
			return 1000.0;
		if (unit == QStringLiteral("um") ||
			unit.startsWith(QStringLiteral("micron")) ||
			unit.startsWith(QStringLiteral("micromet")))
			return 0.001;
		if (unit == QStringLiteral("nm") ||
			unit.startsWith(QStringLiteral("nanomet")))
			return 0.000001;
		return 0.0;
	}

	bool metadataNumber(
		const QString& description,
		const QString& key,
		double& value)
	{
		QString text = tiffMetadataValue(description, key);
		text.replace(QChar(','), QChar('.'));
		bool ok = false;
		const double parsed = text.toDouble(&ok);
		if (!ok || !std::isfinite(parsed))
			return false;
		value = parsed;
		return true;
	}

	enum class TiffByteOrder
	{
		LittleEndian,
		BigEndian
	};

	quint16 tiffUInt16(const char* bytes, TiffByteOrder byteOrder)
	{
		const auto* data = reinterpret_cast<const unsigned char*>(bytes);
		return byteOrder == TiffByteOrder::LittleEndian
			? static_cast<quint16>(data[0] | (data[1] << 8))
			: static_cast<quint16>((data[0] << 8) | data[1]);
	}

	quint32 tiffUInt32(const char* bytes, TiffByteOrder byteOrder)
	{
		const auto* data = reinterpret_cast<const unsigned char*>(bytes);
		return byteOrder == TiffByteOrder::LittleEndian
			? static_cast<quint32>(
				static_cast<quint32>(data[0]) |
				(static_cast<quint32>(data[1]) << 8) |
				(static_cast<quint32>(data[2]) << 16) |
				(static_cast<quint32>(data[3]) << 24))
			: static_cast<quint32>(
				(static_cast<quint32>(data[0]) << 24) |
				(static_cast<quint32>(data[1]) << 16) |
				(static_cast<quint32>(data[2]) << 8) |
				static_cast<quint32>(data[3]));
	}

	bool readTiffBytes(
		QFile& file,
		quint32 offset,
		int byteCount,
		QByteArray& bytes)
	{
		if (byteCount < 0 ||
			static_cast<quint64>(offset) + static_cast<quint64>(byteCount) >
				static_cast<quint64>(file.size()) ||
			!file.seek(static_cast<qint64>(offset)))
			return false;
		bytes = file.read(byteCount);
		return bytes.size() == byteCount;
	}

	int tiffTypeSize(quint16 type)
	{
		switch (type)
		{
		case 1:  // BYTE
		case 2:  // ASCII
		case 6:  // SBYTE
		case 7:  // UNDEFINED
			return 1;
		case 3:  // SHORT
		case 8:  // SSHORT
			return 2;
		case 4:  // LONG
		case 9:  // SLONG
		case 11: // FLOAT
			return 4;
		case 5:  // RATIONAL
		case 10: // SRATIONAL
		case 12: // DOUBLE
			return 8;
		default:
			return 0;
		}
	}

	struct TiffCalibrationTags
	{
		double xResolution = 0.0;
		double yResolution = 0.0;
		quint16 resolutionUnit = 1;
		QString description;
		bool hasXResolution = false;
		bool hasYResolution = false;
	};

	bool readTiffCalibrationTags(
		const QString& fileName,
		TiffCalibrationTags& tags)
	{
		QFile file(fileName);
		if (!file.open(QIODevice::ReadOnly))
			return false;

		QByteArray header;
		if (!readTiffBytes(file, 0, 8, header))
			return false;
		TiffByteOrder byteOrder;
		if (header.startsWith("II"))
			byteOrder = TiffByteOrder::LittleEndian;
		else if (header.startsWith("MM"))
			byteOrder = TiffByteOrder::BigEndian;
		else
			return false;

		// The VTK 8.2 reader used by OCTview3R handles classic TIFF. BigTIFF
		// uses a different IFD layout and is therefore left to the reader's
		// default spacing instead of risking a false calibration.
		if (tiffUInt16(header.constData() + 2, byteOrder) != 42)
			return false;
		const quint32 ifdOffset = tiffUInt32(header.constData() + 4, byteOrder);

		QByteArray countBytes;
		if (!readTiffBytes(file, ifdOffset, 2, countBytes))
			return false;
		const quint16 entryCount = tiffUInt16(countBytes.constData(), byteOrder);
		if (entryCount > 4096)
			return false;

		QByteArray entries;
		if (!readTiffBytes(
				file,
				ifdOffset + 2,
				static_cast<int>(entryCount) * 12,
				entries))
			return false;

		for (quint16 index = 0; index < entryCount; ++index)
		{
			const char* entry = entries.constData() + index * 12;
			const quint16 tag = tiffUInt16(entry, byteOrder);
			if (tag != 270 && tag != 282 && tag != 283 && tag != 296)
				continue;

			const quint16 type = tiffUInt16(entry + 2, byteOrder);
			const quint32 valueCount = tiffUInt32(entry + 4, byteOrder);
			const int typeSize = tiffTypeSize(type);
			const quint64 totalSize =
				static_cast<quint64>(valueCount) * static_cast<quint64>(typeSize);
			if (typeSize == 0 || totalSize == 0 || totalSize > 1024 * 1024)
				continue;

			QByteArray payload;
			if (totalSize <= 4)
				payload = QByteArray(entry + 8, static_cast<int>(totalSize));
			else if (!readTiffBytes(
					file,
					tiffUInt32(entry + 8, byteOrder),
					static_cast<int>(totalSize),
					payload))
				continue;

			if ((tag == 282 || tag == 283) && type == 5 && payload.size() >= 8)
			{
				const quint32 numerator = tiffUInt32(payload.constData(), byteOrder);
				const quint32 denominator =
					tiffUInt32(payload.constData() + 4, byteOrder);
				if (denominator == 0)
					continue;
				const double resolution =
					static_cast<double>(numerator) / denominator;
				if (tag == 282)
				{
					tags.xResolution = resolution;
					tags.hasXResolution = true;
				}
				else
				{
					tags.yResolution = resolution;
					tags.hasYResolution = true;
				}
			}
			else if (tag == 296 && type == 3 && payload.size() >= 2)
			{
				tags.resolutionUnit = tiffUInt16(payload.constData(), byteOrder);
			}
			else if (tag == 270 && type == 2)
			{
				const int terminator = payload.indexOf('\0');
				const int length = terminator >= 0 ? terminator : payload.size();
				tags.description = QString::fromUtf8(payload.constData(), length);
			}
		}
		return true;
	}

	void applyTiffCalibrationInMillimetres(
		const QString& fileName,
		vtkImageData* image)
	{
		if (image == nullptr)
			return;

		TiffCalibrationTags tags;
		if (!readTiffCalibrationTags(fileName, tags))
			return;

		const double descriptionUnitToMm = millimetresPerUnit(
			tiffMetadataValue(tags.description, QStringLiteral("unit")));
		double resolutionUnitToMm = 0.0;
		if (tags.resolutionUnit == 2) // TIFF RESUNIT_INCH
			resolutionUnitToMm = 25.4;
		else if (tags.resolutionUnit == 3) // TIFF RESUNIT_CENTIMETER
			resolutionUnitToMm = 10.0;
		else if (tags.resolutionUnit == 1) // TIFF RESUNIT_NONE
			resolutionUnitToMm = descriptionUnitToMm;

		double spacing[3] = { 1.0, 1.0, 1.0 };
		image->GetSpacing(spacing);
		bool hasPhysicalCalibration = false;
		if (tags.hasXResolution &&
			tags.xResolution > 0.0 &&
			resolutionUnitToMm > 0.0)
		{
			spacing[0] = resolutionUnitToMm / tags.xResolution;
			hasPhysicalCalibration = true;
		}
		if (tags.hasYResolution &&
			tags.yResolution > 0.0 &&
			resolutionUnitToMm > 0.0)
		{
			spacing[1] = resolutionUnitToMm / tags.yResolution;
			hasPhysicalCalibration = true;
		}

		double sliceSpacing = 0.0;
		if (descriptionUnitToMm > 0.0 &&
			metadataNumber(
				tags.description,
				QStringLiteral("spacing"),
				sliceSpacing) &&
			sliceSpacing > 0.0)
		{
			spacing[2] = descriptionUnitToMm * sliceSpacing;
			hasPhysicalCalibration = true;
		}

		if (hasPhysicalCalibration)
			image->SetSpacing(spacing);
	}
}

loading::loading(OpenData* openData)
{
	setData(openData);
}

loading::loading(OpenPoly* openPoly)
{
	setPoly(openPoly);
}

void loading::reportFailure(const QString& message)
{
	reportProgress(0);
	emit failed(message);
	emit finished();
}

void loading::reportProgress(int progress)
{
	const int boundedProgress = qBound(0, progress, 100);
	if (boundedProgress == m_lastProgress)
		return;

	m_lastProgress = boundedProgress;
	emit updateProgress(boundedProgress);
}

void loading::updateWithProgress(
	vtkAlgorithm* algorithm,
	int startProgress,
	int endProgress)
{
	if (algorithm == nullptr)
		return;

	ProgressContext context{
		this,
		qBound(0, startProgress, 100),
		qBound(0, endProgress, 100)
	};
	if (context.endProgress < context.startProgress)
		std::swap(context.startProgress, context.endProgress);

	auto callback = vtkSmartPointer<vtkCallbackCommand>::New();
	callback->SetClientData(&context);
	callback->SetCallback(&loading::vtkProgressCallback);
	const unsigned long observerTag =
		algorithm->AddObserver(vtkCommand::ProgressEvent, callback);

	reportProgress(context.startProgress);
	algorithm->Update();
	reportProgress(context.endProgress);
	algorithm->RemoveObserver(observerTag);
}

void loading::vtkProgressCallback(
	vtkObject* caller,
	unsigned long,
	void* clientData,
	void*)
{
	auto* context = static_cast<ProgressContext*>(clientData);
	auto* algorithm = vtkAlgorithm::SafeDownCast(caller);
	if (context == nullptr || context->worker == nullptr || algorithm == nullptr)
		return;

	const double vtkProgress = qBound(0.0, algorithm->GetProgress(), 1.0);
	const int progress = context->startProgress +
		qRound(vtkProgress * (context->endProgress - context->startProgress));
	context->worker->reportProgress(progress);
}

void loading::loadData()
{
	m_lastProgress = -1;
	reportProgress(0);

	vtkImageData* output = nullptr;

	switch (m_dataFormat)
	{
	case dataType::DATA_RAW:
	{
		if (m_width <= 0 || m_height <= 0 || m_depth <= 0)
		{
			reportFailure(tr("RAW dimensions must be greater than zero."));
			return;
		}

		auto reader = vtkSmartPointer<vtkImageReader>::New();
		reader->SetFileName(m_fileName.toLocal8Bit().constData());
		reader->SetDataExtent(0, m_width - 1, 0, m_height - 1, 0, m_depth - 1);
		reader->SetFileDimensionality(m_depth > 1 ? 3 : 2);
		reader->SetDataOrigin(0.0, 0.0, 0.0);

		if (m_bitsize == bitsizeType::BIT16)
			reader->SetDataScalarTypeToUnsignedShort();
		else if (m_bitsize == bitsizeType::BIT8)
			reader->SetDataScalarTypeToUnsignedChar();
		else
		{
			reportFailure(tr("Unsupported RAW scalar type."));
			return;
		}

		if (m_endian == endianType::BIG)
			reader->SetDataByteOrderToBigEndian();
		else
			reader->SetDataByteOrderToLittleEndian();

		updateWithProgress(reader, 5, 99);
		output = reader->GetOutput();
		if (hasUsableImage(output))
		{
			m_data = vtkSmartPointer<vtkImageData>::New();
			m_data->ShallowCopy(output);
		}
		break;
	}
	case dataType::DATA_TIFF:
	{
		auto reader = vtkSmartPointer<vtkTIFFReader>::New();
		reader->SetFileName(m_fileName.toLocal8Bit().constData());
		updateWithProgress(reader, 5, 99);
		output = reader->GetOutput();
		if (hasUsableImage(output))
		{
			applyTiffCalibrationInMillimetres(m_fileName, output);
			output->SetOrigin(0.0, 0.0, 0.0);
			m_data = vtkSmartPointer<vtkImageData>::New();
			m_data->ShallowCopy(output);
		}
		break;
	}
	case dataType::DATA_VTK:
	{
		auto reader = vtkSmartPointer<vtkStructuredPointsReader>::New();
		reader->SetFileName(m_fileName.toLocal8Bit().constData());
		updateWithProgress(reader, 5, 99);
		output = reader->GetOutput();
		if (hasUsableImage(output))
		{
			m_data = vtkSmartPointer<vtkImageData>::New();
			m_data->ShallowCopy(output);
		}
		break;
	}
	case dataType::DATA_JPEG:
	{
		const QFileInfo fileInfo(m_fileName);
		const QString baseName = fileInfo.completeBaseName();
		const QString suffix = fileInfo.suffix();
		const QRegularExpressionMatch numberMatch =
			QRegularExpression(QStringLiteral("(\\d+)$")).match(baseName);
		const int depth = qMax(1, m_depth);

		auto reader = vtkSmartPointer<vtkJPEGReader>::New();
		if (depth == 1)
		{
			reader->SetFileName(m_fileName.toLocal8Bit().constData());
		}
		else
		{
			if (!numberMatch.hasMatch())
			{
				reportFailure(tr("A JPEG stack filename must end in a slice number."));
				return;
			}

			const QString digitsText = numberMatch.captured(1);
			const QString namePrefix =
				baseName.left(baseName.length() - digitsText.length());
			const QString directoryPrefix =
				QFileInfo(m_fileName).absolutePath() + QStringLiteral("/");
			const QString pattern =
				QStringLiteral("%s") + namePrefix +
				QStringLiteral("%0") + QString::number(digitsText.length()) +
				QStringLiteral("d.") + suffix;

			reader->SetFilePrefix(directoryPrefix.toLocal8Bit().constData());
			reader->SetFilePattern(pattern.toLocal8Bit().constData());
			reader->SetFileNameSliceOffset(digitsText.toInt());
			reader->SetFileDimensionality(3);
			reader->SetDataExtent(
				0, qMax(0, m_width - 1),
				0, qMax(0, m_height - 1),
				0, depth - 1);
		}

		updateWithProgress(reader, 5, 99);
		output = reader->GetOutput();
		if (hasUsableImage(output))
		{
			output->SetOrigin(0.0, 0.0, 0.0);
			m_data = vtkSmartPointer<vtkImageData>::New();
			m_data->ShallowCopy(output);
		}
		break;
	}
	default:
		reportFailure(tr("Unsupported volume-data format."));
		return;
	}

	if (!hasUsableImage(m_data))
	{
		reportFailure(tr("The selected volume data could not be read: %1").arg(m_fileName));
		return;
	}

	reportProgress(100);
	// Transfer one reference across the queued Qt connection. The GUI slot
	// adopts it into a vtkSmartPointer and releases this transfer reference.
	m_data->Register(nullptr);
	emit dataLoaded(m_data);
	emit finished();
}

void loading::loadPoly()
{
	m_lastProgress = -1;
	reportProgress(0);

	// Keep the reader output alive after leaving the individual switch case.
	// Each reader is local to its case and would otherwise release its output
	// before the validation and copy below.
	vtkSmartPointer<vtkPolyData> output;

	switch (m_polyFormat)
	{
	case polyType::POLY_PLY:
	{
		auto reader = vtkSmartPointer<vtkPLYReader>::New();
		reader->SetFileName(m_fileName.toLocal8Bit().constData());
		updateWithProgress(reader, 5, 99);
		output = reader->GetOutput();
		break;
	}
	case polyType::POLY_VTP:
	{
		auto reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
		reader->SetFileName(m_fileName.toLocal8Bit().constData());
		updateWithProgress(reader, 5, 99);
		output = reader->GetOutput();
		break;
	}
	case polyType::POLY_OBJ:
	{
		auto reader = vtkSmartPointer<vtkOBJReader>::New();
		reader->SetFileName(m_fileName.toLocal8Bit().constData());
		updateWithProgress(reader, 5, 99);
		output = reader->GetOutput();
		break;
	}
	case polyType::POLY_STL:
	{
		auto reader = vtkSmartPointer<vtkSTLReader>::New();
		reader->SetFileName(m_fileName.toLocal8Bit().constData());
		updateWithProgress(reader, 5, 99);
		output = reader->GetOutput();
		break;
	}
	case polyType::POLY_VTK:
	{
		auto reader = vtkSmartPointer<vtkPolyDataReader>::New();
		reader->SetFileName(m_fileName.toLocal8Bit().constData());
		updateWithProgress(reader, 5, 99);
		output = reader->GetOutput();
		break;
	}
	case polyType::POLY_G:
	{
		auto reader = vtkSmartPointer<vtkBYUReader>::New();
		reader->SetGeometryFileName(m_fileName.toLocal8Bit().constData());
		updateWithProgress(reader, 5, 99);
		output = reader->GetOutput();
		break;
	}
	case polyType::POLY_VTR:
	{
		auto reader = vtkSmartPointer<vtkXMLRectilinearGridReader>::New();
		auto geometry = vtkSmartPointer<vtkGeometryFilter>::New();
		reader->SetFileName(m_fileName.toLocal8Bit().constData());
		geometry->SetInputConnection(reader->GetOutputPort());
		updateWithProgress(reader, 5, 75);
		updateWithProgress(geometry, 75, 99);
		output = geometry->GetOutput();
		break;
	}
	case polyType::POLY_XYZ:
	{
		auto reader = vtkSmartPointer<vtkSimplePointsReader>::New();
		auto clean = vtkSmartPointer<vtkCleanPolyData>::New();
		auto delaunay = vtkSmartPointer<vtkDelaunay3D>::New();
		auto geometry = vtkSmartPointer<vtkGeometryFilter>::New();
		reader->SetFileName(m_fileName.toLocal8Bit().constData());
		clean->SetInputConnection(reader->GetOutputPort());
		clean->SetTolerance(0.005);
		delaunay->SetInputConnection(clean->GetOutputPort());
		geometry->SetInputConnection(delaunay->GetOutputPort());
		updateWithProgress(reader, 5, 20);
		updateWithProgress(clean, 20, 35);
		updateWithProgress(delaunay, 35, 90);
		updateWithProgress(geometry, 90, 99);
		output = geometry->GetOutput();
		break;
	}
	default:
		reportFailure(tr("Unsupported polygonal-data format."));
		return;
	}

	if (!hasUsablePolyData(output))
	{
		reportFailure(tr("The selected polygonal data could not be read: %1").arg(m_fileName));
		return;
	}

	m_poly = vtkSmartPointer<vtkPolyData>::New();
	m_poly->ShallowCopy(output);

	reportProgress(100);
	m_poly->Register(nullptr);
	emit polyLoaded(m_poly);
	emit finished();
}

void loading::setData(OpenData* openData)
{
	m_fileName = openData->getFileName();
	m_bitsize = openData->getBitsize();
	m_endian = openData->getEndian();
	m_dataFormat = openData->getDataFormat();
	m_width = openData->getWidth();
	m_height = openData->getHeight();
	m_depth = openData->getDepth();
}

void loading::setPoly(OpenPoly* openPoly)
{
	m_fileName = openPoly->getFileName();
	m_polyFormat = openPoly->getPolyFormat();
}
