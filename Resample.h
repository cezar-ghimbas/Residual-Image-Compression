//===========================================================================
//===========================================================================
//===========================================================================
//==  Resample. Author: Costin-Anton BOIANGIU 
//===========================================================================
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
#ifndef __RESAMPLE__H__
#define __RESAMPLE__H__
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
#include "Direct_Access_Image.h"
//===========================================================================
//===========================================================================

/*
 *
 *    Filter Function Definitions
 *
 */

/*

Summary:

- Horizontal filter contribution are calculated on the fly,
as each column is mapped from source to destination image. This lets
us omit having to allocate a temporary full horizontal stretch
of the source image.

- If none of the source pixels within a sampling region differ,
then the output pixel is forced to equal (any of) the source pixel.
This ensures that filters do not corrupt areas of constant color.

- Filter weight contribution results, after summing, are
rounded to the nearest pixel color value instead of
being casted to BYTE (usually an int or char). Otherwise,
artifacting occurs.

*/

/*
*
*    Filter Function Definitions
*
*/

//===========================================================================
//===========================================================================
#define	BoxFilterWidth		(0.5)
#define	HermiteFilterWidth	(1.0)
#define	TriangleFilterWidth	(1.0)
#define	BellFilterWidth		(1.5)
#define	BSplineFilterWidth	(2.0)
#define	Lanczos3FilterWidth	(3.0)
#define	MitchellFilterWidth	(2.0)
#define	__B__	                (1.0 / 3.0)
#define	__C__	                (1.0 / 3.0)
//===========================================================================
//===========================================================================

//----------------------------------------------------------
//! Compute the Box filter function
/*!
\param t The given value to compute on
\return function result
*/
static double BoxFilterFunction(double t)
{
	if (t > -0.5 && t <= 0.5)
		return 1.0;

	return 0.0;
}

//----------------------------------------------------------
//! Compute the Hermite filter function
/*!
\param t The given value to compute on
\return function result
*/
static double HermiteFilterFunction(double t)
{
	// f(t) = 2|t|^3 - 3|t|^2 + 1, -1 <= t <= 1
	if (t < 0.0)
		t = -t;

	if (t < 1.0)
		return (2.0 * t - 3.0) * t * t + 1.0;

	return 0.0;
}

//----------------------------------------------------------
//! Compute the Triangle filter function
/*!
\param t The given value to compute on
\return function result
*/
static double TriangleFilterFunction(double t)
{
	if (t < 0.0)
		t = -t;

	if (t < 1.0)
		return 1.0 - t;

	return 0.0;
}

//----------------------------------------------------------
//! Compute the Bell filter function
/*!
\param t The given value to compute on
\return function result
*/
static double BellFilterFunction(double t)
// box (*) box (*) box
{
	if (t < 0)
		t = -t;

	if (t < 0.5)
		return 0.75 - (t * t);

	if (t < 1.5)
	{
		t -= 1.5;
		return 0.5 * (t * t);
	}

	return 0.0;
}

//----------------------------------------------------------
//! Compute the Spline filter function
/*!
\param t The given value to compute on
\return function result
*/
static double BSplineFilterFunction(double t)
// box (*) box (*) box (*) box
{
	double tt;

	if (t < 0)
		t = -t;

	if (t < 1)
	{
		tt = t * t;
		return (0.5 * tt * t) - tt + (2.0 / 3.0);
	}
	else
		if (t < 2)
		{
			t = 2 - t;
			return (1.0 / 6.0) * (t * t * t);
		}

	return 0.0;
}

//----------------------------------------------------------
//! Compute the sinc function value
/*!
\param t The given value to compute on
\return function result
*/
static inline double sinc(double x)
{
	x *= M_PI;

	if (x != 0)
		return sin(x) / x;

	return 1.0;
}

//----------------------------------------------------------
//! Compute the Lanczos filter function
/*!
\param t The given value to compute on
\return function result
*/
static double Lanczos3FilterFunction(double t)
{
	if (t < 0)
		t = -t;

	if (t < 3.0)
		return sinc(t) * sinc(t / 3.0);

	return 0.0;
}

//----------------------------------------------------------
//! Compute the Mitchell filter function
/*!
\param t The given value to compute on
\return function result
*/
static double MitchellFilterFunction(double t)
{
	double tt;

	tt = t * t;

	if (t < 0)
		t = -t;

	if (t < 1.0)
	{
		t = (((12.0 - 9.0 * __B__ - 6.0 * __C__) * (t * tt)) +
			((-18.0 + 12.0 * __B__ + 6.0 * __C__) * tt) +
			(6.0 - 2 * __B__));

		return t / 6.0;
	}
	else
		if (t < 2.0)
		{
			t = (((-1.0 * __B__ - 6.0 * __C__) * (t * tt)) +
				((6.0 * __B__ + 30.0 * __C__) * tt) +
				((-12.0 * __B__ - 48.0 * __C__) * t) +
				(8.0 * __B__ + 24 * __C__));

			return t / 6.0;
		}

	return 0.0;
}

//===========================================================================
//===========================================================================
typedef struct
{
	double(*FilterFunction)(double);
	double dblFilterWidth;
} Filter;
//===========================================================================
//===========================================================================
static Filter Filters[] =
{
	{ BoxFilterFunction, BoxFilterWidth },
	{ HermiteFilterFunction, HermiteFilterWidth },
	{ TriangleFilterFunction, TriangleFilterWidth },
	{ BellFilterFunction, BellFilterWidth },
	{ BSplineFilterFunction, BSplineFilterWidth },
	{ Lanczos3FilterFunction, Lanczos3FilterWidth },
	{ MitchellFilterFunction, MitchellFilterWidth }
};
//===========================================================================
//===========================================================================
#define FILTER_BOX			0
#define FILTER_HERMITE		1
#define FILTER_TRIANGLE		2
#define FILTER_BELL			3
#define FILTER_BSPLINE		4
#define FILTER_LANCZOS3		5
#define FILTER_MITCHELL		6
//===========================================================================
//===========================================================================
#define NUMBER_OF_FILTERS	(sizeof(Filters) / sizeof(Filters[0]))
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
void Resample(KImage* pImageSource, KImage* pImageDestination, int intFilterType);
long double MSE(KImage* pImageSource, KImage* pImageDestination);
long double PSNR(long double dblMSE);
//===========================================================================
//===========================================================================

#endif
/*! \} */
