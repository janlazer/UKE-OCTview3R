#include "OpenData.h"
#include "ui_opendata.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QList>
#include <QVariant>
#include <QWidget>
#include <QtGlobal>

#include <vtkSmartPointer.h>
#include <vtkTIFFReader.h>
#include <vtkJPEGReader.h>
#include <vtkImageData.h>
#include <vtkStructuredPoints.h>
#include <vtkStructuredPointsReader.h>

OpenData::OpenData(QWidget *parent)
	: QDialog(parent),
	  ui(nullptr),
	  m_windowIsOpen(false),
	  m_validData(false),
	  m_loading(false),
	  m_lastPath("."),
	  m_fileName("Choose file..."),
	  m_bitsize(bitsizeType::BIT8),
	  m_endian(endianType::LITTLE),
	  m_dataFormat(dataType::DATA_UNDEF),
	  m_width(0),
	  m_height(0),
	  m_depth(0)
{
	initGUI();
}

OpenData::~OpenData(void)
{
	delete ui;
}

void OpenData::showDialog()
{
	setLoading(false);
	updateProgress(0);
	if(!this->isWindowOpen()){
		m_windowIsOpen = true;
		show();
	}else{
		raise();
		activateWindow();
	}
}

void OpenData::closeEvent(QCloseEvent *event)
{
	if (m_loading)
	{
		event->ignore();
		return;
	}

	m_windowIsOpen = false;
	QDialog::closeEvent(event);
}

void OpenData::openFile()
{
	//SLOT after pressing Button New File...
	QFileDialog getFileDialog(this, "Open File", m_lastPath, "Volume Data (*.tif *.tiff *.raw *.jpg *.jpeg *.vtk)");
	getFileDialog.setAcceptMode(QFileDialog::AcceptOpen);
	QStringList filters;
	filters << "TIFF files (*.tif *.tiff)" << "RAW files (*.raw)" << "JPEG files (*.jpg *.jpeg)" << "VTK files (*.vtk)";
	getFileDialog.setNameFilters(filters);
	if(getFileDialog.exec() == QDialog::Accepted){
		QString selectedFilter = getFileDialog.selectedNameFilter();
		QString fileName = getFileDialog.selectedFiles().value(0);
		fileAttributes(fileName, selectedFilter);
		this->m_validData = !m_fileName.isEmpty() && m_dataFormat != dataType::DATA_UNDEF;
		updateProgress(0);
	}else{
		return;
	}
}

void OpenData::fileAttributes(QString fileName, QString selectedFilter)
{
	//gets the user input, called by openFile()
	Q_UNUSED(selectedFilter);
	const QString suffix = QFileInfo(fileName).suffix().toLower();

	if(suffix == "raw"){
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
	}else if(suffix == "tif" || suffix == "tiff"){
		m_dataFormat = dataType::DATA_TIFF;
		auto tiff = vtkSmartPointer<vtkTIFFReader>::New();
		tiff->SetFileName(fileName.toLocal8Bit());
		tiff->UpdateInformation();
		int dataExt[6] = { 0, 0, 0, 0, 0, 0 };
		tiff->GetDataExtent(dataExt);
		setWidth(dataExt[1] - dataExt[0] + 1);
		setHeight(dataExt[3] - dataExt[2] + 1);
		setDepth(dataExt[5] - dataExt[4] + 1);
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
	}else if(suffix == "jpg" || suffix == "jpeg"){
		m_dataFormat = dataType::DATA_JPEG;
		auto jpeg = vtkSmartPointer<vtkJPEGReader>::New();
		jpeg->SetFileName(fileName.toLocal8Bit());
		jpeg->UpdateInformation();
		int dataExt[6] = { 0, 0, 0, 0, 0, 0 };
		jpeg->GetDataExtent(dataExt);
		setWidth(dataExt[1] - dataExt[0] + 1);
		setHeight(dataExt[3] - dataExt[2] + 1);
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
	}else if(suffix == "vtk"){
		m_dataFormat = dataType::DATA_VTK;
		auto vtk = vtkSmartPointer<vtkStructuredPointsReader>::New();
		vtk->SetFileName(fileName.toLocal8Bit());
		vtk->UpdateInformation();
		int dataExt[6] = { 0, 0, 0, 0, 0, 0 };
		vtk->GetOutput()->GetExtent(dataExt);
		setWidth(dataExt[1] - dataExt[0] + 1);
		setHeight(dataExt[3] - dataExt[2] + 1);
		setDepth(dataExt[5] - dataExt[4] + 1);
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
	m_lastPath = QFileInfo(fileName).absolutePath();
	ui->label_fileName->setText(m_fileName);
	ui->label_fileName->adjustSize();
}
void OpenData::doAccepted()
{
	setLoading(false);
	m_windowIsOpen = false;
	accept();
}
void OpenData::doRejected()
{
	if (m_loading)
		return;

	m_windowIsOpen = false;
	reject();
}
void OpenData::updateProgress(int value)
{
	if(value < 0){
		ui->progressBar->setRange(0,0);
		ui->progressBar->setFormat(tr("Loading..."));
		return;
	}

	ui->progressBar->setRange(0,100);
	ui->progressBar->setValue(qBound(0, value, 100));
	ui->progressBar->setFormat(value == 0 ? tr("Ready") : QStringLiteral("%p%"));
}

void OpenData::setLoading(bool loading)
{
	if (m_loading == loading)
		return;

	m_loading = loading;
	const QList<QWidget*> controls{
		ui->openFileButton,
		ui->pushButton_cancel,
		ui->pushButton_ok,
		ui->comboBox_endian,
		ui->comboBox_bitsize,
		ui->spinBox_width,
		ui->spinBox_height,
		ui->spinBox_depth
	};
	static const char enabledProperty[] = "_octviewEnabledBeforeLoading";

	for (QWidget* control : controls)
	{
		if (loading)
		{
			control->setProperty(enabledProperty, control->isEnabled());
			control->setEnabled(false);
		}
		else
		{
			const QVariant previousState = control->property(enabledProperty);
			if (previousState.isValid())
			{
				control->setEnabled(previousState.toBool());
				control->setProperty(enabledProperty, QVariant());
			}
		}
	}

	ui->progressBar->setEnabled(true);
}

void OpenData::setFilePath(const QString& path)
{
	m_lastPath = path;
}

void OpenData::startProcessing()
{
	emit signalStartProcess();
}

void OpenData::initGUI()
{
	this->ui = new Ui_OpenData;
	ui->setupUi(this);
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
