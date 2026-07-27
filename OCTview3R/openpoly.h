#pragma once

#include "dataTypes.h"

#include <QDialog>
#include <QString>

class QCloseEvent;
class Ui_OpenPoly;

class OpenPoly : public QDialog
{
	Q_OBJECT

public:
	explicit OpenPoly(QWidget *widget = nullptr);
	OpenPoly(const OpenPoly &) = delete;
	OpenPoly& operator=(const OpenPoly &) = delete;
	~OpenPoly() override;

public slots:
	void initGUI();
	void openFile(); // opens search for path
	void showDialog();
	void doAccepted();
	void doRejected();
	void updateProgress(int);

	//GETTER
	QString getFileName();
	QString getFilePath();
	polyType getPolyFormat();
	
	//SETTER
	void setPolyFormat(int);
	void setFileName(QString);
	void setFilePath(QString);

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

private:
	Ui_OpenPoly *ui;
	bool m_windowIsOpen;
	bool m_validData;
	QString m_lastPath;
	QString m_fileName;
	polyType m_polyFormat;
};
