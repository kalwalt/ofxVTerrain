ofxVTerrain
===========

openframeworks addon for terrain rendering with vTerrain library. You can build your terrain map with the vTerrain tools and save it in a .xml file.
Then you can import it with ofxVTerrain addon in OpenFrameworks. This is still a work in progress!!


Windows
=======

you will find a visual studio solution in the vterrain-example/src folder. You need to adjust Osg src and static lib. Needed also gdal and libMini, lippng,libjpeg. i used Osg binaries from here: 
		
		http://www.alphapixel.com/osg/downloads/free-openscenegraph-binary-downloads

i used the 32 bit version.
 
gdal and other libs from OSGeo4W:

		http://trac.osgeo.org/osgeo4w/

libMini (MINI-10.9.zip) and libMini prebuilt binaries dependencies(libpng,libjpeg,zlib..):

		https://code.google.com/p/libmini/downloads

if you want build yourself the vTerrain libraries grab the source from here:

		http://vterrain.org/Download/

and don't forget to visit the main page:

		http://vterrain.org


useful note : remember to exclude from source the files xmltok_impl.c and xmltok_ns.c 


Linux
=====

I was able to compile it in linux only without makefile. I can't exclude from source the files mentioned above.Go in the OpenFrameworks forum to see the discussion: http://forum.openframeworks.cc/index.php/topic,12325.0.html So maybe you have to adjust some path in the CB project. 
You need Osg, gdal, libpng, lipjpeg, zlib you can install them from console 

		sudo apt-get install openscenegraph libgdal-dev libpng12-dev libjpeg-dev

libMini (MINI-10.9.zip) source :

		https://code.google.com/p/libmini/downloads

if you want build yourself the vTerrain libraries grab the source from here:

		http://vterrain.org/Download/

and don't forget to visit the main page:

		http://vterrain.org

To use
======

in main.cpp just use the setMap function to load your .xml project:

		#include "ofMain.h"
		#include "testApp.h"
		#include "ofxAppVTerrainWindow.h"

		//========================================================================

		int main( ){


		ofxAppVTerrainWindow window;
		ofSetupOpenGL(&window, 800,600, OF_WINDOW);		
    		window.setMap("path/to/myMap.xml");
		// this kicks off the running of my app
		//// can be OF_WINDOW or OF_FULLSCREEN
		// pass in width and height too:
		ofRunApp( new testApp());

		}

and in testApp.h remember to add the include to ofxOsgApp and ofxVTerrain header:

		#include "ofxOsgApp.h"
		#include "ofxVTerrain.h"

and to inherit from ofBaseApp and ofxOsgApp

		class testApp : public ofBaseApp, public ofxOsgApp{
		
		//testApp needed code
		
		}


in testApp.cpp just remember to add in update:


		void testApp::update(){

    		vtGetScene()->DoUpdate();

		}




This little guide is going to be improved and i need to verify a lot of things in the addons.

any feedeback is welcome! and also collaborations!
















		
