#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

enum { STATE_MENU, STATE_GAME };

// --- MINI MOTEUR DE TEXTE VECTORIEL POUR L'IP (SANS FONT/TTF STRUCT) ---
void draw_digit(SDL_Renderer* r, char c, int x, int y) {
    int w = 14, h = 22;
    // Segments d'affichage rétro basés sur le caractère reçu
    int top = (c=='0'||c=='2'||c=='3'||c=='5'||c=='6'||c=='7'||c=='8'||c=='9');
    int tl  = (c=='0'||c=='4'||c=='5'||c=='6'||c=='8'||c=='9');
    int tr  = (c=='0'||c=='1'||c=='2'||c=='3'||c=='4'||c=='7'||c=='8'||c=='9');
    int mid = (c=='2'||c=='3'||c=='4'||c=='5'||c=='6'||c=='8'||c=='9');
    int bl  = (c=='0'||c=='2'||c=='6'||c=='8');
    int br  = (c=='0'||c=='1'||c=='3'||c=='4'||c=='5'||c=='6'||c=='7'||c=='8'||c=='9');
    int bot = (c=='0'||c=='2'||c=='3'||c=='5'||c=='6'||c=='8'||c=='9');

    if(top) SDL_RenderDrawLine(r, x, y, x + w, y);
    if(tl)  SDL_RenderDrawLine(r, x, y, x, y + h/2);
    if(tr)  SDL_RenderDrawLine(r, x + w, y, x + w, y + h/2);
    if(mid) SDL_RenderDrawLine(r, x, y + h/2, x + w, y + h/2);
    if(bl)  SDL_RenderDrawLine(r, x, y + h/2, x, y + h);
    if(br)  SDL_RenderDrawLine(r, x + w, y + h/2, x + w, y + h);
    if(bot) SDL_RenderDrawLine(r, x, y + h, x + w, y + h);
    if(c == '.') {
        SDL_Rect dot = { x + w/2 - 2, y + h - 3, 5, 5 };
        SDL_RenderFillRect(r, &dot);
    }
}

void draw_ip_string(SDL_Renderer* r, const char* str, int x, int y) {
    while(*str) {
        draw_digit(r, *str, x, y);
        x += 22; // Espace entre chaque chiffre
        str++;
    }
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) return 1;
#endif

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("LAN Blaster", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int running = 1;
    int appState = STATE_MENU;
    char server_ip[64] = "127.0.0.1";

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
                    // Autorise uniquement les chiffres et les points pour l'IP
                    if (strlen(server_ip) < 25 && ((*event.text.text >= '0' && *event.text.text <= '9') || *event.text.text == '.')) {
                        strcat(server_ip, event.text.text);
                    }
                }
                else if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(server_ip) > 0) {
                        server_ip[strlen(server_ip) - 1] = '\0';
                    }
                    else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                        serverAddr.sin_addr.s_addr = inet_addr(server_ip);
                        SDL_StopTextInput();
                        appState = STATE_GAME;
                    }
                }
            }
        }

        // --- RENDU : CONFIGURATION RESEAU MENU ---
        if (appState == STATE_MENU) {
            SDL_SetRenderDrawColor(renderer, 15, 20, 35, 255);
            SDL_RenderClear(renderer);

            // Boîtier de saisie IP
            SDL_SetRenderDrawColor(renderer, 25, 35, 55, 255);
            SDL_Rect ipBox = { WINDOW_WIDTH/2 - 220, WINDOW_HEIGHT/2 - 40, 440, 80 };
            SDL_RenderFillRect(renderer, &ipBox);

            SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); // Bordure Cyan Néon
            SDL_RenderDrawRect(renderer, &ipBox);

            // AFFICHAGE DIRECT DE L'IP DANS LA FENÊTRE
            SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); // Couleur d'écriture cyan
            int textWidthOffset = (strlen(server_ip) * 22) / 2;
            draw_ip_string(renderer, server_ip, (WINDOW_WIDTH / 2) - textWidthOffset, (WINDOW_HEIGHT / 2) - 11);

            SDL_SetWindowTitle(window, "LAN Blaster | SAISISSEZ L'IP DU SERVEUR PUIS ENTRÉE");
        }
        // --- RENDU : DUEL EN JEU ---
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

            // Arrière-plan Spatial
            SDL_SetRenderDrawColor(renderer, 10, 10, 18, 255);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 200, 200, 255, 180);
            for(int i=0; i<80; i++) SDL_RenderDrawPoint(renderer, starX[i], starY[i]);

            // Grille Cyberpunk
            SDL_SetRenderDrawColor(renderer, 30, 40, 70, 255);
            for(int x = 0; x < WINDOW_WIDTH; x += 80) SDL_RenderDrawLine(renderer, x, 0, x, WINDOW_HEIGHT);
            for(int y = 0; y < WINDOW_HEIGHT; y += 80) SDL_RenderDrawLine(renderer, 0, y, WINDOW_WIDTH, y);

            // Obstacles
            SDL_Rect obstacles[] = {{250, 50, 40, 300}, {500, 250, 40, 300}};
            for(int o=0; o<2; o++) {
                SDL_SetRenderDrawColor(renderer, 35, 35, 45, 255);
                SDL_RenderFillRect(renderer, &obstacles[o]);
                SDL_SetRenderDrawColor(renderer, 0, 180, 255, 255);
                SDL_RenderDrawRect(renderer, &obstacles[o]);
            }

            // Rendu du Boss (Rose Cyber)
            SDL_SetRenderDrawColor(renderer, 255, 0, 128, 255);
            SDL_Rect enemyRect = {gameState.enemy.x, gameState.enemy.y, PLAYER_SIZE, PLAYER_SIZE};
            SDL_RenderFillRect(renderer, &enemyRect);

            // Barre HP Boss
            SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
            SDL_Rect enemyHPBar = {gameState.enemy.x, gameState.enemy.y - 12, gameState.enemy.hp / 2, 6};
            SDL_RenderFillRect(renderer, &enemyHPBar);

            // Rendu de la Balle du Boss (Laser Violet Électrique)
            if(gameState.enemy.bullet.active) {
                SDL_SetRenderDrawColor(renderer, 160, 32, 240, 255);
                SDL_Rect bRect = {gameState.enemy.bullet.x, gameState.enemy.bullet.y, 35, 8};
                SDL_RenderFillRect(renderer, &bRect);
            }

            // Rendu des Joueurs et de leurs barres de vie respectives
            for(int i=0; i<MAX_PLAYERS; i++) {
                if(gameState.players[i].connected) {
                    if (i == 0) SDL_SetRenderDrawColor(renderer, 50, 220, 50, 255);   // Vert Jade pour J1
                    else SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);

                    SDL_Rect playerRect = {gameState.players[i].x, gameState.players[i].y, PLAYER_SIZE, PLAYER_SIZE};
                    SDL_RenderFillRect(renderer, &playerRect);

                    // BARRE DE VIE DU JOUEUR (Au-dessus du carré)
                    SDL_SetRenderDrawColor(renderer, 0, 255, 100, 255); // Jauge verte
                    SDL_Rect playerHPBar = {gameState.players[i].x, gameState.players[i].y - 12, gameState.players[i].hp / 2, 6};
                    SDL_RenderFillRect(renderer, &playerHPBar);

                    // Projectiles des joueurs (Jaune or)
                    if(gameState.players[i].bullet.active) {
                        SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
                        SDL_Rect bulletRect = {gameState.players[i].bullet.x, gameState.players[i].bullet.y, 35, 8};
                        SDL_RenderFillRect(renderer, &bulletRect);
                    }
                }
            }

            // Dashboard en haut
            char hudTitle[256];
            snprintf(hudTitle, sizeof(hudTitle), 
                     "LAN BLASTER 1V1 | VOTRE VIE: %d HP | BOSS HP: %d%% | VOS POINTS: %d", 
                     gameState.players[0].hp, gameState.enemy.hp, gameState.players[0].score);
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