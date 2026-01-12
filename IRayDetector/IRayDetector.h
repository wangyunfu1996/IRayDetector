#pragma once

#include "iraydetector_global.h"

#include <QObject>

#define DET IRayDetector::Instance()

class IRAYDETECTOR_EXPORT IRayDetector : public QObject
{
	Q_OBJECT

private:
	IRayDetector(QObject* parent = nullptr);
	~IRayDetector();

public:
	static IRayDetector& Instance();

	int Initializte();
	void DeInitializte();

	int GetAttr(int nAttrID, int& nVal);
	int GetAttr(int nAttrID, float& fVal);
	int GetAttr(int nAttrID, std::string& strVal);

	int UpdateMode(std::string mode);
	int GetCurrentCorrectOption(int& sw_offset, int& sw_gain, int& sw_defect);
	int SetCorrectOption(int sw_offset, int sw_gain, int sw_defect);
	int SetPreviewImageEnable(int enable);
	void ClearAcq();
	void StartSeqAcq();
	void StopSeqAcq();

private:
	QString m_uuid;
	QString m_workDirPath;
};

