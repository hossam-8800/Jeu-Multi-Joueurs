#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

#include "common.h"

// États de l'application
enum {
    STATE_MAIN_MENU,
    STATE_IP_MENU,
    STATE_TUTORIAL,
    STATE_SETTINGS,
    STATE_GAME,
    STATE_SOLO_GAME,
    STATE_TEAM_SELECT
};

// Obstacles (client, pour rendu + solo)
SDL_Rect obstacles[] = {
    {160, 100, 50, 140},
    {580, 340, 50, 140},
    {370, 260, 70, 70},
    {120, 430, 90, 50},
    {580, 90, 90, 50},
    {385, 90, 50, 50}
};
int num_obstacles = sizeof(obstacles) / sizeof(obstacles[0]);

int video_mode = VIDEO_WINDOWED;

typedef struct {
    float x, y;
    int active;
    int cooldown;
    int patrol_dir;
} SoloBot;

typedef struct {
    float x, y;
    float dx, dy;
    int active;
    int from_bot;
} SoloBullet;

// --- Texte vectoriel ---
void draw_char(SDL_Renderer* r, char c, int x, int y) {
    int w = 14, h = 22;
    int x0 = x, x1 = x + w;
    int y0 = y, y1 = y + h/2, y2 = y + h;

    if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';

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
        case 'K':
            SDL_RenderDrawLine(r, x0, y0, x0, y2);
            SDL_RenderDrawLine(r, x1, y0, x0, y1);
            SDL_RenderDrawLine(r, x0, y1, x1, y2);
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
        case 'Q':
            SDL_RenderDrawLine(r, x0, y0, x1, y0); SDL_RenderDrawLine(r, x1, y0, x1, y2);
            SDL_RenderDrawLine(r, x1, y2, x0, y2); SDL_RenderDrawLine(r, x0, y2, x0, y0);
            SDL_RenderDrawLine(r, x0 + w/2, y1, x1, y2);
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
        case 'V':
            SDL_RenderDrawLine(r, x0, y0, x0 + w/2, y2);
            SDL_RenderDrawLine(r, x0 + w/2, y2, x1, y0);
            break;
        case 'W':
            SDL_RenderDrawLine(r, x0, y0, x0 + 3, y2);
            SDL_RenderDrawLine(r, x0 + 3, y2, x0 + 7, y1);
            SDL_RenderDrawLine(r, x0 + 7, y1, x0 + 11, y2);
            SDL_RenderDrawLine(r, x0 + 11, y2, x1, y0);
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
        case '-': SDL_RenderDrawLine(r, x0, y1, x1, y1); break;
        case '>': SDL_RenderDrawLine(r, x0, y0, x1, y1); SDL_RenderDrawLine(r, x1, y1, x0, y2); break;
        case ':': {
            SDL_Rect d1 = { x + w/2 - 2, y1 - 2, 4, 4 };
            SDL_Rect d2 = { x + w/2 - 2, y2 - 4, 4, 4 };
            SDL_RenderFillRect(r, &d1); SDL_RenderFillRect(r, &d2);
            break;
        }
    }
}

void draw_string(SDL_Renderer* r, const char* str, int x, int y) {
    while(*str) {
        if (*str != ' ') draw_char(r, *str, x, y);
        x += 22;
        str++;
    }
}

// Collision solo
int check_obstacle_collision(int x, int y, int size) {
    SDL_Rect p_rect = {x, y, size, size};
    for(int o=0; o<num_obstacles; o++) {
        if (SDL_HasIntersection(&p_rect, &obstacles[o])) return 1;
    }
    return 0;
}

// Raycast solo
int check_line_of_sight(int x1, int y1, int x2, int y2) {
    int steps = 25;
    for (int i = 1; i < steps; i++) {
        float t = (float)i / steps;
        int check_x = x1 + (int)((x2 - x1) * t);
        int check_y = y1 + (int)((y2 - y1) * t);
        for (int o = 0; o < num_obstacles; o++) {
            if (check_x >= obstacles[o].x && check_x <= obstacles[o].x + obstacles[o].w &&
                check_y >= obstacles[o].y && check_y <= obstacles[o].y + obstacles[o].h) {
                return 0;
            }
        }
    }
    return 1;
}

void apply_video_mode(SDL_Window* window) {
    if (video_mode == VIDEO_WINDOWED) {
        SDL_SetWindowFullscreen(window, 0);
        SDL_SetWindowBordered(window, SDL_TRUE);
        SDL_SetWindowSize(window, WINDOW_WIDTH, WINDOW_HEIGHT);
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    } else if (video_mode == VIDEO_BORDERLESS) {
        SDL_DisplayMode dm;
        SDL_GetCurrentDisplayMode(0, &dm);
        SDL_SetWindowBordered(window, SDL_FALSE);
        SDL_SetWindowSize(window, dm.w, dm.h);
        SDL_SetWindowPosition(window, 0, 0);
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else if (video_mode == VIDEO_FULLSCREEN) {
        SDL_DisplayMode dm;
        SDL_GetCurrentDisplayMode(0, &dm);
        SDL_SetWindowBordered(window, SDL_TRUE);
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    }
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) return 1;
#endif

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("LAN Blaster Tactical",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int running = 1, appState = STATE_MAIN_MENU, menuSelected = 0;
    char server_ip[64] = "";

    int starX[80], starY[80];
    srand((unsigned int)time(NULL));
    for(int i=0; i<80; i++) {
        starX[i] = rand() % WINDOW_WIDTH;
        starY[i] = rand() % WINDOW_HEIGHT;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

#ifdef _WIN32
    u_long mode = 1; ioctlsocket(sock, FIONBIO, &mode);
#else
    fcntl(sock, F_SETFL, O_NONBLOCK);
#endif

    GameState gameState;
    memset(&gameState, 0, sizeof(GameState));

    float solo_px = 100, solo_py = 300;
    float last_dx = 1.0f, last_dy = 0.0f;
    int solo_kills = 0, player_dead = 0;

    SoloBot bots[6];
    SoloBullet bullets[30];
    for(int i=0; i<30; i++) bullets[i].active = 0;

    SDL_StartTextInput();

    while(running) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) running = 0;

            if (appState == STATE_MAIN_MENU) {
                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_z)
                        menuSelected = (menuSelected - 1 + 4) % 4;
                    else if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
                        menuSelected = (menuSelected + 1) % 4;
                    else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                        if (menuSelected == 0) appState = STATE_IP_MENU;
                        else if (menuSelected == 1) {
                            appState = STATE_SOLO_GAME;
                            solo_px = 70; solo_py = 270; player_dead = 0; solo_kills = 0;
                            bots[0] = (SoloBot){280, 150, 1, 60, 1};  bots[1] = (SoloBot){520, 400, 1, 40, -1};
                            bots[2] = (SoloBot){450, 120, 1, 80, 1};  bots[3] = (SoloBot){220, 440, 1, 50, -1};
                            bots[4] = (SoloBot){690, 150, 1, 30, 1};  bots[5] = (SoloBot){390, 480, 1, 70, -1};
                            for(int i=0; i<30; i++) bullets[i].active = 0;
                        }
                        else if (menuSelected == 2) appState = STATE_TUTORIAL;
                        else if (menuSelected == 3) appState = STATE_SETTINGS;
                    }
                }
            }
            else if (appState == STATE_IP_MENU) {
                if (event.type == SDL_TEXTINPUT) {
                    if (strlen(server_ip) < 25 &&
                        (((*event.text.text >= '0' && *event.text.text <= '9') || *event.text.text == '.')))
                        strcat(server_ip, event.text.text);
                }
                else if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(server_ip) > 0)
                        server_ip[strlen(server_ip) - 1] = '\0';
                    else if (event.key.keysym.sym == SDLK_ESCAPE)
                        appState = STATE_MAIN_MENU;
                    else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                        if(strlen(server_ip) > 0) {
                            serverAddr.sin_addr.s_addr = inet_addr(server_ip);
                            appState = STATE_TEAM_SELECT;
                        }
                    }
                }
            }
            else if (appState == STATE_TEAM_SELECT) {
                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_1) {
                        InputPacket p; memset(&p, 0, sizeof(p));
                        p.team_choice = 1;
                        sendto(sock, (char*)&p, sizeof(p), 0,
                               (struct sockaddr*)&serverAddr, sizeof(serverAddr));
                        appState = STATE_GAME;
                    }
                    else if (event.key.keysym.sym == SDLK_2) {
                        InputPacket p; memset(&p, 0, sizeof(p));
                        p.team_choice = 2;
                        sendto(sock, (char*)&p, sizeof(p), 0,
                               (struct sockaddr*)&serverAddr, sizeof(serverAddr));
                        appState = STATE_GAME;
                    }
                    else if (event.key.keysym.sym == SDLK_ESCAPE) {
                        appState = STATE_MAIN_MENU;
                    }
                }
            }
            else if (appState == STATE_TUTORIAL || appState == STATE_SETTINGS) {
                if (event.type == SDL_KEYDOWN) {
                    if (appState == STATE_SETTINGS) {
                        if (event.key.keysym.sym == SDLK_UP)
                            video_mode = (video_mode + 2) % 3;
                        else if (event.key.keysym.sym == SDLK_DOWN)
                            video_mode = (video_mode + 1) % 3;
                        else if (event.key.keysym.sym == SDLK_RETURN) {
                            apply_video_mode(window);
                        }
                    }
                    if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_RETURN)
                        appState = STATE_MAIN_MENU;
                }
            }
            else if (appState == STATE_GAME || appState == STATE_SOLO_GAME) {
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                    appState = STATE_MAIN_MENU;

                if (appState == STATE_SOLO_GAME && !player_dead &&
                    event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                    for(int i=0; i<30; i++) {
                        if(!bullets[i].active) {
                            bullets[i].x = solo_px + PLAYER_SIZE/2;
                            bullets[i].y = solo_py + PLAYER_SIZE/2;
                            bullets[i].dx = last_dx * 9.0f;
                            bullets[i].dy = last_dy * 9.0f;
                            bullets[i].active = 1;
                            bullets[i].from_bot = 0;
                            break;
                        }
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 8, 10, 18, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 180, 180, 255, 120);
        for(int i=0; i<80; i++) {
            if (appState != STATE_GAME && appState != STATE_SOLO_GAME)
                starX[i] = (starX[i] - 1 + WINDOW_WIDTH) % WINDOW_WIDTH;
            SDL_RenderDrawPoint(renderer, starX[i], starY[i]);
        }
        SDL_SetRenderDrawColor(renderer, 18, 22, 36, 255);
        for(int x = 0; x < WINDOW_WIDTH; x += 60)
            SDL_RenderDrawLine(renderer, x, 0, x, WINDOW_HEIGHT);
        for(int y = 0; y < WINDOW_HEIGHT; y += 60)
            SDL_RenderDrawLine(renderer, 0, y, WINDOW_WIDTH, y);

        if (appState == STATE_MAIN_MENU) {
            SDL_SetWindowTitle(window, "LAN Blaster Tactical | MENU");
            SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
            draw_string(renderer, "LAN BLASTER", WINDOW_WIDTH/2 - 121, 80);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            draw_string(renderer, "TACTICAL PRO", WINDOW_WIDTH/2 - 121, 115);

            const char* choices[] = {"REJOINDRE MULTI", "MODE SOLO BOTS", "TUTORIEL", "PARAMETRES"};
            int offsets[] = {154, 143, 88, 110};
            for(int i=0; i<4; i++) {
                if(menuSelected == i) {
                    SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
                    draw_char(renderer, '>', WINDOW_WIDTH/2 - offsets[i] - 40, 230 + i*50);
                } else {
                    SDL_SetRenderDrawColor(renderer, 120, 140, 160, 255);
                }
                draw_string(renderer, choices[i], WINDOW_WIDTH/2 - offsets[i], 230 + i*50);
            }
        }
        else if (appState == STATE_IP_MENU) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            draw_string(renderer, "ENTREZ L IP DU SERVEUR", WINDOW_WIDTH/2 - 242, 140);
            SDL_Rect ipBox = { WINDOW_WIDTH/2 - 240, WINDOW_HEIGHT/2 - 40, 480, 80 };
            SDL_SetRenderDrawColor(renderer, 15, 20, 30, 255);
            SDL_RenderFillRect(renderer, &ipBox);
            SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
            SDL_RenderDrawRect(renderer, &ipBox);
            if(strlen(server_ip) > 0)
                draw_string(renderer, server_ip,
                    (WINDOW_WIDTH / 2) - (int)(strlen(server_ip)*22)/2,
                    (WINDOW_HEIGHT / 2) - 11);
        }
        else if (appState == STATE_TEAM_SELECT) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            draw_string(renderer, "CHOISIS TON EQUIPE", WINDOW_WIDTH/2 - 220, 140);
            SDL_SetRenderDrawColor(renderer, 0, 200, 255, 255);
            draw_string(renderer, "1 - EQUIPE 1 (GAUCHE)", WINDOW_WIDTH/2 - 220, 240);
            SDL_SetRenderDrawColor(renderer, 255, 80, 80, 255);
            draw_string(renderer, "2 - EQUIPE 2 (DROITE)", WINDOW_WIDTH/2 - 220, 290);
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            draw_string(renderer, "ECHAP - RETOUR", WINDOW_WIDTH/2 - 150, 360);
        }
        else if (appState == STATE_TUTORIAL) {
            SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
            draw_string(renderer, "CONSIGNES TACTIQUES", WINDOW_WIDTH/2 - 209, 60);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            draw_string(renderer, "ZQSD   - DEPLACEMENTS RIGIDES ET COUVERTS", 40, 160);
            draw_string(renderer, "ESPACE - TIRER DANS LA DIRECTION DE COURSE", 40, 210);
            draw_string(renderer, "CAISSES- BLOCS SOLIDES PAS COMPLAISANTS", 40, 280);
            draw_string(renderer, "STEALTH- LES ENNEMIS CACHES SONT INVISIBLES", 40, 330);
            draw_string(renderer, "ECHAP  - RETOUR MENU INSTANTANE", 40, 400);
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            draw_string(renderer, "RETOUR : ENTREE OU ECHAP", WINDOW_WIDTH/2 - 264, 490);
        }
        else if (appState == STATE_SETTINGS) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            draw_string(renderer, "CONFIG COMPETITIVE", WINDOW_WIDTH/2 - 198, 80);

            const char* videoChoices[] = {
                "MODE FENETRE",
                "PLEIN ECRAN FENETRE",
                "PLEIN ECRAN EXCLUSIF"
            };

            for(int i=0; i<3; i++) {
                if (video_mode == i)
                    SDL_SetRenderDrawColor(renderer, 0, 255, 120, 255);
                else
                    SDL_SetRenderDrawColor(renderer, 120, 140, 160, 255);
                draw_string(renderer, videoChoices[i], 100, 200 + i*50);
            }

            SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
            draw_string(renderer, "RETOUR : ENTREE / ECHAP", WINDOW_WIDTH/2 - 220, 480);
        }

        if (appState == STATE_GAME || appState == STATE_SOLO_GAME) {
            for(int o=0; o<num_obstacles; o++) {
                SDL_SetRenderDrawColor(renderer, 25, 28, 42, 255);
                SDL_RenderFillRect(renderer, &obstacles[o]);
                SDL_SetRenderDrawColor(renderer, 240, 90, 40, 255);
                SDL_RenderDrawRect(renderer, &obstacles[o]);
                SDL_RenderDrawLine(renderer, obstacles[o].x, obstacles[o].y,
                                   obstacles[o].x + obstacles[o].w, obstacles[o].y + obstacles[o].h);
                SDL_RenderDrawLine(renderer, obstacles[o].x + obstacles[o].w, obstacles[o].y,
                                   obstacles[o].x, obstacles[o].y + obstacles[o].h);
            }
        }

        if (appState == STATE_GAME) {
            const Uint8* keys = SDL_GetKeyboardState(NULL);
            InputPacket input; memset(&input, 0, sizeof(input));
            if(keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) input.up = 1;
            if(keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) input.down = 1;
            if(keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) input.left = 1;
            if(keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) input.right = 1;
            if(keys[SDL_SCANCODE_SPACE]) input.shoot = 1;
            input.team_choice = 0;

            sendto(sock, (char*)&input, sizeof(input), 0,
                   (struct sockaddr*)&serverAddr, sizeof(serverAddr));

#ifdef _WIN32
            int len = sizeof(serverAddr);
#else
            socklen_t len = sizeof(serverAddr);
#endif
            GameState latestState;
            while(recvfrom(sock, (char*)&latestState, sizeof(latestState), 0,
                           (struct sockaddr*)&serverAddr, &len) > 0) {
                gameState = latestState;
            }

            int localPlayers = 0;
            for(int i=0; i<MAX_PLAYERS; i++) {
                if(!gameState.players[i].connected) continue;
                localPlayers++;

                if(gameState.players[i].team == 1)
                    SDL_SetRenderDrawColor(renderer, 0, 200, 255, 255);
                else if(gameState.players[i].team == 2)
                    SDL_SetRenderDrawColor(renderer, 255, 80, 80, 255);
                else
                    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);

                SDL_Rect pRect = {gameState.players[i].x, gameState.players[i].y,
                                  PLAYER_SIZE, PLAYER_SIZE};
                SDL_RenderFillRect(renderer, &pRect);

                if(gameState.players[i].bullet.active) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                    SDL_Rect bRect = {gameState.players[i].bullet.x - 3,
                                      gameState.players[i].bullet.y - 3, 6, 6};
                    SDL_RenderFillRect(renderer, &bRect);
                }
            }

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            char hud[128];
            sprintf(hud, "T1:%d  T2:%d", gameState.score_team1, gameState.score_team2);
            draw_string(renderer, hud, 20, 20);

            if (gameState.game_active == 0) {
                draw_string(renderer, "EN ATTENTE DES JOUEURS", 20, 60);
            } else if (gameState.game_active == 1) {
                draw_string(renderer, "MATCH EN COURS", 20, 60);
            } else if (gameState.game_active == 2) {
                const char* msg = (gameState.score_team1 >= 6) ?
                    "VICTOIRE EQUIPE 1" : "VICTOIRE EQUIPE 2";
                draw_string(renderer, msg, WINDOW_WIDTH/2 - 176, WINDOW_HEIGHT/2 - 11);
            }
        }
        else if (appState == STATE_SOLO_GAME) {
            const Uint8* keys = SDL_GetKeyboardState(NULL);
            float move_x = 0, move_y = 0;

            if(!player_dead) {
                if(keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) move_y = -4.0f;
                if(keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) move_y = 4.0f;
                if(keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) move_x = -4.0f;
                if(keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) move_x = 4.0f;

                if (move_x != 0 || move_y != 0) {
                    last_dx = move_x; last_dy = move_y;
                    float mag = sqrtf(last_dx*last_dx + last_dy*last_dy);
                    last_dx /= mag; last_dy /= mag;
                }

                solo_py += move_y;
                if(check_obstacle_collision((int)solo_px, (int)solo_py, PLAYER_SIZE) ||
                   solo_py < 0 || solo_py > WINDOW_HEIGHT-PLAYER_SIZE)
                    solo_py -= move_y;

                solo_px += move_x;
                if(check_obstacle_collision((int)solo_px, (int)solo_py, PLAYER_SIZE) ||
                   solo_px < 0 || solo_px > WINDOW_WIDTH-PLAYER_SIZE)
                    solo_px -= move_x;
            }

            int active_bots = 0;
            for(int b=0; b<6; b++) {
                if(!bots[b].active) continue;
                active_bots++;

                bots[b].y += 1.0f * bots[b].patrol_dir;
                if(check_obstacle_collision((int)bots[b].x, (int)bots[b].y, PLAYER_SIZE) ||
                   bots[b].y < 50 || bots[b].y > 530) {
                    bots[b].patrol_dir *= -1;
                }

                int visible = check_line_of_sight(
                    (int)solo_px + PLAYER_SIZE/2, (int)solo_py + PLAYER_SIZE/2,
                    (int)bots[b].x + PLAYER_SIZE/2, (int)bots[b].y + PLAYER_SIZE/2);

                if (visible && !player_dead) {
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 200);
                    SDL_RenderDrawLine(renderer, (int)bots[b].x+16, (int)bots[b].y+16,
                                       (int)solo_px+16, (int)solo_py+16);

                    bots[b].cooldown--;
                    if(bots[b].cooldown <= 0) {
                        for(int i=0; i<30; i++) {
                            if(!bullets[i].active) {
                                bullets[i].x = bots[b].x + PLAYER_SIZE/2;
                                bullets[i].y = bots[b].y + PLAYER_SIZE/2;
                                float b_dx = (solo_px + PLAYER_SIZE/2) - bullets[i].x;
                                float b_dy = (solo_py + PLAYER_SIZE/2) - bullets[i].y;
                                float mag = sqrtf(b_dx*b_dx + b_dy*b_dy);
                                bullets[i].dx = (b_dx / mag) * 6.5f;
                                bullets[i].dy = (b_dy / mag) * 6.5f;
                                bullets[i].active = 1;
                                bullets[i].from_bot = 1;
                                break;
                            }
                        }
                        bots[b].cooldown = 70;
                    }

                    SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
                    SDL_Rect bRect = {(int)bots[b].x, (int)bots[b].y, PLAYER_SIZE, PLAYER_SIZE};
                    SDL_RenderFillRect(renderer, &bRect);
                }
            }

            for(int i=0; i<30; i++) {
                if(!bullets[i].active) continue;
                bullets[i].x += bullets[i].dx;
                bullets[i].y += bullets[i].dy;

                if(check_obstacle_collision((int)bullets[i].x, (int)bullets[i].y, 6)) {
                    bullets[i].active = 0; continue;
                }

                if(bullets[i].from_bot) {
                    if(!player_dead &&
                       bullets[i].x >= solo_px && bullets[i].x <= solo_px+PLAYER_SIZE &&
                       bullets[i].y >= solo_py && bullets[i].y <= solo_py+PLAYER_SIZE) {
                        player_dead = 1; bullets[i].active = 0;
                    }
                    SDL_SetRenderDrawColor(renderer, 255, 120, 0, 255);
                } else {
                    for(int b=0; b<6; b++) {
                        if(bots[b].active &&
                           bullets[i].x >= bots[b].x && bullets[i].x <= bots[b].x+PLAYER_SIZE &&
                           bullets[i].y >= bots[b].y && bullets[i].y <= bots[b].y+PLAYER_SIZE) {
                            bots[b].active = 0; bullets[i].active = 0; solo_kills++;
                        }
                    }
                    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
                }
                SDL_Rect bulRect = {(int)bullets[i].x - 3, (int)bullets[i].y - 3, 6, 6};
                SDL_RenderFillRect(renderer, &bulRect);
            }

            if(!player_dead) {
                SDL_SetRenderDrawColor(renderer, 50, 255, 120, 255);
                SDL_Rect pRect = {(int)solo_px, (int)solo_py, PLAYER_SIZE, PLAYER_SIZE};
                SDL_RenderFillRect(renderer, &pRect);

                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 150);
                int cx = (int)(solo_px + PLAYER_SIZE/2 + last_dx * 25);
                int cy = (int)(solo_py + PLAYER_SIZE/2 + last_dy * 25);
                SDL_RenderDrawLine(renderer, cx - 6, cy, cx + 6, cy);
                SDL_RenderDrawLine(renderer, cx, cy - 6, cx, cy + 6);
            } else {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                draw_string(renderer, "ELIMINE - ECHAP POUR RETOURNER",
                            WINDOW_WIDTH/2 - 319, WINDOW_HEIGHT/2 - 11);
            }

            if(active_bots == 0) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 100, 255);
                draw_string(renderer, "VICTOIRE NETTE - TOUS MORTS",
                            WINDOW_WIDTH/2 - 286, WINDOW_HEIGHT/2 - 11);
            }

            char soloHud[128];
            sprintf(soloHud, "SOLO STEALTH | CIBLES DETRUITES : %d / 6", solo_kills);
            SDL_SetWindowTitle(window, soloHud);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
#ifdef _WIN32
    closesocket(sock); WSACleanup();
#else
    close(sock);
#endif
    return 0;
}