//
//  testApp.cpp
//
//  Created by Walter Perdan on 07.04.13.
//  Copyright 2013 Kalwaltart. All rights reserved.
//
//  partially source derived by ofxOpenSceneGraph addon
//  Created by Stephan Huber on 26.03.11.
//  Copyright 2011 Digital Mind. All rights reserved.

#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){



	printf("Creating the terrain..\n");

	printf("Running..\n");


}

//--------------------------------------------------------------
void testApp::update(){

    vtGetScene()->DoUpdate();

}

//--------------------------------------------------------------
void testApp::draw(){

}

void testApp::exit(){



}

//--------------------------------------------------------------
void testApp::keyPressed(int key){


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
