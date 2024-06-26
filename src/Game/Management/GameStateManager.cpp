//
// Created by ben on 5/16/24.
//

#include "../../../include/Platform/Renderer.h"
#include "../../../include/Game/Management/GameInputManager.h"
#include "../../../include/Game/Management/GameStateManager.h"
#include <algorithm>

std::unique_ptr<GameStateManager> GameStateManager::instance = nullptr;

GameStateManager::GameStateManager() {
    levelOver = false;
}

// Window Singleton Pattern
GameStateManager* GameStateManager::GetInstance() {
    TraceLog(LOG_INFO, "GameStateManager::GetInstance() Invoked."); // Log
    if (instance == nullptr) {
        TraceLog(LOG_INFO, "Creating GameStateManager...");
        instance = std::make_unique<GameStateManager>(); // Typical singleton stuff, but I've implemented the instance as a unique ptr
    }
    return instance.get(); // if the window already exists, simply return it
}

#pragma region update gameState methods

// Update all game objects and handle collisions
void GameStateManager::Update() {
    // Update the camera
    UpdateCamera();
    // Update all game objects and handle collisions
    UpdatePlayers(); // update Players
    // Handle user input
    inputManager->HandleUserInput();
    UpdateMonsters(); // update Monsters
}

// Update all players in the scene by iterating over the players, calling update, and then checking for collisions
void GameStateManager::UpdatePlayers() {

    for (auto& player : players_) {
        player->Update();

        // Iterate over the platforms to check for collisions
        for (auto& platform : platforms_) {
            player->PlatformCollision(platform.get());
        }
        // Iterate over any other objects to check for collisions (especially TILEs)
        #pragma omp parallel for // update the other objects in parallel using threads
        for (auto& other : otherObjects_) {
            player->PlatformCollision(other.get());
        }
    }
}

// Handle player and monster attacks onscreen
void GameStateManager::UpdateAttacks(Player* player) {
    // Get the current player X position
    float playerPosX = player->GetPosition().x;
    // Vector to store delayed updates
    std::vector<GameObject*> toRemove;
    // Iterate over all monsters
    for (auto& monster : monsters_) {
        // Get monster position
        float monsterPosX = monster->GetPosition().x;
        if (player->GetPlayerData()->movingRight_) {
            // Hit monsters 0 to 500px in front of the player
            if (monsterPosX > playerPosX && monsterPosX <= playerPosX + 500) {
                // Handle monster hitting and if they should die!
                monster->SetHealth(monster->GetHealth() - 1);
                if (monster->GetHealth() <= 0) {
                    toRemove.push_back(monster.get());
                }
            }
        }
        else {
            // Hit monsters 0 to 500px in front of the player
            if (monsterPosX < playerPosX && monsterPosX >= playerPosX - 500) {
                monster->SetHealth(monster->GetHealth() - 1);
                // Handle monster hitting and if they should die!
                if (monster->GetHealth() <= 0) {
                    toRemove.push_back(monster.get());
                }
            }
        }
    }
    // Defer deletions of monsters to avoid issues accessing them (memory exceptions!) in the loop above ^
    for (auto& obj : toRemove) {
        RemoveObject(obj);
    }
}

// Update all players in the scene by iterating over the monsters, calling update, and then checking for collisions
void GameStateManager::UpdateMonsters() {
    for (auto& monster : monsters_) {
        monster->Update();
        for (auto& player : players_) {
            monster->CollideWithPlayer(player.get());
        }
        for (auto& platform : platforms_) {
            monster->PlatformCollision(platform.get());
        }
        #pragma omp parallel for // update the other objects in parallel using threads
        for (auto& other : otherObjects_) {
            monster->PlatformCollision(other.get());
        }
    }
}

#pragma endregion

#pragma region object loading/interaction

// Add a game object to the GameStateManager. The GameStateManager maintains unique_ptr ownership over Objects.
// Make sure to pass a std::unique_ptr<GameObject> to it using std::move()
// Do not add nullptr objects, undefined behavior
void GameStateManager::AddObject(std::unique_ptr<GameObject> obj) {
    allGameObjects_.push_back(obj.get());
    if (obj->type_ == PLAYER) {
        players_.push_back(std::unique_ptr<Player>(dynamic_cast<Player*>(obj.release())));
    }
    else if (obj->type_ == MONSTER) {
        monsters_.push_back(std::unique_ptr<Monster>(dynamic_cast<Monster*>(obj.release())));
    }
    else if (obj->type_ == PLATFORM) {
        platforms_.push_back(std::unique_ptr<Platform>(dynamic_cast<Platform*>(obj.release())));
    }
    else {
        otherObjects_.push_back(std::move(obj));
    }
}

void GameStateManager::RemoveObject(GameObject* obj) {
    // Find and remove the object from the allGameObjects_ vector
    allGameObjects_.erase(std::remove(allGameObjects_.begin(), allGameObjects_.end(), obj), allGameObjects_.end());

    // Find and remove the object from the appropriate type-specific vector
    switch (obj->type_) {
        case PLAYER: {
            auto it = std::find_if(players_.begin(), players_.end(), [&obj](const std::unique_ptr<Player>& player) {
                return player.get() == obj;
            });
            if (it != players_.end()) {
                players_.erase(it);
            }
            break;
        }
        case MONSTER: {
            auto it = std::find_if(monsters_.begin(), monsters_.end(), [&obj](const std::unique_ptr<Monster>& monster) {
                return monster.get() == obj;
            });
            if (it != monsters_.end()) {
                monsters_.erase(it);
            }
            break;
        }
        case PLATFORM: {
            auto it = std::find_if(platforms_.begin(), platforms_.end(), [&obj](const std::unique_ptr<Platform>& platform) {
                return platform.get() == obj;
            });
            if (it != platforms_.end()) {
                platforms_.erase(it);
            }
            break;
        }
        default: {
            auto it = std::find_if(otherObjects_.begin(), otherObjects_.end(), [&obj](const std::unique_ptr<GameObject>& other) {
                return other.get() == obj;
            });
            if (it != otherObjects_.end()) {
                otherObjects_.erase(it);
            }
            break;
        }
    }
}

// Return the full GameObject* vector
std::vector<GameObject*> GameStateManager::GetAllObjects() {
    return allGameObjects_;
}

#pragma endregion

#pragma region game state methods

// Init the camera to its base target, offset, rotation, and zoom.
// Currently, supports only one player.
// Call this function just before attempting to render the GameState
// DO NOT CALL UNLESS players_ HAS AT LEAST ONE PLAYER
void GameStateManager::InitCamera() {
    Player* player1 = players_.at(0).get();
    camera.target = (Vector2){player1->GetPosition().x + 20.0f, player1->GetPosition().y + 20.0f};
    camera.offset = (Vector2){(float)GetScreenWidth() / 2.0f, (float)GetScreenHeight() / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

// Update the camera using the players current position
void GameStateManager::UpdateCamera() {
    Player* player1 = players_.at(0).get();
    camera.target = (Vector2){ player1->GetPosition().x + 20, player1->GetPosition().y + 20 };
}

// Return the levelOver boolean flag
bool GameStateManager::IsLevelOver() const {
    return levelOver;
}

// Set levelOver to true
void GameStateManager::SetLevelOver() {
    levelOver = true;
}

// Return the camera
Camera2D GameStateManager::GetCamera() {
    return camera;
}

#pragma endregion

// Retrieve a gameData struct from the GameStateManager, giving the context of the games current state
const gameData* GameStateManager::GetGameData() {
    // IMPORTANT: GetPlayerData, and GameStateManager support ONE PLAYER
    auto data = new gameData;
    data->playerPosition = players_[0].get()->GetPosition();
    data->playerData =  players_[0].get()->GetPlayerData();
    return data;
}

void GameStateManager::InitInput(EngineSettings* settings) {
    inputManager = std::make_unique<GameInputManager>(players_[0].get(), *settings);
}

// Cleanup the vectors on destruct
GameStateManager::~GameStateManager() {
    players_.clear();
    monsters_.clear();
    platforms_.clear();
    otherObjects_.clear();
}
