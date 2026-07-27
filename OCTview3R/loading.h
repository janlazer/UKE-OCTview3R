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
	void dataLoaded(vtkImageData* data);
	void polyLoaded(vtkPolyData* poly);
	void failed(const QString& message);
	void finished();

private:
	void setData(OpenData* openData);
	void setPoly(OpenPoly* openPoly);
	void reportFailure(const QString& message);

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
};
