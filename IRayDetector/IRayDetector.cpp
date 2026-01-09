#include "IRayDetector.h"

#include <qdebug.h>
#include <quuid.h>

IRayDetector::IRayDetector(QObject* parent)
	: QObject(parent)
{
	m_uuid = QUuid::createUuid().toString();
	qDebug() << "构造探测器实例：" << m_uuid;
}

IRayDetector::~IRayDetector()
{
	qDebug() << "析构探测器实例：" << m_uuid;
}

