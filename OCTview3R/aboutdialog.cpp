#include "aboutdialog.h"

#include "ui_aboutdialog.h"

#include <QCoreApplication>
#include <QString>
#include <QtGlobal>

#include <vtkVersion.h>

AboutDialog::AboutDialog(QWidget* parent)
	: QDialog(parent),
	  ui(new Ui_AboutDialog)
{
	ui->setupUi(this);
	ui->versionLabel->setText(
		tr("Version %1").arg(QCoreApplication::applicationVersion()));
	ui->technologyLabel->setText(
		tr("Built with Qt %1 and VTK %2")
			.arg(QString::fromLatin1(qVersion()))
			.arg(QString::fromLatin1(vtkVersion::GetVTKVersion())));
}

AboutDialog::~AboutDialog()
{
	delete ui;
}
