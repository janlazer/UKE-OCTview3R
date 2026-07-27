#pragma once

class vtkObject;
class vtkCommand;
class vtkCallbackCommand;
class vtkProgressObserver;
class vtkImageReader2;
class vtkImageReader;
class vtkTIFFReader;
class vtkJPEGReader;
class vtkImageData;
class vtkStructuredPoints;

#include <QtGui>
#include <QDialog>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include "dataTypes.h"
#include "ui_opendata.h"


class OpenData : public QDialog
{
    Q_OBJECT

public:
    OpenData(QWidget *widget);
	OpenData(const OpenData &);
	~OpenData();
public slots:
	void initGUI();
	void openFile(); // opens search for path
	void showDialog();
	static void ProgressFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);
	void closeEvent(QCloseEvent *event) override { 
		event->ignore();
		emit signalCloseWindow(); 
	}
    //GETTER
    QString getFileName();
	QString getFilePath();
    int getWidth();
    int getHeight();
    int getDepth();
	bitsizeType getBitsize(); // 8-unsigned 8 bit=initial value, 16-unsigned 16bit
    endianType getEndian(); // 1=little Endian, 2=big Endian
	dataType getDataFormat();

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

	//SETTER
    void setWidth(int);
    void setHeight(int);
    void setDepth(int);
	void setDataFormat(int);
    void setBitsize(int);
    void setEndian(int);
private:
	Ui_OpenData *ui;
	bool m_windowIsOpen;
	bool m_validData;
	QString m_lastPath;
    QString m_fileName;
	bitsizeType m_bitsize;
    endianType m_endian;
	dataType m_dataFormat;
    int m_width; 
	int m_height;
	int m_depth;
};