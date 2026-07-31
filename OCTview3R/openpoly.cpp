#include "OpenPoly.h"
#include "ui_openpoly.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QList>
#include <QVariant>
#include <QWidget>
#include <QtGlobal>

OpenPoly::OpenPoly(QWidget *parent)
	: QDialog(parent),
	  ui(nullptr),
	  m_windowIsOpen(false),
	  m_validData(false),
	  m_loading(false),
	  m_lastPath("."),
	  m_fileName("Choose file..."),
	  m_polyFormat(polyType::POLY_UNDEF)
{
	initGUI();
}

OpenPoly::~OpenPoly(void)
{
	delete ui;
}

void OpenPoly::showDialog()
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

void OpenPoly::closeEvent(QCloseEvent *event)
{
	if (m_loading)
	{
		event->ignore();
		return;
	}

	m_windowIsOpen = false;
	QDialog::closeEvent(event);
}

void OpenPoly::openFile()
{
	//SLOT after pressing Button New File...
	QFileDialog getFileDialog(this, "Open File", m_lastPath, "Poly Data (*.vtk *.stl *.ply *.vtp *.obj *.g *.vtr *.xyz)");
	getFileDialog.setAcceptMode(QFileDialog::AcceptOpen);
	QStringList filters;
	filters << "VTK files (*.vtk)" << "STL files (*.stl)" << "PLY files (*.ply)" << "VTP files (*.vtp)" << "OBJ files (*.obj)" << "G files (*.g)" << "VTR files (*.vtr)" << "XYZ files (*.xyz)";
	getFileDialog.setNameFilters(filters);
	if(getFileDialog.exec() == QDialog::Accepted){
		QString selectedFilter = getFileDialog.selectedNameFilter();
		QString fileName = getFileDialog.selectedFiles().value(0);
		fileAttributes(fileName, selectedFilter);
		this->m_validData = !m_fileName.isEmpty() && m_polyFormat != polyType::POLY_UNDEF;
		updateProgress(0);
	}else{
		return;
	}
}

void OpenPoly::fileAttributes(QString fileName, QString selectedFilter)
{
	Q_UNUSED(selectedFilter);
	const QString suffix = QFileInfo(fileName).suffix().toLower();

	if(suffix == "ply")
		m_polyFormat = polyType::POLY_PLY;
	else if(suffix == "vtp")
		m_polyFormat = polyType::POLY_VTP;
	else if(suffix == "obj")
		m_polyFormat = polyType::POLY_OBJ;
	else if(suffix == "stl")
		m_polyFormat = polyType::POLY_STL;
	else if(suffix == "vtk")
		m_polyFormat = polyType::POLY_VTK;
	else if(suffix == "g")
		m_polyFormat = polyType::POLY_G;
	else if(suffix == "vtr")
		m_polyFormat = polyType::POLY_VTR;
	else if(suffix == "xyz")
		m_polyFormat = polyType::POLY_XYZ;
	else
	{
		m_polyFormat = polyType::POLY_UNDEF;
		m_fileName.clear();
		return;
	}

	m_fileName = fileName;
	m_lastPath = QFileInfo(fileName).absolutePath();
	ui->label_fileName->setText(m_fileName);
	ui->label_fileName->adjustSize();
}
void OpenPoly::doAccepted()
{
	setLoading(false);
	m_windowIsOpen = false;
	accept();
}
void OpenPoly::doRejected()
{
	if (m_loading)
		return;

	m_windowIsOpen = false;
	reject();
}
void OpenPoly::updateProgress(int value)
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

void OpenPoly::setLoading(bool loading)
{
	if (m_loading == loading)
		return;

	m_loading = loading;
	const QList<QWidget*> controls{
		ui->openFileButton,
		ui->pushButton_cancel,
		ui->pushButton_ok
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

void OpenPoly::startProcessing()
{
	emit signalStartProcess();
}

void OpenPoly::initGUI()
{
	this->ui = new Ui_OpenPoly;
	ui->setupUi(this);
	ui->label_fileName->setText(m_fileName);
	updateProgress(0);
	connect(ui->openFileButton, SIGNAL(clicked()), this, SLOT(openFile()));
	connect(ui->pushButton_ok, SIGNAL(clicked()), this, SLOT(startProcessing()));
	connect(ui->pushButton_cancel, SIGNAL(clicked()), this, SLOT(doRejected()));
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
