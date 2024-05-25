//
// Created by ben on 5/20/24.
//

#ifndef PLATFORMER_LEVEL_H
#define PLATFORMER_LEVEL_H

#include <memory>
#include "../Renderer/Window.h"
#include "../Renderer/Renderer.h"
#include "../Entities/Player/Player.h"
#include "LevelScene.h"


class Level {
protected:
    std::unique_ptr<Player> player;
    Renderer renderer;
public:
public:
    Level();
    virtual void StartLevel();
    ~Level();
};


#endif //PLATFORMER_LEVEL_H
