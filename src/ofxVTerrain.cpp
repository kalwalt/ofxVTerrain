#include "ofxVTerrain.h"


vtScene *m_Scene = vtGetScene();
ofxVTerrain::ofxVTerrain()
:						//m_Scene(vtGetScene()),
	_hiter(10),
	_yon(10000)
{
	
}


ofxVTerrain::~ofxVTerrain()
{
}

bool ofxVTerrain::createVTScene( char* filename ){
	
	// Look up the camera
	vtCamera *pCamera = m_Scene->GetCamera();
	pCamera->SetHither(_hiter);
	pCamera->SetYon(_yon);

	// The  terrain scene will contain all the terrains that are created.
	g_terrscene = new vtTerrainScene;

	// Set the global data path to look in the many places the sample data might be
	vtStringArray paths;
	paths.push_back(vtString("G:/Data-Distro/"));
	paths.push_back(vtString("../../../Data/"));
	paths.push_back(vtString("../../Data/"));
	paths.push_back(vtString("../Data/"));
	paths.push_back(vtString("Data/"));
	vtSetDataPath(paths);

	// Begin creating the scene, including the sun and sky
	vtGroup *pTopGroup = g_terrscene->BeginTerrainScene();

	// Tell the scene graph to point to this terrain scene
	m_Scene->SetRoot(pTopGroup);

	vtString pfile = FindFileOnPaths(vtGetDataPath(), filename);
	//const char* pfile = ofToDataPath(filename).c_str();

	if (pfile == "")
	{
		printf("Couldn't find terrain parameters Simple.xml\n");
		ofLog(OF_LOG_ERROR,"Couldn't find terrain parameters Simple.xml\n");
	}

	// Create a new vtTerrain, read its parameters from a file
	vtTerrain *pTerr = new vtTerrain;
	pTerr->SetParamFile(pfile);
	pTerr->LoadParams();

	// Add the terrain to the scene, and contruct it
	g_terrscene->AppendTerrain(pTerr);
	if (!g_terrscene->BuildTerrain(pTerr))
	{
		printf("Terrain creation failed: %s\n", (const char *)pTerr->GetLastError());

	}
	g_terrscene->SetCurrentTerrain(pTerr);

	// Create a navigation engine to move around on the terrain
	// Get flight speed from terrain parameters
	float fSpeed = pTerr->GetParams().GetValueFloat(STR_NAVSPEED);

	vtTerrainFlyer *pFlyer = new vtTerrainFlyer(fSpeed);
	pFlyer->AddTarget(pCamera);
	pFlyer->SetHeightField(pTerr->GetHeightField());
	m_Scene->AddEngine(pFlyer);

	// Minimum height over terrain is 100 m
	vtHeightConstrain *pConstrain = new vtHeightConstrain(100);
	pConstrain->AddTarget(pCamera);
	pConstrain->SetHeightField(pTerr->GetHeightField());
	m_Scene->AddEngine(pConstrain);

	VTLOG("Done creating scene.\n");
	return true;

}


void ofxVTerrain::updateVTscene(){

	m_Scene->DoUpdate();



}
