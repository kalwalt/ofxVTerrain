//
//  testApp.h
//
//  Created by Walter Perdan on 07.04.13.
//  Copyright 2013 Kalwaltart. All rights reserved.
//
//  partially source derived by ofxOpenSceneGraph addon
//  Created by Stephan Huber on 26.03.11.
//  Copyright 2011 Digital Mind. All rights reserved.


#pragma once

#include "ofMain.h"
#include "ofxOsgApp.h"
#include "ofxVTerrain.h"

class testApp : public ofBaseApp, public ofxOsgApp{

	public:
		void setup();
		void update();
		void draw();
		void exit();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);


};
