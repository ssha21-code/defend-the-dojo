#include <array>
#include <cmath>
#include <ctime>
#include "ExtraHeader.h"
#include <raylib.h>
#include <raymath.h>
#include <vector>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

constexpr int WINDOW_WIDTH = 1000;
constexpr int WINDOW_HEIGHT = 800;

Texture2D textureFloor;
Texture2D textureNinjaPlayer;
Texture2D textureNinjaEnemy;
Texture2D textureSpear;
Texture2D textureNinjaStar;
Texture2D textureMysteryBox;
Texture2D textureTitleScreen;
Texture2D texturePause;
Texture2D texturePlay;

Font font;

Sound soundFail;
Sound soundKaching;
Sound soundLure;
Sound soundSelection;
Sound soundSpearChange;
Sound soundStab;
Sound soundDash;

Music musicBackground;

void loadMedia() {
    Image imageFloor = LoadImage("images/floor.png");
    Image imageNinjaPlayer = LoadImage("images/ninja_player.png");
    Image imageNinjaEnemy = LoadImage("images/ninja_enemy.png");
    Image imageSpear = LoadImage("images/spear.png");
    Image imageNinjaStar = LoadImage("images/ninja_star.png");
    Image imageMysteryBox = LoadImage("images/mystery_box.png");
    Image imageTitleScreen = LoadImage("images/title_screen.png");
    Image imagePause = LoadImage("images/pause.png");
    Image imagePlay = LoadImage("images/play.png");
    
    textureFloor = LoadTextureFromImage(imageFloor);
    textureNinjaPlayer = LoadTextureFromImage(imageNinjaPlayer);
    textureNinjaEnemy = LoadTextureFromImage(imageNinjaEnemy);
    textureSpear = LoadTextureFromImage(imageSpear);
    textureNinjaStar = LoadTextureFromImage(imageNinjaStar);
    textureMysteryBox = LoadTextureFromImage(imageMysteryBox);
    textureTitleScreen = LoadTextureFromImage(imageTitleScreen);
    texturePause = LoadTextureFromImage(imagePause);
    texturePlay = LoadTextureFromImage(imagePlay);

    font = LoadFontEx("fonts/font.ttf", 64, nullptr, 0);

    soundFail = LoadSound("sounds/fail.wav");
    soundKaching = LoadSound("sounds/kaching.wav");
    soundLure = LoadSound("sounds/lure.wav");
    soundSelection = LoadSound("sounds/selection.wav");
    soundSpearChange = LoadSound("sounds/spear_change.wav");
    soundStab = LoadSound("sounds/stab.wav");
    soundDash = LoadSound("sounds/dash.wav");

    musicBackground = LoadMusicStream("sounds/background.wav");
    musicBackground.looping = true;

    UnloadImage(imageFloor);
    UnloadImage(imageNinjaPlayer);
    UnloadImage(imageNinjaEnemy);
    UnloadImage(imageSpear);
    UnloadImage(imageNinjaStar);
    UnloadImage(imageMysteryBox);
    UnloadImage(imageTitleScreen);
    UnloadImage(imagePause);
    UnloadImage(imagePlay);
}

class Player {
public: 
    Vector2 position;
    Vector2 size;
    Vector2 originalVelocity;
    Vector2 dashVelocity;
    Vector2 dashTarget;
    Vector2 origin;
    std::array<Vector2, 4> rectCorners;
    float dashPower = 0;
    float dashTime = 100.0f;
    float powerUpDuration = 10.0f;
    float maximumDamage = 10.0f;
    float luringSteps = 25.0f;
    float luringCooldown = 10.0f;
    float timeElapsed = 0.0f;
    int hp;
    int maxHp;
    int *gameModeIndex;

    bool isDashing = false;
    bool isImmune = false;
    bool isExtraFast = false;
    bool isExtraDamage = false;
    bool isLuring = false;
    bool checkedImmunity = false;
    bool checkedExtraSpeed = false;
    bool checkedExtraDamage = false;
    bool canLure = false;

    double immunityUpdateTime;
    double extraSpeedUpdateTime;
    double extraDamageUpdateTime;
    double luringUpdateTime;
    
    Player() {
        this->size = {(float)textureNinjaPlayer.width, (float)textureNinjaPlayer.height};
        this->position = {200.0f, 200.0f};
        this->originalVelocity = {8.0f, 8.0f};
        this->dashVelocity = Vector2Scale(originalVelocity, 2.0f);
        this->maxHp = 100;
        this->hp = maxHp;
        this->origin = {size.x / 2.0f, size.y / 2.0f};
        immunityUpdateTime = 0.0;
        extraSpeedUpdateTime = 0.0;
        extraDamageUpdateTime = 0.0;
        luringUpdateTime = 0.0;
    }

    void draw() {
        Rectangle src = {0, 0, size.x, size.y};
        Rectangle dest = {position.x + size.x / 2, position.y + size.y / 2, size.x, size.y};
        DrawTexturePro(textureNinjaPlayer, src, dest, origin, 0, WHITE);
    }
    void update(const float dt) {
        timeElapsed += dt;
        if ((IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) && position.y > 0 && !isDashing) {
            position.y -= originalVelocity.y;
        }
        if ((IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) && position.y + size.y < WINDOW_HEIGHT && !isDashing) {
            position.y += originalVelocity.y;
        }
        if ((IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) && position.x > 0 && !isDashing) {
            position.x -= originalVelocity.x;
        }
        if ((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) && position.x + size.x < WINDOW_WIDTH && !isDashing) {
            position.x += originalVelocity.x;
        }
        
        if ((IsKeyPressed(KEY_E) || IsKeyPressed(KEY_RIGHT_SHIFT)) && canLure) {
            PlaySound(soundLure);
            isLuring = true;
            luringUpdateTime = timeElapsed;
            canLure = false;
        }
        if (IsKeyPressed(KEY_SPACE) && !isDashing) {
            Vector2 mousePos = GetMousePosition();
            isDashing = true;
            dashTarget = mousePos;
            PlaySound(soundDash);
        }
        
        std::array<Vector2, 4> localCorners = {
            Vector2{ -origin.x, -origin.y },
            Vector2{  origin.x, -origin.y },
            Vector2{  origin.x,  origin.y },
            Vector2{ -origin.x,  origin.y }
        };
        for (int i = 0; i < 4; i++) {
            Vector2 rotated = Vector2Rotate(localCorners[i], 0 * DEG2RAD);
            rectCorners[i] = Vector2Add(position, rotated);
        }
        if (isDashing) {
            static bool hasUpdatedValues = false;
            static float spearAngleRad;
            static float velocityX;
            static float velocityY;
            static float acceleratedVelocity = 0;

            if (!hasUpdatedValues) {
                Vector2 center = {position.x + size.x / 2, position.y + size.y / 2};
                Vector2 spearDirection = Vector2Subtract(dashTarget, center);
                spearAngleRad = atan2(spearDirection.y, spearDirection.x);
                hasUpdatedValues = true;
            }
            
            velocityX = cos(spearAngleRad) * (dashVelocity.x + acceleratedVelocity);
            velocityY = sin(spearAngleRad) * (dashVelocity.y + acceleratedVelocity);
            acceleratedVelocity += 0.7f;
            position.x += velocityX;
            position.y += velocityY;
            
            dashPower += isExtraFast ? 8 : 5;
            dashTime -= 5;

            if (dashTime < 0) {
                isDashing = false;
                hasUpdatedValues = false;
                acceleratedVelocity = 0;
            }
            if (dashPower > 100) {
                dashPower = 100;
            }
        }
        if (!isDashing) {
            dashTime += isExtraFast ? 8.0f : 3.0f;
            if (dashTime > 100) {
                dashTime = 100;
            }
            dashPower = 0;
        }
        double currentTime = timeElapsed;
        if (isImmune) {
            if (!checkedImmunity) {
                immunityUpdateTime = currentTime;
                checkedImmunity = true;
            }
            if (currentTime - immunityUpdateTime >= powerUpDuration) {
                isImmune = false;
                checkedImmunity = false;
            }
        }
        if (isExtraFast) {
            if (!checkedExtraSpeed) {
                originalVelocity = Vector2Add(originalVelocity, {3, 3});
                dashVelocity = Vector2Add(dashVelocity, {5, 5});
                extraSpeedUpdateTime = currentTime;
                checkedExtraSpeed = true;
            }
            if (currentTime - extraSpeedUpdateTime >= powerUpDuration) {
                isExtraFast = false;
                originalVelocity = Vector2Subtract(originalVelocity, {3, 3});
                dashVelocity = Vector2Subtract(dashVelocity, {5, 5});
                checkedExtraSpeed = false;
            }
        }
        if (isExtraDamage) {
            if (!checkedExtraDamage) {
                extraDamageUpdateTime = currentTime;
                checkedExtraDamage = true;
            }
            if (currentTime - extraDamageUpdateTime >= powerUpDuration) {
                isExtraDamage = false;
                checkedExtraDamage = false;
            }
        }
        if (isLuring) {
            if (luringSteps <= 0.0f) {
                isLuring = false;
                luringSteps = 25.0f;
            }
        }
        if (currentTime - luringUpdateTime >= luringCooldown) {
            luringUpdateTime = currentTime;
            canLure = true;
        }
        switch (*gameModeIndex) {
            case 1:
                maxHp = 200;
                break;
            case 2:
                maxHp = 150;
                break;
            case 3: 
                maxHp = 100;
                break;
        }
    }
    void setGameModeIndex(int *gameModeIndex) {
        this->gameModeIndex = gameModeIndex;
        switch (*gameModeIndex) {
            case 1:
                maxHp = 200;
                break;
            case 2:
                maxHp = 150;
                break;
            case 3: 
                maxHp = 100;
                break;
        }
    }
};

class Spear {
    public: 
    enum class SpearLevel {
        IRON_LEVEL = 0,
        GOLD_LEVEL,
        DIAMOND_LEVEL,
        RUBY_LEVEL,
    };
    SpearLevel level;
    Vector2 position;
    Vector2 *playerPosition;
    Vector2 size;
    Vector2 origin;
    float angleDeg;
    int maxDamage;
    std::array<Vector2, 4> rectCorners;
    
    bool *isDashing;
    
    Spear(Vector2 *playerPosition, bool *isDashing) {
        this->position = *playerPosition;
        this->playerPosition = playerPosition;
        this->size = {(float)textureSpear.width, (float)textureSpear.height};
        this->angleDeg = 0;
        this->isDashing = isDashing;
        this->origin = {size.x / 2, size.y / 2};
        this->level = SpearLevel::IRON_LEVEL;
        this->maxDamage = 10;
    }

    void draw() {
        Rectangle src = {0, 0, size.x, size.y};
        Rectangle dest = {position.x + textureNinjaPlayer.width / 2, position.y + textureNinjaPlayer.height / 2, size.x, size.y};
        Color spearColor;
        switch (level) {
            case SpearLevel::IRON_LEVEL: spearColor = WHITE; maxDamage = 10; break;
            case SpearLevel::GOLD_LEVEL: spearColor = GOLD; maxDamage = 13; break;
            case SpearLevel::DIAMOND_LEVEL: spearColor = {0, 206, 255, 255}; maxDamage = 16; break;
            case SpearLevel::RUBY_LEVEL: spearColor = {255, 0, 0, 255}; maxDamage = 20; break;
        }
        DrawTexturePro(textureSpear, src, dest, origin, angleDeg, spearColor);
    }

    void update() {
        position = *playerPosition;
        Vector2 spearCenter = {position.x + textureNinjaPlayer.width / 2, position.y + textureNinjaPlayer.height / 2};
        Vector2 mousePos = GetMousePosition();
        Vector2 direction = Vector2Subtract(mousePos, spearCenter);
        float angleRad = atan2(direction.y, direction.x);
        if (!*isDashing) {
            angleDeg = angleRad * RAD2DEG;
        }
        std::array<Vector2, 4> localCorners = {
            Vector2{ -origin.x, -origin.y },
            Vector2{  origin.x, -origin.y },
            Vector2{  origin.x,  origin.y },
            Vector2{ -origin.x,  origin.y }
        };
        for (int i = 0; i < 4; i++) {
            Vector2 rotated = Vector2Rotate(localCorners[i], angleDeg * DEG2RAD);
            rectCorners[i] = Vector2Add(position, rotated);
        }
    }
};

class Attacker {
public: 
    Vector2 position;
    Vector2 size;
    Vector2 originalVelocity = {8.0f, 8.0f};
    Vector2 *playerPosition;
    Vector2 origin;
    std::array<Vector2, 4> rectCorners;
    float playerAngleDeg;
    float hurtCooldown = 1.2f;
    float hurtCooldownUpdateTime = 0;
    float shootingCooldown;
    float distanceMoved = 0;
    double shootingUpdateTime;
    int hp = 20;
    float movingAngleRad = 0;
    float timeElapsed = 0.0f;

    bool hasReachedPosition = false;
    bool shouldShoot = false;
    bool canGetHurt = false;
    bool isDead = false;
    bool *isPlayerLuring;

    Attacker(Vector2 position, Vector2 *playerPosition, bool *isPlayerLuring) {
        this->position = position;
        this->playerPosition = playerPosition;
        this->size = {(float)textureNinjaEnemy.width, (float)textureNinjaEnemy.height};
        this->shootingCooldown = GetRandomValue(10, 20) / 10.0f;
        this->shootingUpdateTime = timeElapsed;
        this->origin = {size.x / 2, size.y / 2};
        this->playerAngleDeg = 0;
        this->isPlayerLuring = isPlayerLuring;
    }

    void draw() {
        Rectangle src = {0, 0, size.x, size.y};
        Rectangle dest = {position.x + size.x / 2, position.y + size.y / 2, size.x, size.y};
        DrawTexturePro(textureNinjaEnemy, src, dest, origin, 0, WHITE);
        if (!isDead) {
            DrawTextEx(font, TextFormat("%i", hp), {position.x + 35, position.y - 40}, 35, 2, BLACK);
        }
    }
    void update(const float dt) {
        timeElapsed += dt;
        Vector2 direction = Vector2Subtract(*playerPosition, position);
        float angleRad = atan2(direction.y, direction.x);
        playerAngleDeg = angleRad * RAD2DEG;
        if (!hasReachedPosition) {
            float velocityX = cos(angleRad) * originalVelocity.x;
            float velocityY = sin(angleRad) * originalVelocity.y;
            position.x += velocityX;
            position.y += velocityY;
            distanceMoved += Vector2Length({velocityX, velocityY});
            if (distanceMoved >= 150.0f) {
                hasReachedPosition = true;
            }
        } else {
            double currentTime = timeElapsed;
            if (currentTime - shootingUpdateTime >= shootingCooldown) {
                shouldShoot = true;
                shootingUpdateTime = currentTime;
            }
            if (!*isPlayerLuring) {
                movingAngleRad += 0.05;
                float velocityX = cos(movingAngleRad) * originalVelocity.x / 2;
                position.x += velocityX;
            }
        }
        double currentTime = timeElapsed;
        if (currentTime - hurtCooldownUpdateTime >= hurtCooldown) {
            canGetHurt = true;
            hurtCooldownUpdateTime = currentTime;
        }
        if (hp <= 0) {
            hp = 0;
            isDead = true;
        }
        std::array<Vector2, 4> localCorners = {
            Vector2{ -origin.x, -origin.y },
            Vector2{  origin.x, -origin.y },
            Vector2{  origin.x,  origin.y },
            Vector2{ -origin.x,  origin.y }
        };
        for (int i = 0; i < 4; i++) {
            Vector2 rotated = Vector2Rotate(localCorners[i], 0 * DEG2RAD);
            rectCorners[i] = Vector2Add(position, rotated);
            DrawLineEx(i != 0 ? rectCorners.at(i - 1) : rectCorners.at(3), i != 3 ? rectCorners.at(i) : rectCorners.at(0), 2, RED);
        }
    }
    void moveTowardsPlayer() {
        if (fabs(position.x - playerPosition->x) > 50.0f || fabs(position.y - playerPosition->y) > 50.0f) {
            Vector2 direction = Vector2Subtract(*playerPosition, position);
            float angleRad = atan2(direction.y, direction.x);
            playerAngleDeg = angleRad * RAD2DEG;
            
            float velocityX = cos(angleRad) * originalVelocity.x;
            float velocityY = sin(angleRad) * originalVelocity.y;
            position.x += velocityX;
            position.y += velocityY;
        }
    }
};

class NinjaStar {
public: 
    Vector2 position;
    Vector2 size;
    Vector2 origin;
    std::array<Vector2, 4> rectCorners;
    Vector2 originalVelocity;
    float angleDeg;
    float angleRotationDeg;
    float acceleratedVelocity;

    bool shouldBeDestroyed = false;

    NinjaStar(Vector2 position, float angleDeg) {
        this->position = position;
        this->size = {(float)textureNinjaStar.width, (float)textureNinjaStar.height};
        this->origin = {size.x / 2, size.y / 2};
        this->originalVelocity = {10.0f, 10.0f};
        this->angleDeg = angleDeg;
        this->angleRotationDeg = 0;
        this->acceleratedVelocity = 0;
    }
    
    void draw() {
        Rectangle src = {0, 0, size.x, size.y};
        Rectangle dest = {position.x + size.x / 2, position.y + size.y / 2, size.x, size.y};
        DrawTexturePro(textureNinjaStar, src, dest, origin, angleRotationDeg, WHITE);
    }
    void update() {
        float angleRad = angleDeg * DEG2RAD;
        acceleratedVelocity += 0.5;
        float velocityX = cos(angleRad) * (originalVelocity.x + acceleratedVelocity);
        float velocityY = sin(angleRad) * (originalVelocity.y + acceleratedVelocity);
        position = Vector2Add(position, {velocityX, velocityY});
        std::array<Vector2, 4> localCorners = {
            Vector2{ -origin.x, -origin.y },
            Vector2{  origin.x, -origin.y },
            Vector2{  origin.x,  origin.y },
            Vector2{ -origin.x,  origin.y }
        };
        for (int i = 0; i < 4; i++) {
            Vector2 rotated = Vector2Rotate(localCorners[i], angleDeg * DEG2RAD);
            rectCorners[i] = Vector2Add(position, rotated);
        }
        angleRotationDeg += 20;
        angleRotationDeg = (int)angleRotationDeg % 180;
    }
};

class MysteryBox {
public: 
    enum class MysteryBoxType {
        IMMUNITY,
        EXTRA_DAMAGE,
        EXTRA_SPEED,
    };
    Vector2 position;
    Vector2 size;
    Vector2 origin;
    std::array<Vector2, 4> rectCorners;
    MysteryBoxType mysteryBoxType;

    bool shouldBeDestroyed = false;

    MysteryBox(Vector2 position, MysteryBoxType mysteryBoxType) {
        this->position = position;
        this->size = {(float)textureMysteryBox.width, (float)textureMysteryBox.height};
        this->origin = {size.x / 2, size.y / 2};
        this->mysteryBoxType = mysteryBoxType;
    }

    void draw() {
        Rectangle src = {0, 0, size.x, size.y};
        Rectangle dest = {position.x + size.x / 2, position.y + size.y / 2, size.x, size.y};
        DrawTexturePro(textureMysteryBox, src, dest, origin, 0, WHITE);
        char text[32];
        switch (mysteryBoxType) {
            case MysteryBoxType::IMMUNITY: strcpy(text, "Immunity"); break;
            case MysteryBoxType::EXTRA_SPEED: strcpy(text, "Extra Speed"); break;
            case MysteryBoxType::EXTRA_DAMAGE: strcpy(text, "Extra Damage"); break;
        }
        float textWidth = MeasureTextEx(font, text, 15, 2).x;
        DrawTextEx(font, text, {position.x, dest.y - 80}, 15, 2, WHITE);
    }
    void update() {
        std::array<Vector2, 4> localCorners = {
            Vector2{ -origin.x, -origin.y },
            Vector2{  origin.x, -origin.y },
            Vector2{  origin.x,  origin.y },
            Vector2{ -origin.x,  origin.y }
        };
        for (int i = 0; i < 4; i++) {
            Vector2 rotated = Vector2Rotate(localCorners[i], 0 * DEG2RAD);
            rectCorners[i] = Vector2Add(position, rotated);
            DrawLineEx(i != 0 ? rectCorners.at(i - 1) : rectCorners.at(3), i != 3 ? rectCorners.at(i) : rectCorners.at(0), 2, RED);
        }
    }
};

class Game {
public: 
    Player player;
    Spear *spear = nullptr;
    std::vector<Attacker> attackers;
    std::vector<NinjaStar> ninjaStars;
    std::vector<MysteryBox> mysteryBoxes;
    int attackersSpawnCount = 5;
    int score = 0;
    int maxSpearUnlocked = 0;
    int gameModeIndex;
    float timeElapsed = 0.0f;
    Vector2 attackerSpawnPositions[5] = {
        {0, WINDOW_HEIGHT / 2}, {WINDOW_WIDTH, WINDOW_HEIGHT / 2}, 
        {WINDOW_WIDTH / 2, 0}, {WINDOW_WIDTH / 2, WINDOW_HEIGHT}, 
        {0, 0}
    };

    bool shouldSpawnAttackers = false;
    bool shouldPurchase = false;
    bool isPaused = false;
    bool isInTitleScreen = true;

    Rectangle startButtonArea = {325, 643, 345, 141};
    Rectangle pausePlayButtonArea;
    Rectangle gameModeArea = {WINDOW_WIDTH - 300, WINDOW_HEIGHT - 150, 250, 70};

    enum class GameMode {
        PEACEFUL, 
        EASY, 
        MEDIUM,
        HARD,
    };

    GameMode gameMode;

    Game() {
        SetRandomSeed(time(NULL));
        loadMedia();
        player = Player();
        gameModeIndex = 0;
        player.setGameModeIndex(&gameModeIndex);
        this->spear = new Spear(&player.position, &player.isDashing);
        this->pausePlayButtonArea = {25.0f, WINDOW_HEIGHT - texturePause.height - 25.0f, (float)texturePause.width, (float)texturePause.height};
        PlayMusicStream(musicBackground);
    }

    void draw() {
        if (isInTitleScreen) {
            GuiSetFont(font);
            GuiSetStyle(DEFAULT, TEXT_SIZE, 35);
            DrawTexture(textureTitleScreen, 0, 0, WHITE);
            GuiSetStyle(DEFAULT, TEXT_SIZE, 30);
            GuiSetStyle(COMBOBOX, COMBO_BUTTON_WIDTH, 70);
            GuiSetStyle(COMBOBOX, TEXT_COLOR_NORMAL, ColorToInt(WHITE));
            GuiSetStyle(COMBOBOX, TEXT_COLOR_FOCUSED, ColorToInt(WHITE));
            GuiSetStyle(COMBOBOX, TEXT_COLOR_PRESSED, ColorToInt(WHITE));

            GuiSetStyle(COMBOBOX, BASE_COLOR_NORMAL, ColorToInt({69, 36, 0, 255}));
            GuiSetStyle(COMBOBOX, BASE_COLOR_FOCUSED, ColorToInt({69, 36, 0, 255}));
            GuiSetStyle(COMBOBOX, BASE_COLOR_PRESSED, ColorToInt({69, 36, 0, 255}));

            GuiSetStyle(COMBOBOX, BORDER_WIDTH, 5);
            GuiSetStyle(COMBOBOX, BORDER_COLOR_NORMAL, ColorToInt(BLACK));
            GuiSetStyle(COMBOBOX, BORDER_COLOR_FOCUSED, ColorToInt(BLACK));
            GuiSetStyle(COMBOBOX, BORDER_COLOR_PRESSED, ColorToInt(BLACK));

            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(BLACK));
            GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(BLACK));
            GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, ColorToInt(BLACK));
            GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, ColorToInt(BLACK));
            GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, ColorToInt(BLACK));
            GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, ColorToInt(BLACK));
            GuiSetStyle(BUTTON, BASE_COLOR_DISABLED, ColorToInt((Color){64, 64, 64, 255}));
            GuiSetStyle(BUTTON, BORDER_COLOR_DISABLED, ColorToInt((Color){64, 64, 64, 255}));
            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(WHITE));

            static int prevGameModeIndex = 0;
            
            GuiComboBox(gameModeArea, "Peaceful;Easy;Medium;Hard", &gameModeIndex);

            if (gameModeIndex != prevGameModeIndex) {
                PlaySound(soundSelection);
                prevGameModeIndex = gameModeIndex;
            }
            
            gameMode = static_cast<GameMode>(gameModeIndex);
        } else {
            Rectangle purchaseButtonBounds = {WINDOW_WIDTH - 240, 85, 200, 50};
            GuiSetStyle(DEFAULT, TEXT_SIZE, 35);
            GuiSetFont(font);
            GuiSetStyle(DEFAULT, TEXT_SPACING, 2);
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(BLACK));
            GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(BLACK));
            GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, ColorToInt(BLACK));
            GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, ColorToInt(BLACK));
            GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, ColorToInt(BLACK));
            GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, ColorToInt(BLACK));
            GuiSetStyle(BUTTON, BASE_COLOR_DISABLED, ColorToInt((Color){64, 64, 64, 255}));
            GuiSetStyle(BUTTON, BORDER_COLOR_DISABLED, ColorToInt((Color){64, 64, 64, 255}));
            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(WHITE));
    
            DrawTexture(textureFloor, 0, 0, WHITE);
            player.draw();
            spear->draw();
            for (auto &mysteryBox: mysteryBoxes) {
                mysteryBox.draw();
            }
            for (auto &attacker: attackers) {
                attacker.draw();
            }
            for (auto &ninjaStar: ninjaStars) {
                ninjaStar.draw();
            }
    
            DrawRectangleLinesEx({25.0f, 25.0f, 325.0f, 25.0f}, 3, BLACK);
            DrawRectangleV({28.0f, 28.0f}, {player.dashTime / 100.0f * 319.0f, 19.0f}, BLACK);
            int temp = MeasureText("Dash Time", 28);
            DrawTextEx(font, "Dash Time", {25 + (325 - temp) / 2.0f, 25}, 28, 2, YELLOW);
            DrawRectangleLinesEx({25.0f, 75.0f, 325.0f, 25.0f}, 3, BLACK);
            DrawRectangleV({28.0f, 78.0f}, {player.dashPower / 100.0f * 319.0f, 19.0f}, BLACK);

            temp = MeasureText("Dash Power", 28);
            DrawTextEx(font, "Dash Power", {25 + (325 - temp) / 2.0f, 75}, 28, 2, DARKGREEN);

            if (gameMode != GameMode::PEACEFUL) {
                DrawRectangleLinesEx({400.0f, 25.0f, 325.0f, 25.0f}, 3, BLACK);
                DrawRectangleV({403.0f, 28.0f}, {(float)player.hp / player.maxHp * 319.0f, 19.0f}, BLACK);
                temp = MeasureText("Health", 28);
                DrawTextEx(font, "Health", {400 + (325 - temp) / 2.0f, 25.0f}, 28, 2,RED);
            }

            DrawRectangleLinesEx({400.0f, 75.0f, 325.0f, 25.0f}, 3, BLACK);
            float lureProgress = (player.timeElapsed - player.luringUpdateTime) / 10.0f;
            lureProgress = Clamp(lureProgress, 0.0f, 1.0f);
            DrawRectangleV({403.0f, 78.0f}, {(float)lureProgress * 319.0f, 19.0f}, BLACK);
            if (player.canLure) {
                DrawRectangleV({403.0f, 78.0f}, {319.0f, 19.0f}, BLACK);
            }
            temp = MeasureText("Lure Cooldown", 28);
            DrawTextEx(font, "Lure Cooldown", {400 + (325 - temp) / 2.0f, 75.0f}, 28, 2, BLUE);
    
            DrawTextEx(font, TextFormat("Score: %i", score), {WINDOW_WIDTH - 220.0f, 25.0f}, 40, 2, BLACK);
            
            if (player.isExtraDamage) {
                DrawRectangleLinesEx({25.0f, 200.0f, 225.0f, 25.0f}, 3, BLACK);
                float extraDamageProgress = (player.timeElapsed - player.extraDamageUpdateTime) / 10.0f;
                extraDamageProgress = Clamp(extraDamageProgress, 0.0f, 1.0f);
                DrawRectangleV({28.0f, 203.0f}, {219.0f - (float)(extraDamageProgress * 219.0f), 19.0f}, BLACK);
                temp = MeasureText("Extra Damage", 25);
                DrawTextEx(font, "Extra Damage", {25 + (225 - temp) / 2.0f, 200.0f}, 25, 2, BLUE);
            }
            
            if (player.isExtraFast) {
                DrawRectangleLinesEx({25.0f, 250.0f, 225.0f, 25.0f}, 3, BLACK);
                DrawRectangleV({28.0f, 253.0f}, {219.0f - (float)((player.timeElapsed - player.extraSpeedUpdateTime) / 10 * 219.0f), 19.0f}, BLACK);
                temp = MeasureText("Extra Speed", 25);
                DrawTextEx(font, "Extra Speed", {25 + (225 - temp) / 2.0f, 250.0f}, 25, 2, BLUE);
            }
            
            if (player.isImmune) {
                DrawRectangleLinesEx({25.0f, 300.0f, 225.0f, 25.0f}, 3, BLACK);
                DrawRectangleV({28.0f, 303.0f}, {219.0f - (float)((player.timeElapsed - player.immunityUpdateTime) / 10 * 219.0f), 19.0f}, BLACK);
                temp = MeasureText("Immunity", 25);
                DrawTextEx(font, "Immunity", {25 + (225 - temp) / 2.0f, 300.0f}, 25, 2, BLUE);
            }
            
            GuiDisable();
            if (score > 25) {
                GuiEnable();
            }
    
            shouldPurchase = GuiButton(purchaseButtonBounds, "Purchase");
            GuiEnable();

            DrawTexture(isPaused ? texturePlay : texturePause, pausePlayButtonArea.x, pausePlayButtonArea.y, WHITE);

        }
    }
    void update(const float dt) {
        timeElapsed += dt;
        Vector2 mousePos = GetMousePosition();
        if (isInTitleScreen) {
            if (CheckCollisionPointRec(mousePos, startButtonArea) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlaySound(soundSelection);
                isInTitleScreen = false;
            }
        } else {
            if (CheckCollisionPointRec(mousePos, pausePlayButtonArea) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlaySound(soundSelection);
                isPaused = !isPaused;
            }
            if (!isPaused) {
                UpdateMusicStream(musicBackground);
                player.update(dt);
                spear->update();
                if (attackers.empty()) {
                    shouldSpawnAttackers = true;
                }
                if (shouldSpawnAttackers) {
                    attackersSpawnCount = GetRandomValue(1, 5);
                    player.hp = player.maxHp;
                    for (int i = 0; i < attackersSpawnCount; i++) {
                        attackers.push_back(Attacker(attackerSpawnPositions[i], &player.position, &player.isLuring));
                    }
                    player.dashPower = 0;
                    shouldSpawnAttackers = false;
                }
                if (shouldPurchase) {
                    Vector2 spawnPos = {(float)GetRandomValue(100, WINDOW_WIDTH - 100), (float)GetRandomValue(100, WINDOW_HEIGHT - 100)};
                    MysteryBox::MysteryBoxType mysteryBoxType = static_cast<MysteryBox::MysteryBoxType>(GetRandomValue(0, 2));
                    mysteryBoxes.push_back(MysteryBox(spawnPos, mysteryBoxType));
                    score -= 5;
                    shouldPurchase = false;
                    PlaySound(soundKaching);
                }
                if (player.isLuring) {
                    for (auto &attacker: attackers) {
                        attacker.moveTowardsPlayer();
                    }
                    player.luringSteps--;
                }
                if (IsKeyPressed(KEY_PERIOD)) {
                    reset();
                }
                if (player.hp <= 0 && gameMode != GameMode::PEACEFUL) {
                    PlaySound(soundFail);
                    reset();
                }
                for (auto &attacker: attackers) {
                    attacker.update(dt);
                    if (attacker.shouldShoot) {
                        ninjaStars.push_back(NinjaStar({attacker.position.x + attacker.size.x / 2.0f, attacker.position.y + attacker.size.y / 2.0f - 50.0f}, attacker.playerAngleDeg));
                        attacker.shouldShoot = false;
                    }
                }
                for (auto &ninjaStar: ninjaStars) {
                    ninjaStar.update();
                }
                for (auto &mysteryBox: mysteryBoxes) {
                    mysteryBox.update();
                }
                updateSpearLevel();
                handleManualSpearSwitch();
                garbageCollect();
                checkForCollisions();
                checkForRemoval();
            }
        }
    }
    void garbageCollect() {
        for (int i = 0; i < attackers.size(); i++) {
            if (attackers.at(i).isDead) {
                score++;
                attackers.erase(attackers.begin() + i);
                i--;
            }
        }
        for (int i = 0; i < ninjaStars.size(); i++) {
            if (ninjaStars.at(i).shouldBeDestroyed) {
                ninjaStars.erase(ninjaStars.begin() + i);
                i--;
            }
        }
        for (int i = 0; i < mysteryBoxes.size(); i++) {
            if (mysteryBoxes.at(i).shouldBeDestroyed) {
                mysteryBoxes.erase(mysteryBoxes.begin() + i);
                i--;
            }
        }
    }
    void checkForCollisions() {
        for (auto &attacker: attackers) {
            if (Collision::CheckCollisionRectCorners(spear->rectCorners, attacker.rectCorners)) {
                if (player.isDashing && attacker.canGetHurt) {
                    float maxDamage = player.isExtraDamage ? spear->maxDamage + 5 : spear->maxDamage;
                    float damage = player.dashPower / 100 * maxDamage;
                    attacker.hp -= damage;
                    attacker.canGetHurt = false;
                    PlaySound(soundStab);
                }
            }
        }
        for (auto &ninjaStar: ninjaStars) {
            if (Collision::CheckCollisionRectCorners(ninjaStar.rectCorners, player.rectCorners) && !player.isImmune) {
                int damage = GetRandomValue(3, 5);
                PlaySound(soundStab);
                player.hp -= damage;
                ninjaStar.shouldBeDestroyed = true;
            }
        }
        for (auto &mysteryBox: mysteryBoxes) {
            if (Collision::CheckCollisionRectCorners(player.rectCorners, mysteryBox.rectCorners) && player.isDashing) {
                switch (mysteryBox.mysteryBoxType) {
                    case MysteryBox::MysteryBoxType::IMMUNITY:
                        player.isImmune = true;
                        player.immunityUpdateTime = player.timeElapsed;
                        break;
                    case MysteryBox::MysteryBoxType::EXTRA_DAMAGE:
                        player.isExtraDamage = true;
                        player.extraDamageUpdateTime = player.timeElapsed;
                        break;
                    case MysteryBox::MysteryBoxType::EXTRA_SPEED:
                        player.isExtraFast = true;
                        player.extraSpeedUpdateTime = player.timeElapsed;
                        break;
                };
                PlaySound(soundSelection);
                mysteryBox.shouldBeDestroyed = true;
            }
        }
    }
    void checkForRemoval() {
        for (auto &ninjaStar: ninjaStars) {
            if (ninjaStar.position.x < -200 || ninjaStar.position.x + ninjaStar.size.x > WINDOW_WIDTH + 200 || ninjaStar.position.y < -200 || ninjaStar.position.y + ninjaStar.size.y > WINDOW_HEIGHT + 200) {
                ninjaStar.shouldBeDestroyed = true;
            }
        }
    }
    void updateSpearLevel() {
        if (score > 50 && maxSpearUnlocked < 3) maxSpearUnlocked = 3;
        else if (score > 20 && maxSpearUnlocked < 2) maxSpearUnlocked = 2;
        else if (score > 10 && maxSpearUnlocked < 1) maxSpearUnlocked = 1;
    }
    void handleManualSpearSwitch() {
        if (IsKeyPressed(KEY_LEFT_ALT)) {
            int newLevel = ((int)spear->level + 1) % 4;
            if (newLevel <= maxSpearUnlocked) {
                spear->level = static_cast<Spear::SpearLevel>(newLevel);
                PlaySound(soundSpearChange);
            } else {
                spear->level = Spear::SpearLevel::IRON_LEVEL;
                if (maxSpearUnlocked > 0) {
                    PlaySound(soundSpearChange);
                }
            }
        }
    }
    void reset() {
        isInTitleScreen = true;
        isPaused = false;
        player.hp = player.maxHp;
        player.position = {200.0f, 200.0f};
        attackers.clear();
        mysteryBoxes.clear();
        ninjaStars.clear();
        score = 0;
        player.isDashing = false;
        player.isExtraDamage = false;
        player.isExtraFast = false;
        player.isImmune = false;
        player.isLuring = false;
        player.canLure = false;
        player.luringUpdateTime = player.timeElapsed;
        maxSpearUnlocked = 0;
        spear->level = Spear::SpearLevel::IRON_LEVEL;
        SetRandomSeed(time(NULL));
        timeElapsed = 0;
    }
};

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Defend The Dojo");
    InitAudioDevice();
    SetTargetFPS(60);

    Game game = Game();

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        float dt = game.isPaused || game.isInTitleScreen ? 0.0f : GetFrameTime();

        game.update(dt);
        game.draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}