#include "OpenPoly.h"

#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkStructuredGridReader.h>
#include <vtkStructuredGridGeometryFilter.h>
#include <vtkProgressObserver.h>
#include <vtkBYUReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyDataReader.h>
#include <vtkSTLReader.h>
#include <vtkStructuredPointsReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkNamedColors.h>
#include <vtkSimplePointsReader.h>
#include <vtkImageDataGeometryFilter.h>

OpenPoly::OpenPoly(QWidget *parent) : QDialog(parent)
{
	initGUI();
}

OpenPoly::OpenPoly(const OpenPoly &od)
{
	ui = od.ui;
	m_windowIsOpen = od.m_windowIsOpen;
	m_validData = od.m_validData;
	m_lastPath = od.m_lastPath;
	m_fileName = od.m_fileName;
	m_polyFormat = od.m_polyFormat;
}

OpenPoly::~OpenPoly(void)
{
	delete ui;
}

void OpenPoly::showDialog()
{
	emit updateProgress(0);
	if(!this->isWindowOpen()){
		show();
	}else{
		raise();
	}
}
void OpenPoly::openFile()
{
	//SLOT after pressing Button New File...
	m_windowIsOpen = true;
	QFileDialog getFileDialog(this, "Open File", m_lastPath, "Poly Data (*.vtk *.stl *.ply *.vtp *.obj *.g *.vtr *.xyz)");
	getFileDialog.setAcceptMode(QFileDialog::AcceptOpen);
	QStringList filters;
	filters << "VTK files (*.vtk)" << "STL files (*.stl)" << "PLY files (*.ply)" << "VTP files (*.vtp)" << "OBJ files (*.obj)" << "G files (*.g)" << "VTR files (*.vtr)" << "XYZ files (*.xyz)";
	getFileDialog.setNameFilters(filters);
	if(getFileDialog.exec() == QDialog::Accepted){
		QString selectedFilter = getFileDialog.selectedNameFilter();
		QString fileName = getFileDialog.selectedFiles()[0];
		fileAttributes(fileName, selectedFilter);
		this->m_validData = true;
		emit updateProgress(0);
	}else{
		QMessageBox::information(this,tr("WARNING"), tr("No valid image data selected"));	
		this->m_validData = false;
	}
}

void OpenPoly::ProgressFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData)
{
	vtkPolyDataReader* tmp = static_cast<vtkPolyDataReader*>(caller);
	OpenPoly *pOpenPoly = reinterpret_cast<OpenPoly*>(clientData);
	qDebug() << QString().setNum(tmp->GetProgress());
	emit pOpenPoly->updateProgress(int(tmp->GetProgress()*100));
} 

void OpenPoly::fileAttributes(QString fileName, QString selectedFilter)
{
	//gets the user input, called by openFile()
	vtkSmartPointer<vtkPolyData> poly = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkStructuredGridGeometryFilter> structuredGridFilter = vtkSmartPointer<vtkStructuredGridGeometryFilter>::New();
	vtkSmartPointer<vtkImageDataGeometryFilter> imageDataFilter = vtkSmartPointer<vtkImageDataGeometryFilter>::New();
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	if(selectedFilter=="PLY files (*.ply)"){
		m_polyFormat = polyType::POLY_PLY;
		vtkPLYReader *ply = vtkPLYReader::New();
		ply->SetFileName(fileName.toLocal8Bit());
		ply->Update();
		poly = ply->GetOutput();
	}else if(selectedFilter=="VTP files (*.vtp)"){
		m_polyFormat = polyType::POLY_VTP;
		vtkXMLPolyDataReader *vtp = vtkXMLPolyDataReader::New();
		vtp->SetFileName(fileName.toLocal8Bit());
		vtp->Update();
		poly = vtp->GetOutput();
	}else if(selectedFilter=="OBJ files (*.obj)"){
		m_polyFormat = polyType::POLY_OBJ;
		vtkOBJReader *obj = vtkOBJReader::New();
		obj->SetFileName(fileName.toLocal8Bit());
		obj->Update();
		poly = obj->GetOutput();;
	}else if(selectedFilter=="STL files (*.stl)"){
		m_polyFormat = polyType::POLY_STL;
		vtkSTLReader *stl = vtkSTLReader::New();
		stl->SetFileName(fileName.toLocal8Bit());
		stl->Update();
		poly = stl->GetOutput();
	}else if(selectedFilter=="VTK files (*.vtk)"){
		m_polyFormat = polyType::POLY_VTK;
		vtkPolyDataReader *vtk = vtkPolyDataReader::New();
		vtk->SetFileName(fileName.toLocal8Bit());
		vtk->Update();
		poly = vtk->GetOutput();
	}else if(selectedFilter=="G files (*.g)"){
		m_polyFormat = polyType::POLY_G;
		vtkBYUReader *g = vtkBYUReader::New();
		g->SetFileName(fileName.toLocal8Bit());
		g->Update();	
		poly = g->GetOutput();;
	}else if(selectedFilter=="VTR files (*.vtr)"){
		m_polyFormat = polyType::POLY_VTR;
		vtkStructuredPointsReader *vtr = vtkStructuredPointsReader::New();
		vtr->SetFileName(fileName.toLocal8Bit());
		vtr->Update();
		imageDataFilter->SetInputConnection(vtr->GetOutputPort());
		imageDataFilter->Update();
		poly = imageDataFilter->GetOutput();
	}else if(selectedFilter=="XYZ files (*.xyz)"){
		m_polyFormat = polyType::POLY_XYZ;
		vtkSimplePointsReader *xyz = vtkSimplePointsReader::New();
		xyz->SetFileName(fileName.toLocal8Bit());
		xyz->Update();
		poly = xyz->GetOutput();
	}else{
		m_polyFormat = polyType::POLY_UNDEF;
		m_fileName = "";
		return;
	}
	mapper->SetInputData(poly);
	actor->SetMapper(mapper);
	double bounds[6];
	actor->GetBounds(bounds);
	setVOI(bounds);

  	//TODO: Add GUI elements to show bounds
	m_fileName = fileName;
	m_lastPath = fileName.section("/",0,-2);
	ui->label_fileName->setText(m_fileName);
	ui->label_fileName->adjustSize();
}
void OpenPoly::doAccepted()
{
	m_windowIsOpen = false;
	accept();
}
void OpenPoly::doRejected()
{
	m_windowIsOpen = false;
	reject();
}
void OpenPoly::doDestroyed()
{
	m_windowIsOpen = false;
}

void OpenPoly::updateProgress(int value)
{
	if(value == 0){
		ui->progressBar->setRange(0,100);
		ui->progressBar->setValue(value);
	}else if(value > 0){
		ui->progressBar->setRange(0,100);
		ui->progressBar->setValue(value);
	}else if(value < 0){
		ui->progressBar->setRange(0,0);
		ui->progressBar->setValue(value);
	}
}

void OpenPoly::startProcessing()
{
	emit signalStartProcess();
}

void OpenPoly::initGUI()
{
	this->ui = new Ui_OpenPoly;
	ui->setupUi(this);
	m_windowIsOpen = false;
	m_lastPath = ".";
	m_fileName = "Choose file...";
	ui->label_fileName->setText(m_fileName);
	updateProgress(0);
	connect(ui->openFileButton, SIGNAL(clicked()), this, SLOT(openFile()));
	connect(ui->pushButton_ok, SIGNAL(clicked()), this, SLOT(startProcessing()));
	connect(ui->pushButton_cancel, SIGNAL(clicked()), this, SLOT(doRejected()));
	connect(this, SIGNAL(signalCloseWindow()), this, SLOT(doRejected()));
	connect(this, SIGNAL(destroyed()), this, SLOT(doDestroyed()));
}
//////////////////////////////////////////////////////////////////////////////////////
//CHECKER/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
bool OpenPoly::isWindowOpen()
{
	return m_windowIsOpen;
}
bool OpenPoly::isValidData()
{
	return this->m_validData;
}
//////////////////////////////////////////////////////////////////////////////////////
//GETTER//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
QString OpenPoly::getFileName()
{
	return this->m_fileName;
}
QString OpenPoly::getFilePath()
{
	return this->m_lastPath;
}
polyType OpenPoly::getPolyFormat()
{
	return this->m_polyFormat;
}
double *OpenPoly::getVOI()
{
	return this->m_VOI;
}
//////////////////////////////////////////////////////////////////////////////////////
//SETTER//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
void OpenPoly::setFileName(QString string)
{
	this->m_fileName = string;
}
void OpenPoly::setFilePath(QString string)
{
	this->m_lastPath = string;
}
void OpenPoly::setPolyFormat(int value)
{
	this->m_polyFormat = (polyType)value;
}
void OpenPoly::setVOI(double *VOI)
{
	std::memcpy(m_VOI, VOI, 6*sizeof(double));
}