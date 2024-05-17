//
// Created by ben on 5/17/24.
//

#ifndef PLATFORMER_RECTANGLE_H
#define PLATFORMER_RECTANGLE_H

#include "GameObject.h"


class Platform : public GameObject {
private:
    Color color{};
public:
    Platform(float pos_x, float pos_y, float dim_x, float dim_y, Color color);
    void Update() override;
    void Draw() override;
};


#endif //PLATFORMER_RECTANGLE_H