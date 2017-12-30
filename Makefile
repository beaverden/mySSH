CC := g++
TARGET := ./bin/server
BUILD_DIR := ./build
SRC_DIR	:= ./src
INCLUDE := ./include /usr/include/openssl

SRCEXT := cpp
SRCS := $(shell find $(SRC_DIR) -type f -name *.$(SRCEXT))
OBJS := $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%, $(SRCS:.$(SRCEXT)=.o))
FLAGS := -std=c++11 #-Wall 
LIB := -L/usr/lib -lssl -lcrypto
INC_DIRS := $(INCLUDE)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))


build:
	$(CC) $(SRCS) -o $(TARGET) $(FLAGS) $(INC_FLAGS) $(LIB) 

.PHONY: clean

clean:
	@echo "Cleaning ..."
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
