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
