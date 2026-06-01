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
#include <fcntl.h>
#endif

#define PORT 5000
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define PLAYER_SIZE 40
#define PLAYER_SPEED 9   
#define BULLET_SPEED 22  
#define MAX_PLAYERS 4

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
    int dirY; 
    Bullet bullet; // Nouvelle variable : la balle du Boss
} Enemy;

typedef struct {
    Player players[MAX_PLAYERS];
    Enemy enemy;
} GameState;

#pragma pack(pop)

#endif