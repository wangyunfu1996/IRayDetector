#pragma once

#include <QObject>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

struct ApplicatioMode
{
	int Index;
	std::string Subset;
	std::string Activity;
	std::string AcqMode;
	std::string Binning;
	std::string Zoom;
};

class ApplicatioModeFileHelper : public QObject
{
	Q_OBJECT

public:
	ApplicatioModeFileHelper(QObject* parent = nullptr);
	~ApplicatioModeFileHelper();

	// 从 XML 读取
	static bool readFromXml(QXmlStreamReader& xml);

	// 写入 XML
	static void writeToXml(QXmlStreamWriter& xml);
};

