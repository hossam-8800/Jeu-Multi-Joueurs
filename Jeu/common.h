#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>
#endif

#define PORT 5000

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define PLAYER_SIZE 40
#define PLAYER_SPEED 6
#define BULLET_SPEED 14
#define MAX_PLAYERS 4

// Force l'alignement memoire strict a 1 octet pour eviter les bugs de desynchronisation reseau
#pragma pack(push, 1)

typedef struct {
    int up;
    int down;
    int left;
    int right;
    int shoot;
} InputPacket;

typedef struct {
    int active;
    int x;
    int y;
} Bullet;

typedef struct {
    int connected;
    int x;
    int y;
    int hp;
    int score;
    Bullet bullet; 
} Player;

typedef struct {
    int x;
    int y;
    int hp;
} Enemy;

typedef struct {
    Player players[MAX_PLAYERS];
    Enemy enemy;
} GameState;

#pragma pack(pop)

#endif