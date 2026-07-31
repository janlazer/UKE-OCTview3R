#pragma once

#include <QDialog>

class Ui_AboutDialog;

class AboutDialog final : public QDialog
{
public:
	explicit AboutDialog(QWidget* parent = nullptr);
	AboutDialog(const AboutDialog&) = delete;
	AboutDialog& operator=(const AboutDialog&) = delete;
	~AboutDialog() override;

private:
	Ui_AboutDialog* ui;
};
