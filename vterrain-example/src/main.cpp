#include "ofMain.h"
#include "testApp.h"
#include "ofxAppVTerrainWindow.h"

//========================================================================
int main( ){

    
	ofxAppVTerrainWindow window;
	ofSetupOpenGL(&window, 800,600, OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	//// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp( new testApp());

}
