#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
    #include <winsock2.h>
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
#endif

#include "common.h"

// Obstacles pour la collision serveur
struct { int x, y, w, h; } s_obstacles[] = {
    {160, 100, 50, 140}, {580, 340, 50, 140}, {370, 260, 70, 70},
    {120, 430, 90, 50},  {580, 90, 90, 50},   {385, 90, 50, 50}
};
int num_s_obstacles = 6;

int check_server_collision(int x, int y) {
    for(int i=0; i<num_s_obstacles; i++) {
        if (x < s_obstacles[i].x + s_obstacles[i].w && x + PLAYER_SIZE > s_obstacles[i].x &&
            y < s_obstacles[i].y + s_obstacles[i].h && y + PLAYER_SIZE > s_obstacles[i].y) {
            return 1;
        }
    }
    return 0;
}

void reset_round(GameState* gs) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!gs->players[i].connected || gs->players[i].team == 0) continue;

        if (gs->players[i].team == 1) {
            gs->players[i].x = 100;
            gs->players[i].y = 250;
        } else if (gs->players[i].team == 2) {
            gs->players[i].x = 650;
            gs->players[i].y = 250;
        }
        gs->players[i].hp = 100;
        gs->players[i].bullet.active = 0;
    }
    gs->round_active = 1;
}

int main() {
#ifdef _WIN32
    WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serverAddr, clientAddrs[MAX_PLAYERS];
    memset(clientAddrs, 0, sizeof(clientAddrs));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    GameState gameState;
    memset(&gameState, 0, sizeof(gameState));
    gameState.game_active = 0;
    gameState.score_team1 = 0;
    gameState.score_team2 = 0;
    gameState.round_active = 0;

    printf("Serveur Tactique 1v1 / 2v2 sur le port %d...\n", PORT);

    while(1) {
        struct sockaddr_in from;
        socklen_t fromLen = sizeof(from);
        InputPacket input;
        int bytes = recvfrom(sock, (char*)&input, sizeof(input), 0, (struct sockaddr*)&from, &fromLen);
        if (bytes <= 0) continue;

        int p_index = -1;
        for(int i=0; i<MAX_PLAYERS; i++) {
            if(gameState.players[i].connected &&
               clientAddrs[i].sin_addr.s_addr == from.sin_addr.s_addr &&
               clientAddrs[i].sin_port == from.sin_port) {
                p_index = i;
                break;
            }
        }

        if(p_index == -1) {
            for(int i=0; i<MAX_PLAYERS; i++) {
                if(!gameState.players[i].connected) {
                    p_index = i;
                    gameState.players[i].connected = 1;
                    gameState.players[i].hp = 100;
                    gameState.players[i].score = 0;
                    gameState.players[i].team = 0;
                    gameState.players[i].ready = 0;
                    gameState.players[i].bullet.active = 0;
                    gameState.players[i].x = 50 + i*60;
                    gameState.players[i].y = 500;
                    clientAddrs[i] = from;
                    printf("[JOIN] Joueur %d connecte !\n", p_index);
                    break;
                }
            }
        }

        if(p_index == -1) continue;

        Player* p = &gameState.players[p_index];

        // Match terminé : on ignore les inputs sauf si on veut reset (optionnel)
        if (gameState.game_active == 2) {
            // On pourrait gérer un reset ici si besoin
        }

        // Phase de sélection d'équipe / salon
        if (!gameState.game_active) {
            if (input.team_choice == 1) {
                p->team = 1;
                p->ready = 1;
                p->x = 100;
                p->y = 250;
                printf("[TEAM] Joueur %d -> Equipe 1\n", p_index);
            } else if (input.team_choice == 2) {
                p->team = 2;
                p->ready = 1;
                p->x = 650;
                p->y = 250;
                printf("[TEAM] Joueur %d -> Equipe 2\n", p_index);
            }

            int t1 = 0, t2 = 0, total_ready = 0;
            for(int i=0; i<MAX_PLAYERS; i++) {
                if(gameState.players[i].connected && gameState.players[i].ready) {
                    total_ready++;
                    if(gameState.players[i].team == 1) t1++;
                    if(gameState.players[i].team == 2) t2++;
                }
            }

            if (total_ready >= 2 && t1 > 0 && t2 > 0) {
                gameState.game_active = 1;
                gameState.round_active = 0;
                gameState.score_team1 = 0;
                gameState.score_team2 = 0;
                printf("[MATCH] Lancement du match !\n");
            }
        }
        // Match en cours
        else if (gameState.game_active == 1 && p->hp > 0) {
            if (!gameState.round_active) {
                reset_round(&gameState);
            }

            int speed = 4;
            int next_x = p->x;
            int next_y = p->y;

            if(input.up) next_y -= speed;
            if(input.down) next_y += speed;
            if(!check_server_collision(p->x, next_y) &&
               next_y >= 0 && next_y <= WINDOW_HEIGHT-PLAYER_SIZE)
                p->y = next_y;

            if(input.left) next_x -= speed;
            if(input.right) next_x += speed;
            if(!check_server_collision(next_x, p->y) &&
               next_x >= 0 && next_x <= WINDOW_WIDTH-PLAYER_SIZE)
                p->x = next_x;

            if(input.shoot && !p->bullet.active) {
                p->bullet.active = 1;
                p->bullet.x = p->x + PLAYER_SIZE/2;
                p->bullet.y = p->y + PLAYER_SIZE/2;
            }
        }

        // Update projectiles
        if (gameState.game_active == 1 && gameState.round_active) {
            for(int i=0; i<MAX_PLAYERS; i++) {
                if(!gameState.players[i].connected || !gameState.players[i].bullet.active) continue;

                if(gameState.players[i].team == 1)
                    gameState.players[i].bullet.x += 8;
                else if(gameState.players[i].team == 2)
                    gameState.players[i].bullet.x -= 8;

                int bx = gameState.players[i].bullet.x;
                int by = gameState.players[i].bullet.y;

                if(check_server_collision(bx, by) || bx < 0 || bx > WINDOW_WIDTH) {
                    gameState.players[i].bullet.active = 0;
                    continue;
                }

                for(int j=0; j<MAX_PLAYERS; j++) {
                    if(!gameState.players[j].connected ||
                       gameState.players[j].hp <= 0 ||
                       gameState.players[i].team == 0 ||
                       gameState.players[j].team == 0 ||
                       gameState.players[i].team == gameState.players[j].team)
                        continue;

                    if(bx >= gameState.players[j].x && bx <= gameState.players[j].x + PLAYER_SIZE &&
                       by >= gameState.players[j].y && by <= gameState.players[j].y + PLAYER_SIZE) {

                        gameState.players[j].hp -= 25;
                        gameState.players[i].bullet.active = 0;

                        if(gameState.players[j].hp <= 0) {
                            gameState.players[i].score++;
                            if (gameState.players[i].team == 1)
                                gameState.score_team1++;
                            else if (gameState.players[i].team == 2)
                                gameState.score_team2++;

                            printf("[KILL] Joueur %d tue Joueur %d | Score T1:%d T2:%d\n",
                                   i, j, gameState.score_team1, gameState.score_team2);

                            gameState.round_active = 0;

                            if (gameState.score_team1 >= 6 || gameState.score_team2 >= 6) {
                                gameState.game_active = 2;
                                printf("[MATCH] Victoire de l'equipe %d !\n",
                                       (gameState.score_team1 >= 6) ? 1 : 2);
                            }
                        }
                        break;
                    }
                }
            }
        }

        // Broadcast
        for(int i=0; i<MAX_PLAYERS; i++) {
            if(gameState.players[i].connected) {
                sendto(sock, (char*)&gameState, sizeof(gameState), 0,
                       (struct sockaddr*)&clientAddrs[i], sizeof(clientAddrs[i]));
            }
        }
    }

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif
    return 0;
}