IMAGE_OBJS=image/jpeg_image.o image/png_image.o image/load_image.o image/image.o image/imagecombiner.o
OBJS += $(IMAGE_OBJS)
SUBLIBS += image/image.a

image/image.a: $(IMAGE_OBJS)
	$(AR) rc image/image.a $(IMAGE_OBJS)
	$(RANLIB) image/image.a
