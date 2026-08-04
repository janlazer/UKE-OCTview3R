#pragma once

#include <QObject>
#include <QMetaType>
#include <QString>

#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

#include "dataTypes.h"

class OpenData;
class OpenPoly;
class vtkAlgorithm;
class vtkObject;

Q_DECLARE_METATYPE(vtkImageData*)
Q_DECLARE_METATYPE(vtkPolyData*)

class loading final : public QObject
{
	Q_OBJECT

public:
	explicit loading(OpenData* openData);
	explicit loading(OpenPoly* openPoly);
	~loading() override = default;

public slots:
	void loadData();
	void loadPoly();

signals:
	void updateProgress(int progress);
	void dataLoaded(vtkImageData* data, unsigned int spacingInMillimetresMask);
	void polyLoaded(vtkPolyData* poly);
	void failed(const QString& message);
	void finished();

private:
	void setData(OpenData* openData);
	void setPoly(OpenPoly* openPoly);
	void reportFailure(const QString& message);
	void reportProgress(int progress);
	void updateWithProgress(vtkAlgorithm* algorithm, int startProgress, int endProgress);
	static void vtkProgressCallback(
		vtkObject* caller,
		unsigned long eventId,
		void* clientData,
		void* callData);

	vtkSmartPointer<vtkImageData> m_data;
	vtkSmartPointer<vtkPolyData> m_poly;
	QString m_fileName;
	bitsizeType m_bitsize = bitsizeType::BIT_UNDEF;
	endianType m_endian = endianType::ENDIAN_UNDEF;
	dataType m_dataFormat = dataType::DATA_UNDEF;
	polyType m_polyFormat = polyType::POLY_UNDEF;
	int m_width = 0;
	int m_height = 0;
	int m_depth = 0;
	int m_lastProgress = -1;
	unsigned int m_spacingInMillimetresMask = 0;
};
