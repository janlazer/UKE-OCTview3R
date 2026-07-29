#pragma once

#include "dataTypes.h"

#include <QDialog>
#include <QString>

class QCloseEvent;
class Ui_OpenData;

class OpenData : public QDialog
{
    Q_OBJECT

public:
    explicit OpenData(QWidget *widget = nullptr);
	OpenData(const OpenData &) = delete;
	OpenData& operator=(const OpenData &) = delete;
	~OpenData() override;

public slots:
	void initGUI();
	void openFile(); // opens search for path
	void showDialog();
	void doAccepted();
	void doRejected();
	void updateProgress(int);
	void setFilePath(const QString& path);

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
	void signalStartProcess();

protected:
	void closeEvent(QCloseEvent *event) override;

private slots:
	//GENERAL 
	void fileAttributes(QString, QString);
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
