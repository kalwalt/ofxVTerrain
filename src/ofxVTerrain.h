#pragma once

#include "ofMain.h"

#include <osgViewer/View>

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/NavEngines.h"
#include "vtlib/vtosg/OSGEventHandler.h"
#include "vtdata/DataPath.h"
#include "vtdata/vtLog.h"

class ofxVTerrain
{
public:
	ofxVTerrain();

	bool createVTScene( char* filename );
	void updateVTscene();
	virtual ~ofxVTerrain();

private:
	vtTerrainScene *g_terrscene;
	
	//vtScene *m_Scene;
	float _hiter;
	float _yon;
};

