OPENGL_OBJS=opengl/fpscount.o opengl/fontttf.o \
	    opengl/texture.o opengl/glwindow.o opengl/extensions.o
OBJS += $(OPENGL_OBJS)
SUBLIBS += opengl/opengl.a

opengl/opengl.a: $(OPENGL_OBJS)
	$(AR) rc opengl/opengl.a $(OPENGL_OBJS)
	$(RANLIB) opengl/opengl.a
