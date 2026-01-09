#pragma once

#include "iraydetector_global.h"

#include <QObject>

class IRAYDETECTOR_EXPORT IRayDetector : public QObject
{
	Q_OBJECT

public:
	IRayDetector(QObject* parent = nullptr);
	~IRayDetector();

private:
	QString m_uuid;
};

