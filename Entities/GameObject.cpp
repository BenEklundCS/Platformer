//
// Created by ben on 5/17/24.
//

#include "GameObject.h"

GameObject::~GameObject() = default;

Vector2 GameObject::GetPosition() {
    return position_;
}

Vector2 GameObject::GetDimensions() {
    return dimensions_;
}