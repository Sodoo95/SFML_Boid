#pragma once
#include <stdio.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include "SmallFish.h"
#include "Shark_Head.h"
#include "Shark_Mid.h"
#include "Shark_End.h"

using namespace std;
using namespace sf;

struct     Position     { Vector2f position; };
struct     Rotation     { Angle angle; };
struct ForwardDirection { Vector2f forwardDirection; };
struct      CellID      { int id; };
struct     BoidColor    { Color color; };
struct    Neighbours    { vector<Vector2f> pos; };
struct   NeiDirection   { vector<Vector2f> dir; };
struct   BoidSpeed      { float speed; };
struct      BoidID      { unsigned int id; };
struct    BoidSprite    { 
    Sprite sprite;
    BoidSprite() = default;
    BoidSprite(const Texture& texture) : sprite(texture) {}
};

struct CellPosDir { Vector2f pos; Vector2f dir; };

class Game {
private:
    RenderWindow    _window;
    unsigned int    _windowHeight           = 800;
    unsigned int    _windowWidth            = 1200;
    int             _borderSize             = 80;

    unsigned int    _boidsNumber            = 1300;
    float           _boidSpeed              = 100.0f;
    float           _boidRadius             = 4.0f;
    float           _boidSteerSpeed         = 3;
    float           _boidCoherence          = 100;
    float           _boidSeperation         = 10;
    float           _boidCushion            = 30;
    Color           _boidColor              = Color(0, 0, 0, 0);
    Vector2f        _boidForwardDirection   { 0,0 };
    Vector2f        _boidAvoidance          { 0,0 };
    Vector2f        _sharkTail              { 0,0 };
    Vector2f        _sharkLeft              { 0,0 };
    Vector2f        _sharkTailTail          { 0,0 };
    Vector2f        _sharkForwardDir        { 0,0 };
    Color           _boidCIDcolor[24];
    vector<vector<CellPosDir>> _boidGrid{};

    Texture smallFishTexture;
    Texture sharkHeadTexture;
    Texture sharkMidTexture;
    Texture sharkEndTexture;

    double temp = 0.0f;

    bool    isDebug         = false;

    Time    _dt             = Time::Zero;
    float   _deltaTime      = 0.0f;
    Clock   _deltaClock;
    entt::registry _registry;
    
    void Start();
    void Update(float deltaTime);
    void Render();
    void HandleInput();
    
    Vector2f normalize              (const Vector2f& v);
    Vector2f getDirectionTo         (const Vector2f& start, const Vector2f& target);
    Vector2f getDirectionOpposite   (const Vector2f& start, const Vector2f& target);
    float    getDistance            (const Vector2f& a, const Vector2f& b);
    float    getDistanceSq          (const Vector2f& a, const Vector2f& b);    
    void     drawLine               (const Vector2f& p1, const Vector2f& p2);
    vector<int> getNeighbours       (int index);

public:
    Game();
    void Run();
};