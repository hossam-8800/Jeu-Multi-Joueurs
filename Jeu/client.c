#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

enum { STATE_MENU, STATE_GAME };

int main(int argc, char* argv[]) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) return 1;
#endif

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("LAN Blaster - CONFIGURATION RESEAU", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int running = 1;
    int appState = STATE_MENU;
    char server_ip[64] = "127.0.0.1";
    int ip_changed = 1; 

    // Génération d'un décor d'étoiles fixe en arrière-plan
    int starX[80], starY[80];
    srand(time(NULL));
    for(int i=0; i<80; i++) {
        starX[i] = rand() % WINDOW_WIDTH;
        starY[i] = rand() % WINDOW_HEIGHT;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);
#else
    fcntl(sock, F_SETFL, O_NONBLOCK);
#endif

    GameState gameState;
    memset(&gameState, 0, sizeof(GameState));

    SDL_StartTextInput();

    while(running) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) running = 0;

            if (appState == STATE_MENU) {
                if (event.type == SDL_TEXTINPUT) {
                    if (strlen(server_ip) < 60) {
                        strcat(server_ip, event.text.text);
                        ip_changed = 1; 
                    }
                }
                else if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(server_ip) > 0) {
                        server_ip[strlen(server_ip) - 1] = '\0';
                        ip_changed = 1; 
                    }
                    else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                        serverAddr.sin_addr.s_addr = inet_addr(server_ip);
                        SDL_StopTextInput();
                        appState = STATE_GAME;
                    }
                }
            }
        }

        // --- RENDU MENU ---
        if (appState == STATE_MENU) {
            SDL_SetRenderDrawColor(renderer, 15, 20, 35, 255);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 30, 45, 70, 255);
            SDL_Rect ipBox = { WINDOW_WIDTH/2 - 250, WINDOW_HEIGHT/2 - 40, 500, 80 };
            SDL_RenderFillRect(renderer, &ipBox);

            SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &ipBox);

            if (ip_changed) {
                char menuTitle[128];
                snprintf(menuTitle, sizeof(menuTitle), "LAN Blaster | REJOINDRE L'IP : [%s] (Appuie sur ENTRÉE)", server_ip);
                SDL_SetWindowTitle(window, menuTitle);
                ip_changed = 0; 
            }
        }
        // --- RENDU JEU ---
        else if (appState == STATE_GAME) {
            const Uint8* keys = SDL_GetKeyboardState(NULL);
            InputPacket input = {0};

            if(keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) input.up = 1;
            if(keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) input.down = 1;
            if(keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) input.left = 1;
            if(keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) input.right = 1;
            if(keys[SDL_SCANCODE_SPACE]) input.shoot = 1;

            sendto(sock, (char*)&input, sizeof(input), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

#ifdef _WIN32
            int len = sizeof(serverAddr);
#else
            socklen_t len = sizeof(serverAddr);
#endif
            GameState latestState;
            while(recvfrom(sock, (char*)&latestState, sizeof(latestState), 0, (struct sockaddr*)&serverAddr, &len) > 0) {
                gameState = latestState;
            }

            // 1. Fond Espace d'ambiance
            SDL_SetRenderDrawColor(renderer, 10, 10, 18, 255);
            SDL_RenderClear(renderer);

            // Dessin des étoiles en fond
            SDL_SetRenderDrawColor(renderer, 200, 200, 255, 180);
            for(int i=0; i<80; i++) {
                SDL_RenderDrawPoint(renderer, starX[i], starY[i]);
            }

            // 2. Lignes de grille Sci-Fi au sol et plafond
            SDL_SetRenderDrawColor(renderer, 30, 40, 70, 255);
            for(int x = 0; x < WINDOW_WIDTH; x += 80) {
                SDL_RenderDrawLine(renderer, x, 0, x, WINDOW_HEIGHT);
            }
            for(int y = 0; y < WINDOW_HEIGHT; y += 80) {
                SDL_RenderDrawLine(renderer, 0, y, WINDOW_WIDTH, y);
            }

            // 3. Obstacles stylisés avec contour Néon
            SDL_Rect obstacles[] = {{250, 50, 40, 300}, {500, 250, 40, 300}};
            for(int o=0; o<2; o++) {
                SDL_SetRenderDrawColor(renderer, 35, 35, 45, 255); // Intérieur sombre
                SDL_RenderFillRect(renderer, &obstacles[o]);
                SDL_SetRenderDrawColor(renderer, 0, 180, 255, 255); // Contour Bleu Cyan clair
                SDL_RenderDrawRect(renderer, &obstacles[o]);
            }

            // 4. Boss Ennemi (Carré violet/rose d'arcade)
            SDL_SetRenderDrawColor(renderer, 255, 0, 128, 255);
            SDL_Rect enemy = {gameState.enemy.x, gameState.enemy.y, PLAYER_SIZE, PLAYER_SIZE};
            SDL_RenderFillRect(renderer, &enemy);

            // Barre de vie adaptative (Vert -> Orange -> Rouge)
            if (gameState.enemy.hp > 50) SDL_SetRenderDrawColor(renderer, 50, 255, 50, 255);
            else if (gameState.enemy.hp > 20) SDL_SetRenderDrawColor(renderer, 255, 150, 0, 255);
            else SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
            SDL_Rect enemyHPBar = {gameState.enemy.x, gameState.enemy.y - 12, gameState.enemy.hp / 2, 6};
            SDL_RenderFillRect(renderer, &enemyHPBar);

            // 5. Joueurs et Lasers éclair
            for(int i=0; i<MAX_PLAYERS; i++) {
                if(gameState.players[i].connected) {
                    if (i == 0) SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);      // J1 Rouge vif
                    else if (i == 1) SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);   // J2 Cyan néon
                    else if (i == 2) SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);   // J3 Jaune
                    else SDL_SetRenderDrawColor(renderer, 150, 50, 250, 255);

                    SDL_Rect playerRect = {gameState.players[i].x, gameState.players[i].y, PLAYER_SIZE, PLAYER_SIZE};
                    SDL_RenderFillRect(renderer, &playerRect);

                    // Projectiles : Traînées de laser de feu
                    if(gameState.players[i].bullet.active) {
                        SDL_SetRenderDrawColor(renderer, 255, 200, 50, 255);
                        SDL_Rect bulletRect = {gameState.players[i].bullet.x, gameState.players[i].bullet.y, 35, 8};
                        SDL_RenderFillRect(renderer, &bulletRect);
                    }
                }
            }

            // 6. MISE À JOUR DU DASHBOARD / LIVE HUD DANS LE TITRE
            char hudTitle[256];
            snprintf(hudTitle, sizeof(hudTitle), 
                     "LAN BLASTER | BOSS HP: %d%% | SCORE -> J0: %d  ||  J1: %d", 
                     gameState.enemy.hp, gameState.players[0].score, gameState.players[1].score);
            SDL_SetWindowTitle(window, hudTitle);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#endif
    return 0;
}