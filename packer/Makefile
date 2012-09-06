PACKER_OBJS=packer/file.o packer/memfile.o packer/pakfile.o \
	    packer/filereader.o packer/zfile.o
OBJS += $(PACKER_OBJS)
SUBLIBS += packer/packer.a

packer/packer.a: $(PACKER_OBJS)
	$(AR) rc packer/packer.a $(PACKER_OBJS)
	$(RANLIB) packer/packer.a
