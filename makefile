rwildcard = $(foreach d,$(wildcard $(addsuffix *,$(1))),$(call rwildcard,$(d)/,$(2)) $(filter $(subst *,%,$(2)),$(d)))

CXX := arm-raspbian10-linux-gnueabihf-g++

SRC_DIR := src
OBJ_DIR := temp
BIN_DIR := bin

EXE := $(BIN_DIR)/vision_program
#SRC := $(wildcard $(SRC_DIR)/*.cpp)
SRC_T := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/extras/*.cpp)
#SRC_R := $(call rwildcard,src/,*.cpp *.c)
OBJ := $(SRC_T:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
#OBJ := $(addprefix $(OBJ_DIR)/,$(SRC_R:%.c=%.o))

CPPFLAGS := -Iinclude -Iinclude/opencv -MMD -MP
CFLAGS := -Wall
LDFLAGS := -Llib
LDLIBS := -lm -lpigpio -lwpilibc -lwpiHal -lcameraserver -lntcore -lcscore -lopencv_dnn -lopencv_highgui -lopencv_ml -lopencv_objdetect -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_videoio -lopencv_imgcodecs -lopencv_features2d -lopencv_video -lopencv_photo -lopencv_imgproc -lopencv_flann -lopencv_core -lwpiutil -latomic

.PHONY: all clean #install

all: $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CXX) -pthread -g -o $@ $(LDFLAGS) $^ $(LDLIBS) -Wl,--unresolved-symbols=ignore-in-shared-libs

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) -pthread -g -Og -c -o $@ -std=c++17 $(CPPFLAGS) $(CFLAGS) $<

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

#install: all
#	cp runCamera $(BIN_DIR)

clean:
	@$(RM) -rv $(OBJ_DIR)

-include $(OBJ:.o=.d)
