#include <QtWidgets/QApplication>

#include "../IRayDetector/IRayDetectorWidget.h"
#include "../IRayDetector/NDT1717MA.h"
#include "../IRayDetector/QtLogger.h"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	QtLogger::initialize();
	IRayDetectorWidget window;
	window.show();
	return app.exec();
}
