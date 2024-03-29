CC := g++
BUILD_DIR := ./build/*
BIN_DIR := ./bin/*
SRCEXT := cpp

# Server
TARGET_SERVER := ./bin/server
SERVER_SRC	:= ./server/src ./common/src
SERVER_INC  := ./server/include ./common/include
SERVER_INC_FLAGS := $(addprefix -I,$(SERVER_INC))
SERVER_SOURCES := $(shell find $(SERVER_SRC) -type f -name *.$(SRCEXT))

# Client 
TARGET_CLIENT := ./bin/client
CLIENT_SRC	:= ./client/src ./common/src
CLIENT_INC  := ./client/include ./common/include
CLIENT_INC_FLAGS := $(addprefix -I,$(CLIENT_INC))
CLIENT_SOURCES := $(shell find $(CLIENT_SRC) -type f -name *.$(SRCEXT))

# Shell
TARGET_SHELL := ./bin/shell
SHELL_SRC	 := ./shell/src
SHELL_INC    := ./shell/include
SHELL_INC_FLAGS := $(addprefix -I,$(SHELL_INC))
SHELL_SOURCES := $(shell find $(SHELL_SRC) -type f -name *.$(SRCEXT))

COMMON_FLAGS := $(addprefix -I,$(COMMON_INCLUDE))

FLAGS := -std=c++11 -Wno-format-security #-Wall 
CRYPT_INC := /usr/include/openssl
CRYPT_FLG := $(addprefix -I,$(CRYPT_INC))
CRYPT_LIB := -L/usr/lib -lssl -lcrypto -lcrypt
LIB := -pthread -lpthread


server:
	$(CC) $(SERVER_SOURCES) -o $(TARGET_SERVER) $(FLAGS) $(SERVER_INC_FLAGS) $(COMMON_FLAGS) $(CRYPT_FLG) $(CRYPT_LIB) $(LIB) 

client:
	$(CC) $(CLIENT_SOURCES) -o $(TARGET_CLIENT) $(FLAGS) $(CLIENT_INC_FLAGS) $(COMMON_FLAGS) $(CRYPT_FLG) $(CRYPT_LIB) $(LIB) 

shell:
	$(CC) $(SHELL_SOURCES) -o $(TARGET_SHELL) $(FLAGS) $(SHELL_INC_FLAGS) $(COMMON_FLAGS) $(LIB) 

.PHONY: clean 

clean:
	@echo "Cleaning ..."
	$(RM) -r $(BUILD_DIR)
	$(RM) -r $(BIN_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
