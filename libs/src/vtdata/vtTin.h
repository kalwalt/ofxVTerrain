//
// vtTin.h
//
// Copyright (c) 2002-2009 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTTINH
#define VTTINH

#include "MathTypes.h"
#include "Projections.h"
#include "HeightField.h"
#include "vtString.h"

// a type useful for the Merge algorithm
typedef vtArray<int> Bin;

class BinArray
{
public:
	BinArray(int cols, int rows) {
		iData = new Bin[cols*rows];
		iRows = rows;
		iCols = cols;
	}
	~BinArray() {
		delete [] iData;
	}
	Bin *GetBin(int col, int row)
	{
		if (col < 0 || col >= iCols) return NULL;
		if (row < 0 || row >= iRows) return NULL;
		return iData + iCols*row + col;
	}
	int GetMemoryUsed() const
	{
		int bins = iCols * iRows;
		int bytes = sizeof(BinArray) + sizeof(Bin*) * bins;
		for (int i = 0; i < bins; i++)
			bytes += (sizeof(int) * iData[i].GetSize());
		return bytes;
	}
private:
	Bin *iData;
	int	iCols, iRows;
};

/**
 * This class represents a TIN, a 'triangulated irregular network'.  A TIN
 * consists of a set of vertices connected by triangles with no regularity.
 * However this class does expect to operate on a particular kind of
 * TIN, specifically a heightfield TIN.
 *
 * The triangles are defined by indices into the vertex array, so this is
 * an "indexed TIN".
 */
class vtTin : public vtHeightField3d
{
public:
	vtTin();
	virtual ~vtTin();

	uint NumVerts() const { return m_vert.GetSize(); }
	uint NumTris() const { return m_tri.GetSize()/3; }

	void AddVert(const DPoint2 &p, float z);
	void AddVert(const DPoint2 &p, float z, FPoint3 &normal);
	void AddTri(int v1, int v2, int v3, int surface_type = -1);
	void RemVert(int v);
	void RemTri(int t);

	// Native file I/O.
	bool Read(const char *fname);
	bool ReadHeader(const char *fname);
	bool ReadBody(const char *fname);
	bool Write(const char *fname, bool progress_callback(int) = NULL) const;

	// Import/Export.
	bool ReadDXF(const char *fname, bool progress_callback(int) = NULL);
	bool ReadADF(const char *fname, bool progress_callback(int) = NULL);
	bool ReadGMS(const char *fname, bool progress_callback(int) = NULL);
	bool ReadPLY(const char *fname, bool progress_callback(int) = NULL);
	bool WriteGMS(const char *fname, bool progress_callback(int) = NULL) const;
	bool WriteDAE(const char *fname, bool progress_callback(int) = NULL) const;
	bool WriteWRL(const char *fname, bool progress_callback(int) = NULL) const;
	bool WriteOBJ(const char *fname, bool progress_callback(int) = NULL) const;
	bool WritePLY(const char *fname, bool progress_callback(int) = NULL) const;
	bool WriteDXF(const char *fname, bool progress_callback(int) = NULL) const;
	void FreeData();

	uint AddSurfaceType(const vtString &surface_texture, bool bTiled = false);
	void SetSurfaceType(int iTri, int surface_type);

	bool ComputeExtents();
	void Offset(const DPoint2 &p);
	void Scale(float fFactor);
	void VertOffset(float fAmount);
	bool ConvertProjection(const vtProjection &proj_new);

	// Accessors
	void GetVert(int v, DPoint2 &p, float &z) const { p = m_vert.GetAt(v); z = m_z[v]; }
	void GetTri(int t, int &v0, int &v1, int &v2) const { v0 = m_tri[t*3]; v1 = m_tri[t*3+1]; v2 = m_tri[t*3+2]; }
	int *GetAtTri(int t) const { return m_tri.GetData() + (t*3); }

	// Implement required vtHeightField methods
	virtual bool FindAltitudeOnEarth(const DPoint2 &p, float &fAltitude,
		bool bTrue = false) const;
	virtual bool FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude,
		bool bTrue = false, int iCultureFlags=0, FPoint3 *vNormal = NULL) const;

	// Avoid implementing HeightField3d virtual methods
	bool CastRayToSurface(const FPoint3 &point, const FPoint3 &dir,
		FPoint3 &result) const { return false; }

	void CleanupClockwisdom();
	int RemoveUnusedVertices();
	void AppendFrom(vtTin *pTin);
	double GetTriMaxEdgeLength(int iTri) const;
	void MergeSharedVerts(bool progress_callback(int) = NULL);
	bool HasVertexNormals() const { return m_vert_normal.GetSize() != 0; }
	int RemoveTrianglesBySegment(const DPoint2 &ep1, const DPoint2 &ep2);
	void SetupTriangleBins(int bins, bool progress_callback(int) = NULL);
	int MemoryNeededToLoad() const;

	vtProjection	m_proj;

protected:
	bool TestTriangle(int tri, const DPoint2 &p, float &fAltitude) const;
	bool _ReadTin(FILE *fp);
	bool _ReadTinHeader(FILE *fp);
	bool _ReadTinBody(FILE *fp);
	bool _ReadTinOld(FILE *fp);

	void _UpdateIndicesInInBin(int bin);
	void _CompareBins(int bin1, int bin2);

	DLine2			m_vert;
	vtArray<float>	m_z;
	vtArray<int>	m_tri;
	FLine3			m_vert_normal;

	// Surface Types
	vtArray<int>	m_surfidx;
	vtStringArray	m_surftypes;
	vtArray<bool>	m_surftype_tiled;

	// These members are used only during MergeSharedVerts
	int *m_bReplace;
	Bin *m_vertbin;
	Bin *m_tribin;

	// This is used to speed up FindAltitudeOnEarth
	BinArray *m_trianglebins;
	DPoint2 m_BinSize;

	int m_file_data_start, m_file_verts, m_file_tris;	// Used while reading ITF
};


#endif // VTTINH
