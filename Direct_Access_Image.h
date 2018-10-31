//===========================================================================
//===========================================================================
//===========================================================================
//==  Direct_Access Image. Author: Costin-Anton BOIANGIU
//===========================================================================
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
#ifndef __DIRECT_ACCESS_IMAGE__H__
#define __DIRECT_ACCESS_IMAGE__H__
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
#include "./FreeImage/FreeImage.h"
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
#define GRAYSCALE_BLACK_PIXEL	0x00
#define GRAYSCALE_WHITE_PIXEL	0xFF
#define GRAYSCALE_PIXEL_EXT		0x100
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
#ifdef _UNICODE
inline FIBITMAP* FreeImage_Load_Wrapper(FREE_IMAGE_FORMAT fif, const _TCHAR* fileName, int flags = 0)
{
	return FreeImage_LoadU(fif, fileName, flags);
}

inline FREE_IMAGE_FORMAT FreeImage_GetFIFFromFilename_Wrapper(const _TCHAR* fileName)
{
	return FreeImage_GetFIFFromFilenameU(fileName);
}

inline FREE_IMAGE_FORMAT FreeImage_GetFileType_Wrapper(const _TCHAR* fileName, int size = 0)
{
	return FreeImage_GetFileTypeU(fileName, size);
}

inline BOOL FreeImage_Save_Wrapper(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, const _TCHAR* fileName, int flags = 0)
{
	return FreeImage_SaveU(fif, dib, fileName, flags);
}
#else
inline FIBITMAP* FreeImage_Load_Wrapper(FREE_IMAGE_FORMAT fif, const _TCHAR* fileName, int flags=0)
{
	return FreeImage_Load(fif, fileName, flags);
}

inline FREE_IMAGE_FORMAT FreeImage_GetFIFFromFilename_Wrapper(const _TCHAR* fileName)
{
	return FreeImage_GetFIFFromFilename(fileName);
}

inline FREE_IMAGE_FORMAT FreeImage_GetFileType_Wrapper(const _TCHAR* fileName, int size=0)
{
	return FreeImage_GetFileType(fileName, size);
}

inline BOOL FreeImage_Save_Wrapper(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, const _TCHAR* fileName, int flags=0)
{
	return FreeImage_Save(fif, dib, fileName, flags);
}
#endif
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
// Save formats
//===========================================================================
//===========================================================================
#define SAVE_BMP_DEFAULT			0
#define SAVE_BMP_SAVE_RLE			1

#define SAVE_EXR_DEFAULT			2
#define SAVE_EXR_FLOAT				3
#define SAVE_EXR_NONE				4
#define SAVE_EXR_ZIP				5
#define SAVE_EXR_PIZ				6
#define SAVE_EXR_PXR24				7
#define SAVE_EXR_B44				8
#define SAVE_EXR_LC					9

#define SAVE_J2K_DEFAULT			10

#define SAVE_JP2_DEFAULT			11

#define SAVE_JPEG_DEFAULT			12
#define SAVE_JPEG_QUALITYSUPERB		13
#define SAVE_JPEG_QUALITYGOOD		14
#define SAVE_JPEG_QUALITYNORMAL		15
#define SAVE_JPEG_QUALITYAVERAGE	16
#define SAVE_JPEG_QUALITYBAD		17
#define SAVE_JPEG_PROGRESSIVE		18
#define SAVE_JPEG_SUBSAMPLING_411	19
#define SAVE_JPEG_SUBSAMPLING_420	20
#define SAVE_JPEG_SUBSAMPLING_422	21
#define SAVE_JPEG_SUBSAMPLING_433	22
#define SAVE_JPEG_OPTIMIZE			23
#define SAVE_JPEG_BASELINE			24

#define SAVE_PNG_DEFAULT			25
#define SAVE_PNG_Z_BEST_SPEED		        26
#define SAVE_PNG_Z_DEFAULT_COMPRESSION	    27
#define SAVE_PNG_Z_BEST_COMPRESSION	        28
#define SAVE_PNG_Z_NO_COMPRESSION	        29
#define SAVE_PNG_INTERLACED			30

#define SAVE_PNM_DEFAULT			31
#define SAVE_PNM_SAVE_RAW			32
#define SAVE_PNM_SAVE_ASCII			33

#define SAVE_TIFF_DEFAULT			34
#define SAVE_TIFF_CMYK				35
#define SAVE_TIFF_PACKBITS			36
#define SAVE_TIFF_DEFLATE			37
#define SAVE_TIFF_ADOBE_DEFLATE		38
#define SAVE_TIFF_NONE				39
#define SAVE_TIFF_CCITTFAX3			40
#define SAVE_TIFF_CCITTFAX4			41
#define SAVE_TIFF_LZW				42
#define SAVE_TIFF_JPEG				43
#define SAVE_TIFF_LOGLUV			44

#define SAVE_TARGA_DEFAULT			45
#define SAVE_TARGA_SAVE_RLE			46
#define SAVE_NO_FORMAT				47 // always make sure it has highest values
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
struct KRGBColor
{
	BYTE b, g, r;

	KRGBColor()
	{
		r = 255;
		g = 255;
		b = 255;
	}

	KRGBColor(BYTE r, BYTE g, BYTE b)
	{
		this->r = r;
		this->g = g;
		this->b = b;
	}

	BYTE ToGrayscale()
	{
		return BYTE(double(0.299) * r + double(0.587) * g + double(0.114) * b + 0.5);
	}

	double Grayscale()
	{
		return double(0.299) * r + double(0.587) * g + double(0.114) * b;
	}
};
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
class KImage
{

protected:
	int intWidth;		// horizontal size of the image in pixels
	int intHeight;		// vertical size of the image in pixels
	unsigned intBPP;

	int intLineRasterSize;

private:
	BYTE **pDataMatrix;		// pointer to lines table of image
	FIBITMAP *fbit;
	bool boolIsValid;

public:

	//===========================================================================
	//===========================================================================
	FIBITMAP* Get_FIBITMAP()
	{
		return this->fbit;
	}

	//===========================================================================
	//===========================================================================
	void LoadFIBITMAP(FIBITMAP *fbit)
	{
		this->fbit = fbit;
		this->intHeight = FreeImage_GetHeight(this->fbit);
		this->intWidth = FreeImage_GetWidth(this->fbit);
		this->intBPP = FreeImage_GetBPP(this->fbit);

		assert(intWidth > 0 && intHeight > 0);
		assert(intBPP == 1 || intBPP == 8 || intBPP == 24);

		this->intLineRasterSize = FreeImage_GetLine(this->fbit);

		pDataMatrix = new BYTE *[intHeight];
		for (int intY = intHeight - 1; intY >= 0; intY--)
		{
			pDataMatrix[intY] = new BYTE[intLineRasterSize];
			BYTE* crtLine = FreeImage_GetScanLine(this->fbit, intHeight - 1 - intY);
			memcpy(pDataMatrix[intY], crtLine, intLineRasterSize);
		}
	}

	//===========================================================================
	//===========================================================================
	KImage(KImage &imageOther)
	{
		if (!imageOther.IsValid())
		{
			boolIsValid = false;
			return;
		}

		this->fbit = FreeImage_Clone(imageOther.Get_FIBITMAP());
		LoadFIBITMAP(this->fbit);
		boolIsValid = true;
	}

	//===========================================================================
	//===========================================================================
	KImage(FIBITMAP *fbit)
	{
		this->fbit = FreeImage_Clone(fbit);
		LoadFIBITMAP(this->fbit);
		boolIsValid = true;
	}

	//===========================================================================
	//===========================================================================
	KImage(int intSizeX, int intSizeY, int intBPP)
	{
		if (intBPP == 1)
		{
			this->fbit = FreeImage_Allocate(intSizeX, intSizeY, intBPP);

			RGBQUAD *pal = FreeImage_GetPalette(this->fbit);

			pal[0].rgbRed = 0;
			pal[0].rgbGreen = 0;
			pal[0].rgbBlue = 0;

			pal[1].rgbRed = 255;
			pal[1].rgbGreen = 255;
			pal[1].rgbBlue = 255;
		}
		else
			if (intBPP == 8)
			{
				this->fbit = FreeImage_Allocate(intSizeX, intSizeY, intBPP);

				RGBQUAD *pal = FreeImage_GetPalette(this->fbit);

				for (int i = 0; i < 0x100; i++)
				{
					pal[i].rgbRed = (BYTE)i;
					pal[i].rgbGreen = (BYTE)i;
					pal[i].rgbBlue = (BYTE)i;
				}
			}
			else
				if (intBPP == 24)
					this->fbit = FreeImage_Allocate(intSizeX, intSizeY, intBPP, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
				else
					this->fbit = FreeImage_Allocate(intSizeX, intSizeY, intBPP);

		LoadFIBITMAP(this->fbit);
		boolIsValid = true;
	}

	//===========================================================================
	//===========================================================================
	KImage(const TCHAR *strFileName)
	{
		this->boolIsValid = false;
		this->fbit = NULL;
		FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

		fif = FreeImage_GetFileType_Wrapper(strFileName, 0);

		if (fif == FIF_UNKNOWN) {
			fif = FreeImage_GetFIFFromFilename_Wrapper(strFileName);
		}

		if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
			this->fbit = FreeImage_Load_Wrapper(fif, strFileName, 0);

			LoadFIBITMAP(this->fbit);
			this->boolIsValid = true;;
		}
	}

	//===========================================================================
	//===========================================================================
	~KImage()
	{
		if (this->fbit == NULL)
		{
			assert(false);
			return;
		}

		FreeImage_Unload(this->fbit);

		if (pDataMatrix == NULL)
		{
			assert(false);
			return;
		}

		for (int intY = intHeight - 1; intY >= 0; intY--)
			delete[] pDataMatrix[intY];

		delete[] pDataMatrix;
	}

	//===========================================================================
	//===========================================================================
	int GetWidth()
	{
		return intWidth;
	}

	//===========================================================================
	//===========================================================================
	int GetHeight()
	{
		return intHeight;
	}

	//===========================================================================
	//===========================================================================
	bool IsValid()
	{
		if (this == NULL)
			return false;
		if (!boolIsValid)
			return false;
		if (intWidth <= 0 || intHeight <= 0 || (intBPP != 1 && intBPP != 8 && intBPP != 24))
			return false;
		return true;
	}

	//===========================================================================
	//===========================================================================
	unsigned GetBPP()
	{
		return intBPP;
	}

	//===========================================================================
	//===========================================================================
	BYTE **GetDataMatrix()
	{
		return pDataMatrix;
	}

	//===========================================================================
	//===========================================================================
	bool ValidateCoordinates(int x, int y)
	{
		if (x < 0 || x >= intWidth || y < 0 || y >= intHeight)
			return false;

		return true;
	}

	//===========================================================================
	//===========================================================================
	bool Get1BPPPixel(int x, int y)
	{
		assert(intBPP == 1);
#ifdef _EXTENSIVE_DEBUG_
		assert(ValidateCoordinates(x, y));
#endif

		return (((pDataMatrix[y][x >> 3] & (1 << (7 - (x & 0x07)))) == 0) ? false : true);
	}

	//===========================================================================
	//===========================================================================
	void Put1BPPPixel(int x, int y, bool boolIsWhite)
	{
		assert(intBPP == 1);
#ifdef _EXTENSIVE_DEBUG_
		assert(ValidateCoordinates(x, y));
#endif

		if (boolIsWhite)
			pDataMatrix[y][x >> 3] |= (1 << (7 - (x & 0x07)));
		else
			pDataMatrix[y][x >> 3] &= ~(1 << (7 - (x & 0x07)));
	}

	//===========================================================================
	//===========================================================================
	BYTE Get8BPPPixel(int x, int y)
	{
		assert(intBPP == 8);
#ifdef _EXTENSIVE_DEBUG_
		assert(ValidateCoordinates(x, y));
#endif

		return pDataMatrix[y][x];
	}

	//===========================================================================
	//===========================================================================
	void Put8BPPPixel(int x, int y, BYTE color)
	{
		assert(intBPP == 8);
#ifdef _EXTENSIVE_DEBUG_
		assert(ValidateCoordinates(x, y));
#endif

		pDataMatrix[y][x] = color;
	}

	//===========================================================================
	//===========================================================================
	void Get24BPPPixel(int x, int y, KRGBColor *pColor)
	{
		assert(intBPP == 24);
#ifdef _EXTENSIVE_DEBUG_
		assert(ValidateCoordinates(x, y));
#endif

		memcpy(pColor, &pDataMatrix[y][x * sizeof(KRGBColor)], sizeof(KRGBColor));
	}

	//===========================================================================
	//===========================================================================
	void Put24BPPPixel(int x, int y, KRGBColor *pColor)
	{
		assert(intBPP == 24);
#ifdef _EXTENSIVE_DEBUG_
		assert(ValidateCoordinates(x, y));
#endif

		memcpy(&pDataMatrix[y][x * sizeof(KRGBColor)], pColor, sizeof(KRGBColor));
	}

	//===========================================================================
	//===========================================================================
	void GetPixel(int x, int y, KRGBColor *pColor)
	{
		BYTE color;

		switch (intBPP)
		{
		case 1:
			color = Get1BPPPixel(x, y);
			pColor->r = pColor->g = pColor->b = color;
			break;
		case 8:
			color = Get8BPPPixel(x, y);
			pColor->r = pColor->g = pColor->b = color;
			break;
		case 24:
			Get24BPPPixel(x, y, pColor);
			break;
		default:
			assert(false);
			break;
		}
	}

	//===========================================================================
	//===========================================================================
	void PutPixel(int x, int y, KRGBColor *pColor)
	{
		switch (intBPP)
		{
		case 1:
			Put1BPPPixel(x, y, pColor->ToGrayscale() >= 0x80);
			break;
		case 8:
			Put8BPPPixel(x, y, pColor->ToGrayscale());
			break;
		case 24:
			Put24BPPPixel(x, y, pColor);
			break;
		default:
			assert(false);
			break;
		}
	}

	//===========================================================================
	//===========================================================================
	void GetResolution(int &intResolutionX, int &intResolutionY)
	{
		intResolutionX = int(FreeImage_GetDotsPerMeterX(this->fbit) / 39.37);
		intResolutionY = int(FreeImage_GetDotsPerMeterY(this->fbit) / 39.37);
	}

	//===========================================================================
	//===========================================================================
	void SetResolution(int intResolutionX, int intResolutionY)
	{
		FreeImage_SetDotsPerMeterX(this->fbit, int(intResolutionX * 39.37));
		FreeImage_SetDotsPerMeterY(this->fbit, int(intResolutionY * 39.37));
	}

	//===========================================================================
	//===========================================================================
	void ReflectCoordinates(int &x, int &y)
	{
		//reflect coordinates corresponding to OpenCV defined-style: BORDER_REFLECT_101 (gfedcb|abcdefgh|gfedcba)
		while (unsigned(x) >= unsigned(intWidth))
		{
			if (x < 0)
				x = -x;
			else
				x = ((intWidth - 1) << 1) - x;
		}

		while (unsigned(y) >= unsigned(intHeight))
		{
			if (y < 0)
				y = -y;
			else
				y = ((intHeight - 1) << 1) - y;
		}
	}

	//===========================================================================
	//===========================================================================
	bool Reflected_Get1BPPPixel(int x, int y)
	{
		assert(intBPP == 1);
		ReflectCoordinates(x, y);
		return (((pDataMatrix[y][x >> 3] & (1 << (7 - (x & 0x07)))) == 0) ? false : true);
	}

	//===========================================================================
	//===========================================================================
	BYTE Reflected_Get8BPPPixel(int x, int y)
	{
		assert(intBPP == 8);
		ReflectCoordinates(x, y);
		return pDataMatrix[y][x];
	}

	//===========================================================================
	//===========================================================================
	void Reflected_Get24BPPPixel(int x, int y, KRGBColor *pColor)
	{
		assert(intBPP == 24);
		ReflectCoordinates(x, y);
		memcpy(pColor, &pDataMatrix[y][x * sizeof(KRGBColor)], sizeof(KRGBColor));
	}

	//===========================================================================
	//===========================================================================
	void Reflected_GetPixel(int x, int y, KRGBColor *pColor)
	{
		BYTE color;
		ReflectCoordinates(x, y);

		switch (intBPP)
		{
		case 1:
			color = Reflected_Get1BPPPixel(x, y);
			pColor->r = pColor->g = pColor->b = color;
			break;
		case 8:
			color = Reflected_Get8BPPPixel(x, y);
			pColor->r = pColor->g = pColor->b = color;
			break;
		case 24:
			Reflected_Get24BPPPixel(x, y, pColor);
			break;
		default:
			assert(false);
			break;
		}
	}

	//===========================================================================
	//===========================================================================
	void SaveAs(const TCHAR *strFileName, unsigned intFormatType = SAVE_TIFF_DEFAULT);
	//===========================================================================
	//===========================================================================
	void Crop(int top, int bottom, int left, int right);
	//===========================================================================
	//===========================================================================

	//===========================================================================
	//===========================================================================
	KImage* ConvertToGreyscale()
	{
		FIBITMAP* grey;
		switch (intBPP)
		{
		case 1:
			return NULL;
		case 8:
			return new KImage(*this);
		case 24:
			grey = FreeImage_ConvertToGreyscale(this->fbit);
			if (grey == NULL)
				return NULL;
			return new KImage(grey);
		default:
			return NULL;
		}
	}

	//===========================================================================
	//===========================================================================
	KImage* Rotate(double angle, const void* bkcolor = 0)
	{
		FIBITMAP* new_fbit = FreeImage_Rotate(this->fbit, angle, bkcolor);
		if (new_fbit == NULL)
			return NULL;

		return new KImage(new_fbit);
	}
	//===========================================================================
	//===========================================================================

	//===========================================================================
	//===========================================================================
	static void __GaussianBlurOneChannel(int intImageWidth, int intImageHeight,
		BYTE ** pLineInput, BYTE ** pLineOutput, double dblRadius);
	//===========================================================================
	//===========================================================================
	bool GaussianBlur(double dblRadius);
	//===========================================================================
	//===========================================================================
};
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
#endif //__DIRECT_ACCESS_IMAGE__H__
//===========================================================================
//===========================================================================
