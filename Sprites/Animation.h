//
// Created by ben on 5/20/24.
//

#ifndef PLATFORMER_ANIMATION_H
#define PLATFORMER_ANIMATION_H

#include "TextureManager.h"

class Animation {
    // Animation
private:
    GameTexture gameTexture_{};
    int currentFrame_;
    float frameTime_;
    int frameCount_;
    float frameDuration_;
    bool replay_;
public:
    Animation(GameTexture gameTexture, int frameCount, float frameDuration, bool replay);
    [[nodiscard]] Texture2D GetTexture() const;
    [[nodiscard]] Rectangle GetCurrentRect() const;
    void Animate();
    void Reset();
    void FlipX(bool movingRight);
};


#endif //PLATFORMER_ANIMATION_H
