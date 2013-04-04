//
// vtHeightField.h
//
// Copyright (c) 2002-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef HEIGHTFIELDH
#define HEIGHTFIELDH

#include <limits.h>			// for SHRT_MIN
#include "LocalConversion.h"

class vtBitmapBase;
#define INVALID_ELEVATION	SHRT_MIN

/**
 * This small class describes how to map elevation (as from a heightfield)
 * onto a set of colors.
 */
class ColorMap
{
public:
	ColorMap();
	bool Save(const char *fname) const;
	bool Load(const char *fname);
	void Add(float elev, const RGBi &color);
	void RemoveAt(int num);
	void Clear();
	int Num() const;
	void GenerateColors(std::vector<RGBi> &table, int iTableSize, float fMin, float fMax) const;

	bool m_bBlend;
	bool m_bRelative;
	std::vector<float> m_elev;
	std::vector<RGBi> m_color;
};


/**
 * A heightfield is any collection of surfaces such that, given a horizontal
 * X,Y position, there exists only a single elevation value.
 */
class vtHeightField
{
public:
	vtHeightField();
	virtual ~vtHeightField() {}

	// Initialize the vtHeightField
	void Initialize(const DRECT &extents, float fMinHeight, float fMaxHeight);

	virtual bool FindAltitudeOnEarth(const DPoint2 &p, float &fAltitude, bool bTrue = false) const = 0;

	/** Test if a point is within the extents of the grid. */
	bool ContainsEarthPoint(const DPoint2 &p, bool bInclusive = false) const
	{
		return m_EarthExtents.ContainsPoint(p, bInclusive);
	}

	/** Returns the geographic extents of the grid. */
	DRECT &GetEarthExtents()			{ return m_EarthExtents; }
	const DRECT &GetEarthExtents() const { return m_EarthExtents; }

	/** Set the geographic extents of the grid. */
	virtual void SetEarthExtents(const DRECT &ext);
	void GetHeightExtents(float &fMinHeight, float &fMaxHeight) const;

protected:
	// minimum and maximum height values for the whole heightfield
	float	m_fMinHeight, m_fMaxHeight;

	DRECT	m_EarthExtents;		// raw extents (geographic or projected)
};


// Culture flags: one bit for each kind of culture one could test for
// using CultureExtension.
#define CE_STRUCTURES	1
#define CE_ROADS		2
#define CE_ALL			(CE_STRUCTURES|CE_ROADS)

class CultureExtension
{
public:
	virtual bool FindAltitudeOnCulture(const FPoint3 &p3, float &fAltitude,
		bool bTrue, int iCultureFlags) const = 0;
};

/**
 * This class extents vtHeightField with the abilty to operate in 'world'
 *  coordinates, that is, an artificial meters-based 3D coordinate system
 *  which is much better suited for many tasks than trying to operate
 *  directly on the raw 2D 'earth' coordinates.
 */
class vtHeightField3d : public vtHeightField
{
public:
	vtHeightField3d();
	virtual ~vtHeightField3d() {}

	void Initialize(const LinearUnits units, const DRECT &earthextents,
		float fMinHeight, float fMaxHeight);

	// override heightfield method
	virtual void SetEarthExtents(const DRECT &ext);

	/**
	 * Given a point in world coordinates, determine the elevation at
	 * that point.
	 *
	 * \param p3 The point to test.  Only the X and Z values are used.
	 * \param fAltitude The resulting elevation at that point, by reference.
	 * \param bTrue True to test true elevation.  False to test the displayed
	 *		elevation (possibly exaggerated.)
	 * \param iCultureFlags Pass 0 to test only the heightfield itself,
	 *		non-zero to test any culture objects which may be sitting on
	 *		the heightfield.  Values include:
	 *		- CE_STRUCTURES	Test structures on the ground.
	 *		- CE_ROADS Test roads on the ground.
	 *		- CE_ALL Test everything on the ground.
	 * \param vNormal If you supply a pointer to a vector, it will be set
	 *		to the upward-pointing surface normal at the ground point.
	 *
	 * \return true if successful, false if there was nothing found at that
	 *		point.
	 */
	virtual bool FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude,
		bool bTrue = false, int iCultureFlags = 0,
		FPoint3 *vNormal = NULL) const = 0;

	/// Find the intersection point of a ray with the heightfield
	virtual bool CastRayToSurface(const FPoint3 &point, const FPoint3 &dir,
		FPoint3 &result) const = 0;

	int PointIsAboveTerrain(const FPoint3 &p) const;

	bool ConvertEarthToSurfacePoint(const DPoint2 &epos, FPoint3 &p3,
		int iCultureFlags = 0, bool bTrue = false);

	bool ContainsWorldPoint(float x, float z);
	void GetCenter(FPoint3 &center);

	void SetCulture(CultureExtension *ext) { m_pCulture = ext; }

	FRECT	m_WorldExtents;		// cooked (OpenGL) extents (in the XZ plane)
	vtLocalConversion	m_Conversion;

protected:
	void UpdateWorldExtents();

	float	m_fDiagonalLength;
	CultureExtension *m_pCulture;
};


/**
 * This class extends vtHeightField3d with the knowledge of operating
 * on a regular grid of elevation values.
 */
class vtHeightFieldGrid3d : public vtHeightField3d
{
public:
	vtHeightFieldGrid3d();
	virtual ~vtHeightFieldGrid3d() {}

	void Initialize(const LinearUnits units, const DRECT &earthextents,
		float fMinHeight, float fMaxHeight, int cols, int rows);

	// override heightfield method
	virtual void SetEarthExtents(const DRECT &ext);

	bool CastRayToSurface(const FPoint3 &point, const FPoint3 &dir,
		FPoint3 &result) const;
	bool LineOfSight(const FPoint3 &point1, const FPoint3 &point2) const;
	DPoint2 GetSpacing() const;
	FPoint2 GetWorldSpacing() const;
	void GetDimensions(int &nColumns, int &nRows) const;
	int NumColumns() { return m_iColumns; }
	int NumRows() { return m_iRows; }
	void EarthToGrid(const DPoint2 &epos, IPoint2 &ipos);
	void WorldToGrid(const FPoint3 &pos, IPoint2 &ipos);

	float GetInterpolatedElevation(double findex_x, double findex_y) const;
	int FindNumUnknown();

	// all grids must be able to return the elevation at a grid point
	virtual float GetElevation(int iX, int iZ, bool bTrue = false) const = 0;
	virtual void GetWorldLocation(int i, int j, FPoint3 &loc, bool bTrue = false) const = 0;

	bool ColorDibFromElevation(vtBitmapBase *pBM, const ColorMap *cmap,
		int iGranularity, const RGBAi &nodata, bool progress_callback(int) = NULL);
	bool ColorDibFromTable(vtBitmapBase *pBM, std::vector<RGBi> &table,
		float fMin, float fMax, const RGBAi &nodata, bool progress_callback(int) = NULL);

	void ShadeDibFromElevation(vtBitmapBase *pBM, const FPoint3 &light_dir,
		float fLightFactor, float fAmbient = 0.1f, float fGamma = 1.0f,
		bool bTrue = false, bool progress_callback(int) = NULL);
	void ShadeQuick(vtBitmapBase *pBM, float light_factor, bool bTrue = false,
		bool progress_callback(int) = NULL);
	void ShadowCastDib(vtBitmapBase *pBM, const FPoint3 &ight_dir,
		float fLightFactor, float fAmbient, bool progress_callback(int) = NULL);

protected:
	int		m_iColumns, m_iRows;
	float	m_fXStep, m_fZStep;	// step size between the World grid points
	double	m_dXStep, m_dYStep;	// step size between the Earth grid points
};

#endif	// HEIGHTFIELDH


