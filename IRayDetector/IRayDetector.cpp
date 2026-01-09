#include "IRayDetector.h"

#include <qdebug.h>
#include <quuid.h>

IRayDetector::IRayDetector(QObject* parent)
	: QObject(parent)
{
	m_uuid = QUuid::createUuid().toString();
	qDebug() << "¹¹ÔìÌ½²âÆ÷ÊµÀý£º" << m_uuid;
}

IRayDetector::~IRayDetector()
{
	qDebug() << "Îö¹¹Ì½²âÆ÷ÊµÀý£º" << m_uuid;
}

