#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "common.h"

// Les différents états de notre application
enum { STATE_MAIN_MENU, STATE_IP_MENU, STATE_TUTORIAL, STATE_SETTINGS, STATE_GAME };

// --- MOTEUR DE TEXTE VECTORIEL COMPLET (ALPHABET + CHIFFRES) ---
void draw_char(SDL_Renderer* r, char c, int x, int y) {
    int w = 14, h = 22;
    int x0 = x, x1 = x + w;
    int y0 = y, y1 = y + h/2, y2 = y + h;

    if (c >= 'a' && c <= 'z') c = c - 'a' + 'A'; // Conversion automatique en majuscules

    switch(c) {
        case 'A':
            SDL_RenderDrawLine(r, x0, y2, x0, y0); SDL_RenderDrawLine(r, x0, y0, x1, y0);
            SDL_RenderDrawLine(r, x1, y0, x1, y2); SDL_RenderDrawLine(r, x0, y1, x1, y1);
            break;
        case 'B':
            SDL_RenderDrawLine(r, x0, y0, x0, y2); SDL_RenderDrawLine(r, x0, y0, x1, y0);
            SDL_RenderDrawLine(r, x1, y0, x1, y1); SDL_RenderDrawLine(r, x0, y1, x1, y1);
            SDL_RenderDrawLine(r, x1, y1, x1, y2); SDL_RenderDrawLine(r, x0, y2, x1, y2);
            break;
        case 'C':
            SDL_RenderDrawLine(r, x1, y0, x0, y0); SDL_RenderDrawLine(r, x0, y0, x0, y2);
            SDL_RenderDrawLine(r, x0, y2, x1, y2);
            break;
        case 'D':
            SDL_RenderDrawLine(r, x0, y0, x0, y2); SDL_RenderDrawLine(r, x0, y0, x1, y0);
            SDL_RenderDrawLine(r, x1, y0, x1, y2); SDL_RenderDrawLine(r, x0, y2, x1, y2);
            break;
        case 'E':
            SDL_RenderDrawLine(r, x1, y0, x0, y0); SDL_RenderDrawLine(r, x0, y0, x0, y2);
            SDL_RenderDrawLine(r, x0, y2, x1, y2); SDL_RenderDrawLine(r, x0, y1, x1, y1);
            break;
        case 'F':
            SDL_RenderDrawLine(r, x1, y0, x0, y0); SDL_RenderDrawLine(r, x0, y0, x0, y2);
            SDL_RenderDrawLine(r, x0, y1, x1, y1);
            break;
        case 'G':
            SDL_RenderDrawLine(r, x1, y0, x0, y0); SDL_RenderDrawLine(r, x0, y0, x0, y2);
            SDL_RenderDrawLine(r, x0, y2, x1, y2); SDL_RenderDrawLine(r, x1, y2, x1, y1);
            SDL_RenderDrawLine(r, x1, y1, x0 + w/2, y1);
            break;
        case 'H':
            SDL_RenderDrawLine(r, x0, y0, x0, y2); SDL_RenderDrawLine(r, x1, y0, x1, y2);
            SDL_RenderDrawLine(r, x0, y1, x1, y1);
            break;
        case 'I':
            SDL_RenderDrawLine(r, x0 + w/2, y0, x0 + w/2, y2);
            SDL_RenderDrawLine(r, x0, y0, x1, y0); SDL_RenderDrawLine(r, x0, y2, x1, y2);
            break;
        case 'J':
            SDL_RenderDrawLine(r, x1, y0, x1, y2); SDL_RenderDrawLine(r, x1, y2, x0, y2);
            SDL_RenderDrawLine(r, x0, y2, x0, y1);
            break;
        case 'L':
            SDL_RenderDrawLine(r, x0, y0, x0, y2); SDL_RenderDrawLine(r, x0, y2, x1, y2);
            break;
        case 'M':
            SDL_RenderDrawLine(r, x0, y2, x0, y0); SDL_RenderDrawLine(r, x0, y0, x0 + w/2, y1);
            SDL_RenderDrawLine(r, x0 + w/2, y1, x1, y0); SDL_RenderDrawLine(r, x1, y0, x1, y2);
            break;
        case 'N':
            SDL_RenderDrawLine(r, x0, y2, x0, y0); SDL_RenderDrawLine(r, x0, y0, x1, y2);
            SDL_RenderDrawLine(r, x1, y2, x1, y0);
            break;
        case 'O':
            SDL_RenderDrawLine(r, x0, y0, x1, y0); SDL_RenderDrawLine(r, x1, y0, x1, y2);
            SDL_RenderDrawLine(r, x1, y2, x0, y2); SDL_RenderDrawLine(r, x0, y2, x0, y0);
            break;
        case 'P':
            SDL_RenderDrawLine(r, x0, y2, x0, y0); SDL_RenderDrawLine(r, x0, y0, x1, y0);
            SDL_RenderDrawLine(r, x1, y0, x1, y1); SDL_RenderDrawLine(r, x0, y1, x1, y1);
            break;
        case 'R':
            SDL_RenderDrawLine(r, x0, y2, x0, y0); SDL_RenderDrawLine(r, x0, y0, x1, y0);
            SDL_RenderDrawLine(r, x1, y0, x1, y1); SDL_RenderDrawLine(r, x0, y1, x1, y1);
            SDL_RenderDrawLine(r, x0, y1, x1, y2);
            break;
        case 'S':
            SDL_RenderDrawLine(r, x1, y0, x0, y0); SDL_RenderDrawLine(r, x0, y0, x0, y1);
            SDL_RenderDrawLine(r, x0, y1, x1, y1); SDL_RenderDrawLine(r, x1, y1, x1, y2);
            SDL_RenderDrawLine(r, x1, y2, x0, y2);
            break;
        case 'T':
            SDL_RenderDrawLine(r, x0, y0, x1, y0); SDL_RenderDrawLine(r, x0 + w/2, y0, x0 + w/2, y2);
            break;
        case 'U':
            SDL_RenderDrawLine(r, x0, y0, x0, y2); SDL_RenderDrawLine(r, x0, y2, x1, y2);
            SDL_RenderDrawLine(r, x1, y2, x1, y0);
            break;
        case 'X':
            SDL_RenderDrawLine(r, x0, y0, x1, y2); SDL_RenderDrawLine(r, x1, y0, x0, y2);
            break;
        case 'Y':
            SDL_RenderDrawLine(r, x0, y0, x0 + w/2, y1); SDL_RenderDrawLine(r, x1, y0, x0 + w/2, y1);
            SDL_RenderDrawLine(r, x0 + w/2, y1, x0 + w/2, y2);
            break;
        case 'Z':
            SDL_RenderDrawLine(r, x0, y0, x1, y0); SDL_RenderDrawLine(r, x1, y0, x0, y2);
            SDL_RenderDrawLine(r, x0, y2, x1, y2);
            break;
        case '_':
            SDL_RenderDrawLine(r, x0, y2, x1, y2);
            break;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': {
            int top = (c=='0'||c=='2'||c=='3'||c=='5'||c=='6'||c=='7'||c=='8'||c=='9');
            int tl  = (c=='0'||c=='4'||c=='5'||c=='6'||c=='8'||c=='9');
            int tr  = (c=='0'||c=='1'||c=='2'||c=='3'||c=='4'||c=='7'||c=='8'||c=='9');
            int mid = (c=='2'||c=='3'||c=='4'||c=='5'||c=='6'||c=='8'||c=='9');
            int bl  = (c=='0'||c=='2'||c=='6'||c=='8');
            int br  = (c=='0'||c=='1'||c=='3'||c=='4'||c=='5'||c=='6'||c=='7'||c=='8'||c=='9');
            int bot = (c=='0'||c=='2'||c=='3'||c=='5'||c=='6'||c=='8'||c=='9');

            if(top) SDL_RenderDrawLine(r, x0, y0, x1, y0);
            if(tl)  SDL_RenderDrawLine(r, x0, y0, x0, y1);
            if(tr)  SDL_RenderDrawLine(r, x1, y0, x1, y1);
            if(mid) SDL_RenderDrawLine(r, x0, y1, x1, y1);
            if(bl)  SDL_RenderDrawLine(r, x0, y1, x0, y2);
            if(br)  SDL_RenderDrawLine(r, x1, y1, x1, y2);
            if(bot) SDL_RenderDrawLine(r, x0, y2, x1, y2);
            break;
        }
        case '.': {
            SDL_Rect dot = { x + w/2 - 2, y2 - 3, 5, 5 };
            SDL_RenderFillRect(r, &dot);
            break;
        }
        case '-':
            SDL_RenderDrawLine(r, x0, y1, x1, y1);
            break;
        case '>':
            SDL_RenderDrawLine(r, x0, y0, x1, y1);
            SDL_RenderDrawLine(r, x1, y1, x0, y2);
            break;
    }
}

void draw_string(SDL_Renderer* r, const char* str, int x, int y) {
    while(*str) {
        if (*str != ' ') draw_char(r, *str, x, y);
        x += 22; 
        str++;
    }
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) return 1;
#endif

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("LAN Blaster Tactical", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int running = 1;
    int appState = STATE_MAIN_MENU; 
    int menuSelected = 0;          
    char server_ip[64] = ""; 

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

            // --- GESTION DES MENUS D'ACCUEIL ---
            if (appState == STATE_MAIN_MENU) {
                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_z) {
                        menuSelected = (menuSelected - 1 + 3) % 3;
                    }
                    else if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) {
                        menuSelected = (menuSelected + 1) % 3;
                    }
                    else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                        if (menuSelected == 0) appState = STATE_IP_MENU;
                        else if (menuSelected == 1) appState = STATE_TUTORIAL;
                        else if (menuSelected == 2) appState = STATE_SETTINGS;
                    }
                }
            }
            else if (appState == STATE_IP_MENU) {
                if (event.type == SDL_TEXTINPUT) {
                    if (strlen(server_ip) < 25 && ((*event.text.text >= '0' && *event.text.text <= '9') || *event.text.text == '.')) {
                        strcat(server_ip, event.text.text);
                    }
                }
                else if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(server_ip) > 0) {
                        server_ip[strlen(server_ip) - 1] = '\0';
                    }
                    else if (event.key.keysym.sym == SDLK_ESCAPE) {
                        appState = STATE_MAIN_MENU;
                    }
                    else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                        if(strlen(server_ip) > 0) {
                            serverAddr.sin_addr.s_addr = inet_addr(server_ip);
                            appState = STATE_GAME;
                        }
                    }
                }
            }
            else if (appState == STATE_TUTORIAL || appState == STATE_SETTINGS) {
                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_RETURN) {
                        appState = STATE_MAIN_MENU;
                    }
                }
            }
            // --- NOUVEAU : RETOUR AU MENU DEPUIS LE JEU ACTIF ---
            else if (appState == STATE_GAME) {
                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        appState = STATE_MAIN_MENU; // Rage quit / Retour instantané !
                    }
                }
            }
        }

        // --- ARRIÈRE-PLAN DÉFILANT COMPLET ---
        SDL_SetRenderDrawColor(renderer, 8, 10, 18, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 180, 180, 255, 120);
        for(int i=0; i<80; i++) {
            if (appState != STATE_GAME) starX[i] = (starX[i] - 1 + WINDOW_WIDTH) % WINDOW_WIDTH;
            SDL_RenderDrawPoint(renderer, starX[i], starY[i]);
        }

        SDL_SetRenderDrawColor(renderer, 18, 22, 36, 255);
        for(int x = 0; x < WINDOW_WIDTH; x += 60) SDL_RenderDrawLine(renderer, x, 0, x, WINDOW_HEIGHT);
        for(int y = 0; y < WINDOW_HEIGHT; y += 60) SDL_RenderDrawLine(renderer, 0, y, WINDOW_WIDTH, y);


        // --- RENDU : MENU PRINCIPAL ---
        if (appState == STATE_MAIN_MENU) {
            SDL_SetWindowTitle(window, "LAN Blaster Tactical | MENU");

            SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
            draw_string(renderer, "LAN BLASTER", WINDOW_WIDTH/2 - (11 * 22)/2, 90);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            draw_string(renderer, "TACTICAL CS", WINDOW_WIDTH/2 - (11 * 22)/2, 125);

            if(menuSelected == 0) {
                SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
                draw_char(renderer, '>', WINDOW_WIDTH/2 - 210, 250);
            } else SDL_SetRenderDrawColor(renderer, 120, 140, 160, 255);
            draw_string(renderer, "REJOINDRE PARTIE", WINDOW_WIDTH/2 - 170, 250);

            if(menuSelected == 1) {
                SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
                draw_char(renderer, '>', WINDOW_WIDTH/2 - 130, 310);
            } else SDL_SetRenderDrawColor(renderer, 120, 140, 160, 255);
            draw_string(renderer, "TUTORIEL MAP", WINDOW_WIDTH/2 - 90, 310);

            if(menuSelected == 2) {
                SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
                draw_char(renderer, '>', WINDOW_WIDTH/2 - 140, 370);
            } else SDL_SetRenderDrawColor(renderer, 120, 140, 160, 255);
            draw_string(renderer, "PARAMETRES", WINDOW_WIDTH/2 - 100, 370);

            SDL_SetRenderDrawColor(renderer, 70, 85, 110, 255);
            draw_string(renderer, "Z-S POUR CHOISIR - ENTREE", WINDOW_WIDTH/2 - (25 * 22)/2, 520);
        }

        // --- RENDU : SAISIE IP ---
        else if (appState == STATE_IP_MENU) {
            SDL_SetWindowTitle(window, "LAN Blaster | SAISIE IP");
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            draw_string(renderer, "ENTREZ L-IP DU SERVEUR", WINDOW_WIDTH/2 - (22 * 22)/2, 140);

            SDL_Rect ipBox = { WINDOW_WIDTH/2 - 240, WINDOW_HEIGHT/2 - 40, 480, 80 };
            SDL_SetRenderDrawColor(renderer, 15, 20, 30, 255);
            SDL_RenderFillRect(renderer, &ipBox);
            SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255); 
            SDL_RenderDrawRect(renderer, &ipBox);

            if(strlen(server_ip) > 0) {
                int textWidthOffset = (strlen(server_ip) * 22) / 2;
                draw_string(renderer, server_ip, (WINDOW_WIDTH / 2) - textWidthOffset, (WINDOW_HEIGHT / 2) - 11);
            }

            SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
            draw_string(renderer, "ECHAP POUR RETOURNER", WINDOW_WIDTH/2 - (20 * 22)/2, 450);
        }

        // --- RENDU : TUTORIEL MIS À JOUR ---
        else if (appState == STATE_TUTORIAL) {
            SDL_SetWindowTitle(window, "LAN Blaster | REBELS INFO");
            SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
            draw_string(renderer, "CONSIGNES TACTIQUES", WINDOW_WIDTH/2 - (19 * 22)/2, 60);

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            draw_string(renderer, "ZQSD   - DEPLACEMENTS ULTRA NERVEUX", 60, 160);
            draw_string(renderer, "ESPACE - TIRER / PRENDRE LA LIGNE", 60, 210);
            draw_string(renderer, "CAISSES- CACHE-TOI DERRIERE POUR", 60, 280);
            draw_string(renderer, "         EVITER LES TIRS DU BOSS", 60, 320);
            draw_string(renderer, "ECHAP  - ABANDONNER (RETOUR MENU)", 60, 390);

            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            draw_string(renderer, "RETOUR : ENTRÉE OU ECHAP", WINDOW_WIDTH/2 - (24 * 22)/2, 490);
        }

        // --- RENDU : PARAMÈTRES ---
        else if (appState == STATE_SETTINGS) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            draw_string(renderer, "CONFIG COMPETITIVE", WINDOW_WIDTH/2 - (18 * 22)/2, 80);
            SDL_SetRenderDrawColor(renderer, 120, 140, 160, 255);
            draw_string(renderer, "MODE NET  - NETWORK UDP", 100, 200);
            draw_string(renderer, "TICKRATE  - 64 TICK RATE SYSTEM", 100, 260);
            draw_string(renderer, "TIR LOCAL - COUVERTURE ACTIVE", 100, 320);
            SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
            draw_string(renderer, "RETOUR : ENTRÉE", WINDOW_WIDTH/2 - (15 * 22)/2, 480);
        }

        // --- RENDU : GAMEPLAY COMPLET STYLE CS:GO ---
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

            // --- LEVEL DESIGN TACTIQUE : 6 OBSTACLES / CAISSES DE COUVERTURE ---
            SDL_Rect obstacles[] = {
                {160, 100, 45, 140},  // Pilier Gauche
                {580, 340, 45, 140},  // Pilier Droit
                {370, 260, 60, 60},   // Caisse Centrale Tactique
                {120, 420, 80, 40},   // Bunker Bas-Gauche
                {600, 100, 80, 40},   // Bunker Haut-Droit
                {380, 80, 40, 40}     // Petite caisse Top
            };
            int num_obstacles = sizeof(obstacles) / sizeof(obstacles[0]);

            for(int o=0; o<num_obstacles; o++) {
                // Fond de la caisse en gris blindé
                SDL_SetRenderDrawColor(renderer, 22, 26, 38, 255);
                SDL_RenderFillRect(renderer, &obstacles[o]);
                // Bordure orange/cyan style laser indus
                SDL_SetRenderDrawColor(renderer, 240, 100, 40, 255);
                SDL_RenderDrawRect(renderer, &obstacles[o]);
                // Lignes de renfort tactique (Croix sur les caisses)
                SDL_RenderDrawLine(renderer, obstacles[o].x, obstacles[o].y, obstacles[o].x + obstacles[o].w, obstacles[o].y + obstacles[o].h);
                SDL_RenderDrawLine(renderer, obstacles[o].x + obstacles[o].w, obstacles[o].y, obstacles[o].x, obstacles[o].y + obstacles[o].h);
            }

            // --- LE BOSS ENNEMI ---
            SDL_SetRenderDrawColor(renderer, 255, 40, 100, 255);
            SDL_Rect enemyRect = {gameState.enemy.x, gameState.enemy.y, PLAYER_SIZE, PLAYER_SIZE};
            SDL_RenderFillRect(renderer, &enemyRect);

            // Jauge de HP du Boss en Rouge Flash
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_Rect enemyHPBar = {gameState.enemy.x, gameState.enemy.y - 12, gameState.enemy.hp / 2, 6};
            SDL_RenderFillRect(renderer, &enemyHPBar);

            // Tir Plasma Violet du Boss
            if(gameState.enemy.bullet.active) {
                SDL_SetRenderDrawColor(renderer, 200, 50, 255, 255);
                SDL_Rect bRect = {gameState.enemy.bullet.x, gameState.enemy.bullet.y, 35, 8};
                SDL_RenderFillRect(renderer, &bRect);
            }

            // --- RENDU ET STYLISATION DES JOUEURS CONTRE LE MUR ---
            for(int i=0; i<MAX_PLAYERS; i++) {
                if(gameState.players[i].connected) {
                    // Attribution des couleurs de team
                    if (i == 0) SDL_SetRenderDrawColor(renderer, 50, 255, 100, 255);    // Joueur Vert 
                    else if (i == 1) SDL_SetRenderDrawColor(renderer, 0, 220, 255, 255);  // Joueur Bleu Cyan
                    else if (i == 2) SDL_SetRenderDrawColor(renderer, 255, 220, 0, 255);  // Joueur Jaune
                    else SDL_SetRenderDrawColor(renderer, 255, 100, 255, 255);

                    int px = gameState.players[i].x;
                    int py = gameState.players[i].y;

                    SDL_Rect playerRect = {px, py, PLAYER_SIZE, PLAYER_SIZE};
                    SDL_RenderFillRect(renderer, &playerRect);

                    // Barre de vie Joueur
                    SDL_SetRenderDrawColor(renderer, 0, 255, 128, 255);
                    SDL_Rect playerHPBar = {px, py - 12, gameState.players[i].hp / 2, 6};
                    SDL_RenderFillRect(renderer, &playerHPBar);

                    // AJOUT : Réticule de visée tactique (Crosshair style shooter)
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 180);
                    SDL_RenderDrawLine(renderer, px + PLAYER_SIZE + 10, py + PLAYER_SIZE/2, px + PLAYER_SIZE + 22, py + PLAYER_SIZE/2);
                    SDL_RenderDrawLine(renderer, px + PLAYER_SIZE + 16, py + PLAYER_SIZE/2 - 6, px + PLAYER_SIZE + 16, py + PLAYER_SIZE/2 + 6);

                    // Traînée de laser incendiaire 
                    if(gameState.players[i].bullet.active) {
                        SDL_SetRenderDrawColor(renderer, 255, 230, 50, 255); // Jaune étincelle flash
                        SDL_Rect bulletRect = {gameState.players[i].bullet.x, gameState.players[i].bullet.y, 35, 8};
                        SDL_RenderFillRect(renderer, &bulletRect);
                    }
                }
            }

            // --- INTERFACE HUD DE MATCH & FEED DE COMBAT (KILLFEED CS:GO STYLE) ---
            // Titre du match dynamique dans la fenêtre
            char hudTitle[256];
            snprintf(hudTitle, sizeof(hudTitle), "MATCH LIVE | J0 SCORE: %d | J1 SCORE: %d | ECHAP POUR QUITTER", gameState.players[0].score, gameState.players[1].score);
            SDL_SetWindowTitle(window, hudTitle);

            // Draw un encadré de Killfeed factice persistant en haut à droite pour la tension compétitive !
            SDL_SetRenderDrawColor(renderer, 255, 70, 70, 200);
            draw_string(renderer, "J0 _ BOSS", WINDOW_WIDTH - 220, 30);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 150);
            draw_string(renderer, "LIVE MATCH", 30, 30);
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