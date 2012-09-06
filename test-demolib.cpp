/*
 * Can hardly be called a "test program for the demolib" anymore,
 * as it's been used as the main system for two productions already ;-)
 */
#define CINTERFACE 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#ifdef __GNUC__
#include <GL/glext.h>
#endif

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502
#endif

#include "demolib_prefs.h"

#include "math/vector.h"
#include "main/mainloop.h"
#include "main/event.h"
#include "main/demohandler.h"
#include "main/imagehandler.h"
#include "main/fpshandler.h"
#include "main/timerhandler.h"
#include "main/shadowhandler.h"
#include "main/shadowrecthandler.h"
#include "main/lighthandler.h"
#include "main/fovhandler.h"
#include "main/foghandler.h"
#include "main/heightmaptunnelhandler.h"
#include "main/twisthandler.h"
#include "main/particlepathhandler.h"
#include "main/camerahandler.h"
#include "main/inverthandler.h"
#include "main/overlayanimhandler.h"
#include "main/backgroundhandler.h"
#include "main/uquadshandler.h"
#include "main/interferenceheightmaphandler.h"
#include "main/fonthandler.h"

#include "audio/musichandler.h"

#include "image/image.h"
#include "opengl/texture.h"
//#include "opengl/fpscount.h"
#include "exception.h"

int main(int argc, char **argv)
{
#if DEMOLIB_SCREENSAVER
	/* scan through and check that we do not preview ;-) */
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "/p") == 0) {
			exit(0);
		}
	}
#endif
	
#if !DEMOLIB_SILENT
	printf("Loading and precalculating...\n");
#endif

	srand(time(NULL));
	
	try {
		MainLoop *demo = new MainLoop(argc, argv);
		File *demoscript = load_file("demo.xml");

		demo->add_handler(new HandlerFactory<DemoHandler>("demo"));
		demo->add_handler(new HandlerFactory<MusicHandler>("music"));
//		demo->add_handler(new HandlerFactory<FPSHandler>("fpscounter"));
//		demo->add_handler(new HandlerFactory<TimerHandler>("timer"));
		demo->add_handler(new HandlerFactory<ImageHandler>("image"));
		demo->add_handler(new HandlerFactory<ObjHandler>("objmodel"));
		demo->add_handler(new HandlerFactory<ShadowHandler>("shadow"));
		demo->add_handler(new HandlerFactory<ShadowRectHandler>("shadowrect"));
		demo->add_handler(new HandlerFactory<LightHandler>("light"));
		demo->add_handler(new HandlerFactory<LightHandler>("poslight"));
		demo->add_handler(new HandlerFactory<FOVHandler>("fov"));
		demo->add_handler(new HandlerFactory<FogHandler>("fog"));
		demo->add_handler(new HandlerFactory<HeightmapTunnelHandler>("heightmaptunnel"));
		demo->add_handler(new HandlerFactory<TwistHandler>("twist"));
		demo->add_handler(new HandlerFactory<ParticlePathHandler>("particlepath"));
		demo->add_handler(new HandlerFactory<CameraHandler>("camera"));
		demo->add_handler(new HandlerFactory<InvertHandler>("invert"));
		demo->add_handler(new HandlerFactory<OverlayAnimHandler>("overlayanim"));
		demo->add_handler(new HandlerFactory<BackgroundHandler>("background"));
//		demo->add_handler(new HandlerFactory<BlobHandler>("blobs"));
		demo->add_handler(new HandlerFactory<UQuadsHandler>("uquads"));
		demo->add_handler(new HandlerFactory<InterferenceHeightmapHandler>("interferenceheightmap"));
		demo->add_handler(new HandlerFactory<FontHandler>("text"));

		demo->parse(demoscript);
		delete demoscript;

		demo->run();
		delete demo;
	} catch (Exception *e) {
#if __linux__
		fprintf(stderr, "Unhandled exception: %s\n", e->get_error());
#else
                MessageBox(NULL, e->get_error(), "Unhandled exception!", 0);
#endif
	}

	return 0;
}
