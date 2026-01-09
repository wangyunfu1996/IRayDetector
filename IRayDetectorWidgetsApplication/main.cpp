#include <QtWidgets/QApplication>

#include "IRayDetectorWidgetsApplication.h"
#include "QtLogger.h"

#include "../IRayDetector/IRayDetector.h"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	QtLogger::initialize();

	IRayDetector* pIRayDetector = new IRayDetector;

	IRayDetectorWidgetsApplication window;
	window.show();
	return app.exec();
}
