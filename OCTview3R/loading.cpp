#include "loading.h"

#include "opendata.h"
#include "openpoly.h"

#include <QFileInfo>
#include <QRegularExpression>

#include <vtkBYUReader.h>
#include <vtkCleanPolyData.h>
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

namespace
{
bool hasUsableImage(vtkImageData* image)
{
	return image != nullptr && image->GetNumberOfPoints() > 0;
}

bool hasUsablePolyData(vtkPolyData* poly)
{
	return poly != nullptr && poly->GetNumberOfPoints() > 0;
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
	emit updateProgress(0);
	emit failed(message);
	emit finished();
}

void loading::loadData()
{
	emit updateProgress(-1);

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
		reader->SetDataOrigin(-0.5 * m_width, -0.5 * m_height, -0.5 * m_depth);

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

		reader->Update();
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
		reader->Update();
		output = reader->GetOutput();
		if (hasUsableImage(output))
		{
			int dimensions[3] = { 0, 0, 0 };
			output->GetDimensions(dimensions);
			output->SetOrigin(
				-0.5 * dimensions[0],
				-0.5 * dimensions[1],
				-0.5 * dimensions[2]);
			m_data = vtkSmartPointer<vtkImageData>::New();
			m_data->ShallowCopy(output);
		}
		break;
	}
	case dataType::DATA_VTK:
	{
		auto reader = vtkSmartPointer<vtkStructuredPointsReader>::New();
		reader->SetFileName(m_fileName.toLocal8Bit().constData());
		reader->Update();
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

		reader->Update();
		output = reader->GetOutput();
		if (hasUsableImage(output))
		{
			int dimensions[3] = { 0, 0, 0 };
			output->GetDimensions(dimensions);
			output->SetOrigin(
				-0.5 * dimensions[0],
				-0.5 * dimensions[1],
				-0.5 * dimensions[2]);
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

	emit updateProgress(100);
	// Transfer one reference across the queued Qt connection. The GUI slot
	// adopts it into a vtkSmartPointer and releases this transfer reference.
	m_data->Register(nullptr);
	emit dataLoaded(m_data);
	emit finished();
}

void loading::loadPoly()
{
	emit updateProgress(-1);

	vtkPolyData* output = nullptr;

	switch (m_polyFormat)
	{
	case polyType::POLY_PLY:
	{
		auto reader = vtkSmartPointer<vtkPLYReader>::New();
		reader->SetFileName(m_fileName.toLocal8Bit().constData());
		reader->Update();
		output = reader->GetOutput();
		break;
	}
	case polyType::POLY_VTP:
	{
		auto reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
		reader->SetFileName(m_fileName.toLocal8Bit().constData());
		reader->Update();
		output = reader->GetOutput();
		break;
	}
	case polyType::POLY_OBJ:
	{
		auto reader = vtkSmartPointer<vtkOBJReader>::New();
		reader->SetFileName(m_fileName.toLocal8Bit().constData());
		reader->Update();
		output = reader->GetOutput();
		break;
	}
	case polyType::POLY_STL:
	{
		auto reader = vtkSmartPointer<vtkSTLReader>::New();
		reader->SetFileName(m_fileName.toLocal8Bit().constData());
		reader->Update();
		output = reader->GetOutput();
		break;
	}
	case polyType::POLY_VTK:
	{
		auto reader = vtkSmartPointer<vtkPolyDataReader>::New();
		reader->SetFileName(m_fileName.toLocal8Bit().constData());
		reader->Update();
		output = reader->GetOutput();
		break;
	}
	case polyType::POLY_G:
	{
		auto reader = vtkSmartPointer<vtkBYUReader>::New();
		reader->SetGeometryFileName(m_fileName.toLocal8Bit().constData());
		reader->Update();
		output = reader->GetOutput();
		break;
	}
	case polyType::POLY_VTR:
	{
		auto reader = vtkSmartPointer<vtkXMLRectilinearGridReader>::New();
		auto geometry = vtkSmartPointer<vtkGeometryFilter>::New();
		reader->SetFileName(m_fileName.toLocal8Bit().constData());
		geometry->SetInputConnection(reader->GetOutputPort());
		geometry->Update();
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
		geometry->Update();
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

	emit updateProgress(100);
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
