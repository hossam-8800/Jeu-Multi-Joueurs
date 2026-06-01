#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

enum { STATE_MENU, STATE_GAME };

int main(int argc, char* argv[]) {
    // 1. INITIALISATION RÉSEAU WINDOWS (Impérativement AVANT le socket)
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("[ERREUR] Échec de l'initialisation Winsock.\n");
        return 1;
    }
#endif

    // Affichage préventif du tutoriel dans le terminal
    printf("=========================================\n");
    printf("         LAN BLASTER - TUTORIEL          \n");
    printf("=========================================\n");
    printf("- Déplacements : Touches Z, Q, S, D ou Flèches\n");
    printf("- Tirer : Barre ESPACE (Tir vers la droite)\n");
    printf("- Configuration : Saisissez l'IP directement dans la fenêtre du jeu.\n");
    printf("=========================================\n\n");

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("LAN Blaster - CONFIGURATION RESEAU", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int running = 1;
    int appState = STATE_MENU;
    char server_ip[64] = "127.0.0.1"; // IP par défaut
    int ip_changed = 1; 

    // Création du socket
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

    // Activer la saisie de texte SDL
    SDL_StartTextInput();

    while(running) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) running = 0;

            // --- GESTION DU MENU DE CONFIGURATION RESEAU ---
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
                        SDL_SetWindowTitle(window, "LAN Blaster - En Jeu !");
                        printf("\n[OK] IP validée : %s. Passage en mode JEU !\n", server_ip);
                    }
                }
            }
        }

        // --- PHASE DE MENU ---
        if (appState == STATE_MENU) {
            SDL_SetRenderDrawColor(renderer, 25, 35, 55, 255);
            SDL_RenderClear(renderer);

            // Dessin du champ de saisie central (vide)
            SDL_SetRenderDrawColor(renderer, 45, 60, 90, 255);
            SDL_Rect ipBox = { WINDOW_WIDTH/2 - 250, WINDOW_HEIGHT/2 - 40, 500, 80 };
            SDL_RenderFillRect(renderer, &ipBox);

            SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &ipBox);

            // MISE À JOUR DU TITRE + LOG TERMINAL
            if (ip_changed) {
                char menuTitle[128];
                snprintf(menuTitle, sizeof(menuTitle), "TAPEZ L'IP : [%s] (Puis ENTRÉE)", server_ip);
                SDL_SetWindowTitle(window, menuTitle);
                
                // Log de secours pour voir ce que tu écris directement dans VS Code
                printf("\rSaisie IP actuelle : %s          ", server_ip);
                fflush(stdout); 

                ip_changed = 0; 
            }
        }
        // --- PHASE DE JEU ---
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

            SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
            SDL_RenderClear(renderer);

            // Obstacles
            SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
            SDL_Rect wall1 = {250, 50, 40, 300};
            SDL_Rect wall2 = {500, 250, 40, 300};
            SDL_RenderFillRect(renderer, &wall1);
            SDL_RenderFillRect(renderer, &wall2);

            // Cible Ennemie
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_Rect enemy = {gameState.enemy.x, gameState.enemy.y, PLAYER_SIZE, PLAYER_SIZE};
            SDL_RenderFillRect(renderer, &enemy);

            // Vie Ennemi
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_Rect enemyHPBar = {gameState.enemy.x, gameState.enemy.y - 12, gameState.enemy.hp / 2, 6};
            SDL_RenderFillRect(renderer, &enemyHPBar);

            // Joueurs
            for(int i=0; i<MAX_PLAYERS; i++) {
                if(gameState.players[i].connected) {
                    if (i == 0) SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
                    else if (i == 1) SDL_SetRenderDrawColor(renderer, 50, 50, 255, 255);
                    else if (i == 2) SDL_SetRenderDrawColor(renderer, 255, 255, 50, 255);
                    else SDL_SetRenderDrawColor(renderer, 255, 100, 200, 255);

                    SDL_Rect playerRect = {gameState.players[i].x, gameState.players[i].y, PLAYER_SIZE, PLAYER_SIZE};
                    SDL_RenderFillRect(renderer, &playerRect);

                    if(gameState.players[i].bullet.active) {
                        SDL_SetRenderDrawColor(renderer, 255, 255, 100, 255);
                        SDL_Rect bulletRect = {gameState.players[i].bullet.x, gameState.players[i].bullet.y, 30, 12};
                        SDL_RenderFillRect(renderer, &bulletRect);
                    }
                }
            }
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
#else
    close(sock);
#endif
    return 0;
}