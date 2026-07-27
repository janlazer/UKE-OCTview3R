#pragma once

class vtkImageData;
class vtkImageReader2;
class vtkImageReader;
class vtkTIFFReader;
class vtkJPEGReader;
class vtkImageData;
class vtkBYUReader;
class vtkOBJReader;
class vtkPLYReader;
class vtkPolyDataReader;
class vtkPolyData;
class vtkSTLReader;
class vtkXMLPolyDataReader;
class vtkStructuredPointsReader;
class vtkStructuredGridReader;
class vtkStructuredGridGeometryFilter;
class vtkUnstructuredGrid;
class vtkGeometryFilter;
class vtkSimplePointsReader;
class vtkImageDataGeometryFilter;
class vtkDelaunay3D;
class vtkCleanPolyData;
class vtkDataSetMapper;

#include <QThread>

#include "opendata.h"
#include "openpoly.h"

class loading : public QThread
{
	Q_OBJECT
public:
	loading(OpenData* od);
	loading(OpenPoly* op);
	~loading();
public slots:
	void loadData();
	void loadPoly();
	void setData(OpenData *od);
	void setPoly(OpenPoly *op);
signals:
	void updateProgress(int progr);
	void loaded(vtkImageReader2 *data);
	void loaded(vtkPolyData *poly);
private:
	vtkImageReader2 *m_data;
	vtkPolyData *m_poly;
	QString m_lastPath;
	QString m_fileName;
	bitsizeType m_bitsize;
	endianType m_endian;
	dataType m_dataFormat;
	polyType m_polyFormat;
	int m_width; 
	int m_height;
	int m_depth;
	double m_VOI[6];
};
