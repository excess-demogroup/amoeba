# Main system :-)
AUDIO_OBJS = audio/musichandler.o

# Audio providers
AUDIO_OBJS += audio/vorbis.o

# Audio drivers (outputs)
ifeq ($(DESTPLATFORM),win32)
AUDIO_OBJS += audio/win32_dsound.o
else
AUDIO_OBJS += audio/linux_oss.o
endif

OBJS += $(AUDIO_OBJS)
SUBLIBS += audio/audio.a

audio/audio.a: $(AUDIO_OBJS)
	$(AR) rc audio/audio.a $(AUDIO_OBJS)
	$(RANLIB) audio/audio.a
