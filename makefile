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

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

#SRC_EXT := cpp c s S

PROGRAM := $(BIN_DIR)/vision_program
SRCS := $(call rwildcard,$(SRC_DIR)/,*.cpp *.c *.S *.s)
OBJS := $(SRCS:$(SRC_DIR)/%=$(OBJ_DIR)/%.o)

CDEBUG := -g -Og -DDEBUG
LDEBUG := -g
CRELEASE := -O3 -DRELEASE
LRELEASE :=

CPPFLAGS := -pthread -Iinclude -Iinclude/opencv -MMD -MP
CFLAGS := -Wall
ASMFLAGS := -mcpu=cortex-a72 -mfpu=neon-fp-armv8
LDFLAGS := -pthread -Wall -Llib -Wl,--unresolved-symbols=ignore-in-shared-libs
LDLIBS := -lm -lpigpio -lwpilibc -lwpiHal -lcameraserver -lntcore -lcscore -lopencv_dnn -lopencv_highgui -lopencv_ml \
	-lopencv_objdetect -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_calib3d \
	-lopencv_videoio -lopencv_imgcodecs -lopencv_features2d -lopencv_video -lopencv_photo -lopencv_imgproc -lopencv_flann \
	-lopencv_core -lwpiutil -latomic

ifeq ($(OS),Windows_NT) #windows
CXX := $(CROSS_PREFIX)$(CXX)
OS := windows
RM-R := del /s
else #native
OS := native
RM-R := rm -r
endif

ifeq ($(mode),release) #release
COPT := $(CRELEASE)
LOPT := $(LRELEASE)
else #ifneq (,$(findstring debug,$(mode))) #debug
COPT := $(CDEBUG)
LOPT := $(LDEBUG)
endif

.PHONY: build rebuild clean install copy

build: $(PROGRAM)
	@echo "Building: $(OS)-$(mode)"

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
$(PROGRAM): $(OBJS) | $(BIN_DIR)
	$(CXX) $(LOPT) -o $@ $(LDFLAGS) $(foreach file,$(^F),$(OBJ_DIR)/$(file)) $(LDLIBS)

#create dirs if nonexistant
$(BIN_DIR) $(OBJ_DIR):
	mkdir $@

copy:
ifeq ($(OS),windows)
	scp $(PROGRAM) pi@wpilibpi:/home/pi/uploaded
	scp frc.json pi@wpilibpi:/boot
else
	cp $(PROGRAM) /home/pi/uploaded
	chmod +x /home/pi/uploaded
	cp frc.json /boot
endif

install: build copy

clean:
	$(RM-R) $(OBJ_DIR)

-include $(OBJS:.o=.d)