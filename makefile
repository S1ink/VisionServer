# https://www.gnu.org/software/make/manual/html_node/index.html
# https://stackoverflow.com/questions/30573481/how-to-write-a-makefile-with-separate-source-and-header-directories
# https://stackoverflow.com/questions/2826029/passing-additional-variables-from-command-line-to-make
# https://web.stanford.edu/class/archive/cs/cs107/cs107.1174/guide_make.html
# https://github.com/KRMisha/Makefile/blob/master/Makefile

# Compiler Options: https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html
# -pthread -> link POSIX thread library - use in compiling AND linking
# -g -> compile for use with debug tools (GDB)
# -O -> optimization preference (O0 is default, O{1-3} set different levels (3 is the most optimized), Og optimizes but includes debug info, -Ofast and -Os also exist)
# -c -> compile only (no linking)
# -o{file} -> output to {file}, usually a '.o'
# -I{path} -> INCLUDE this path in header search
# -W(all) -> compiler warning level (commonly set to 'all', but other options exist)
# -l{libname} -> link with {libname} - remember that libraries all start with 'lib' then end in an extention - {libname} only refers to the name between those - ex. libLIBRARY.so -> -lLIBRARY
# -L{path} -> add a path in which to search for specified libs (-l's, see above)
# -Wl,{option} -> pass an option to the linker (this is only here because it was used in the source Makefile)
# -mcpu/mfpu -> architecture/feature options(for using NEON): https://gist.github.com/fm4dd/c663217935dc17f0fc73c9c81b0aa845

rwildcard = $(foreach d,$(wildcard $(addsuffix *,$(1))),$(call rwildcard,$(d)/,$(2)) $(filter $(subst *,%,$(2)),$(d)))

mode ?= release

CROSS_PREFIX := arm-raspbian10-linux-gnueabihf-
CXX := g++
AR := ar

ifeq ($(OS),Windows_NT)
CXX := $(CROSS_PREFIX)$(CXX)
AR := $(CROSS_PREFIX)$(AR)
OS := windows
RM-R := del /s /y
CP := copy
DIR := \\

else #native
OS := native
RM-R := rm -r
CP := cp
DIR := /
endif

SRC_DIR := src
CORE_SUBDIR := core
OBJ_DIR := obj
OUT_DIR := out
HEADER_DIR := $(OUT_DIR)$(DIR)include

#SRC_EXT := cpp c s S
#$(foreach ext,$(SRC_EXT),*.$(ext))

PNAME := vision_program
LNAME := 3407visionserver

PROGRAM := $(OUT_DIR)/$(PNAME)
LIBSTATIC := $(OUT_DIR)/lib$(LNAME).a
#LIBSHARED := $(OUT_DIR)/lib$(LNAME).so

SRCS := $(call rwildcard,$(SRC_DIR)/,*.cpp *.c *.S *.s)
OBJS := $(SRCS:$(SRC_DIR)/%=$(OBJ_DIR)/%.o)

LIB_SRCS := $(call rwildcard,$(SRC_DIR)/$(CORE_SUBDIR)/,*.cpp *.c *.S *.s)
LIB_OBJS := $(LIB_SRCS:$(SRC_DIR)/%=$(OBJ_DIR)/%.o)
ifeq ($(OS),windows)
HEADERS := $(subst /,\,$(call rwildcard,$(SRC_DIR)/$(CORE_SUBDIR)/,*.h *.hpp *.inc))
else
HEADERS := $(call rwildcard,$(SRC_DIR)/$(CORE_SUBDIR)/,*.h *.hpp *.inc)
endif
COPY_HEADERS := $(HEADERS:$(SRC_DIR)\$(CORE_SUBDIR)%=$(HEADER_DIR)%)

CDEBUG := -g -Og -DDEBUG
LDEBUG := -g
CRELEASE := -O3 -DRELEASE
LRELEASE :=

CPPFLAGS := -pthread -Iinclude -Ireferences -Iinclude/opencv4 -MMD -MP
CFLAGS := -Wall -fpermissive
ASMFLAGS := -mcpu=cortex-a72 -mfpu=neon-fp-armv8
LDFLAGS := -pthread -Wall -Llib -Wl,--unresolved-symbols=ignore-in-shared-libs
LDLIBS := -lm -lpigpio -ledgetpu -ltensorflowlite -lwpilibc -lwpiHal -lcameraserver -lntcore -lcscore -lopencv_gapi \
	-lopencv_highgui -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_video \
	-lopencv_calib3d -lopencv_features2d -lopencv_dnn -lopencv_flann -lopencv_videoio -lopencv_imgcodecs \
	-lopencv_imgproc -lopencv_core -lwpiutil -latomic

ifeq ($(mode),release)
COPT := $(CRELEASE)
LOPT := $(LRELEASE)
else
COPT := $(CDEBUG)
LOPT := $(LDEBUG)
endif

.PHONY: build static rebuild clean install copy #shared

build: $(PROGRAM)
#	@echo "Building: $(OS)-$(mode)"

static: $(LIBSTATIC) $(COPY_HEADERS)

#shared: $(LIBSHARED) $(COPY_HEADERS)

rebuild: build | clean

#compile for each source
$(OBJ_DIR)/%.cpp.o : $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(COPT) -c -o $(OBJ_DIR)/$(@F) -std=c++17 $(CPPFLAGS) $(CFLAGS) $<

$(OBJ_DIR)/%.c.o : $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CXX) $(COPT) -c -o $(OBJ_DIR)/$(@F) -std=c++17 $(CPPFLAGS) $(CFLAGS) $<

$(OBJ_DIR)/%.S.o : $(SRC_DIR)/%.S | $(OBJ_DIR)
	$(CXX) $(COPT) -c -o $(OBJ_DIR)/$(@F) -std=c++17 $(ASMFLAGS) $(CPPFLAGS) $(CFLAGS) $<

$(OBJ_DIR)/%.s.o : $(SRC_DIR)/%.s | $(OBJ_DIR)
	$(CXX) $(COPT) -c -o $(OBJ_DIR)/$(@F) -std=c++17 $(ASMFLAGS) $(CPPFLAGS) $(CFLAGS) $<

#link all objects
$(PROGRAM): $(OBJS) | $(OUT_DIR)
	$(CXX) $(LOPT) -o $@ $(LDFLAGS) $(foreach file,$(^F),$(OBJ_DIR)/$(file)) $(LDLIBS)

$(LIBSTATIC): $(LIB_OBJS) | $(OUT_DIR)
	$(AR) rcs $@ $(foreach file,$(^F),$(OBJ_DIR)/$(file))

$(COPY_HEADERS): $(HEADER_DIR)% : $(SRC_DIR)\$(CORE_SUBDIR)% | $(HEADER_DIR)
	$(CP) $< $@

#create dirs if nonexistant
$(OUT_DIR) $(OBJ_DIR):
	mkdir $@

$(HEADER_DIR): | $(OUT_DIR)
	mkdir $@

copy: $(COPY_HEADERS)
# ifeq ($(OS),windows)
# 	scp $(PROGRAM) pi@wpilibpi:/home/pi/uploaded
# 	scp frc.json pi@wpilibpi:/boot
# else
# 	cp $(PROGRAM) /home/pi/uploaded
# 	chmod +x /home/pi/uploaded
# 	cp frc.json /boot
# endif

install: build copy

clean:
	$(RM-R) $(OBJ_DIR)
	$(RM-R) $(OUT_DIR)

-include $(OBJS:.o=.d)