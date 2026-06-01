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

// Définition rigide des mêmes caisses/obstacles sur le serveur
struct { int x, y, w, h; } s_obstacles[] = {
    {160, 100, 50, 140}, {580, 340, 50, 140}, {370, 260, 70, 70},
    {120, 430, 90, 50},  {580, 90, 90, 50},   {385, 90, 50, 50}
};
int num_s_obstacles = 6;

int check_server_collision(int x, int y) {
    for(int i=0; i<num_s_obstacles; i++) {
        if (x < s_obstacles[i].x + s_obstacles[i].w && x + PLAYER_SIZE > s_obstacles[i].x &&
            y < s_obstacles[i].y + s_obstacles[i].h && y + PLAYER_SIZE > s_obstacles[i].y) {
            return 1; // Collision !
        }
    }
    return 0;
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

    GameState gameState; memset(&gameState, 0, sizeof(gameState));
    printf("Serveur Tactique 1v1 / 2v2 instancie sur le port %d...\n", PORT);

    while(1) {
        struct sockaddr_in from; socklen_t fromLen = sizeof(from);
        InputPacket input;
        int bytes = recvfrom(sock, (char*)&input, sizeof(input), 0, (struct sockaddr*)&from, &fromLen);
        if (bytes <= 0) continue;

        int p_index = -1;
        for(int i=0; i<MAX_PLAYERS; i++) {
            if(gameState.players[i].connected &&
               clientAddrs[i].sin_addr.s_addr == from.sin_addr.s_addr &&
               clientAddrs[i].sin_port == from.sin_port) {
                p_index = i; break;
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
                    // Position de spawn initiale au salon
                    gameState.players[i].x = 50 + i*60;
                    gameState.players[i].y = 500;
                    clientAddrs[i] = from;
                    printf("[JOIN] Joueur %d connecte !\n", p_index);
                    break;
                }
            }
        }

        if(p_index == -1) continue; // Complet

        Player* p = &gameState.players[p_index];

        // Étape 1 : Phase de sélection de l'équipe (Salon d'attente)
        if (!gameState.game_active) {
            if (input.team_choice == 1) {
                p->team = 1; p->ready = 1; p->x = 100; p->y = 250;
                printf("[TEAM] Joueur %d -> Equipe 1\n", p_index);
            }
            if (input.team_choice == 2) {
                p->team = 2; p->ready = 1; p->x = 650; p->y = 250;
                printf("[TEAM] Joueur %d -> Equipe 2\n", p_index);
            }

            // Lancer la partie si au moins 2 joueurs sont prêts dans des équipes différentes
            int t1 = 0, t2 = 0, total_ready = 0;
            for(int i=0; i<MAX_PLAYERS; i++) {
                if(gameState.players[i].connected && gameState.players[i].ready) {
                    total_ready++;
                    if(gameState.players[i].team == 1) t1++;
                    if(gameState.players[i].team == 2) t2++;
                }
            }
            if (total_ready >= 2 && t1 > 0 && t2 > 0) {
                gameState.game_active = 1; // Lancement du match 1v1 ou 2v2 !
                printf("[MATCH] Tout le monde est pret ! Lancement de l'affrontement.\n");
            }
        } 
        // Étape 2 : Phase de Jeu Active
        else if (gameState.game_active && p->hp > 0) {
            int speed = 4;
            int next_x = p->x, next_y = p->y;

            if(input.up) next_y -= speed;
            if(input.down) next_y += speed;
            if(!check_server_collision(p->x, next_y) && next_y >= 0 && next_y <= WINDOW_HEIGHT-PLAYER_SIZE)
                p->y = next_y;

            if(input.left) next_x -= speed;
            if(input.right) next_x += speed;
            if(!check_server_collision(next_x, p->y) && next_x >= 0 && next_x <= WINDOW_WIDTH-PLAYER_SIZE)
                p->x = next_x;

            // Gestion du tir rectiligne (droite si équipe 1, gauche si équipe 2)
            if(input.shoot && !p->bullet.active) {
                p->bullet.active = 1;
                p->bullet.x = p->x + PLAYER_SIZE/2;
                p->bullet.y = p->y + PLAYER_SIZE/2;
            }
        }

        // Update de la physique des projectiles réseau
        for(int i=0; i<MAX_PLAYERS; i++) {
            if(gameState.players[i].connected && gameState.players[i].bullet.active) {
                // Équipe 1 tire vers la droite, Équipe 2 vers la gauche
                if(gameState.players[i].team == 1) gameState.players[i].bullet.x += 8;
                else gameState.players[i].bullet.x -= 8;

                int bx = gameState.players[i].bullet.x;
                int by = gameState.players[i].bullet.y;

                // Collision balle contre obstacle
                if(check_server_collision(bx, by) || bx < 0 || bx > WINDOW_WIDTH) {
                    gameState.players[i].bullet.active = 0;
                }

                // Collision balle contre joueur adverse (Friendly Fire désactivé)
                for(int j=0; j<MAX_PLAYERS; j++) {
                    if(gameState.players[j].connected &&
                       gameState.players[j].hp > 0 &&
                       gameState.players[i].team != 0 &&
                       gameState.players[j].team != 0 &&
                       gameState.players[i].team != gameState.players[j].team) {

                        if(bx >= gameState.players[j].x && bx <= gameState.players[j].x + PLAYER_SIZE &&
                           by >= gameState.players[j].y && by <= gameState.players[j].y + PLAYER_SIZE) {
                            gameState.players[j].hp -= 25; // 4 balles pour tuer
                            gameState.players[i].bullet.active = 0;
                            if(gameState.players[j].hp <= 0) {
                                gameState.players[i].score++;
                                printf("[KILL] Joueur %d tue Joueur %d\n", i, j);
                            }
                        }
                    }
                }
            }
        }

        // Broadcast de l'état du monde à tous les participants
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