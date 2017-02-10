
srcs=$(wildcard *.c *.s)
objs=$(srcs:%=$(OBJDIR)/%.o)

$(TARG): $(objs)
	@ echo "[ar]  " lib$(LIBNAME)
	@ $(ar) rcs $(TARG) $(objs)

$(OBJDIR)/%.c.o: %.c
	@ echo "[cc]  " $<
	@ $(cc) $(cflags) $< -c -o $@

$(OBJDIR)/%.s.o: %.s
	@ echo "[asm] " $<
	@ $(as) $(asflags) $< -c -o $@
