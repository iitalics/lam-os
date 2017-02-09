
srcs=$(wildcard *.c *.s)
objs=$(srcs:%=$(OBJDIR)/%.o)

$(LIBNAME): $(TARG)

$(TARG): $(objs)
	@ echo AR lib$(LIBNAME)
	@ $(ar) rcs $(TARG) $(objs)

$(OBJDIR)/%.c.o: %.c
	@ echo "CC " $<
	@ $(cc) $(cflags) $< -c -o $@

$(OBJDIR)/%.s.o: %.s
	@ echo "AS " $<
	@ $(as) $(cflags) $< -c -o $@
