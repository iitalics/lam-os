
# libraries
libraries=
libraries+= boot

# architecture
arch?=i386
arch-dir=arch-$(arch)
gen-dir=generic
ifeq ($(wildcard $(arch-dir)/),)
	err:=$(error unsupported architecture: "$(arch)")
endif

# output
img=os-$(arch).bin

# load compiler settings
include $(arch-dir)/settings.make
include $(gen-dir)/settings.make
export cc=$(cc-base) -std=c99
export as=$(as-base) -Wall
export ld=$(cc-base)
export ar

# build and library directories
build-dir=$(arch-dir)/.build
build-dirs=$(build-dir) $(libraries:%=$(build-dir)/%)
archives=$(libraries:%=$(build-dir)/lib%.a)

# general rules
.PHONY: all
all: pre $(archives) post
.PHONY: pre
pre: $(err) $(build-dirs) find-libs
.PHONY: post
post: $(img)
.PHONY: never-satisfied
never-satisfied:

# find libraries (error if any lib doesn't exist)
.PHONY: find-libs
find-libs: $(libraries:%=find-lib-%)
find-lib-%: lib=$(@:find-lib-%=%)
find-lib-%: dir=$(wildcard $(arch-dir)/$(lib) $(gen-dir)/$(lib))
find-lib-%: never-satisfied
	@ if [ -z $(dir) ]; then \
			echo "ERROR: library \"$(lib)\" does not exist for architecture \"$(arch)\""; \
			exit 1; fi

# build directories
$(build-dir):
	mkdir $@
$(build-dir)/%:
	mkdir $@

# build libraries
$(build-dir)/lib%.a: lib=$(@:$(build-dir)/lib%.a=%)
$(build-dir)/lib%.a: dir=$(wildcard $(arch-dir)/$(lib) $(gen-dir)/$(lib))
$(build-dir)/lib%.a: objdir=$(@:$(build-dir)/lib%.a=$(build-dir)/%)
$(build-dir)/lib%.a: never-satisfied
	@ echo "== building: $(lib) =="
	@ TARG=../../$@ OBJDIR=../../$(objdir) LIBNAME=$(lib) \
			make --no-print-directory -f ../../library.make -C $(dir) \


# linker
link-script=$(arch-dir)/link.x
#ifeq ($(wildcard $(link-script)),)
#	err:=$(error no link script $(link-script))
#endif

$(img): $(archives)
	@ echo
	@ echo BUILDING IMAGE $(img)
	@	$(ld) $(archives) -o $@

# clean rules
.PHONY: clean
clean:
	rm -fr $(build-dir) $(img)

.PHONY: clean-img rebuild-img
clean-img:
	rm $(img)
rebuild-img: clean-img all

clean-lib-%: lib=$(@:clean-lib-%=%)
clean-lib-%: never-satisfied
	rm -r $(build-dir)/$(lib)
