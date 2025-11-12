#include "Game.h"
#include <stdio.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <numbers>
#include <cmath>
#include <iostream>

void Game::Start() {     
    

    for (unsigned int i = 0; i < _boidsNumber; i++) {
        entt::entity entity = _registry.create();

        Vector2f pos;
        if (i >= 0 && i < 3) {
            pos.x = float(_windowWidth / 2);
            pos.y = float(_windowHeight / 2);
        }
        else {
            pos.x = float(rand() % _windowWidth - 100);
            pos.y = float(rand() % _windowHeight - 100);
        }
            
                                                            
        Angle angle = degrees(float(rand() % 360));
        float rotRad = float(angle.wrapUnsigned().asRadians() - numbers::pi / 2.0f);

        Vector2f forwardDir;
            forwardDir.x = cos(rotRad);
            forwardDir.y = sin(rotRad);

        Position posComp;        
            posComp.position = pos;

        ForwardDirection fDirComp;
            fDirComp.forwardDirection = forwardDir;

        Rotation rotComp;
            rotComp.angle = angle;
        
        Color colorComp = Color(rand() & 100, (rand() & 100) + 155, (rand() & 100) + 155, 255);

        float speedComp = _boidSpeed;

        unsigned int boidIdComp;
        if (i == 0) boidIdComp = 0;
        else if (i > 0 && i < 3) boidIdComp = i;
        else boidIdComp = 3;



        _registry.emplace     <Position>     (entity, posComp);
        _registry.emplace <ForwardDirection> (entity, fDirComp);
        _registry.emplace     <Rotation>     (entity, rotComp);
        _registry.emplace      <CellID>      (entity, 0);
        _registry.emplace     <BoidColor>    (entity, colorComp);
        _registry.emplace     <Neighbours>   (entity);
        _registry.emplace    <NeiDirection>  (entity);
        _registry.emplace    <BoidSpeed>  (entity, speedComp);
        _registry.emplace    <BoidID>  (entity, boidIdComp);

        if (i == 0) auto& boidSprite = _registry.emplace    <BoidSprite>(entity, sharkHeadTexture);
        else if (i == 1) auto& boidSprite = _registry.emplace    <BoidSprite>(entity, sharkMidTexture);
        else if (i == 2) auto& boidSprite = _registry.emplace    <BoidSprite>(entity, sharkEndTexture);
        else auto& boidSprite = _registry.emplace    <BoidSprite>    (entity, smallFishTexture);
    }  

    

}

void Game::Update(float deltaTime) {
    _boidGrid.clear();
    _boidGrid.resize(24);

    auto view = _registry.view<Position, CellID, Neighbours, ForwardDirection, NeiDirection>();
    for (auto [entity, pos, cid, neighs, fDir, nDir] : view.each()) {
        int newCellID = int(pos.position.x / 200) + int(pos.position.y / 200) * 6;

        if (newCellID < 0) newCellID = 0;
        if (newCellID >= 24) newCellID = 23;

        _registry.patch<CellID>(entity, [newCellID](auto& c) { c.id = newCellID; });

        _boidGrid[newCellID].push_back({ pos.position, fDir.forwardDirection });
    }

    for (auto [entity, pos, cid, neighs, fDir, nDir] : view.each()) {
        neighs.pos.clear();
        nDir.dir.clear();
        auto neighbourCells = getNeighbours(cid.id);

        for (int n : neighbourCells) {
            for (auto& otherEntity : _boidGrid[n]) {
                if (otherEntity.pos.x == pos.position.x && otherEntity.pos.y == pos.position.y)
                    continue;
                float distSq = getDistanceSq(pos.position, otherEntity.pos);
                if (distSq < _boidCoherence * _boidCoherence) {
                    neighs.pos.push_back(otherEntity.pos);
                    nDir.dir.push_back(otherEntity.dir);
                }                    
            }
        }
    }
    

    auto viewMovement = _registry.view<Position, ForwardDirection, Rotation, Neighbours, NeiDirection, BoidSpeed, BoidID>();
    viewMovement.each
    ([&]
    (   auto entity, 
        Position& pos, 
        ForwardDirection& fDir, 
        Rotation& rot, 
        Neighbours& neighs, 
        NeiDirection& nDir, 
        BoidSpeed& speed, 
        BoidID& bid) {
        if (bid.id == 3) {
            Vector2f newDirection = fDir.forwardDirection;


            if (neighs.pos.size() > 0) {
                float dist = 0, minDist = numeric_limits<float>::max();
                Vector2f tempPos{ 0,0 }, tempDir{ 0,0 }, medianPoint{ 0,0 }, medianDir{ 0,0 }, closest{ 0,0 };

                for (size_t i = 0; i < neighs.pos.size(); i++) {
                    Vector2f otherPos = neighs.pos[i];
                    tempPos += otherPos;

                    float dist = getDistance(pos.position, otherPos);
                    if (dist < minDist) {
                        minDist = dist;
                        closest = otherPos;
                    }
                }

                medianPoint = { tempPos.x / neighs.pos.size(), tempPos.y / neighs.pos.size() };

                for (Vector2f otherDir : nDir.dir) tempDir += otherDir;
                medianDir = normalize(tempDir);


                if (minDist < _boidSeperation) newDirection = getDirectionOpposite(pos.position, closest);
                else if (minDist > _boidSeperation + _boidCushion) newDirection = getDirectionTo(pos.position, medianPoint);
                else newDirection = medianDir;
            }

            Vector2f borderAvoid{ 0.f, 0.f };
            const float borderForce = 50.f;

            if (pos.position.x < _borderSize) {
                borderAvoid += {1.f, 0.f};
                pos.position.x += borderForce * deltaTime;
            }
            if (pos.position.x > _windowWidth - _borderSize) {
                borderAvoid += {-1.f, 0.f};
                pos.position.x -= borderForce * deltaTime;
            }
            if (pos.position.y < _borderSize) {
                borderAvoid += {0.f, 1.f};
                pos.position.y += borderForce * deltaTime;
            }
            if (pos.position.y > _windowHeight - _borderSize) {
                borderAvoid += {0.f, -1.f};
                pos.position.y -= borderForce * deltaTime;
            }

            if (borderAvoid.x != 0.f || borderAvoid.y != 0.f) {
                newDirection = normalize(borderAvoid);
            }



            if (getDistance(pos.position, _boidAvoidance) < 150) newDirection = getDirectionOpposite(pos.position, _boidAvoidance);

            float steerSpeed = _boidSteerSpeed * deltaTime;
            fDir.forwardDirection = normalize(
                fDir.forwardDirection + (newDirection - fDir.forwardDirection) * steerSpeed
            );


            pos.position += fDir.forwardDirection * speed.speed * deltaTime;

            float angleDeg = float(atan2(fDir.forwardDirection.y, fDir.forwardDirection.x) * (180.0f / numbers::pi));
            rot.angle = degrees(angleDeg + 90.0f);
        }
        if(bid.id == 0) {
            _boidAvoidance = pos.position;
            Vector2f newDirection;
            Vector2i mousePos = sf::Mouse::getPosition(_window);


            newDirection = getDirectionTo(pos.position, Vector2f(float(mousePos.x), float(mousePos.y)));
            
            temp += 0.3;
            float power = 0.8f;
            
            if (newDirection.y <= 0) newDirection.x += float(sin(temp)) * power;
            if (newDirection.y > 0) newDirection.x -= float(sin(temp)) * power;
            if (newDirection.x <= 0) newDirection.y += float(sin(temp)) * power;
            if (newDirection.x > 0) newDirection.y -= float(sin(temp)) * power;

            //printf("new  %f, %f\n", newDirection.x, newDirection.y);
            //printf("fDir %f, %f\n", fDir.forwardDirection.x, fDir.forwardDirection.y);


            float steerSpeed = _boidSteerSpeed * deltaTime;
            fDir.forwardDirection = normalize(
                fDir.forwardDirection + (newDirection - fDir.forwardDirection) * steerSpeed
            );

            if (Mouse::isButtonPressed(Mouse::Button::Left)) speed.speed = _boidSpeed * 2;
            else speed.speed = _boidSpeed;
            

            pos.position += fDir.forwardDirection * speed.speed * deltaTime;


            float angleDeg = float(atan2(fDir.forwardDirection.y, fDir.forwardDirection.x) * (180.0f / numbers::pi));
            rot.angle = degrees(angleDeg + 90.0f); 
            _sharkTail = { pos.position.x - (30 * fDir.forwardDirection.x), pos.position.y - (30 * fDir.forwardDirection.y) };
        }
        if (bid.id == 1) {
            Vector2f newDirection;

            newDirection = getDirectionTo(pos.position, _sharkTail);


            float steerSpeed = 20 * deltaTime;
            fDir.forwardDirection = normalize(
                fDir.forwardDirection + (newDirection - fDir.forwardDirection) * steerSpeed
            );

            if (Mouse::isButtonPressed(Mouse::Button::Left)) speed.speed = _boidSpeed * 2;
            else speed.speed = _boidSpeed;

            pos.position = { _sharkTail.x - (fDir.forwardDirection.x), _sharkTail.y + (fDir.forwardDirection.y) };//fDir.forwardDirection * speed.speed * deltaTime;


            float angleDeg = float(atan2(fDir.forwardDirection.y, fDir.forwardDirection.x) * (180.0f / numbers::pi));
            rot.angle = degrees(angleDeg + 90.0f);
            _sharkTailTail = { pos.position.x - (30 * fDir.forwardDirection.x), pos.position.y - (30 * fDir.forwardDirection.y) };

        }
        if (bid.id == 2) {
            Vector2f newDirection;

            newDirection = getDirectionTo(pos.position, _sharkTailTail);


            float steerSpeed = 15 * deltaTime;
            fDir.forwardDirection = normalize(
                fDir.forwardDirection + (newDirection - fDir.forwardDirection) * steerSpeed
            );

            if (Mouse::isButtonPressed(Mouse::Button::Left)) speed.speed = _boidSpeed * 2;
            else speed.speed = _boidSpeed;

            pos.position = { _sharkTailTail.x - (fDir.forwardDirection.x), _sharkTailTail.y + (fDir.forwardDirection.y) };//fDir.forwardDirection * speed.speed * deltaTime;


            float angleDeg = float(atan2(fDir.forwardDirection.y, fDir.forwardDirection.x) * (180.0f / numbers::pi));
            rot.angle = degrees(angleDeg + 90.0f);
        }
        
    });

}    

void Game::Render() {    

    auto view = _registry.view<Position, Rotation, BoidSprite, BoidID>();
    size_t entityCount = view.size_hint();

    view.each([&](auto entity, Position& pos, Rotation& rot, BoidSprite& bs, BoidID& bid ) {      
        
        if (bid.id == 0) {
            bs.sprite.setPosition(pos.position);
            bs.sprite.setRotation(rot.angle);
            //bs.sprite.setScale({ 2, 2 });
            //bs.sprite.setColor({ 150,150,0,255 });

            FloatRect bounds = bs.sprite.getLocalBounds();
            bs.sprite.setOrigin({ bounds.size.x / 2, bounds.size.y / 2 });

            _window.draw(bs.sprite);
        }
        if (bid.id == 1) {
            bs.sprite.setPosition(pos.position);
            bs.sprite.setRotation(rot.angle);
            bs.sprite.setScale({ 0.9, 1 });
            //bs.sprite.setColor({ 255,255,255,255 });

            FloatRect bounds = bs.sprite.getLocalBounds();
            bs.sprite.setOrigin({ bounds.size.x / 2, bounds.size.y / 3 });

            _window.draw(bs.sprite);
        }
        if (bid.id == 2) {
            bs.sprite.setPosition(pos.position);
            bs.sprite.setRotation(rot.angle);
            bs.sprite.setScale({ 0.6, 1 });
            //bs.sprite.setColor({ 255,255,255,255 });

            FloatRect bounds = bs.sprite.getLocalBounds();
            bs.sprite.setOrigin({ bounds.size.x / 2, bounds.size.y / 2 });

            _window.draw(bs.sprite);
        }
        if (bid.id == 3) {
            bs.sprite.setPosition(pos.position);
            bs.sprite.setRotation(rot.angle);
            bs.sprite.setScale({ 0.5, 0.5 });

            FloatRect bounds = bs.sprite.getLocalBounds();
            bs.sprite.setOrigin({ bounds.size.x / 2, bounds.size.y / 2 });

            _window.draw(bs.sprite);
        }
        
        });

    // Debug rendering - only draw for a subset
    if (isDebug) {
        auto viewPos = _registry.view<Position, ForwardDirection, Neighbours>();
        int debugCount = 0;
        const int maxDebugDraw = 1000; // Only draw debug info for first 1000

        for (auto [entity, pos, fDir, neighs] : viewPos.each()) {
            if (debugCount++ >= maxDebugDraw) break;

            CircleShape dot(2);
                dot.setOrigin({ 2, 2 });
                dot.setPosition(pos.position);
                dot.setFillColor(Color::Red);

            Vector2f forwardPosition = pos.position + fDir.forwardDirection * 25.f;

            CircleShape frontDot(2);
                frontDot.setOrigin({ 2, 2 });
                frontDot.setPosition(forwardPosition);
                frontDot.setFillColor(Color::Red);

            _window.draw(dot);
            _window.draw(frontDot);
            Vector2f temp{ 0,0 }, medianPoint{0,0};
            for (Vector2f otherPos : neighs.pos) {
                temp += otherPos;
                //drawLine(pos.position, otherPos);
            }
            medianPoint = { temp.x / neighs.pos.size(), temp.y / neighs.pos.size() };
            drawLine(pos.position, medianPoint);
            
        }
    }


}


void Game::HandleInput() {

}

Vector2f Game::normalize(const Vector2f& v) {
    float len = std::sqrt(v.x * v.x + v.y * v.y);
    if (len == 0) return sf::Vector2f(0.f, 0.f);
    return sf::Vector2f(v.x / len, v.y / len); }

Vector2f Game::getDirectionOpposite(const Vector2f& start, const Vector2f& target) {
    sf::Vector2f diff = start - target; 
    return normalize(diff); }

float Game::getDistance(const Vector2f& a, const Vector2f& b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy); }

float Game::getDistanceSq(const Vector2f& a, const Vector2f& b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return dx * dx + dy * dy; }

void Game::drawLine(const Vector2f& p1, const Vector2f& p2)
{
    sf::Vertex line[] =
    {
        sf::Vertex(p1, {0,255,0,255}),
        sf::Vertex(p2, {0,255,0,255})
    };
    _window.draw(line, 2, PrimitiveType::Lines);
}

vector<int> Game::getNeighbours(int index)
{
    std::vector<int> neighbors;

    int r = index / 6;
    int c = index % 6;

    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue; // skip self

            int nr = r + dr;
            int nc = c + dc;

            if (nr >= 0 && nr < 4 && nc >= 0 && nc < 6) {
                neighbors.push_back(nr * 4 + nc);
            }
        }
    }

    return neighbors;
}

Vector2f Game::getDirectionTo(const Vector2f& start, const Vector2f& target) {
    sf::Vector2f diff = target - start;
    return normalize(diff); }

Game::Game() {
    _window.create(VideoMode({ _windowWidth, _windowHeight }), "Boids Simulation");
    _window.setFramerateLimit(30); 
    if (!smallFishTexture.loadFromMemory(SmallFish_png, SmallFish_png_len)) {
        std::cerr << "Failed to load smallFishTexture!" << std::endl;
        _window.close();        
    }
    if (!sharkHeadTexture.loadFromMemory(Sharke_Head_png, Sharke_Head_png_len)) {
        std::cerr << "Failed to load sharkHeadTexture!" << std::endl;
        _window.close();
    }
    if (!sharkMidTexture.loadFromMemory(Sharke_Mid_png, Sharke_Mid_png_len)) {
        std::cerr << "Failed to load sharkMidTexture!" << std::endl;
        _window.close();
    }
    if (!sharkEndTexture.loadFromMemory(Sharke_End_png, Sharke_End_png_len)) {
        std::cerr << "Failed to load sharkEndTexture!" << std::endl;
        _window.close();
    }
}

void Game::Run() {   
    Start();
    while (_window.isOpen())  {  
        while (const std::optional event = _window.pollEvent()) { 
            if (event->is<sf::Event::Closed>()) _window.close();   
        }
        _dt = _deltaClock.restart();
        _deltaTime = _dt.asSeconds();

        Update(_deltaTime);

        _window.clear(sf::Color(10, 30, 60));
        Render();
        _window.display(); 
    }
}