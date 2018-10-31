//===========================================================================
//===========================================================================
//===========================================================================
//==  Resample. Author: Costin-Anton BOIANGIU 
//===========================================================================
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
#include "stdafx.h"
#include "Resample.h"
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
typedef struct
{
	int intPixel;
	double dblWeight;
} KContribution;
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
typedef struct
{
	int intNumberOfContributors;
	KContribution *pContribution;
} KContributionArray;
//===========================================================================
//===========================================================================

/*
 *
 *    ComputeXContribution()
 *
 *    Calculates the filter weights for a single target column.
 *    contributionX->pContribution must be de-allocated afterwards.
 *
 */
//! Calculates the filter weights for a single target column
/*!
\param contributionX Receiver of contribution info
\param dblXScale Horizontal zooming scale
\param dblFilterWidth Filter sampling width
\param intDestinationWidth Target bitmap width
\param intSourceWidth Source bitmap width
\param intColumn BYTE column in source bitmap being processed
*/
//===========================================================================
//===========================================================================
static void ComputeXContribution(

	// Receiver of contribution info
	KContributionArray* contributionX,

	// Horizontal zooming scale
	double dblXScale,

	// Filter sampling width
	double dblFilterWidth,

	// Target bitmap width
	int /*intDestinationWidth*/,

	// Source bitmap width
	int intSourceWidth,

	// Filter function
	double(*FilterFunction)(double),

	// BYTE column in source bitmap being processed
	int intColumn)
{
	double dblWidth;
	double dblScale;
	double dblCenter, dblLeft, dblRight;
	double dblWeight;
	int i, j, k, intRight;

	if (dblXScale < 1.0)
	{
		// Shrinking image
		dblWidth = dblFilterWidth / dblXScale;
		dblScale = 1.0 / dblXScale;

		contributionX->intNumberOfContributors = 0;
		contributionX->pContribution = new KContribution[(int)(dblWidth * 2 + 1)];

		dblCenter = (double)intColumn / dblXScale;
		dblLeft = ceil(dblCenter - dblWidth);
		dblRight = floor(dblCenter + dblWidth);
		intRight = int(dblRight);

		for (i = (int)dblLeft; i <= intRight; i++)
		{
			dblWeight = dblCenter - (double)i;
			dblWeight = (*FilterFunction)(dblWeight / dblScale) / dblScale;
			if (i < 0)
				k = -i;
			else
				if (i >= intSourceWidth)
					k = (intSourceWidth - i) + intSourceWidth - 1;
				else
					k = i;

			j = contributionX->intNumberOfContributors++;
			contributionX->pContribution[j].intPixel = k;
			contributionX->pContribution[j].dblWeight = dblWeight;
		}

	}
	else
	{
		// Expanding image
		contributionX->intNumberOfContributors = 0;
		contributionX->pContribution = new KContribution[(int)(dblFilterWidth * 2 + 1)];

		dblCenter = (double)intColumn / dblXScale;
		dblLeft = ceil(dblCenter - dblFilterWidth);
		dblRight = floor(dblCenter + dblFilterWidth);
		intRight = int(dblRight);

		for (i = (int)dblLeft; i <= intRight; i++)
		{
			dblWeight = dblCenter - (double)i;
			dblWeight = (*FilterFunction)(dblWeight);
			if (i < 0)
				k = -i;
			else
				if (i >= intSourceWidth)
					k = (intSourceWidth - i) + intSourceWidth - 1;
				else
					k = i;

			j = contributionX->intNumberOfContributors++;
			contributionX->pContribution[j].intPixel = k;
			contributionX->pContribution[j].dblWeight = dblWeight;
		}
	}
}
//===========================================================================
//===========================================================================

/*
 *
 *    Resample(...) - Resizes bitmaps while resampling them.
 *
 */
//! Resizes bitmaps while resampling them
/*!
\param source The given source image
\param destination The given destination image
\param intFilterType The given filter type
*/
//===========================================================================
//===========================================================================
static void Resample1Channel(KImage* source, KImage* destination, int intFilterType = 0)
{
	double* temporary;
	double dblXScale, dblYScale;	        // Resample scale factors
	int intXIndex;
	int i, j, k, intRight;                // loop variables
	int intPixelNumber;			// pixel number
	double dblCenter, dblLeft, dblRight;	// filter calculation variables
	double dblWidth, dblScale, dblWeight;	// filter calculation variables
	double dblPixel1, dblPixel2;
	bool boolPixelDelta;
	KContributionArray *pContributionY;	// array of contribution lists
	KContributionArray ContributionX;
	double(*FilterFunction)(double);
	double dblFilterWidth;

	if (intFilterType < 0 || intFilterType >= sizeof(Filters) / sizeof(Filter))
		return;

	FilterFunction = Filters[intFilterType].FilterFunction;
	dblFilterWidth = Filters[intFilterType].dblFilterWidth;

	// create intermediate column to hold horizontal destination column Resample
	temporary = new double[source->GetHeight()];

	dblXScale = (double)destination->GetWidth() / (double)source->GetWidth();

	// Build y weights 
	// pre-calculate filter contribution for a column
	pContributionY = new KContributionArray[destination->GetHeight()];

	dblYScale = (double)destination->GetHeight() / (double)source->GetHeight();

	if (dblYScale < 1.0)
	{
		dblWidth = dblFilterWidth / dblYScale;
		dblScale = 1.0 / dblYScale;
		for (i = 0; i < destination->GetHeight(); i++)
		{
			pContributionY[i].intNumberOfContributors = 0;
			pContributionY[i].pContribution = new KContribution[(int)(dblWidth * 2 + 1)];

			dblCenter = (double)i / dblYScale;
			dblLeft = ceil(dblCenter - dblWidth);
			dblRight = floor(dblCenter + dblWidth);
			intRight = int(dblRight);

			for (j = (int)dblLeft; j <= intRight; j++)
			{
				dblWeight = dblCenter - (double)j;
				dblWeight = (*FilterFunction)(dblWeight / dblScale) / dblScale;
				if (j < 0)
				{
					intPixelNumber = -j;
				}
				else
					if (j >= source->GetHeight())
					{
						intPixelNumber = (source->GetHeight() - j) + source->GetHeight() - 1;
					}
					else
					{
						intPixelNumber = j;
					}

				k = pContributionY[i].intNumberOfContributors++;
				pContributionY[i].pContribution[k].intPixel = intPixelNumber;
				pContributionY[i].pContribution[k].dblWeight = dblWeight;
			}
		}
	}
	else
	{
		for (i = 0; i < destination->GetHeight(); i++)
		{
			pContributionY[i].intNumberOfContributors = 0;
			pContributionY[i].pContribution = new KContribution[(int)(dblFilterWidth * 2 + 1)];

			dblCenter = (double)i / dblYScale;
			dblLeft = ceil(dblCenter - dblFilterWidth);
			dblRight = floor(dblCenter + dblFilterWidth);
			intRight = int(dblRight);

			for (j = (int)dblLeft; j <= intRight; j++)
			{
				dblWeight = dblCenter - (double)j;
				dblWeight = (*FilterFunction)(dblWeight);
				if (j < 0)
				{
					intPixelNumber = -j;
				}
				else
					if (j >= source->GetHeight())
					{
						intPixelNumber = (source->GetHeight() - j) + source->GetHeight() - 1;
					}
					else
					{
						intPixelNumber = j;
					}

				k = pContributionY[i].intNumberOfContributors++;
				pContributionY[i].pContribution[k].intPixel = intPixelNumber;
				pContributionY[i].pContribution[k].dblWeight = dblWeight;
			}
		}
	}

	for (intXIndex = 0; intXIndex < destination->GetWidth(); intXIndex++)
	{
		ComputeXContribution(&ContributionX, dblXScale, dblFilterWidth,
			destination->GetWidth(), source->GetWidth(), FilterFunction, intXIndex);

		// Apply horizontal filter to make destination column in temporary.
		for (k = 0; k < source->GetHeight(); k++)
		{
			boolPixelDelta = false;
			dblPixel1 = source->Get8BPPPixel(ContributionX.pContribution[0].intPixel, k);
			dblWeight = dblPixel1 * ContributionX.pContribution[0].dblWeight;

			for (j = 1; j < ContributionX.intNumberOfContributors; j++)
			{
				dblPixel2 = source->Get8BPPPixel(ContributionX.pContribution[j].intPixel, k);
				if (dblPixel2 != dblPixel1)
					boolPixelDelta = true;
				dblWeight += dblPixel2 * ContributionX.pContribution[j].dblWeight;
			}

			if (boolPixelDelta)
			{
#ifdef _TRUNCATE_FILTER_PHASE_1        
				if (dblWeight < GRAYSCALE_BLACK_PIXEL)
					temporary[k] = GRAYSCALE_BLACK_PIXEL;
				else
					if (dblWeight > GRAYSCALE_WHITE_PIXEL)
						temporary[k] = GRAYSCALE_WHITE_PIXEL;
					else
#endif          
						temporary[k] = dblWeight;
			}
			else
				temporary[k] = dblPixel1;
		}
		// next row in temp column
		delete[] ContributionX.pContribution;

		// The temp column has been built. Now stretch it 
		//   vertically into destination column.
		for (i = 0; i < destination->GetHeight(); i++)
		{
			boolPixelDelta = false;
			dblPixel1 = temporary[pContributionY[i].pContribution[0].intPixel];
			dblWeight = dblPixel1 * pContributionY[i].pContribution[0].dblWeight;

			for (j = 1; j < pContributionY[i].intNumberOfContributors; j++)
			{
				dblPixel2 = temporary[pContributionY[i].pContribution[j].intPixel];
				if (dblPixel2 != dblPixel1)
					boolPixelDelta = true;
				dblWeight += dblPixel2 * pContributionY[i].pContribution[j].dblWeight;
			}

			if (boolPixelDelta)
			{
				if (dblWeight < GRAYSCALE_BLACK_PIXEL)
					destination->Put8BPPPixel(intXIndex, i, GRAYSCALE_BLACK_PIXEL);
				else
					if (dblWeight > GRAYSCALE_WHITE_PIXEL)
						destination->Put8BPPPixel(intXIndex, i, GRAYSCALE_WHITE_PIXEL);
					else
						destination->Put8BPPPixel(intXIndex, i, (BYTE)(dblWeight + 0.5));
			}
			else
				destination->Put8BPPPixel(intXIndex, i, (BYTE)(dblPixel1));
		}
		// next destination row 
	}
	// next destination column
	delete[] temporary;
	// de-allocate the memory allocated for vertical filter weights
	for (i = 0; i < destination->GetHeight(); i++)
		delete[] pContributionY[i].pContribution;
	delete[] pContributionY;
}
//===========================================================================
//===========================================================================

/*
 *
 *    Resample(...) - Resizes bitmaps while resampling them.
 *
 *    Observations: does not modify image DPI resolution
 *
 */
//! Resizes bitmaps while resampling them without modifying image DPI resolution
/*!
\param pImageSource The given image source
\param pImageDestination The image destination
\param intFilterType The given filter type
*/
//===========================================================================
//===========================================================================
void Resample(KImage* pImageSource, KImage* pImageDestination, int intFilterType)
{
	if (pImageSource->GetBPP() != 8 || pImageDestination->GetBPP() != 8)
	{
		assert(false);
		return;
	}

	Resample1Channel(pImageSource, pImageDestination, intFilterType);
}
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
long double MSE(KImage* pImageSource, KImage* pImageDestination)
{
	if (pImageSource->GetWidth() != pImageDestination->GetWidth() ||
		pImageSource->GetHeight() != pImageDestination->GetHeight() ||
		pImageSource->GetBPP() != 8 ||
		pImageDestination->GetBPP() != 8)
	{
		assert(false);
		return GRAYSCALE_PIXEL_EXT * GRAYSCALE_PIXEL_EXT;
	}

	long double dblSquareSum = 0.0;
	for (int y = pImageSource->GetHeight() - 1; y >= 0; y--)
		for (int x = pImageSource->GetWidth() - 1; x >= 0; x--)
		{
			int intDelta = int(pImageSource->Get8BPPPixel(x, y)) - int(pImageDestination->Get8BPPPixel(x, y));
			dblSquareSum += intDelta * intDelta;
		}

	return dblSquareSum / (long double(pImageSource->GetWidth()) * long double(pImageSource->GetHeight()));
}
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
long double PSNR(long double dblMSE)
{
	//Maximum may be reached at 144.52dB for two 64K x 64K 8BPP images differing by 1 pixel-value
	if (dblMSE == 0.0)
		return 150.0;
	return 10.0 * log10(long double(255.0) * long double(255.0) / dblMSE);
}
//===========================================================================
//===========================================================================
