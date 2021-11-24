# https://www.gnu.org/software/make/manual/html_node/index.html
# https://stackoverflow.com/questions/30573481/how-to-write-a-makefile-with-separate-source-and-header-directories
# https://stackoverflow.com/questions/2826029/passing-additional-variables-from-command-line-to-make
# https://web.stanford.edu/class/archive/cs/cs107/cs107.1174/guide_make.html

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

#rwildcard = $(foreach d,$(wildcard $(addsuffix *,$(1))),$(call rwildcard,$(d)/,$(2)) $(filter $(subst *,%,$(2)),$(d)))

mode ?= windows-release

CROSS_PREFIX := arm-raspbian10-linux-gnueabihf-
CXX := g++

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

PROGRAM := $(BIN_DIR)/vision_program
#SRC := $(wildcard $(SRC_DIR)/*.cpp)
SRC_T := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/extras/*.cpp)
#SRC_R := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/**/*.cpp)
#SRC_R := $(call rwildcard,$(SRC_DIR)/,*.cpp)
SRCS := $(SRC_T)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CDEBUG := -g -Og
LDEBUG := -g
CRELEASE := -O3
LRELEASE :=

CPPFLAGS := -pthread -Iinclude -Iinclude/opencv -MMD -MP
CFLAGS := -Wall
LDFLAGS := -pthread -Wall -Llib -Wl,--unresolved-symbols=ignore-in-shared-libs
LDLIBS := -lm -lpigpio -lwpilibc -lwpiHal -lcameraserver -lntcore -lcscore -lopencv_dnn -lopencv_highgui -lopencv_ml -lopencv_objdetect -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_videoio -lopencv_imgcodecs -lopencv_features2d -lopencv_video -lopencv_photo -lopencv_imgproc -lopencv_flann -lopencv_core -lwpiutil -latomic

ifneq (,$(findstring windows,$(mode))) #windows
CXX := $(CROSS_PREFIX)$(CXX)
else #native
endif

ifneq (,$(findstring release,$(mode))) #release
COPT := $(CRELEASE)
LOPT := $(LRELEASE)
else #ifneq (,$(findstring debug,$(mode))) #debug
COPT := $(CDEBUG)
LOPT := $(LDEBUG)
endif

.PHONY: all rebuild clean

all: $(PROGRAM)

rebuild: all | clean

#compile for each source
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(COPT) -c -o $(OBJ_DIR)/$(@F) -std=c++17 $(CPPFLAGS) $(CFLAGS) $<

#link all objects
$(PROGRAM): $(OBJS) | $(BIN_DIR)
	$(CXX) $(LOPT) -o $@ $(LDFLAGS) $(foreach file,$(^F),$(OBJ_DIR)/$(file)) $(LDLIBS)

#create dirs if nonexistant
$(BIN_DIR) $(OBJ_DIR):
	mkdir $@

#install: all
#	cp runCamera $(BIN_DIR)

clean:
ifneq (,$(findstring windows,$(mode)))#find a better solution for windows, as this doesn't work if the dirs are not empty (kind of pointless)
	rmdir $(OBJ_DIR) $(BIN_DIR)
else 
	$(RM) -rv $(OBJ_DIR) $(BIN_DIR)
endif

-include $(OBJS:.o=.d)