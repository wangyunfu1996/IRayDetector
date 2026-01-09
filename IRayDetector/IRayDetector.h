#pragma once

#include "iraydetector_global.h"

#include <QObject>

class IRAYDETECTOR_EXPORT IRayDetector : public QObject
{
	Q_OBJECT

public:
	IRayDetector(QObject* parent = nullptr);
	~IRayDetector();

	int Initializte();
	void DeInitializte();

	void SingleAcq();
	void StartSeqAcq();
	void StopSeqAcq();

private:
	QString m_uuid;
};

