ROOT_DIR  := ./
SRC_DIR   := $(ROOT_DIR)src/
BUILD_DIR := $(ROOT_DIR)build/
OBJS_DIR  := $(BUILD_DIR)objs/
TARGET 	  := $(BUILD_DIR)topological_linking

# absolute path simply because annoying to recalc rel path if we move this proj
RAND_INCLUDE := /home/seang/Dev/Git/UT_Austin/random123/include/

SRCS := $(shell find $(SRC_DIR) -name "*.c" | xargs -I {} basename {})
OBJS := $(SRCS:%.c=$(OBJS_DIR)%.o)
DEPS := $(OBJS:.o=.d)

LIB_INC := $(shell pkg-config --cflags gtk+-3.0 gl)

INC_DIRS  := $(shell find $(SRC_DIR) -type d)
INC_DIRS  += $(RAND_INCLUDE)

INC_FLAGS := $(addprefix -I,$(INC_DIRS))
INC_FLAGS += $(LIB_INC)

LIBS := $(shell pkg-config --libs gtk+-3.0 gl)

CC 	   := gcc 
CFLAGS := -g -std=c99 -DGL_GLEXT_PROTOTYPES
CFLAGS += $(INC_FLAGS) -MMD -MP 

LD 		 := gcc
LD_FLAGS := -g -Wall $(LIB_INC)

MKDIR := mkdir -p
RM 	  := rm -rf

$(TARGET): $(OBJS)
	$(LD) $(LD_FLAGS) $(OBJS) -o $@ $(LIBS)

$(OBJS_DIR)%.o: $(SRC_DIR)%.c 
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)

.PHONY: clean
clean:
	$(RM) $(OBJS_DIR)*

-include $(DEPS)

