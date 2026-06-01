#include "common.h"

GameState gameState;
struct sockaddr_in clients[MAX_PLAYERS];

void initGame() {
    for(int i=0; i<MAX_PLAYERS; i++) {
        gameState.players[i].connected = 0;
        gameState.players[i].score = 0;
        gameState.players[i].bullet.active = 0;
    }
    gameState.enemy.hp = 100;
    gameState.enemy.x = 650;
    gameState.enemy.y = 280;
    gameState.enemy.dirY = 1; 
    gameState.enemy.bullet.active = 0;
}

int main() {
    srand(time(NULL));
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    printf("[SERVEUR] Lance sur le port %d. En attente de joueurs...\n", PORT);

#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);
#else
    fcntl(sock, F_SETFL, O_NONBLOCK);
#endif

    initGame();

    while(1) {
        InputPacket input;
        struct sockaddr_in clientAddr;
#ifdef _WIN32
        int len = sizeof(clientAddr);
#else
        socklen_t len = sizeof(clientAddr);
#endif

        while(recvfrom(sock, (char*)&input, sizeof(input), 0, (struct sockaddr*)&clientAddr, &len) > 0) {
            int playerId = -1;
            for(int i=0; i<MAX_PLAYERS; i++) {
                if(gameState.players[i].connected && 
                   clients[i].sin_addr.s_addr == clientAddr.sin_addr.s_addr &&
                   clients[i].sin_port == clientAddr.sin_port) {
                    playerId = i;
                    break;
                }
            }

            if(playerId == -1) {
                for(int i=0; i<MAX_PLAYERS; i++) {
                    if(!gameState.players[i].connected) {
                        playerId = i;
                        gameState.players[i].connected = 1;
                        gameState.players[i].x = 80;
                        gameState.players[i].y = 150 + i * 100;
                        gameState.players[i].hp = 100; // Init HP joueur
                        clients[i] = clientAddr;
                        printf("[CONNEXION] Joueur %d connecte !\n", i);
                        break;
                    }
                }
            }

            if(playerId != -1) {
                Player* p = &gameState.players[playerId];
                if(input.up) p->y -= PLAYER_SPEED;
                if(input.down) p->y += PLAYER_SPEED;
                if(input.left) p->x -= PLAYER_SPEED;
                if(input.right) p->x += PLAYER_SPEED;

                if(p->x < 0) p->x = 0;
                if(p->y < 0) p->y = 0;
                if(p->x > WINDOW_WIDTH - PLAYER_SIZE) p->x = WINDOW_WIDTH - PLAYER_SIZE;
                if(p->y > WINDOW_HEIGHT - PLAYER_SIZE) p->y = WINDOW_HEIGHT - PLAYER_SIZE;

                if(input.shoot && !p->bullet.active) {
                    p->bullet.x = p->x + PLAYER_SIZE;
                    p->bullet.y = p->y + (PLAYER_SIZE / 2) - 4;
                    p->bullet.active = 1;
                }
            }
        }

        // --- IA DU BOSS (Mouvements) ---
        gameState.enemy.y += (6 * gameState.enemy.dirY); 
        if(gameState.enemy.y <= 10 || gameState.enemy.y >= WINDOW_HEIGHT - PLAYER_SIZE - 10) {
            gameState.enemy.dirY *= -1; 
        }

        // --- IA DU BOSS (Tir automatique style 1v1) ---
        static int bossShootCooldown = 0;
        bossShootCooldown++;
        if(bossShootCooldown >= 35) { // Le boss tente de tirer toutes les 35 frames
            if(!gameState.enemy.bullet.active) {
                gameState.enemy.bullet.x = gameState.enemy.x - 20;
                gameState.enemy.bullet.y = gameState.enemy.y + (PLAYER_SIZE / 2) - 4;
                gameState.enemy.bullet.active = 1;
            }
            bossShootCooldown = 0;
        }

        // Déplacement et collision de la balle du Boss (droite vers gauche)
        if(gameState.enemy.bullet.active) {
            gameState.enemy.bullet.x -= BULLET_SPEED;
            if(gameState.enemy.bullet.x < 0) {
                gameState.enemy.bullet.active = 0;
            }

            // Vérification impact sur un des joueurs connectés
            for(int i=0; i<MAX_PLAYERS; i++) {
                if(gameState.players[i].connected && gameState.enemy.bullet.active) {
                    Player* p = &gameState.players[i];
                    if(gameState.enemy.bullet.x < p->x + PLAYER_SIZE &&
                       gameState.enemy.bullet.x + 25 > p->x &&
                       gameState.enemy.bullet.y + 8 > p->y &&
                       gameState.enemy.bullet.y < p->y + PLAYER_SIZE) {
                        
                        p->hp -= 20; // Le joueur prend cher (-20 HP)
                        gameState.enemy.bullet.active = 0;

                        if(p->hp <= 0) { // Mort du joueur
                            p->hp = 100;
                            p->x = 80;
                            p->y = 150 + i * 100;
                            if(p->score > 0) p->score--; 
                        }
                    }
                }
            }
        }

        // Mise à jour des projectiles des joueurs
        for(int i=0; i<MAX_PLAYERS; i++) {
            if(gameState.players[i].connected && gameState.players[i].bullet.active) {
                gameState.players[i].bullet.x += BULLET_SPEED;
                
                if(gameState.players[i].bullet.x > WINDOW_WIDTH) {
                    gameState.players[i].bullet.active = 0;
                }

                if(gameState.players[i].bullet.active &&
                   gameState.players[i].bullet.x + 35 > gameState.enemy.x &&
                   gameState.players[i].bullet.x < gameState.enemy.x + PLAYER_SIZE &&
                   gameState.players[i].bullet.y + 8 > gameState.enemy.y &&
                   gameState.players[i].bullet.y < gameState.enemy.y + PLAYER_SIZE) {
                    
                    gameState.enemy.hp -= 10;
                    gameState.players[i].bullet.active = 0;

                    if(gameState.enemy.hp <= 0) {
                        gameState.players[i].score++;
                        gameState.enemy.hp = 100;
                        gameState.enemy.y = rand() % 400 + 50; 
                    }
                }
            }
        }

        for(int i=0; i<MAX_PLAYERS; i++) {
            if(gameState.players[i].connected) {
                sendto(sock, (char*)&gameState, sizeof(gameState), 0, 
                       (struct sockaddr*)&clients[i], sizeof(clients[i]));
            }
        }

#ifdef _WIN32
        Sleep(16);
#else
        usleep(16000);
#endif
    }
    return 0;
}