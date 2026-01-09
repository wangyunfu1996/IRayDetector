#include <QtWidgets/QApplication>

#include "IRayDetectorWidgetsApplication.h"
#include "QtLogger.h"

#include "../IRayDetector/IRayDetector.h"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	QtLogger::initialize();

	IRayDetector* pIRayDetector = new IRayDetector;
	if (pIRayDetector->Initializte() != 0)
	{
		qDebug() << "探测器初始化失败！";
		pIRayDetector->DeInitializte();
	}

	IRayDetectorWidgetsApplication window;
	window.show();
	return app.exec();
}
