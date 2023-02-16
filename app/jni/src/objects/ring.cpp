#include "ring.h"
#include "engine/tools.h"
#include "engine/error.h"

void TA_Ring::load(TA_Point newPosition)
{
    position = newPosition;
    setPosition(position);
    TA_Sprite::load("objects/ring.png", 8, 8);
    TA_Sprite::loadAnimationsFromFile("objects/ring_animations.xml");
    TA_Sprite::setAnimation("ring");
    hitbox.setRectangle({0, 0}, {7, 7});
}

bool TA_Ring::checkPawnCollision(TA_Polygon &hitbox)
{
    int flags;
    objectSet->checkCollision(hitbox, flags);
    return (flags & TA_COLLISION_SOLID) || (flags & TA_COLLISION_HALF_SOLID) || (flags & TA_COLLISION_PUSHABLE);
}

bool TA_Ring::update()
{
    velocity.y += grv;
    TA_Point topLeft{0, 0}, bottomRight{8, 8};
    int flags = moveAndCollide(topLeft, bottomRight, velocity);
    setPosition(position);
    if((flags & TA_GROUND_COLLISION) && velocity.y > 0) {
        velocity.y *= -slowdown;
        if(velocity.y > -0.5) {
            velocity.y = 0;
        }
    }
    if((flags & TA_CEIL_COLLISION) && velocity.y < 0) {
        velocity.y *= -1;
    }

    hitbox.setPosition(position);
    objectSet->checkCollision(hitbox, flags);
    if(flags & TA_COLLISION_CHARACTER) {
        return false;
    }

    timer += TA::elapsedTime;
    if(timer > maxTime) {
        return false;
    }
    return true;
}