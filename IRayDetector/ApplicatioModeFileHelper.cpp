#include "ApplicatioModeFileHelper.h"

ApplicatioModeFileHelper::ApplicatioModeFileHelper(QObject* parent)
	: QObject(parent)
{}

ApplicatioModeFileHelper::~ApplicatioModeFileHelper()
{}

bool ApplicatioModeFileHelper::readFromXml(QXmlStreamReader & xml)
{
    if (xml.tokenType() != QXmlStreamReader::StartElement || xml.name() != "Item")
        return false;

    ApplicatioMode mode;
    while (xml.readNextStartElement()) {
        QStringRef name = xml.name();

        if (name == "Index")
            mode.Index = xml.readElementText().toInt();
        else if (name == "Subset")
            mode.Subset = xml.readElementText().toStdString();
        else if (name == "Activity")
            mode.Activity = xml.readElementText().toStdString();
        else if (name == "AcqMode")
            mode.AcqMode = xml.readElementText().toStdString();
        else if (name == "Binning")
            mode.Binning = xml.readElementText().toStdString();
        else if (name == "Zoom")
            mode.Zoom = xml.readElementText().toStdString();
        //else if (name == "ROIColStart")
        //    mode.Index = xml.readElementText().toInt();
        //else if (name == "ROIColEnd")
        //    mode.Index = xml.readElementText().toInt();
        //else if (name == "ROIRowStart")
        //    mode.Index = xml.readElementText().toInt();
        //else if (name == "ROIRowEnd")
        //    mode.Index = xml.readElementText().toInt();
        //else if (name == "PGA")
        //    mode.Index = xml.readElementText();
        //else if (name == "VT")
        //    mode.Index = xml.readElementText().toDouble();
        //else if (name == "SequenceIntervalTime_us")
        //    mode.Index = xml.readElementText().toLongLong();
        //else if (name == "SetDelayTime_ms")
        //    mode.Index = xml.readElementText().toInt();
        //else if (name == "HwPreoffsetDiscardNumberDeforeAcq")
        //    mode.Index = xml.readElementText().toInt();
        //else if (name == "HwPreoffsetAcqNumber")
        //    mode.Index = xml.readElementText().toInt();
        //else if (name == "HwCorrectionEn")
        //    mode.Index = xml.readElementText();
        //else if (name == "FluroSync")
        //    mode.Index = xml.readElementText();
        //else if (name == "NumberOfFramesDiscardedInSeqAcq")
        //    mode.Index = xml.readElementText().toInt();
        //else if (name == "IntegrationMethod")
        //    mode.Index = xml.readElementText();
        //else if (name == "CofPGA")
        //    mode.Index = xml.readElementText();
        //else if (name == "OffsetType")
        //    mode.Index = xml.readElementText();
        else
            xml.skipCurrentElement();
    }

    return true;
}

void ApplicatioModeFileHelper::writeToXml(QXmlStreamWriter& xml)
{
}

