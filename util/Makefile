UTIL_OBJS=util/hashtable.o
OBJS += $(UTIL_OBJS)
SUBLIBS += util/util.a

util/util.a: $(UTIL_OBJS)
	$(AR) rc util/util.a $(UTIL_OBJS)
	$(RANLIB) util/util.a
