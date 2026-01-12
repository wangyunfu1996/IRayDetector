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

	int sw_offset{ -1 };
	int sw_gain{ -1 };
	int sw_defect{ -1 };
	DET.GetCurrentCorrectOption(sw_offset, sw_gain, sw_defect);
	ui.checkBox_offset->setChecked(sw_offset == 1);
	ui.checkBox_gain->setChecked(sw_gain == 1);
	ui.checkBox_defect->setChecked(sw_defect == 1);

	auto updateCorrectOption = [this]() {
		int sw_offset = ui.checkBox_offset->isChecked();
		int sw_gain = ui.checkBox_gain->isChecked();
		int sw_defect = ui.checkBox_defect->isChecked();
		DET.SetCorrectOption(sw_offset, sw_gain, sw_defect);
	};

	connect(ui.checkBox_offset, &QCheckBox::toggled, this, [this, updateCorrectOption](bool checked) {
		Q_UNUSED(checked);
		updateCorrectOption();
		});
	connect(ui.checkBox_gain, &QCheckBox::toggled, this, [this, updateCorrectOption](bool checked) {
		Q_UNUSED(checked);
		updateCorrectOption();
		});
	connect(ui.checkBox_defect, &QCheckBox::toggled, this, [this, updateCorrectOption](bool checked) {
		Q_UNUSED(checked);
		updateCorrectOption();
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

