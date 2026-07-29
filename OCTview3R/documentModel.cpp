#include "documentModel.h"

#include "transformableImagePlaneWidget.h"
#include "viewerData.h"

#include <vtkActor.h>
#include <vtkBox.h>
#include <vtkClipPolyData.h>
#include <vtkColorTransferFunction.h>
#include <vtkExtractVOI.h>
#include <vtkImageData.h>
#include <vtkImageLuminance.h>
#include <vtkImageMapToColors.h>
#include <vtkImageMedian3D.h>
#include <vtkImagePlaneWidget.h>
#include <vtkImageThreshold.h>
#include <vtkImplicitPlaneWidget.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPlane.h>
#include <vtkPlaneCollection.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkTransform.h>
#include <vtkVolume.h>

#include <algorithm>

DocumentModel::~DocumentModel() = default;

ImageData& DocumentModel::create()
{
	auto document = std::make_unique<ImageData>();
	document->threshold = vtkSmartPointer<vtkImageThreshold>::New();
	document->colorFun = vtkSmartPointer<vtkColorTransferFunction>::New();
	document->opacityFun = vtkSmartPointer<vtkPiecewiseFunction>::New();
	document->planeWidget =
		vtkSmartPointer<TransformableImagePlaneWidget>::New();
	document->median = vtkSmartPointer<vtkImageMedian3D>::New();
	document->extractVOI = vtkSmartPointer<vtkExtractVOI>::New();
	document->volume = vtkSmartPointer<vtkVolume>::New();
	document->actor = vtkSmartPointer<vtkActor>::New();
	document->transform = vtkSmartPointer<vtkTransform>::New();
	document->luminance = vtkSmartPointer<vtkImageLuminance>::New();
	document->colorMap = vtkSmartPointer<vtkImageMapToColors>::New();
	document->volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
	document->polyMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	document->polyNormals = vtkSmartPointer<vtkPolyDataNormals>::New();
	document->polyActor = vtkSmartPointer<vtkActor>::New();
	document->polyClipBox = vtkSmartPointer<vtkBox>::New();
	document->polyClipper = vtkSmartPointer<vtkClipPolyData>::New();
	document->clipPlane = vtkSmartPointer<vtkPlane>::New();
	document->implicitPlane = vtkSmartPointer<vtkImplicitPlaneWidget>::New();
	document->image = vtkSmartPointer<vtkImageData>::New();
	document->planeCollection = vtkSmartPointer<vtkPlaneCollection>::New();
	document->fileChanged = true;

	ImageData& result = *document;
	m_documents.emplace_back(std::move(document));
	m_activeIndex = static_cast<int>(m_documents.size()) - 1;
	return result;
}

DocumentModel::Document DocumentModel::takeAt(int index)
{
	if (index < 0 || index >= size())
		return {};

	Document document = std::move(m_documents[static_cast<std::size_t>(index)]);
	m_documents.erase(m_documents.begin() + index);
	if (m_documents.empty())
		m_activeIndex = -1;
	else
		m_activeIndex = std::min(index, size() - 1);
	return document;
}

void DocumentModel::clear()
{
	m_documents.clear();
	m_activeIndex = -1;
}

ImageData* DocumentModel::at(int index) const
{
	if (index < 0 || index >= size())
		return nullptr;
	return m_documents[static_cast<std::size_t>(index)].get();
}

ImageData* DocumentModel::active() const
{
	return at(m_activeIndex);
}

int DocumentModel::activeIndex() const
{
	return m_activeIndex;
}

void DocumentModel::setActiveIndex(int index)
{
	m_activeIndex = index >= 0 && index < size() ? index : -1;
}

int DocumentModel::indexOf(const ImageData* document) const
{
	for (std::size_t i = 0; i < m_documents.size(); ++i)
	{
		if (m_documents[i].get() == document)
			return static_cast<int>(i);
	}
	return -1;
}

int DocumentModel::size() const
{
	return static_cast<int>(m_documents.size());
}

bool DocumentModel::empty() const
{
	return m_documents.empty();
}

const DocumentModel::Documents& DocumentModel::documents() const
{
	return m_documents;
}
