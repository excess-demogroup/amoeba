# Main loop
MAIN_OBJS = main/mainloop.o main/demohandler.o \
		main/event.o main/factory.o \
		main/curve.o main/linearcurve.o main/autosplinecurve.o main/linecurve.o main/piprecalc.o

# Misc. 2D "effects"
MAIN_OBJS += main/imagehandler.o main/inverthandler.o main/overlayanimhandler.o main/backgroundhandler.o main/fonthandler.o

# Loaded 3D models
MAIN_OBJS += main/object.o main/objhandler.o

# 3D model deformations
MAIN_OBJS += main/shadowhandler.o main/shadowrecthandler.o \
	     main/twisthandler.o main/uquadshandler.o

# 3D-effects and -handlers
MAIN_OBJS += main/lighthandler.o \
	main/fovhandler.o \
	main/foghandler.o \
	main/camerahandler.o \
        main/heightmaptunnelhandler.o \
	main/particlepathhandler.o \
	main/interferenceheightmaphandler.o

# Debugging stuff
MAIN_OBJS += main/fpshandler.o main/timerhandler.o

ifeq ($(DESTPLATFORM),linux)
MAIN_OBJS += main/linux-config/linux-config.o
else
# the resource file is included in the top-level Makefile
MAIN_OBJS += main/win32-config/win32-config.o
endif

# ---

OBJS += $(MAIN_OBJS)
SUBLIBS += main/main.a

main/main.a: $(MAIN_OBJS)
	$(AR) rc main/main.a $(MAIN_OBJS)
	$(RANLIB) main/main.a
