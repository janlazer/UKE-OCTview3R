#pragma once

class vtkObject;
class vtkCommand;
class vtkProgressObserver;
class vtkPolyData;
class vtkPolyDataReader;
class vtkPolyDataMapper;
class vtkStructuredGridReader;
class vtkStructuredGridGeometryFilter;
class vtkStructuredPointsReader;
class vtkBYUReader;
class vtkOBJReader;
class vtkPLYReader;
class vtkSTLReader;
class vtkXMLPolyDataReader;
class vtkActor;
class vtkProperty;
class vtkNamedColors;
class vtkSimplePointsReader;
class vtkImageDataGeometryFilter;

#include <QtGui>
#include <QDialog>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include "dataTypes.h"
#include "ui_openpoly.h"

class OpenPoly : public QDialog
{
	Q_OBJECT

public:
	OpenPoly(QWidget *widget);
	OpenPoly(const OpenPoly &);
	~OpenPoly();
public slots:
	void initGUI();
	void openFile(); // opens search for path
	void showDialog();
	static void ProgressFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);
	void closeEvent(QCloseEvent *event) override{ 
		event->ignore();
		emit signalCloseWindow(); 
	}

	//GETTER
	QString getFileName();
	QString getFilePath();
	polyType getPolyFormat();
	double *getVOI();
	
	//SETTER
	void setPolyFormat(int);
	void setFileName(QString);
	void setFilePath(QString);
	void setVOI(double*);

	//CHECKER
	bool isValidData();
	bool isWindowOpen();

signals:
	void signalCloseWindow();
	void signalStartProcess();

private slots:
	//GENERAL 
	void fileAttributes(QString, QString);
	void doAccepted();
	void doRejected();
	void doDestroyed();
	void updateProgress(int);
	void startProcessing();

private:
	Ui_OpenPoly *ui;
	bool m_windowIsOpen;
	bool m_validData;
	QString m_lastPath;
	QString m_fileName;
	polyType m_polyFormat;
	double m_VOI[6];
};