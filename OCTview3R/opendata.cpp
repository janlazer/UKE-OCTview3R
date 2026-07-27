#include "OpenData.h"

#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkImageReader2.h>
#include <vtkImageReader.h>
#include <vtkTIFFReader.h>
#include <vtkJPEGReader.h>
#include <vtkImageData.h>
#include <vtkStructuredPoints.h>
#include <vtkStructuredPointsReader.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkProgressObserver.h>
#include <vtkCommand.h>
#include <vtkCallbackCommand.h>

OpenData::OpenData(QWidget *parent) : QDialog(parent)
{
	initGUI();
}

OpenData::OpenData(const OpenData &od)
{
	ui = od.ui;
	m_windowIsOpen = od.m_windowIsOpen;
	m_validData = od.m_validData;
	m_lastPath = od.m_lastPath;
    m_fileName = od.m_fileName;
	m_bitsize = od.m_bitsize;
    m_endian = od.m_endian;
	m_dataFormat = od.m_dataFormat;
    m_width = od.m_width; 
	m_height = od.m_height;
	m_depth = od.m_depth;
}

OpenData::~OpenData(void)
{
	delete ui;
}

void OpenData::showDialog()
{
	emit updateProgress(0);
	if(!this->isWindowOpen()){
		
		show();
	}else{
		raise();
	}
}
void OpenData::openFile()
{
	//SLOT after pressing Button New File...
	m_windowIsOpen = true;
	QFileDialog getFileDialog(this, "Open File", m_lastPath, "Volume Data (*.tif *.raw *.jpg *.vtk)");
	getFileDialog.setAcceptMode(QFileDialog::AcceptOpen);
	QStringList filters;
	filters << "TIF files (*.tif)" << "RAW files (*.raw)" << "JPG files (*.jpg)" << "VTK files (*.vtk)";
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
void OpenData::ProgressFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData)
{
	vtkTIFFReader* tmp = static_cast<vtkTIFFReader*>(caller);
	OpenData *pOpenData = reinterpret_cast<OpenData*>(clientData);
	qDebug() << QString().setNum(tmp->GetProgress());
	emit pOpenData->updateProgress(int(tmp->GetProgress()*100));
} 
#include <vtkGenericDataObjectReader.h>
void OpenData::fileAttributes(QString fileName, QString selectedFilter)
{
	//gets the user input, called by openFile()
	if(selectedFilter=="RAW files (*.raw)"){
		m_dataFormat = dataType::DATA_RAW;
		ui->comboBox_endian->setEnabled(true);
		ui->label_endian->setEnabled(true);
		ui->comboBox_bitsize->setEnabled(true);
		ui->label_format->setEnabled(true);
		ui->spinBox_height->setEnabled(true);
		ui->label_height->setEnabled(true);
		ui->spinBox_width->setEnabled(true);
		ui->label_width->setEnabled(true);
		ui->spinBox_depth->setEnabled(true);
		ui->label_depth->setEnabled(true);
	}else if(selectedFilter=="TIF files (*.tif)"){
		m_dataFormat = dataType::DATA_TIFF;
		vtkTIFFReader *tiff = vtkTIFFReader::New();
		tiff->SetFileName(fileName.toLocal8Bit());
		tiff->UpdateInformation();
		int dataExt[6];
		tiff->GetDataExtent(dataExt);
		setWidth(dataExt[1] + 1);
		setHeight(dataExt[3] + 1);
		setDepth(dataExt[5] + 1);
		//CHECK BITSIZE
		switch(tiff->GetDataScalarType()){
			case VTK_UNSIGNED_CHAR:
				setBitsize(bitsizeType::BIT8);
				break;
			case VTK_UNSIGNED_SHORT:
				setBitsize(bitsizeType::BIT16);
				break;
		}
		//CHECK ENDIANNESS
		QString switchString = QString::fromUtf8(tiff->GetDataByteOrderAsString());
		if(switchString=="BigEndian"){
			setEndian(endianType::BIG);
		}else if(switchString=="LittleEndian"){
			setEndian(endianType::LITTLE);
		}
		ui->comboBox_endian->setEnabled(false);
		ui->label_endian->setEnabled(false);
		ui->comboBox_bitsize->setEnabled(false);
		ui->label_format->setEnabled(false);
		ui->spinBox_height->setEnabled(false);
		ui->label_height->setEnabled(false);
		ui->spinBox_width->setEnabled(false);
		ui->label_width->setEnabled(false);
		ui->spinBox_depth->setEnabled(false);
		ui->label_depth->setEnabled(false);
	}else if(selectedFilter=="JPG files (*.jpg)"){
		m_dataFormat = dataType::DATA_JPEG;
		vtkJPEGReader *jpeg = vtkJPEGReader::New();
		jpeg->SetFileName(fileName.toLocal8Bit());
		jpeg->UpdateWholeExtent();
		int *dimensions = new int(2);
		dimensions = jpeg->GetOutput()->GetDimensions();
		setWidth(dimensions[0]);
		setHeight(dimensions[1]);
		ui->comboBox_endian->setEnabled(true);
		ui->label_endian->setEnabled(true);
		ui->comboBox_bitsize->setEnabled(false);
		ui->label_format->setEnabled(false);
		ui->spinBox_height->setEnabled(false);
		ui->label_height->setEnabled(false);
		ui->spinBox_width->setEnabled(false);
		ui->label_width->setEnabled(false);
		ui->spinBox_depth->setEnabled(true);
		ui->label_depth->setEnabled(true);
	}else if(selectedFilter=="VTK files (*.vtk)"){
		m_dataFormat = dataType::DATA_VTK;
		vtkStructuredPointsReader *vtk = vtkStructuredPointsReader::New();
		vtk->SetFileName(fileName.toLocal8Bit());
		vtk->UpdateWholeExtent();
		int dataExt[6];
		vtk->GetUpdateExtent(dataExt);
		setWidth(dataExt[1] + 1);
		setHeight(dataExt[3] + 1);
		setDepth(dataExt[5] + 1);
		//CHECK BITSIZE
		/*switch(vtk->GetDataScalarType()){
			case VTK_UNSIGNED_CHAR:
				setBitsize(bitsizeType::BIT8);
				break;
			case VTK_UNSIGNED_SHORT:
				setBitsize(bitsizeType::BIT16);
				break;
		}
		//CHECK ENDIANNESS
		QString switchString = QString::fromUtf8(vtk->GetDataByteOrderAsString());
		if(switchString=="BigEndian"){
			setEndian(endianType::BIG);
		}else if(switchString=="LittleEndian"){
			setEndian(endianType::LITTLE);
		}*/
		ui->comboBox_endian->setEnabled(false);
		ui->label_endian->setEnabled(false);
		ui->comboBox_bitsize->setEnabled(false);
		ui->label_format->setEnabled(false);
		ui->spinBox_height->setEnabled(false);
		ui->label_height->setEnabled(false);
		ui->spinBox_width->setEnabled(false);
		ui->label_width->setEnabled(false);
		ui->spinBox_depth->setEnabled(false);
		ui->label_depth->setEnabled(false);
	}else{
		m_dataFormat = dataType::DATA_UNDEF;
		m_fileName = "";
		ui->comboBox_endian->setEnabled(false);
		ui->label_endian->setEnabled(false);
		ui->comboBox_bitsize->setEnabled(false);
		ui->label_format->setEnabled(false);
		ui->spinBox_height->setEnabled(false);
		ui->label_height->setEnabled(false);
		ui->spinBox_width->setEnabled(false);
		ui->label_width->setEnabled(false);
		ui->spinBox_depth->setEnabled(false);
		ui->label_depth->setEnabled(false);
		return;
	}
	m_fileName = fileName;
	m_lastPath = fileName.section("/",0,-2);
	ui->label_fileName->setText(m_fileName);
	ui->label_fileName->adjustSize();
}
void OpenData::doAccepted()
{
	m_windowIsOpen = false;
	accept();
}
void OpenData::doRejected()
{
	m_windowIsOpen = false;
	reject();
}
void OpenData::doDestroyed()
{
	m_windowIsOpen = false;
}

void OpenData::updateProgress(int value)
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

void OpenData::startProcessing()
{
	emit signalStartProcess();
}

void OpenData::initGUI()
{
	this->ui = new Ui_OpenData;
	ui->setupUi(this);
	m_windowIsOpen = false;
	m_lastPath = ".";
	m_fileName = "Choose file...";
	ui->label_fileName->setText(m_fileName);
	updateProgress(0);
	connect(ui->openFileButton, SIGNAL(clicked()), this, SLOT(openFile()));
	connect(ui->spinBox_width, SIGNAL(valueChanged(int)), this, SLOT(setWidth(int)));
	connect(ui->spinBox_height, SIGNAL(valueChanged(int)), this, SLOT(setHeight(int)));
	connect(ui->spinBox_depth, SIGNAL(valueChanged(int)), this, SLOT(setDepth(int)));
	connect(ui->comboBox_bitsize, SIGNAL(currentIndexChanged(int)), this, SLOT(setBitsize(int)));
	connect(ui->comboBox_endian, SIGNAL(currentIndexChanged(int)), this, SLOT(setEndian(int)));
	connect(ui->pushButton_ok, SIGNAL(clicked()), this, SLOT(startProcessing()));
	connect(ui->pushButton_cancel, SIGNAL(clicked()), this, SLOT(doRejected()));
	connect(this, SIGNAL(signalCloseWindow()), this, SLOT(doRejected()));
	connect(this, SIGNAL(destroyed()), this, SLOT(doDestroyed()));
}
//////////////////////////////////////////////////////////////////////////////////////
//CHECKER/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
bool OpenData::isWindowOpen()
{
	return m_windowIsOpen;
}

bool OpenData::isValidData()
{
	return this->m_validData;
}
//////////////////////////////////////////////////////////////////////////////////////
//GETTER//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
QString OpenData::getFileName()
{
	return this->m_fileName;
}

QString OpenData::getFilePath()
{
	return this->m_lastPath;
}

bitsizeType OpenData::getBitsize()
{
	m_bitsize = (bitsizeType)ui->comboBox_bitsize->currentIndex(); // 0-8bit or 1-16bit
	return this->m_bitsize;
}
endianType OpenData::getEndian()
{
	m_endian = (endianType)ui->comboBox_endian->currentIndex(); // 0-little or 1-big
	return this->m_endian;
}

dataType OpenData::getDataFormat()
{
	return this->m_dataFormat;
}

int OpenData::getWidth()
{
	m_width  = ui->spinBox_width->value();
	return this->m_width;
}
int OpenData::getHeight()
{
	m_height = ui->spinBox_height->value();
	return this->m_height;
}
int OpenData::getDepth()
{
	m_depth  = ui->spinBox_depth->value();
	return this->m_depth;
}
//////////////////////////////////////////////////////////////////////////////////////
//SETTER//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
void OpenData::setWidth(int value)
{
	ui->spinBox_width->setValue(value);
	this->m_width=value;
}
void OpenData::setHeight(int value)
{
	ui->spinBox_height->setValue(value);
	this->m_height=value;
}
void OpenData::setDepth(int value)
{
	ui->spinBox_depth->setValue(value);
	this->m_depth=value;
}

void OpenData::setDataFormat(int value)
{
	this->m_dataFormat = (dataType)value;
}

void OpenData::setBitsize(int value)
{
	ui->comboBox_bitsize->setCurrentIndex(value);
	this->m_bitsize = (bitsizeType)value;
}
void OpenData::setEndian(int value)
{
	ui->comboBox_endian->setCurrentIndex(value);
	this->m_endian = (endianType)value;
}
