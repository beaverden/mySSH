CC := g++
BUILD_DIR := ./build
SRCEXT := cpp

# Server
TARGET_SERVER := ./bin/server
SERVER_SRC	:= ./server/src
SERVER_INC  := ./server/include
SERVER_INC_FLAGS := $(addprefix -I,$(SERVER_INC))
SERVER_SOURCES := $(shell find $(SERVER_SRC) -type f -name *.$(SRCEXT))
SERVER_OBJECTS := $(patsubst $(SERVER_SRC)/%, $(BUILD_DIR)/%, $(SERVER_SOURCES:.$(SRCEXT)=.o))

# Client 
TARGET_CLIENT := ./bin/client
CLIENT_SRC	:= ./client/src
CLIENT_INC  := ./client/include
SERVER_INC_FLAGS := $(addprefix -I,$(CLIENT_INC))
CLIENT_SOURCES := $(shell find $(CLIENT_SRC) -type f -name *.$(SRCEXT))
CLIENT_OBJECTS := $(patsubst $(CLIENT_SRC)/%, $(BUILD_DIR)/%, $(CLIENT_SOURCES:.$(SRCEXT)=.o))

COMMON_INCLUDE := /usr/include/openssl
COMMON_FLAGS := $(addprefix -I,$(COMMON_INCLUDE))

FLAGS := -std=c++11 #-Wall 
LIB := -L/usr/lib -lssl -lcrypto


server:
	$(CC) $(SERVER_SOURCES) -o $(TARGET_SERVER) $(FLAGS) $(SERVER_INC_FLAGS) $(COMMON_FLAGS) $(LIB) 

client:
	$(CC) $(CLIENT_SOURCES) -o $(TARGET_CLIENT) $(FLAGS) $(CLIENT_INC_FLAGS) $(COMMON_FLAGS) $(LIB) 

.PHONY: clean 

clean:
	@echo "Cleaning ..."
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
