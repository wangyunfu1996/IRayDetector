#include "TiffHelper.h"

#include "../libtiff/include/tiff.h"
#include "../libtiff/include/tiffio.h"

TiffHelper::TiffHelper(QObject* parent)
	: QObject(parent)
{}

TiffHelper::~TiffHelper()
{}

QImage TiffHelper::ReadImage(const std::string& file_path)
{
	TIFF* tif = TIFFOpen(file_path.c_str(), "r");
	if (!tif)
	{
		return QImage();
	}

	uint16_t width = 0, height = 0;
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

	uint16_t bitsPerSample = 8, samplesPerPixel = 1, photometric = 0;
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
	TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
	TIFFGetFieldDefaulted(tif, TIFFTAG_PHOTOMETRIC, &photometric);

	// 压缩信息
	uint16_t compression = COMPRESSION_NONE;
	TIFFGetFieldDefaulted(tif, TIFFTAG_COMPRESSION, &compression);

	// 方向
	uint16_t orientation = ORIENTATION_TOPLEFT;
	TIFFGetFieldDefaulted(tif, TIFFTAG_ORIENTATION, &orientation);

	// 获取总页数
	int pageCount = 0;
	do {
		pageCount++;
	} while (TIFFReadDirectory(tif));

	QImage image;
	if (bitsPerSample == 16 && samplesPerPixel == 1) {
		image = QImage(width, height, QImage::Format_Grayscale16);
		tmsize_t scanline = TIFFScanlineSize(tif);
		uint16_t* buf = (uint16_t*)_TIFFmalloc(scanline);

		for (uint32_t row = 0; row < height; row++) {
			TIFFReadScanline(tif, buf, row, 0);
			uint16_t* imageLine = reinterpret_cast<uint16_t*>(image.scanLine(row));
			memcpy(imageLine, buf, width * sizeof(uint16_t));
		}
		_TIFFfree(buf);
	}

	return image;
}

