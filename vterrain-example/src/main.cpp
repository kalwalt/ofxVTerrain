//
//  main.cpp
//
//  Created by Walter Perdan on 07.04.13.
//  Copyright 2013 Kalwaltart. All rights reserved.
//
//  partially source derived by ofxOpenSceneGraph addon
//  Created by Stephan Huber on 26.03.11.
//  Copyright 2011 Digital Mind. All rights reserved.

#include "ofMain.h"
#include "testApp.h"
#include "ofxAppVTerrainWindow.h"

//========================================================================
int main( ){


	ofxAppVTerrainWindow window;
	ofSetupOpenGL(&window, 800,600, OF_WINDOW);			// <-------- setup the GL context
    window.setMap("Terrains/Simple.xml");
	// this kicks off the running of my app
	//// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp( new testApp());

}
