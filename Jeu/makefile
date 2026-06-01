# --- CONFIG SDL2 (MSYS2 MinGW64) ---
SDL2_INC = C:/msys64/mingw64/include
SDL2_LIB = C:/msys64/mingw64/lib

# --- COMPILATEUR ---
CC = gcc

# --- FLAGS ---
CFLAGS = -I$(SDL2_INC)
LDFLAGS_CLIENT = -L$(SDL2_LIB) -lmingw32 -lSDL2main -lSDL2 -lws2_32
LDFLAGS_SERVER = -lws2_32

# --- CIBLES ---
all: server client

server: server.c common.h
    $(CC) server.c -o server.exe $(LDFLAGS_SERVER)

client: client.c common.h
    $(CC) client.c -o client.exe $(CFLAGS) $(LDFLAGS_CLIENT)

clean:
    del /Q server.exe client.exe 2>nul || true
