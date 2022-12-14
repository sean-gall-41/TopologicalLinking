# global paths
ROOT_DIR        := ./
TEXTURE_DIR     := $(ROOT_DIR)textures/
# NOTE: for now, shader dir is the bkgd dir
BKGD_SHADER_DIR      := $(ROOT_DIR)shaders/bkgd/
SRC_DIR              := $(ROOT_DIR)src/
BUILD_DIR            := $(ROOT_DIR)build/
SRC_OBJ_DIR          := $(BUILD_DIR)src/
BKGD_SHADER_OBJ_DIR  := $(BUILD_DIR)shaders/bkgd/
TEXTURE_OBJ_DIR      := $(BUILD_DIR)textures/
TARGET               := $(BUILD_DIR)topological_linking

# absolute path simply because annoying to recalc rel path if we move this proj
RAND_INCLUDE := /home/seang/Dev/Git/UT_Austin/random123/include/

SRCS            := $(shell find $(SRC_DIR) -name "*.c" | xargs -I {} basename {})
SHADERS         := $(shell find $(BKGD_SHADER_DIR) -name "*.glsl" | xargs -I {} basename {})
TEXTURES        := $(shell find $(TEXTURE_DIR) -name "*.svg" | xargs -I {} basename {})

# all object files with path info
SRC_OBJS        := $(SRCS:%.c=$(SRC_OBJ_DIR)%.o)
SHADER_OBJS     := $(SHADERS:%.glsl=$(BKGD_SHADER_OBJ_DIR)%.o)
TEXTURE_OBJS    := $(TEXTURES:%.svg=$(TEXTURE_OBJ_DIR)%.o)

SRC_DEPS        := $(SRC_OBJS:.o=.d)

LIB_INC         := $(shell pkg-config --cflags gtk+-3.0 gl)
LIB_INC         += $(addprefix -I,$(RAND_INCLUDE))

LIBS            := $(shell pkg-config --libs gtk+-3.0 gl)
LIBS            += -lm

CFLAGS          := -std=c99 -DGL_GLEXT_PROTOTYPES
CFLAGS          += $(LIB_INC) -MMD -MP 

CHK_DIR_EXISTS  := test -d 
MKDIR           := mkdir -p
RM              := rm -rf

# GtkGLArea included in GTK+3.16: check if we have that version
ifneq ($(shell pkg-config --atleast-version=3.16 gtk+-3.0 && echo 1 || echo 0),1)
	$(error $(shell pkg-config --print-errors --atleast-version=3.16 gtk+-3.0))
endif

# do all make tasks
all: check_dirs $(TARGET)

# check existence of build dirs
check_dirs: $(BUILD_DIR)
	@$(CHK_DIR_EXISTS) $(SRC_OBJ_DIR) || $(MKDIR) $(SRC_OBJ_DIR)
	@$(CHK_DIR_EXISTS) $(BKGD_SHADER_OBJ_DIR) || $(MKDIR) $(BKGD_SHADER_OBJ_DIR)
	@$(CHK_DIR_EXISTS) $(TEXTURE_OBJ_DIR) || $(MKDIR) $(TEXTURE_OBJ_DIR)

$(BUILD_DIR):
	@$(CHK_DIR_EXISTS) $(BUILD_DIR) || $(MKDIR) $(BUILD_DIR)

# build the main target
$(TARGET): $(SRC_OBJS) $(SHADER_OBJS) $(TEXTURE_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

# convert our svg into png
$(TEXTURE_DIR)%.png: $(TEXTURE_DIR)%.svg
	rsvg-convert --format png --output $@ $^

# compile main sources
$(SRC_OBJ_DIR)%.o: $(SRC_DIR)%.c 
	$(CC) $(CFLAGS) -o $@ -c $<

# link shaders as binary input (remember, compiled from source during runtime)
$(BKGD_SHADER_OBJ_DIR)%.o: $(BKGD_SHADER_DIR)%.glsl
	$(LD) -r -b binary -o $@ $^

# create binary input texture data from png
$(TEXTURE_OBJ_DIR)%.o: $(TEXTURE_DIR)%.png
	$(LD) -r -b binary -o $@ $^

# remove all build fiels and dirs
.PHONY: clean
clean:
	$(RM) $(TEXTURE_DIR)*.png
	$(RM) $(SRC_OBJ_DIR) $(BKGD_SHADER_OBJ_DIR) $(TEXTURE_OBJ_DIR)
	$(RM) $(TARGET)
	$(RM) $(BUILD_DIR)

-include $(SRC_DEPS)

