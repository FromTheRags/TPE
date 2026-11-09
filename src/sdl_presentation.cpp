#include "sdl_presentation.h"
#include "utils.h"

#include <cstdio>
#include <cstdlib>
#include <string>

#if defined(__APPLE__)
    #include <SDL/SDL.h>
#elif defined(__linux__) || defined(__unix__)
    #if __has_include(<SDL/SDL.h>)
        #include <SDL/SDL.h>
    #else
        #include <SDL.h>
    #endif
#else
    #if __has_include(<SDL/SDL.h>)
        #include <SDL/SDL.h>
    #else
        #include <SDL.h>
    #endif
#endif

void launchSdlPresentation()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::fprintf(stderr, "Erreur SDL_Init: %s\n", SDL_GetError());
        return;
    }

    const int BASE_W = 1040;
    const int BASE_H = 832;

    SDL_WM_SetCaption("TPE - Presentation Interactive (SDL 1.2) [F11: Plein ecran]", "TPE");

    bool isFullScreen = false;

    auto setVideo = [&](bool fs) -> SDL_Surface* {
        Uint32 flags = SDL_HWSURFACE | SDL_DOUBLEBUF;
        int w = BASE_W;
        int h = BASE_H;
        if (fs) {
            flags |= SDL_FULLSCREEN;
            const SDL_VideoInfo *info = SDL_GetVideoInfo();
            if (info && info->current_w > 0 && info->current_h > 0) {
                w = info->current_w;
                h = info->current_h;
            }
        }
        return SDL_SetVideoMode(w, h, 32, flags);
    };

    SDL_Surface *ecran = setVideo(isFullScreen);
    if (!ecran) {
        std::fprintf(stderr, "Erreur SDL_SetVideoMode: %s\n", SDL_GetError());
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        return;
    }

    std::string fondPath = getAssetPathStd("FOND.bmp");
    std::string robotPath = getAssetPathStd("robot.bmp");
    std::string retourPath = getAssetPathStd("retour.bmp");
    std::string creditsPath = getAssetPathStd("credits.bmp");

    SDL_Surface *menuSurface = SDL_LoadBMP(fondPath.c_str());
    SDL_Surface *robotSurface = SDL_LoadBMP(robotPath.c_str());
    SDL_Surface *retourSurface = SDL_LoadBMP(retourPath.c_str());
    SDL_Surface *creditsSurface = SDL_LoadBMP(creditsPath.c_str());

    if (retourSurface) {
        // Set colorkey green (0, 255, 0)
        SDL_SetColorKey(retourSurface, SDL_SRCCOLORKEY, SDL_MapRGB(retourSurface->format, 0, 255, 0));
    }

    enum State {
        MENU,
        PRESENTATION,
        CREDITS,
        QUIT
    };

    State state = MENU;
    int bubbleIndex = 1;
    SDL_Surface *currentBubble = nullptr;

    auto loadBubble = [&](int idx) -> SDL_Surface* {
        std::string bname = "bulle" + std::to_string(idx) + ".bmp";
        std::string bpath = getAssetPathStd(bname);
        return SDL_LoadBMP(bpath.c_str());
    };

    bool running = true;
    while (running && state != QUIT) {
        if (state == MENU) {
            // Fill background with uniform light sky color matching FOND edge
            SDL_FillRect(ecran, nullptr, SDL_MapRGB(ecran->format, 246, 249, 252));

            int ox = 0;
            int oy = 0;
            if (menuSurface) {
                ox = (ecran->w - menuSurface->w) / 2;
                oy = (ecran->h - menuSurface->h) / 2;
                SDL_Rect dst = { (Sint16)ox, (Sint16)oy, 0, 0 };
                SDL_BlitSurface(menuSurface, nullptr, ecran, &dst);
            }
            SDL_Flip(ecran);

            bool inMenu = true;
            SDL_Event event;
            while (inMenu) {
                if (SDL_WaitEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        state = QUIT;
                        inMenu = false;
                    } else if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_ESCAPE) {
                            if (isFullScreen) {
                                isFullScreen = false;
                                ecran = setVideo(isFullScreen);
                                inMenu = false; // redraw menu
                            } else {
                                state = QUIT;
                                inMenu = false;
                            }
                        } else if (event.key.keysym.sym == SDLK_F11 || event.key.keysym.sym == SDLK_f) {
                            isFullScreen = !isFullScreen;
                            ecran = setVideo(isFullScreen);
                            inMenu = false; // redraw menu
                        }
                    } else if (event.type == SDL_MOUSEBUTTONUP) {
                        int mx = event.button.x - ox;
                        int my = event.button.y - oy;

                        // Button 1: Présentation Générale (315..730, 215..287)
                        if (mx >= 315 && mx <= 730 && my >= 215 && my <= 287) {
                            state = PRESENTATION;
                            bubbleIndex = 1;
                            inMenu = false;
                        }
                        // Button 2: Simulateur (315..735, 350..410)
                        else if (mx >= 315 && mx <= 735 && my >= 350 && my <= 410) {
                            // Return to main Qt simulation
                            state = QUIT;
                            inMenu = false;
                        }
                        // Button 3: Crédits (315..730, 474..542)
                        else if (mx >= 315 && mx <= 730 && my >= 474 && my <= 542) {
                            state = CREDITS;
                            inMenu = false;
                        }
                    }
                }
            }
        }
        else if (state == PRESENTATION) {
            bool inPres = true;
            SDL_Event event;

            while (inPres && state == PRESENTATION) {
                // Clear screen to clean white background
                SDL_FillRect(ecran, nullptr, SDL_MapRGB(ecran->format, 255, 255, 255));

                int cx = (ecran->w - 800) / 2;
                int cy = (ecran->h - 600) / 2;

                // Draw Robot at center
                if (robotSurface) {
                    SDL_Rect dstRobot;
                    dstRobot.x = (Sint16)((ecran->w - robotSurface->w) / 2);
                    dstRobot.y = (Sint16)((ecran->h - robotSurface->h) / 2 + 50);
                    SDL_BlitSurface(robotSurface, nullptr, ecran, &dstRobot);
                }

                // Draw Back Button at bottom-left
                if (retourSurface) {
                    SDL_Rect dstRetour = { (Sint16)(cx + 30), (Sint16)(cy + 520), 0, 0 };
                    SDL_BlitSurface(retourSurface, nullptr, ecran, &dstRetour);
                }

                // Load and draw current speech bubble
                if (currentBubble) {
                    SDL_FreeSurface(currentBubble);
                    currentBubble = nullptr;
                }
                currentBubble = loadBubble(bubbleIndex);
                if (currentBubble) {
                    SDL_Rect dstBubble = { (Sint16)(cx + 40), (Sint16)(cy + 30), 0, 0 };
                    SDL_BlitSurface(currentBubble, nullptr, ecran, &dstBubble);
                }

                SDL_Flip(ecran);

                if (SDL_WaitEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        state = QUIT;
                        inPres = false;
                    } else if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_ESCAPE) {
                            state = MENU;
                            inPres = false;
                        } else if (event.key.keysym.sym == SDLK_F11 || event.key.keysym.sym == SDLK_f) {
                            isFullScreen = !isFullScreen;
                            ecran = setVideo(isFullScreen);
                            // redraw loop continues
                        } else if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_SPACE) {
                            if (bubbleIndex < 6) {
                                bubbleIndex++;
                            } else {
                                state = MENU;
                                inPres = false;
                            }
                        } else if (event.key.keysym.sym == SDLK_LEFT) {
                            if (bubbleIndex > 1) {
                                bubbleIndex--;
                            }
                        }
                    } else if (event.type == SDL_MOUSEBUTTONUP) {
                        int mx = event.button.x;
                        int my = event.button.y;

                        // Check back arrow button (cx+30..cx+100, cy+520..cy+590)
                        if (mx >= cx + 30 && mx <= cx + 100 && my >= cy + 520 && my <= cy + 590) {
                            if (bubbleIndex == 1) {
                                state = MENU;
                                inPres = false;
                            } else {
                                bubbleIndex--;
                            }
                        } else {
                            // Click anywhere else advances bubble
                            if (bubbleIndex < 6) {
                                bubbleIndex++;
                            } else {
                                state = MENU;
                                inPres = false;
                            }
                        }
                    }
                }
            }
        }
        else if (state == CREDITS) {
            SDL_FillRect(ecran, nullptr, SDL_MapRGB(ecran->format, 0, 0, 0));

            int ox = 0;
            int oy = 0;
            if (creditsSurface) {
                ox = (ecran->w - creditsSurface->w) / 2;
                oy = (ecran->h - creditsSurface->h) / 2;
                SDL_Rect dst = { (Sint16)ox, (Sint16)oy, 0, 0 };
                SDL_BlitSurface(creditsSurface, nullptr, ecran, &dst);
            }

            // Draw back button
            if (retourSurface) {
                SDL_Rect dstRetour = { (Sint16)(ox + 30), (Sint16)(oy + (creditsSurface ? creditsSurface->h : BASE_H) - 80), 0, 0 };
                SDL_BlitSurface(retourSurface, nullptr, ecran, &dstRetour);
            }
            SDL_Flip(ecran);

            bool inCred = true;
            SDL_Event event;
            while (inCred) {
                if (SDL_WaitEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        state = QUIT;
                        inCred = false;
                    } else if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_F11 || event.key.keysym.sym == SDLK_f) {
                            isFullScreen = !isFullScreen;
                            ecran = setVideo(isFullScreen);
                            inCred = false; // redraw credits
                        } else {
                            state = MENU;
                            inCred = false;
                        }
                    } else if (event.type == SDL_MOUSEBUTTONUP) {
                        state = MENU;
                        inCred = false;
                    }
                }
            }
        }
    }

    // Cleanup surfaces
    if (currentBubble) SDL_FreeSurface(currentBubble);
    if (menuSurface) SDL_FreeSurface(menuSurface);
    if (robotSurface) SDL_FreeSurface(robotSurface);
    if (retourSurface) SDL_FreeSurface(retourSurface);
    if (creditsSurface) SDL_FreeSurface(creditsSurface);

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}
