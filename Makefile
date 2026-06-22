CC = gcc
CFLAGS = -g  -I src/include -xc -std=c11
LDFLAGS = -L src/lib
LIBS = -l:raylib.dll -lopengl32 -lgdi32 -lwinmm -lcomdlg32

SRC = $(wildcard src/*.c) $(wildcard src/tools/*.c)
OBJ = $(patsubst src/%.c, build/%.o, $(patsubst src/tools/%.c, build/tools/%.o, $(SRC)))
TARGET = notitze.exe
BIN_DIR = build\bin
BIN_TARGET = $(BIN_DIR)/$(TARGET)


.PHONY: all clean

all: $(BIN_TARGET)

$(BIN_TARGET): $(OBJ)
	@cmd /C "if not exist build mkdir build"
	@cmd /C "if not exist $(BIN_DIR) mkdir $(BIN_DIR)"
	$(CC) $(OBJ) $(LDFLAGS) $(LIBS) -o $@
	@cmd /C "if exist src\\lib\\raylib.dll copy /Y src\\lib\\raylib.dll $(BIN_DIR)\\ >nul"

build/%.o: src/%.c
	@cmd /C "if not exist build mkdir build"
	$(CC) $(CFLAGS) -c $< -o $@

build/tools/%.o: src/tools/%.c
	@cmd /C "if not exist build\tools mkdir build\tools"
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-cmd /C rmdir /S /Q build 2>nul
