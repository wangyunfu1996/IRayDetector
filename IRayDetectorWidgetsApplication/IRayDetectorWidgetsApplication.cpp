#include "IRayDetectorWidgetsApplication.h"

#include <qdebug.h>

#include "../IRayDetector/IRayDetector.h"

IRayDetectorWidgetsApplication::IRayDetectorWidgetsApplication(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	ui.comboBox_mode->addItem("Mode5");
	ui.comboBox_mode->addItem("Mode6");
	ui.comboBox_mode->addItem("Mode7");
	ui.comboBox_mode->addItem("Mode8");

	connect(ui.comboBox_mode, &QComboBox::currentTextChanged, this, [this]() {
		DET.UpdateMode(ui.comboBox_mode->currentText().toStdString());
		});

	connect(ui.pushButton_nVal, &QPushButton::clicked, this, [this]() {
		ui.lineEdit_msg->clear();
		ui.lineEdit_nVal->clear();

		int nAttrID = ui.lineEdit_nAttrID->text().toInt();
		int nVal{ -1 };
		int ret = DET.GetAttr(nAttrID, nVal);
		if (0 == ret)
		{
			ui.lineEdit_nVal->setText(QString::number(nVal));
		}
		else
		{
			ui.lineEdit_msg->setText("读取错误！");
		}
		});

	connect(ui.pushButton_fVal, &QPushButton::clicked, this, [this]() {
		ui.lineEdit_msg->clear();
		ui.lineEdit_fVal->clear();

		int nAttrID = ui.lineEdit_nAttrID->text().toInt();
		float fVal{ -1.0 };
		int ret = DET.GetAttr(nAttrID, fVal);
		if (0 == ret)
		{
			ui.lineEdit_fVal->setText(QString::number(fVal));
		}
		else
		{
			ui.lineEdit_msg->setText("读取错误！");
		}
		});

	connect(ui.pushButton_strVal, &QPushButton::clicked, this, [this]() {
		ui.lineEdit_msg->clear();
		ui.lineEdit_strVal->clear();

		int nAttrID = ui.lineEdit_nAttrID->text().toInt();
		std::string strVal;
		int ret = DET.GetAttr(nAttrID, strVal);
		if (0 == ret)
		{
			ui.lineEdit_strVal->setText(QString::fromStdString(strVal));
		}
		else
		{
			ui.lineEdit_msg->setText("读取错误！");
		}
		});

}

IRayDetectorWidgetsApplication::~IRayDetectorWidgetsApplication()
{}

