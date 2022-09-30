#ifndef TA_GAME_H
#define TA_GAME_H

#include <chrono>
#include <SDL.h>
#include "TA_ScreenStateMachine.h"

class TA_Game {
private:
    const int baseHeight = 144;
    const double pixelAspectRatio = 0.95;
    const int soundFrequency = 44100, soundChunkSize = 1024;

    SDL_Event event;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime, currentTime;
    TA_ScreenStateMachine screenStateMachine;

public:
    TA_Game();
    ~TA_Game();
    bool process();
    void update();
};

#endif //TA_GAME_H
