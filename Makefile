include config.mak

CFLAGS=-Wall -ffast-math -fstrict-aliasing -g
CXXFLAGS=-Wall -ffast-math -fstrict-aliasing -fexceptions -g
CPPFLAGS=-I.
#CFLAGS=-Wall -I. 
#CXXFLAGS=-Wall -I.
LDFLAGS=

ifeq ($(PLATFORM),linux)
  ifeq ($(DESTPLATFORM),win32)
    PREFIX=i686-w64-mingw32-
    CC=$(PREFIX)gcc
    CXX=$(PREFIX)g++
    AR=$(PREFIX)ar
    RANLIB=$(PREFIX)ranlib
    WINDRES=$(PREFIX)windres
    CFLAGS += -mwindows
    CXXFLAGS += -mwindows
    CPPFLAGS += -I/usr/$(PREFIX)/include/freetype
  else
    AR=ar
    RANLIB=ranlib
    EXTRALIBS=
    #EXTRALIBS=-lefence
    #EXTRALIBS=/usr/src/dmalloc-4.6.0/libdmallocth.a /usr/src/dmalloc-4.6.0/dmallocc.o
    CPPFLAGS += $(shell freetype-config --cflags) $(shell pkg-config --cflags gtk+-2.0)
    ifeq ($(LINUXVARIANT),gcc)
      CC=gcc
      CXX=g++
    else 
      CC=/opt/intel/compiler50/ia32/bin/icc
      CXX=/opt/intel/compiler50/ia32/bin/icc
      CFLAGS=-O3 -tpp6 -axiMK -xM -Xc -pc32 -unroll
      CXXFLAGS=-O3 -tpp6 -axiMK -xM -Xc -pc32 -unroll -Kc++eh
      LDFLAGS+=
      CPPFLAGS += -I/opt/intel/compiler50/ia32/include -I/usr/local/include -D__ICC__
    endif
  endif
else
  AR=ar
  RANLIB=ranlib
  CC=gcc
  CXX=g++
  WINDRES=windres
  ifeq ($(WIN32VARIANT),cygwin)
    CFLAGS += -mwindows -DBOOL=bool -DWIN32=1
    CXXFLAGS += -mwindows -DBOOL=bool -DWIN32=1
  endif
endif

SUBDIRS=main opengl audio image packer math util
VPATH=$(SUBDIRS)
override CPPFLAGS += $(patsubst %,-I%,$(subst :, ,$(VPATH)))

.PHONY: clean

ifeq ($(DESTPLATFORM),linux)
EXE=amoeba
else
EXE=test-demolib.exe
endif

all: $(EXE)

OBJS=test-demolib.o exception.o
include $(patsubst %,%/Makefile,$(SUBDIRS))
include Makefile.dep

clean:
	rm -f $(EXE) $(OBJS) $(SUBLIBS) $(DEPS)

ifeq ($(DESTPLATFORM),linux)
$(EXE): $(SUBLIBS) exception.o test-demolib.o 
#	$(CXX) $(LDFLAGS) -o test-demolib test-demolib.o exception.o $(SUBLIBS) -lm -lGL -lGLU /usr/lib/libexpat.a /usr/local/lib/tinylib/libpng.a /usr/local/lib/tinylib/libz.a -L/usr/X11R6/lib/ -lXxf86vm -lX11 -lXext /usr/local/lib/tinylib/libjpeg.a /usr/local/lib/tinylib/libfreetype.a /usr/local/lib/tinylib/libvorbisfile.a /usr/local/lib/tinylib/libvorbis.a /usr/local/lib/tinylib/libogg.a $(EXTRALIBS)
	$(CXX) $(LDFLAGS) -o $(EXE) test-demolib.o exception.o $(SUBLIBS) -lm -lGL -lGLU -lexpat -lpng -lz -L/usr/X11R6/lib/ -lXxf86vm -lX11 -ljpeg `freetype-config --libs` -lvorbisfile -ldl $(EXTRALIBS)
else
%.o:  %.rc
	$(WINDRES) $< $@
	
$(EXE): $(SUBLIBS) exception.o test-demolib.o main/win32-config/dialog.o
	$(CXX) $(CXXFLAGS) -static -o test-demolib.exe test-demolib.o exception.o main/win32-config/dialog.o $(SUBLIBS) -lm -lpng -ljpeg -lz -lopengl32 -lglu32 -lgdi32 -lexpat -lvorbisfile -lvorbis -logg -lfreetype
endif

install: $(EXE)
	/usr/bin/install -s $(EXE) $(DESTDIR)/usr/bin/
