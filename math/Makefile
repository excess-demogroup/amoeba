MATH_OBJS = math/vector.o
OBJS += $(MATH_OBJS)
SUBLIBS += math/math.a

math/math.a: $(MATH_OBJS)
	$(AR) rc math/math.a $(MATH_OBJS)
	$(RANLIB) math/math.a
