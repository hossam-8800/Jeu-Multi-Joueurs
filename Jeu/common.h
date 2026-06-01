#ifndef COMMON_H
#define COMMON_H

#define PORT 8888
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define PLAYER_SIZE 32
#define MAX_PLAYERS 4

// Constantes pour les modes vidéo
enum { VIDEO_WINDOWED, VIDEO_BORDERLESS, VIDEO_FULLSCREEN };

typedef struct {
    int up;
    int down;
    int left;
    int right;
    int shoot;
    int team_choice; // 0: pas choisi, 1: Équipe 1, 2: Équipe 2
} InputPacket;

typedef struct {
    int x, y;
    int active;
} Bullet;

typedef struct {
    int x, y;
    int hp;
    int score;
    int connected;
    int team;        // 1 ou 2
    int ready;       // Si le joueur a choisi son équipe
    Bullet bullet;
} Player;

typedef struct {
    Player players[MAX_PLAYERS];
    int game_active;     // 0: Salon / sélection, 1: Match en cours, 2: Match terminé
    int score_team1;
    int score_team2;
    int round_active;    // 0: entre manches, 1: manche en cours
} GameState;

#endif