BIN_NAME    = socksProxy 
SRC_DIR     = src
BIN_DIR     = bin
INCLUDE_DIR = include

CC = clang
CFLAGS = -Wall -Wextra -Werror=implicit-function-declaration -Wpedantic -O2 -g

INCLUDE_PATHS = -I$(INCLUDE_DIR)

SRC_FILES = $(shell find $(SRC_DIR) -type f -name "*.c")
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.o, $(SRC_FILES))

SUDO		  	= sudo
DEL_FILE      	= rm -f
CHK_DIR_EXISTS	= test -d
MKDIR         	= mkdir -p
COPY          	= cp -f
COPY_FILE     	= cp -f
COPY_DIR      	= cp -f -R
INSTALL_FILE   	= install -m 644 -p
INSTALL_PROGRAM = install -m 755 -p
INSTALL_DIR   	= cp -f -R
DEL_FILE      	= rm -f
SYMLINK       	= ln -f -s
DEL_DIR       	= rmdir
MOVE          	= mv -f
TAR           	= tar -cf
COMPRESS      	= gzip -9f
LIBS_DIRS     	= -I./include/
SED           	= sed
STRIP         	= strip

all: $(BIN_DIR)/$(BIN_NAME)

build: $(BIN_DIR)/$(BIN_NAME)

$(BIN_DIR)/$(BIN_NAME): $(OBJ_FILES)
	@echo "CC 		| $@"
	@mkdir -p $(BIN_DIR)
	@$(CC) $(INCLUDE_PATHS) -o $@ $(OBJ_FILES) 
	@$(DEL_FILE) $(OBJ_FILES)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "CC 		| $@"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INCLUDE_PATHS) -c $< -o $@

clean:
	@echo "Clean..."
	@rm -rf $(BIN_DIR)

run: all
	@echo "Running $(BIN_NAME)" ; echo
	@./$(BIN_DIR)/$(BIN_NAME)
