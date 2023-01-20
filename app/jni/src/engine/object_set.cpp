#include "object_set.h"
#include "objects/explosion.h"
#include "objects/bomb.h"
#include "objects/breakable_block.h"
#include "objects/particle.h"
#include "objects/ring.h"
#include "objects/walker.h"
#include "engine/error.h"

TA_Object::TA_Object(TA_ObjectSet *newObjectSet)
{
    objectSet = newObjectSet;
    setCamera(objectSet->getCamera());
}

void TA_ObjectSet::update()
{
    for(TA_Object *currentObject : deleteList) {
        delete currentObject;
    }
    for(TA_Object *currentObject : spawnedObjects) {
        objects.push_back(currentObject);
    }
    deleteList.clear();
    spawnedObjects.clear();

    std::vector<TA_Object*> newObjects;
    for(TA_Object *currentObject : objects) {
        if(currentObject->update()) {
            newObjects.push_back(currentObject);
        }
        else {
            deleteList.push_back(currentObject);
        }
    }
    objects = newObjects;
}

void TA_ObjectSet::draw(int priority)
{
    for(TA_Object *currentObject : objects) {
        if(currentObject->getDrawPriority() == priority) {
            currentObject->draw();
        }
    }
}

void TA_ObjectSet::checkCollision(TA_Polygon &hitbox, int &flags)
{
    flags = 0;
    tilemap->checkCollision(hitbox, flags);
    for(TA_Object *currentObject : objects) {
        if(currentObject->checkCollision(hitbox)) {
            flags |= currentObject->getCollisionType();
        }
    }
    for(TA_Object *currentObject : deleteList) {
        if(currentObject->checkCollision(hitbox)) {
            flags |= currentObject->getCollisionType();
        }
    }
    if(characterHitbox->intersects(hitbox)) {
        flags |= TA_COLLISION_CHARACTER;
    }
}

void TA_ObjectSet::spawnObject(TA_Object *object)
{
    spawnedObjects.push_back(object);
}

void TA_ObjectSet::spawnExplosion(TA_Point position, int delay)
{
    auto *explosion = new TA_Explosion(this);
    explosion->load(position, delay);
    spawnObject(explosion);
}

void TA_ObjectSet::spawnBomb(TA_Point position, bool direction, TA_BombMode mode)
{
    auto *bomb = new TA_Bomb(this);
    bomb->load(position, direction, mode);
    spawnObject(bomb);
}

void TA_ObjectSet::spawnBreakableBlock(TA_Point position, bool dropsRing)
{
    auto *block = new TA_BreakableBlock(this);
    block->load(position, dropsRing);
    spawnObject(block);
}

void TA_ObjectSet::spawnParticle(std::string filename, TA_Point position, TA_Point velocity, TA_Point delta)
{
    auto *particle = new TA_Particle(this);
    particle->load(filename, position, velocity, delta);
    spawnObject(particle);
}

void TA_ObjectSet::spawnRing(TA_Point position)
{
    auto *ring = new TA_Ring(this);
    ring->load(position);
    spawnObject(ring);
}

void TA_ObjectSet::spawnWalker(TA_Point position, int range, bool flip)
{
    auto *walker = new TA_Walker(this);
    walker->load(position, range, flip);
    spawnObject(walker);
}

void TA_ObjectSet::spawnWalkerBullet(TA_Point position, bool direction)
{
    auto *walkerBullet = new TA_WalkerBullet(this);
    walkerBullet->load(position, direction);
    spawnObject(walkerBullet);
}

TA_ObjectSet::~TA_ObjectSet()
{
    for(TA_Object *currentObject : objects) {
        delete currentObject;
    }
}
