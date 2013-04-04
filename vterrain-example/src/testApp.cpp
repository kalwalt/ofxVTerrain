#include "testApp.h"

vtTerrainScene *g_terrscene;
// Get a handle to the vtScene - one is already created for you
vtScene *pScene = vtGetScene();

bool CreateScene()
{
	

	// Look up the camera
	vtCamera *pCamera = pScene->GetCamera();
	pCamera->SetHither(10);
	pCamera->SetYon(100000);

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
	pScene->SetRoot(pTopGroup);

	vtString pfile = FindFileOnPaths(vtGetDataPath(), "Terrains/Simple.xml");
	if (pfile == "")
	{
		printf("Couldn't find terrain parameters Simple.xml\n");
		return false;
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
		return false;
	}
	g_terrscene->SetCurrentTerrain(pTerr);

	// Create a navigation engine to move around on the terrain
	// Get flight speed from terrain parameters
	float fSpeed = pTerr->GetParams().GetValueFloat(STR_NAVSPEED);

	vtTerrainFlyer *pFlyer = new vtTerrainFlyer(fSpeed);
	pFlyer->AddTarget(pCamera);
	pFlyer->SetHeightField(pTerr->GetHeightField());
	pScene->AddEngine(pFlyer);

	// Minimum height over terrain is 100 m
	vtHeightConstrain *pConstrain = new vtHeightConstrain(100);
	pConstrain->AddTarget(pCamera);
	pConstrain->SetHeightField(pTerr->GetHeightField());
	pScene->AddEngine(pConstrain);

	VTLOG("Done creating scene.\n");
	return true;
}


//--------------------------------------------------------------
void testApp::setup(){

	// Log messages to make troubleshooting easier
	VTSTARTLOG("debug.txt");
	VTLOG("vTerrainExample\n");

	printf("Creating the terrain..\n");
	//CreateScene();
	terrain.createVTScene("Terrains/Simple.xml");
	printf("Running..\n");

	
}

//--------------------------------------------------------------
void testApp::update(){
    vtGetScene()->DoUpdate();
	terrain.updateVTscene();
}

//--------------------------------------------------------------
void testApp::draw(){

}

void testApp::exit(){

	

}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    vtGetScene()->OnKey(key, 0);

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	 // std::cout << "mouseMoved" << x << "/" << y << std::endl;

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){

}
