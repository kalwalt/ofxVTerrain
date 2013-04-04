//
// HeightField.cpp
//
// Copyright (c) 2001-2009 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "HeightField.h"
#include "vtDIB.h"
#include "vtLog.h"
#include "FilePath.h"

//
// Class implementation: ColorMap
//
ColorMap::ColorMap()
{
	m_bBlend = true;
	m_bRelative = true;
}

bool ColorMap::Save(const char *fname) const
{
	// watch out for %f
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	FILE *fp = vtFileOpen(fname, "wb");
	if (!fp)
		return false;
	fprintf(fp, "colormap1\n");
	fprintf(fp, "blend: %d\n", m_bBlend);
	fprintf(fp, "relative: %d\n", m_bRelative);
	int size = m_elev.size();
	fprintf(fp, "size %d\n", size);
	for (int i = 0; i < size; i++)
	{
		fprintf(fp, "\telev %f color %d %d %d\n", m_elev[i],
			m_color[i].r, m_color[i].g, m_color[i].b);
	}
	fclose(fp);
	return true;
}

bool ColorMap::Load(const char *fname)
{
	// watch out for %f
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)
		return false;

	char buf[80];
	fgets(buf, 80, fp);
	if (strncmp(buf, "colormap1", 9))
		return false;

	while (fgets(buf, 80, fp) != NULL)
	{
		int ival;
		if (!strncmp(buf, "blend", 5))
		{
			sscanf(buf, "blend: %d\n", &ival);
			m_bBlend = (ival != 0);
		}

		else if (!strncmp(buf, "relative", 8))
		{
			sscanf(buf, "relative: %d\n", &ival);
			m_bRelative = (ival != 0);
		}

		else if (!strncmp(buf, "size", 4))
		{
			int size;
			sscanf(buf, "size %d\n", &size);

			m_elev.resize(size);
			m_color.resize(size);
			for (int i = 0; i < size; i++)
			{
				float f;
				short r, g, b;
				fscanf(fp, "\telev %f color %hd %hd %hd\n", &f, &r, &g, &b);
				m_elev[i] = f;
				m_color[i].Set(r, g, b);
			}
		}
	}
	fclose(fp);
	return true;
}

/**
 * Add a color entry, keeping the elevation values sorted.
 */
void ColorMap::Add(float elev, const RGBi &color)
{
	float next, previous = -1E9;
	int size = m_elev.size();
	for (int i = 0; i < size+1; i++)
	{
		if (i < size)
			next = m_elev[i];
		else
			next = 1E9;
		if (previous <= elev && elev <= next)
		{
			m_elev.insert(m_elev.begin() + i, elev);
			m_color.insert(m_color.begin() + i, color);
			return;
		}
	}
}

void ColorMap::RemoveAt(int num)
{
	m_elev.erase(m_elev.begin()+num);
	m_color.erase(m_color.begin()+num);
}

void ColorMap::Clear()
{
	m_elev.clear();
	m_color.clear();
}

int ColorMap::Num() const
{
	return m_elev.size();
}

/**
 * Generate an array of interpolated colors from this ColorMap.
 *
 * \param table An empty table ready to fill with the interpolated colors.
 * \param iTableSize The desired number of elements in the resulting table.
 * \param fMin, fMax The elevation range to interpolate over.
 */
void ColorMap::GenerateColors(std::vector<RGBi> &table, int iTableSize,
							  float fMin, float fMax) const
{
	if (m_color.size() < 2)
		return;

	float fRange = fMax - fMin;
	float step = fRange/iTableSize;

	int current = 0;
	int num = Num();
	RGBi c1, c2;
	float base=0, next, bracket_size=0, fraction;

	if (m_bRelative == true)
	{
		bracket_size = fRange / (num - 1);
		current = -1;
	}

	RGBi c3;
	for (int i = 0; i < iTableSize; i++)
	{
		float elev = fMin + (step * i);
		if (m_bRelative)
		{
			// use regular divisions
			int bracket = (int) ((elev-fMin) / fRange * (num-1));
			if (bracket != current)
			{
				current = bracket;
				base = fMin + bracket * bracket_size;
				c1 = m_color[current];
				c2 = m_color[current+1];
			}
		}
		else
		{
			// use absolute elevations
			while (current < num-1 && elev >= m_elev[current])
			{
				c1 = m_color[current];
				c2 = m_color[current+1];
				base = m_elev[current];
				next = m_elev[current+1];
				bracket_size = next - base;
				current++;
			}
		}
		if (m_bBlend)
		{
			fraction = (elev - base) / bracket_size;
			c3 = c1 * (1-fraction) + c2 * fraction;
		}
		else
			c3 = c1;
		table.push_back(c3);
	}

	// Add one to catch top data
	c3 = table[iTableSize-1];
	table.push_back(c3);
}

/////////////////////

vtHeightField::vtHeightField()
{
	m_EarthExtents.SetRect(0, 0, 0, 0);
	m_fMinHeight = INVALID_ELEVATION;
	m_fMaxHeight = INVALID_ELEVATION;
}

void vtHeightField::Initialize(const DRECT &extents, float fMinHeight,
							   float fMaxHeight)
{
	m_EarthExtents = extents;
	m_fMinHeight = fMinHeight;
	m_fMaxHeight = fMaxHeight;
}

void vtHeightField::SetEarthExtents(const DRECT &ext)
{
	m_EarthExtents = ext;
}

/**
 * Gets the minimum and maximum height values.  The values are placed in the
 * arguments by reference.  You must have first called ComputeHeightExtents.
 */
void vtHeightField::GetHeightExtents(float &fMinHeight, float &fMaxHeight) const
{
	fMinHeight = m_fMinHeight;
	fMaxHeight = m_fMaxHeight;
}


/**
 * \return 0 if below terrain, 1 if above terrain, -1 if off terrain.
 */
int vtHeightField3d::PointIsAboveTerrain(const FPoint3 &p) const
{
	float alt;
	if (!FindAltitudeAtPoint(p, alt))
		return -1;
	if (alt < p.y)
		return 1;
	else
		return 0;
}


/**
 * Converts a earth coordinate (project or geographic) to a world coordinate
 * on the surface of the heightfield.
 *
 * \return true if successful, false if there was no elevation at that point.
 */
bool vtHeightField3d::ConvertEarthToSurfacePoint(const DPoint2 &epos,
									FPoint3 &p3, int iCultureFlags, bool bTrue)
{
	// convert earth -> XZ
	m_Conversion.ConvertFromEarth(epos, p3.x, p3.z);

	// look up altitude
	return FindAltitudeAtPoint(p3, p3.y, bTrue, iCultureFlags);
}

/**
 * Tests whether a given point is within the current terrain
 */
bool vtHeightField3d::ContainsWorldPoint(float x, float z)
{
	const FRECT &we = m_WorldExtents;
	return (x > we.left && x < we.right && z < we.bottom && z > we.top);
}


void vtHeightField3d::GetCenter(FPoint3 &center)
{
	FPoint2 c;
	m_WorldExtents.Center(c);
	center.x = c.x;
	center.z = c.y;
	center.y = 0.0f;
}


/////////////////////////////////////////////////////////////////////////////

vtHeightField3d::vtHeightField3d()
{
	m_pCulture = NULL;
}

void vtHeightField3d::Initialize(const LinearUnits units,
	const DRECT &earthextents, float fMinHeight, float fMaxHeight)
{
	vtHeightField::Initialize(earthextents, fMinHeight, fMaxHeight);

	m_Conversion.Setup(units, m_EarthExtents);
	UpdateWorldExtents();
}

void vtHeightField3d::SetEarthExtents(const DRECT &ext)
{
	vtHeightField::SetEarthExtents(ext);
	m_Conversion.SetOrigin(DPoint2(m_EarthExtents.left, m_EarthExtents.bottom));
	UpdateWorldExtents();
}

void vtHeightField3d::UpdateWorldExtents()
{
	m_Conversion.convert_earth_to_local_xz(
		m_EarthExtents.left, m_EarthExtents.bottom,
		m_WorldExtents.left, m_WorldExtents.bottom);

	m_Conversion.convert_earth_to_local_xz(
		m_EarthExtents.right, m_EarthExtents.top,
		m_WorldExtents.right, m_WorldExtents.top);

	FPoint2 hypo(m_WorldExtents.Width(), m_WorldExtents.Height());
	m_fDiagonalLength = hypo.Length();
}


/////////////////////////////////////////////////////////////////////////////
// vtHeightFieldGrid3d

vtHeightFieldGrid3d::vtHeightFieldGrid3d()
{
	m_iColumns = 0;
	m_iRows = 0;
	m_fXStep = 0.0f;
	m_fZStep = 0.0f;
}

void vtHeightFieldGrid3d::Initialize(const LinearUnits units,
	const DRECT& earthextents, float fMinHeight, float fMaxHeight,
	int cols, int rows)
{
	// first initialize parent
	vtHeightField3d::Initialize(units, earthextents, fMinHeight, fMaxHeight);

	m_iColumns = cols;
	m_iRows = rows;

	m_fXStep = m_WorldExtents.Width() / (m_iColumns-1);
	m_fZStep = -m_WorldExtents.Height() / (m_iRows-1);

	m_dXStep = m_EarthExtents.Width() / (m_iColumns-1);
	m_dYStep = m_EarthExtents.Height() / (m_iRows-1);
}

void vtHeightFieldGrid3d::SetEarthExtents(const DRECT &ext)
{
	vtHeightField3d::SetEarthExtents(ext);

	// update step values
	m_dXStep = m_EarthExtents.Width() / (m_iColumns-1);
	m_dYStep = m_EarthExtents.Height() / (m_iRows-1);
}

/** Get the grid spacing, the width of each column and row.
 */
DPoint2 vtHeightFieldGrid3d::GetSpacing() const
{
	return DPoint2(m_dXStep, m_dYStep);
}

FPoint2 vtHeightFieldGrid3d::GetWorldSpacing() const
{
	return FPoint2(m_fXStep, m_fZStep);
}

/** Get the grid size of the grid.
 * The values are placed into the arguments by reference.
 * \param nColumns The number of columns (east-west)
 * \param nRows THe number of rows (north-south)
 */
void vtHeightFieldGrid3d::GetDimensions(int &nColumns, int &nRows) const
{
	nColumns = m_iColumns;
	nRows = m_iRows;
}

/** Convert a point from earth coordinates to the integer coordinates
 * of the grid.
 */
void vtHeightFieldGrid3d::EarthToGrid(const DPoint2 &epos, IPoint2 &ipos)
{
	ipos.x = (int) ((epos.x - m_EarthExtents.left) / m_dXStep);
	ipos.y = (int) ((epos.y - m_EarthExtents.bottom) / m_dYStep);
}

/** Convert a point from world coordinates to the integer coordinates
 * of the grid.
 */
void vtHeightFieldGrid3d::WorldToGrid(const FPoint3 &pos, IPoint2 &ipos)
{
	ipos.x = (int) ((pos.x - m_WorldExtents.left) / m_fXStep);
	ipos.y = (int) ((m_WorldExtents.bottom - pos.z) / m_fZStep);
}

/**
 * Get the interpolated height of the grid at a specific grid coordinate,
 * where the coordinates can be non-integer; the result is interpolated
 * between the source heixels when possible (i.e. not at the edge)
 *
 * \param findex_x Floating point index, from 0 to width in heixels.
 * \param findex_y Floating point index, from 0 to height in heixels.
 */
float vtHeightFieldGrid3d::GetInterpolatedElevation(double findex_x, double findex_y) const
{
	// Require the point to be inside the grid
	if (findex_x < 0 || findex_x > m_iColumns-1)
		return INVALID_ELEVATION;
	if (findex_y < 0 || findex_y > m_iRows-1)
		return INVALID_ELEVATION;

	int index_x = (int) findex_x;
	int index_y = (int) findex_y;

	float diff_x = (float) (findex_x - index_x);
	float diff_y = (float) (findex_y - index_y);

	if (index_x == m_iColumns-1)
	{
		// On right edge
		index_x --;
		diff_x = 1.0f;
	}
	if (index_y == m_iRows-1)
	{
		// On top edge
		index_y --;
		diff_y = 1.0f;
	}

	float fDataBL = GetElevation(index_x, index_y);
	float fDataBR = GetElevation(index_x+1, index_y);
	float fDataTL = GetElevation(index_x, index_y+1);
	float fDataTR = GetElevation(index_x+1, index_y+1);

	int valid = 0;
	if (fDataBL != INVALID_ELEVATION)
		valid++;
	if (fDataBR != INVALID_ELEVATION)
		valid++;
	if (fDataTL != INVALID_ELEVATION)
		valid++;
	if (fDataTR != INVALID_ELEVATION)
		valid++;

	float fData;
	if (valid == 4)	// all valid
	{
		// do bilinear filtering
		fData = (float) (fDataBL +
				(fDataBR-fDataBL)*diff_x +
				(fDataTL-fDataBL)*diff_y +
				(fDataTR-fDataTL-fDataBR+fDataBL)*diff_x*diff_y);
	}
	else if (valid > 0)
	{
		// Look for closest valid nearest neighbor
		float dist[4];
		float value[4];

		value[0] = fDataBL;
		value[1] = fDataBR;
		value[2] = fDataTL;
		value[3] = fDataTR;

		if (fDataBL != INVALID_ELEVATION)
			dist[0] = fabs(diff_x*diff_x) + fabs(diff_y*diff_y);
		else
			dist[0] = 3;	// Not valid, use a value > 2

		if (fDataBR != INVALID_ELEVATION)
			dist[1] = fabs((1-diff_x)*(1-diff_x)) + fabs(diff_y*diff_y);
		else
			dist[1] = 3;

		if (fDataTL != INVALID_ELEVATION)
			dist[2] = fabs(diff_x*diff_x) + fabs((1-diff_y)*(1-diff_y));
		else
			dist[2] = 3;

		if (fDataTR != INVALID_ELEVATION)
			dist[3] = fabs((1-diff_x)*(1-diff_x)) + fabs((1-diff_y)*(1-diff_y));
		else
			dist[3] = 3;

		float closest = 4;
		int closest_index;
		for (int i = 0; i < 4; i++)
		{
			if (dist[i] < closest)
			{
				closest = dist[i];
				closest_index = i;
			}
		}
		fData = value[closest_index];
	}
	else
		fData = INVALID_ELEVATION;

	return fData;
}

/**
 * Count the number of unknown (invalid) heixels in this grid.
 */
int vtHeightFieldGrid3d::FindNumUnknown()
{
	int count = 0;
	for (int i = 0; i < m_iColumns; i++)
		for (int j = 0; j < m_iRows; j++)
			if (GetElevation(i, j) == INVALID_ELEVATION)
				count++;
	return count;
}

/**
 * Tests a ray against a heightfield grid.
 *
 * Note: This algorithm is not guaranteed to give absolutely correct results,
 * but it is reasonably fast and efficient.  It checks a series of points
 * along the ray against the terrain.  When a pair of points (segment) is
 * found to straddle the terrain, it refines the segment in a binary fashion.
 *
 * Since the length of the test is proportional to a single grid element,
 * there is a small chance that it will give results that are off by a small
 * distance (less than 1 grid element)
 *
 * \return true if hit terrain.  The resulting point of intersection is
 *		placed in the 'result' argument.
 */
bool vtHeightFieldGrid3d::CastRayToSurface(const FPoint3 &point,
										   const FPoint3 &dir, FPoint3 &result) const
{
	float alt;
	bool bOn = FindAltitudeAtPoint(point, alt);

	// special case: straight up or down
	float mag2 = sqrt(dir.x*dir.x+dir.z*dir.z);
	if (fabs(mag2) < .000001)
	{
		result = point;
		result.y = alt;
		if (!bOn)
			return false;
		if (dir.y > 0)	// points up
			return (point.y < alt);
		else
			return (point.y > alt);
	}

	if (bOn && point.y < alt)
		return false;	// already firmly underground

	// adjust magnitude of dir until 2D component has a good magnitude
	float smallest = std::min(m_fXStep, m_fZStep);
	float adjust = smallest / mag2;
	FPoint3 dir2 = dir * adjust;

	bool found_above = false;
	FPoint3 p = point, lastp = point;
	while (true)
	{
		// are we out of bounds and moving away?
		if (p.x < m_WorldExtents.left && dir2.x < 0)
			return false;
		if (p.x > m_WorldExtents.right && dir2.x > 0)
			return false;
		if (p.z < m_WorldExtents.top && dir2.z < 0)
			return false;
		if (p.z > m_WorldExtents.bottom && dir2.z > 0)
			return false;

		bOn = FindAltitudeAtPoint(p, alt);
		if (bOn)
		{
			if (p.y > alt)
				found_above = true;
			else
				break;
		}
		lastp = p;
		p += dir2;
	}
	if (!found_above)
		return false;

	// now, do a binary search to refine the result
	FPoint3 p0 = lastp, p1 = p, p2;
	for (int i = 0; i < 10; i++)
	{
		p2 = (p0 + p1) / 2.0f;
		int above = PointIsAboveTerrain(p2);
		if (above == 1)	// above
			p0 = p2;
		else if (above == 0)	// below
			p1 = p2;
	}
	p2 = (p0 + p1) / 2.0f;
	// make sure it's precisely on the ground
	FindAltitudeAtPoint(p2, p2.y);
	result = p2;
	return true;
}


/**
 * \return true if a line of sight exists between point1 and point2.
 */
bool vtHeightFieldGrid3d::LineOfSight(const FPoint3 &point1,
									  const FPoint3 &point2) const
{
	float alt;
	bool bOn;

	// first check if either point is below ground
	bOn = FindAltitudeAtPoint(point1, alt);
	if (bOn && point1.y < alt)
		return false;
	bOn = FindAltitudeAtPoint(point2, alt);
	if (bOn && point2.y < alt)
		return false;

	// special case: straight up or down
	FPoint3 dir = point2 - point1;
	float mag2 = sqrt(dir.x*dir.x+dir.z*dir.z);
	if (fabs(mag2) < .000001)
		return true;

	// adjust magnitude of dir until 2D component has a good magnitude
	float smallest = std::min(m_fXStep, m_fZStep);
	int steps = (int) (mag2 / smallest) + 1;
	if (steps < 2)
		steps = 2;
	dir /= (float) steps;

	FPoint3 p = point1;
	for (int i = 0; i < steps+1; i++)
	{
		bOn = FindAltitudeAtPoint(p, alt);
		if (bOn && p.y < alt)	// hit the ground
			return false;
		p += dir;
	}
	return true;	// visible, didn't hit the ground
}

/**
 * Use the height data in the grid to fill a bitmap with colors.
 *
 * \param pBM			The bitmap to be colored.
 * \param cmap			The mapping of elevation values to colors.
 * \param iGranularity  The smoothness of the mapping, expressed as the size
 *			of the internal mapping table.  2000 is a generally good value.
 * \param nodata		The color to use for NODATA areas, where there are no elevation values.
 * \param progress_callback If supplied, this function will be called back
 *			with a value of 0 to 100 as the operation progresses.
 *
 * \return true if any invalid elevation values were encountered.
 */
bool vtHeightFieldGrid3d::ColorDibFromElevation(vtBitmapBase *pBM,
	const ColorMap *cmap, int iGranularity, const RGBAi &nodata,
	bool progress_callback(int))
{
	if (!pBM || !cmap)
		return false;

	VTLOG1("ColorDibFromElevation:");

	float fMin, fMax;
	GetHeightExtents(fMin, fMax);
	float fRange = fMax - fMin;
	bool bFlat = (fRange < 0.0001f);
	if (bFlat)
	{
		// avoid numeric trouble with flat terrains by growing range
		fMin -= 1;
		fMax += 1;
		fRange = fMax - fMin;
	}

	VTLOG(" table of %d values, first [%d %d %d],\n",
		cmap->Num(), cmap->m_color[0].r, cmap->m_color[0].g, cmap->m_color[0].b);
	VTLOG("\tmin %g, max %g, range %g, granularity %d\n",
		fMin, fMax, fRange, iGranularity);

	// Rather than look through the color map for each pixel, pre-build
	//  a color lookup table once - should be faster in nearly all cases.
	std::vector<RGBi> table;
	cmap->GenerateColors(table, iGranularity, fMin, fMax);

	return ColorDibFromTable(pBM, table, fMin, fMax, nodata, progress_callback);
}

/**
 * Use the height data in the grid and a colormap fill a bitmap with colors.
 * Any undefined heixels in the source will be fill with red (255,0,0).
 *
 * \param pBM			The bitmap to be colored.
 * \param table			The table of colors.
 * \param fMin, fMax	The range of valid elevation values expect in the input.
 * \param nodata		The color to use for NODATA areas, where there are no elevation values.
 * \param progress_callback If supplied, this function will be called back
 *			with a value of 0 to 100 as the operation progresses.
 *
 * \return true if any invalid elevation values were encountered.
 */
bool vtHeightFieldGrid3d::ColorDibFromTable(vtBitmapBase *pBM,
	   std::vector<RGBi> &table, float fMin, float fMax, const RGBAi &nodata,
	   bool progress_callback(int))
{
	VTLOG1(" ColorDibFromTable:");
	int w = pBM->GetWidth();
	int h = pBM->GetHeight();
	int depth = pBM->GetDepth();
	int gw, gh;
	GetDimensions(gw, gh);

	VTLOG(" dib size %d x %d, grid %d x %d.. ", w, h, gw, gh);

	bool bExact = (w == gw && h == gh);
	double ratiox = (double)(gw-1)/(w-1), ratioy = (double)(gh-1)/(h-1);

	float fRange = fMax - fMin;
	uint iGranularity = table.size()-1;
	bool has_invalid = false;
	RGBi nodata_24bit(nodata.r, nodata.g, nodata.b);
	double x, y;
	float elev;

	// now iterate over the texels
	for (int i = 0; i < w; i++)
	{
		if (progress_callback != NULL)
		{
			if ((i&7) == 0)
				progress_callback(i * 100 / w);
		}
		x = i * ratiox;		// find corresponding location in height grid

		for (int j = 0; j < h; j++)
		{
			y = j * ratioy;

			if (bExact)
				elev = GetElevation(i, j);
			else
				elev = GetInterpolatedElevation(x, y);
			if (elev == INVALID_ELEVATION)
			{
				if (depth == 32)
					pBM->SetPixel32(i, h-1-j, nodata);
				else
					pBM->SetPixel24(i, h-1-j, nodata_24bit);
				has_invalid = true;
				continue;
			}
			uint table_entry = (uint) ((elev - fMin) / fRange * iGranularity);
			if (table_entry > iGranularity-1)
				table_entry = iGranularity-1;
			if (depth == 32)
				pBM->SetPixel32(i, h-1-j, table[table_entry]);
			else
				pBM->SetPixel24(i, h-1-j, table[table_entry]);
		}
	}
	VTLOG("Done.\n");
	return has_invalid;
}

/**
 * Perform simple shading of a bitmap, based on this grid's elevation values.
 * Lighting is computing using the dot product of the surface normal with
 * the light direction.  This is often called "dot-product lighting".
 *
 * \param pBM	The bitmap to shade.
 * \param light_dir	Direction vector of the light.
 * \param fLightFactor Value from 0 (no shading) to 1 (full shading)
 * \param fAmbient Ambient light values from 0 to 1, a typical value is 0.1.
 * \param fGamma Gamma values from 0 to 1, values less than 1 boost the brightness curve.
 * \param bTrue	If true, use the real elevation values, ignoring vertical exaggeration.
 * \param progress_callback	If supplied, will be called with values from 0 to 100.
 */
void vtHeightFieldGrid3d::ShadeDibFromElevation(vtBitmapBase *pBM, const FPoint3 &light_dir,
	float fLightFactor, float fAmbient, float fGamma, bool bTrue, bool progress_callback(int))
{
	// consider upward-pointing normal vector, rather than downward-pointing
	FPoint3 light_direction = -light_dir;

	int w = pBM->GetWidth();
	int h = pBM->GetHeight();
	int gw = m_iColumns, gh = m_iRows;

	double ratiox = (double)(gw-1)/(w-1), ratioy = (double)(gh-1)/(h-1);

	// For purposes of shading, we need to look at adjacent heixels which are
	//  at least one grid cell away:
	int xOffset = (int)ratiox;
	int yOffset = (int)ratioy;
	if (xOffset < 1) xOffset = 1;
	if (yOffset < 1) yOffset = 1;

	int depth = pBM->GetDepth();
	int x, y;

	// Center, Left, Right, Top, Bottom
	FPoint3 c, l, r, t, b, v3;

	// iterate over the texels
	for (int j = 0; j < h; j++)
	{
		if (progress_callback != NULL)
		{
			if ((j&7) == 0)
				progress_callback(j * 100 / h);
		}
		// find corresponding location in terrain
		y = (int) (j * ratioy);
		for (int i = 0; i < w; i++)
		{
			x = (int) (i * ratiox);

			GetWorldLocation(x, y, c, bTrue);
			if (c.y == INVALID_ELEVATION)
				continue;

			// Check to see what surrounding values are valid
			GetWorldLocation(x-xOffset, y, l, bTrue);
			GetWorldLocation(x+xOffset, y, r, bTrue);
			GetWorldLocation(x, y+yOffset, t, bTrue);
			GetWorldLocation(x, y-yOffset, b, bTrue);

			FPoint3 p1, p2, p3, p4;

			// compute surface normal
			if (l.y != INVALID_ELEVATION)
				p1 = l;
			else
				p1 = c;

			if (r.y != INVALID_ELEVATION)
				p2 = r;
			else
				p2 = c;

			if (t.y != INVALID_ELEVATION)
				p3 = t;
			else
				p3 = c;

			if (b.y != INVALID_ELEVATION)
				p4 = b;
			else
				p4 = c;

#if 0
			// The naive way is to use the surface vectors and cross them to
			//  find the normal.
			FPoint3 v1 = p2 - p1;
			FPoint3 v2 = p3 - p4;

			// This provides some 'exaggeration' for the lighting
			v1.y *= fLightFactor;
			v2.y *= fLightFactor;

			v3 = v1.Cross(v2);
#else
			// This is equivalent to the cross product, and the overall
			//  render is 2% faster
			v3.Set((p1.y - p2.y)*fLightFactor/(p2.x - p1.x), 1,
				   (p3.y - p4.y)*fLightFactor/(p4.z - p3.z));
#endif
			v3.Normalize();

			float shade = v3.Dot(light_direction); // shading 0 (dark) to 1 (light)

			// Most of the values are in the bottom half of the 0-1 range, so push
			//  them upwards with a gamma factor.
			if (fGamma != 1.0f)
				shade = powf(shade, fGamma);

			// boost with ambient light
			shade += fAmbient;

			// Never shade below zero, can cause RGB wraparound
			if (shade < 0)
				shade = 0;
			if (shade > 1.1f)
				shade = 1.1f;

			// combine color and shading
			if (depth == 8)
				pBM->ScalePixel8(i, h-1-j, shade);
			else if (depth == 24)
				pBM->ScalePixel24(i, h-1-j, shade);
			else if (depth == 32)
				pBM->ScalePixel32(i, h-1-j, shade);
		}
	}
}

/**
 * Quickly produce a shading-like effect by scanning over the bitmap once,
 * using the east-west slope to produce lightening/darkening.
 * The bitmap must be the same size as the elevation grid, or a power of 2 smaller.
 */
void vtHeightFieldGrid3d::ShadeQuick(vtBitmapBase *pBM, float fLightFactor,
									 bool bTrue, bool progress_callback(int))
{
	int w = pBM->GetWidth();
	int h = pBM->GetHeight();
	int depth = pBM->GetDepth();

	int stepx = m_iColumns / w;
	int stepy = m_iRows / h;

	int i, j;	// indices into bitmap
	int x, y;	// indices into elevation

	RGBi rgb;
	RGBAi rgba;

	for (j = 0; j < h; j++)
	{
		if (progress_callback != NULL)
		{
			if ((j&7) == 0)
				progress_callback(j * 100 / h);
		}
		// find corresponding location in heightfield
		y = m_iRows-1 - (j * stepy);
		for (i = 0; i < w; i++)
		{
			if (depth == 32)
				pBM->GetPixel32(i, j, rgba);
			else
				pBM->GetPixel24(i, j, rgb);

			int x_offset = 0;
			if (i == w-1)
				x_offset = -1;

			x = i * stepx;
			float value = GetElevation(x + x_offset, y, bTrue);
			if (value == INVALID_ELEVATION)
			{
				// Do not touch pixels in nodata areas
				continue;
			}

			float value2 = GetElevation(x+1 + x_offset, y, bTrue);
			if (value2 == INVALID_ELEVATION)
				value2 = value;
			short diff = (short) ((value2 - value) / m_fXStep * fLightFactor);

			// clip to keep values under control
			if (diff > 128)
				diff = 128;
			else if (diff < -128)
				diff = -128;
			if (depth == 32)
			{
				rgba.r += diff;
				rgba.g += diff;
				rgba.b += diff;
				if (rgba.r < 0) rgba.r = 0;
				else if (rgba.r > 255) rgba.r = 255;
				if (rgba.g < 0) rgba.g = 0;
				else if (rgba.g > 255) rgba.g = 255;
				if (rgba.b < 0) rgba.b = 0;
				else if (rgba.b > 255) rgba.b = 255;
				pBM->SetPixel32(i, j, rgba);
			}
			else
			{
				rgb.r = rgb.r + diff;
				rgb.g = rgb.g + diff;
				rgb.b = rgb.b + diff;
				if (rgb.r < 0) rgb.r = 0;
				else if (rgb.r > 255) rgb.r = 255;
				if (rgb.g < 0) rgb.g = 0;
				else if (rgb.g > 255) rgb.g = 255;
				if (rgb.b < 0) rgb.b = 0;
				else if (rgb.b > 255) rgb.b = 255;
				pBM->SetPixel24(i, j, rgb);
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////
// Begin shadow-casting code.
//

class LightMap
{
public:
	LightMap(int w, int h)
	{
		m_w = w;
		m_h = h;
		m_data = new uchar*[m_w];
		int rows, cols;
		for (cols = 0; cols < m_w; cols++)
			m_data[cols] = new uchar[m_h];
		for (cols = 0; cols < m_w; cols++)
			for (rows = 0; rows < m_h ; rows++)
				m_data[cols][rows] = 0;
	}
	~LightMap()
	{
		// Dispose with the temporary arrays
		for (int cols = 0 ; cols < m_w ; ++cols)
			delete [] m_data[cols];
		delete [] m_data;
	}
	void Set(int x, int y, uchar val) { m_data[x][y] = val; }
	uchar Get(int x, int y) { return m_data[x][y]; }

	uchar **m_data;
	int m_w, m_h;
};


inline DPoint2 GridPos(const DPoint2 &base, const DPoint2 &spacing, int i, int j)
{
	return DPoint2(base.x + spacing.x * i, base.y + spacing.y * j);
}

/**
 * ShadowCastDib - method to create shadows over the terrain based on the
 * angle of the sun.
 *
 * \param pBM	An interface to the bitmap to be shaded.
 * \param light_dir	The direction of the light, in world coordinates, coming
 *		down toward the terrain.  For example, (-1,-1,0) is pointing down
 *		where the sun would be at 9AM on the equator.  Despite this example,
 *		light_dir should be normalized to unit length.
 * \param fLightFactor	Amount of shading, from 0 to 1.  A value of 0 means
 *		no lighting, 1 means full lighting.
 * \param fAmbient	Amount of ambient light, from 0 to 1.  A typical value is 0.1.
 * \param progress_callback	Optional callback for progress notification.
 */
/* Core code contributed by Kevin Behilo, 2/20/04.
 *
 * Possible TODO: add code to soften and blend shadow edges
 *  (see aliasing comments in source).
 *
 * Definite TODO: the whole thing can be sped up by precalculating the
 *  surface normals once.  In fact that should be placed in a separate Shading
 *  Context, so that it could be re-used for quickly re-shading multiple times.
 */
void vtHeightFieldGrid3d::ShadowCastDib(vtBitmapBase *pBM, const FPoint3 &light_dir,
	float fLightFactor, float fAmbient, bool progress_callback(int))
{
	int w = pBM->GetWidth();
	int h = pBM->GetHeight();

	int gw, gh;
	GetDimensions(gw, gh);

	// Compute area that we will sample for shading, bounded by the texel
	//  centers, which are 1/2 texel in from the grid extents.
	DPoint2 texel_size(m_EarthExtents.Width() / w, m_EarthExtents.Height() / h);
	DRECT texel_area = m_EarthExtents;
	texel_area.Grow(-texel_size.x/2, -texel_size.y/2);
	DPoint2 texel_base(texel_area.left, texel_area.bottom);

	bool b8bit = (pBM->GetDepth() == 8);
	int i, j;

	// These values are hardcoded here but could be exposed in the GUI
	float sun =  0.7f;

	// If we have light that's pointing UP, rather than down at the terrain,
	//  then it's only going to take a really long time to produce a
	//  completely dark terrain.  We can catch this case up front.
	if (light_dir.y > 0)
	{
		for (i = 0; i < w; i++)
		{
			for (j = 0; j < h; j++)
			{
				if (b8bit)
					pBM->ScalePixel8(i, j, fAmbient);
				else
					pBM->ScalePixel24(i, j, fAmbient);
			}
		}
		return;
	}

	// Create array to hold flags
	LightMap lightmap(w, h);

	// This factor is used when applying shading to non-shadowed areas to
	// try and keep the "contrast" down to a min. (still get "patches" of
	// dark/light spots though).
	// It is initialized to 1.0, because in case there are no shadows at all
	//  (such as at noon) we still need a reasonable value.
	float darkest_shadow = 1.0;

	// For the vector used to cast shadows, we need it in grid coordinates,
	//  which are (Column,Row) where Row is north.  But the direction passed
	//  in uses OpenGL coordinates where Z is south.  So flip Z.
	FPoint3 grid_light_dir = light_dir;
	grid_light_dir.z = -grid_light_dir.z;

	// Scale the light vector such that the X or Z component (whichever is
	//  larger) is 1.  This is will serve as our direction vector in grid
	//  coordinates, when drawing a line across the grid to cast the shadow.
	//
	// Code adapted from aaron_torpy:
	// http://www.geocities.com/aaron_torpy/algorithms.htm
	//
	float f, HScale;
	if ( fabs(grid_light_dir.x) > fabs(grid_light_dir.z) )
	{
		HScale = m_fXStep;
		f = fabs(light_dir.x);
	}
	else
	{
		HScale = m_fZStep;
		f = fabs(light_dir.z);
	}
	grid_light_dir /= f;

	int i_init, i_final, i_incr;
	int j_init, j_final, j_incr;
	if (grid_light_dir.x > 0)
	{
		i_init=0;
		i_final=w;
		i_incr=1;
	}
	else
	{
		i_init=w-1;
		i_final=-1;
		i_incr=-1;
	}
	if (grid_light_dir.z > 0)
	{
		j_init=0;
		j_final=h;
		j_incr=1;
	}
	else
	{
		j_init=h-1;
		j_final=-1;
		j_incr=-1;
	}

	// First pass: find each point that it is in shadow.
	DPoint2 pos;
	float shadowheight, elevation;
	FPoint3 normal;
	FPoint3 p3;
	int x, z;
	float shade;

	for (j = j_init; j != j_final; j += j_incr)
	{
		if (progress_callback != NULL)
		{
			if ((j&7) == 0)
				progress_callback(abs(j-j_init) * 100 / h);
		}
		for (i = i_init; i != i_final; i += i_incr)
		{
			pos = GridPos(texel_base, texel_size, i, j);
			FindAltitudeOnEarth(pos, shadowheight, true);

			if (shadowheight == INVALID_ELEVATION)
			{
				// set a flag so we won't visit this one again
				lightmap.Set(i, j, 1);
				continue;
			}

			bool Under_Out = false;
			for (int k = 1; Under_Out == false; k++)
			{
				x = (int) (i + grid_light_dir.x*k + 0.5f);
				z = (int) (j + grid_light_dir.z*k + 0.5f);
				shadowheight += grid_light_dir.y * HScale;

				if ((x<0) || (x>w-1) || (z<0) || (z>h-1))
				{
					Under_Out = true; // Out of the grid
					break;
				}

				pos = GridPos(texel_base, texel_size, x, z);
				FindAltitudeOnEarth(pos, elevation, true);

				// skip holes in the grid
				if (elevation == INVALID_ELEVATION)
					continue;

				if (elevation > shadowheight)
				{
					if (k>1)
						Under_Out = true; // Under the terrain
					break;
				}

				// Combine color and shading.
				// Only do shadow if we have not shaded this i,j before.
				if (lightmap.Get(x,z) < 1)
				{
					// 3D elevation query to get slope
					m_Conversion.ConvertFromEarth(pos, p3.x, p3.z);
					FindAltitudeAtPoint(p3, p3.y, true, 0, &normal);

					//*****************************************
					// Here the Sun(r, g, b) = 0 because we are in the shade
					// therefore I(r, g, b) = Amb(r, g, b) * (0.5*N[z] + 0.5)

				//	shade =  sun*normal.Dot(-light_direction) + fAmbient * (0.5f*normal.y + 0.5f);
					shade =  fAmbient * (0.5f*normal.y + 0.5f);
					//*****************************************
					//*****************************************
					if (darkest_shadow > shade)
						darkest_shadow = shade;

					//Rather than doing the shading at this point we may want to
					//simply save the value into the LightMap array. Then apply
					//some anti-aliasing or edge softening algorithm to the LightMap.
					//Once that's done, apply the whole LightMap to the DIB.
					if (b8bit)
						pBM->ScalePixel8(x, h-1-z, shade);
					else
						pBM->ScalePixel24(x, h-1-z, shade);

					//set a flag to show that this texel has been shaded.
					// (or set to value of the shading - see comment above)
					lightmap.Set(x, z, lightmap.Get(x, z)+1);
				}
			}
		} //for i
	} //for j

	// For dot-product lighting, we use the normal 3D vector, only inverted
	//  so that we can compare it to the upward-pointing ground normals.
	FPoint3 inv_light_dir = -light_dir;

	// Second pass.  Now we are going to loop through the LightMap and apply
	//  the full lighting formula to each texel that has not been shaded yet.
	for (j = 0; j < h; j++)
	{
		if (progress_callback != NULL)
		{
			if ((j&7) == 0)
				progress_callback(j * 100 / h);
		}
		for (i = 0; i < w; i++)
		{
			if (lightmap.Get(i, j) > 0)
				continue;

			pos = GridPos(texel_base, texel_size, i, j);

			// 2D elevation query to check for holes in the grid
			FindAltitudeOnEarth(pos, elevation, true);
			if (elevation == INVALID_ELEVATION)
				continue;

			// 3D elevation query to get slope
			m_Conversion.ConvertFromEarth(pos, p3.x, p3.z);
			FindAltitudeAtPoint(p3, p3.y, true, 0, &normal);

			//*****************************************
			//*****************************************
			//shade formula based on:
			//http://www.geocities.com/aaron_torpy/algorithms.htm#calc_intensity

			// The Amb value was arbitrarily chosen
			// Need to experiment more to determine the best value
			// Perhaps calculating Sun(r, g, b) and Amb(r, g, b) for a
			//  given time of day (e.g. warmer colors close to sunset)
			// or give control to user since textures will differ

			// I(r, g, b) = Sun(r, g, b) * scalarprod(N, v) + Amb(r, g, b) * (0.5*N[z] + 0.5)
			shade = sun * normal.Dot(inv_light_dir);

			// It's a reasonable assuption that an angle of 45 degrees is
			//  sufficient to fully illuminate the ground.
			shade /= .7071f;

			// Now add ambient component
			shade += fAmbient * (0.5f*normal.y + 0.5f);

			// Maybe clipping values can be exposed to the user as well.
			// Clip - don't shade down below lowest ambient level
			if (shade < darkest_shadow)
				shade = darkest_shadow;
			else if (shade > 1.2f)
				shade = 1.2f;

			// Push the value of 'shade' toward 1.0 by the fLightFactor factor.
			// This means that fLightFactor=0 means no lighting, 1 means full lighting.
			float diff = 1 - shade;
			diff = diff * (1 - fLightFactor);
			shade += diff;

			// Rather than doing the shading at this point we may want to
			// simply save the value into the LightMap array. Then apply
			// some anti-aliasing or edge softening algorithm to the LightMap.
			// Once that's done, apply the whole LightMap to the DIB.
			// LightMap[I][J]= shade; // set to value of the shading - see comment above)
			if (b8bit)
				pBM->ScalePixel8(i, h-1-j, shade);
			else
				pBM->ScalePixel24(i, h-1-j, shade);
		}
	}

	// Possible TODO: Apply edge softening algorithm (?)
}

