#include "Loading.h"

#include <vtkImageData.h>
#include <vtkImageReader2.h>
#include <vtkImageReader.h>
#include <vtkTIFFReader.h>
#include <vtkJPEGReader.h>
#include <vtkStructuredPointsReader.h>
#include <vtkStructuredGridReader.h>
#include <vtkStructuredGridGeometryFilter.h>
#include <vtkBYUReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkSTLReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyData.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkGeometryFilter.h>
#include <vtkSimplePointsReader.h>
#include <vtkDelaunay3D.h>
#include <vtkCleanPolyData.h>
#include <vtkDataSetMapper.h>
#include <vtkUnstructuredGrid.h>

loading::loading(OpenData* od)
{
	setData(od);
}

loading::loading(OpenPoly* op)
{
	setPoly(op);
}

loading::~loading(void)
{

}

void loading::loadData()
{
	emit updateProgress(-1);

	vtkImageReader2 *data = vtkImageReader2::New();
	vtkImageReader *raw = vtkImageReader::New();
	vtkTIFFReader *tiff = vtkTIFFReader::New();
	vtkJPEGReader *jpeg = vtkJPEGReader::New();
	vtkStructuredPointsReader *vtk = vtkStructuredPointsReader::New();
	switch(m_dataFormat){
		case DATA_UNDEF:
			data = nullptr;
			break;
		case DATA_RAW:
			if(m_depth > 1){
				raw->SetFileName(m_fileName.toLocal8Bit());
				raw->SetDataExtent(0,m_width-1,0,m_height-1,0,m_depth-1);
				raw->SetFileDimensionality(3);
				raw->SetDataOrigin(-0.5*m_width,-0.5*m_height,-0.5*m_depth);
				//CHECK BITSIZE
				switch(m_bitsize){
					case bitsizeType::BIT16: 
						raw->SetDataScalarTypeToUnsignedShort();
						break;
					case bitsizeType::BIT8:
						raw->SetDataScalarTypeToUnsignedChar();
						break;
					default:
						break;
				}
				//CHECK ENDIANNESS
				switch(m_endian){
					case endianType::BIG:
						raw->SetDataByteOrderToBigEndian();
						break;
					case endianType::LITTLE:
						raw->SetDataByteOrderToLittleEndian();
						break;
				}
				raw->UpdateInformation();
				raw->Update();
				data = (vtkImageReader2*)raw;
				break;
			}
			data = nullptr;
			break;
		case DATA_TIFF:
			tiff->SetFileName(m_fileName.toStdString().c_str());
			tiff->UpdateInformation();
			int dataExt[6];
			tiff->GetDataExtent(dataExt);
			m_width = dataExt[1] + 1;
			m_height = dataExt[3] + 1;
			m_depth = dataExt[5] + 1;
			if(m_depth > 1){
				tiff->SetDataOrigin(-0.5*m_width,-0.5*m_height,-0.5*m_depth);
				//CHECK BITSIZE
				switch(tiff->GetDataScalarType()){
					case VTK_UNSIGNED_CHAR:
						m_bitsize = bitsizeType::BIT8;
						tiff->SetDataScalarTypeToUnsignedChar();
						break;
					case VTK_UNSIGNED_SHORT:
						m_bitsize = bitsizeType::BIT16;
						tiff->SetDataScalarTypeToUnsignedShort();
						break;
				}
				//CHECK ENDIANNESS
				QString switchString = QString::fromUtf8(tiff->GetDataByteOrderAsString());
				if(switchString=="BigEndian"){
					m_endian = endianType::BIG;
					tiff->SetDataByteOrderToBigEndian();
				}else if(switchString=="LittleEndian"){
					m_endian= endianType::LITTLE;
					tiff->SetDataByteOrderToLittleEndian();
				}
				tiff->UpdateInformation();
				tiff->UpdateWholeExtent();
 				tiff->Update();
				data = (vtkImageReader2*)tiff;
				break;
			}
			data = nullptr;
			break;
		case DATA_VTK:
			vtk->SetFileName(m_fileName.toStdString().c_str());
			vtk->UpdateInformation();
			int vtkDataExt[6];
			vtk->GetUpdateExtent(vtkDataExt);
			m_width = vtkDataExt[1] + 1;
			m_height = vtkDataExt[3] + 1;
			m_depth = vtkDataExt[5] + 1;
			if(m_depth > 1){
				vtk->UpdateInformation();
				data = (vtkImageReader2*)vtk;
				break;
			}
			data = nullptr;
			break;
		case DATA_JPEG:
			QFileInfo info = m_fileName.toLocal8Bit();
			QString path = info.path();
			QString baseName = info.baseName();
			int length = baseName.length();
			QString tmp = "";
			int digits = 0;
			if(baseName.at(length-1).isDigit()){
				digits++;
				tmp += baseName.at(length-1);
				for(int i = length - 2; i > 0; i--){
					if(baseName.at(i).isDigit()){
						digits++;
						tmp += baseName.at(i);
					}
					else break;
				}
			}
			// reversing tmp!
			QByteArray ba = tmp.toLocal8Bit();
			char *d = ba.data();
			std::reverse(d, d + tmp.length());
			tmp = QString(d);
			QString namePrefix = baseName.mid(0, length - digits);
			int nameNumber = baseName.mid(length - digits, length).toInt();
			jpeg->SetFilePrefix((path+"/").toLocal8Bit());
			jpeg->SetFileDimensionality(3);
			switch(digits){
				case 1:	jpeg->SetFilePattern(("%s"+namePrefix+"%01d.jpg").toLocal8Bit()); break;
				case 2:	jpeg->SetFilePattern(("%s"+namePrefix+"%02d.jpg").toLocal8Bit()); break;
				case 3: jpeg->SetFilePattern(("%s"+namePrefix+"%03d.jpg").toLocal8Bit()); break;
				case 4: jpeg->SetFilePattern(("%s"+namePrefix+"%04d.jpg").toLocal8Bit()); break;
				case 5: jpeg->SetFilePattern(("%s"+namePrefix+"%05d.jpg").toLocal8Bit()); break;
				default: jpeg->SetFilePattern(("%s"+namePrefix+".jpg").toLocal8Bit()); break;
			}
			jpeg->UpdateWholeExtent();
			int *dimensions = jpeg->GetOutput()->GetDimensions();
			m_width = dimensions[0];
			m_height = dimensions[1];
			if(m_depth > 1){
				jpeg->SetDataOrigin((-0.5)*m_width,(-0.5)*m_height,-0.5*m_depth);
				jpeg->SetDataSpacing(1, 1, 1);
				jpeg->SetDataExtent(0, m_width-1, 0,m_height-1, nameNumber, nameNumber+m_depth-1); 
				switch(m_endian){
					case endianType::BIG:
						jpeg->SetDataByteOrderToBigEndian();
						break;
					case endianType::LITTLE:
						jpeg->SetDataByteOrderToLittleEndian();
						break;
				}
				jpeg->SetDataScalarTypeToUnsignedChar();
				jpeg->UpdateInformation();
				jpeg->UpdateWholeExtent();
				jpeg->Update();
				data = (vtkImageReader2*)jpeg;
				break;
			}
			data = nullptr;
			break;
	}
	m_data = data;
	emit updateProgress(100);
	emit loaded(data);
}

void loading::loadPoly()
{
	emit updateProgress(-1);

	vtkPLYReader *ply = vtkPLYReader::New();
	vtkXMLPolyDataReader *vtp = vtkXMLPolyDataReader::New();
	vtkOBJReader *obj = vtkOBJReader::New();
	vtkSTLReader *stl = vtkSTLReader::New();
	vtkPolyDataReader *vtk = vtkPolyDataReader::New();
	vtkBYUReader *g = vtkBYUReader::New();
	vtkStructuredPointsReader *vtr = vtkStructuredPointsReader::New();
	vtkImageDataGeometryFilter *geomFilter = vtkImageDataGeometryFilter::New();
	vtkSimplePointsReader *xyz = vtkSimplePointsReader::New();
	vtkPolyData *poly = vtkPolyData::New();
	vtkGeometryFilter *geometryFilter = vtkGeometryFilter::New();
	vtkDelaunay3D *delaunay3D = vtkDelaunay3D::New();
	vtkCleanPolyData *clean = vtkCleanPolyData::New();
	int dataExt[6] = {0,0,0,0,0,0};
	switch(m_polyFormat){
		case POLY_UNDEF:
			poly = nullptr;
			break;
		case POLY_PLY:
			ply->SetFileName(m_fileName.toLocal8Bit());
			ply->Update();
			poly = ply->GetOutput();;
			break;
		case POLY_VTP:
			vtp->SetFileName(m_fileName.toLocal8Bit());
			vtp->Update();
			poly = vtp->GetOutput();
			break;
		case POLY_OBJ:
			obj->SetFileName(m_fileName.toLocal8Bit());
			obj->Update();
			poly = obj->GetOutput();
			break;
		case POLY_STL:
			stl->SetFileName(m_fileName.toLocal8Bit());
			stl->Update();
			poly = stl->GetOutput();
			break;
		case POLY_VTK:
			vtk->SetFileName(m_fileName.toLocal8Bit());
			vtk->Update();
			poly = vtk->GetOutput();
			break;
		case POLY_G:
			g->SetGeometryFileName(m_fileName.toLocal8Bit());
			g->Update();
			poly = g->GetOutput();
			break;
		case POLY_VTR:
			vtr->SetFileName(m_fileName.toLocal8Bit());
			vtr->Update();
			geometryFilter->SetInputConnection(vtr->GetOutputPort());
			geometryFilter->Update();
			poly = geometryFilter->GetOutput();
		case POLY_XYZ:
			xyz->SetFileName(m_fileName.toLocal8Bit());
			xyz->Update();
			clean->SetInputData(xyz->GetOutput());
			clean->SetTolerance(0.005);
			clean->Update();
			delaunay3D->SetInputData(clean->GetOutput());
			delaunay3D->Update();
			geometryFilter->SetInputData(delaunay3D->GetOutput());
			geometryFilter->Update();
			poly = geometryFilter->GetOutput();
			break;
	}
	m_poly = poly;
	emit updateProgress(100);
	emit loaded(poly);
}

void loading::setData(OpenData* od)
{
	m_lastPath = od->getFilePath();
	m_fileName = od->getFileName();
	m_bitsize = od->getBitsize();
	m_endian = od->getEndian();
	m_dataFormat = od->getDataFormat();
	m_polyFormat = polyType(-1);
	m_width = od->getWidth(); 
	m_height = od->getHeight();
	m_depth = od->getDepth();
}

void loading::setPoly(OpenPoly* op)
{
	m_lastPath = op->getFilePath();
	m_fileName = op->getFileName();
	m_bitsize = bitsizeType(-1);
	m_endian = endianType(-1);
	m_dataFormat = dataType(-1);
	m_polyFormat = op->getPolyFormat();
	std::memcpy(m_VOI, op->getVOI(), 6*sizeof(double));
}