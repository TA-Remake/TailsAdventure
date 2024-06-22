#include "mecha_golem.h"
#include "mecha_golem_bomb.h"
#include "explosion.h"
#include <algorithm>

void TA_MechaGolem::load()
{
    headSprite.load("objects/mecha_golem/head.png", 24, 32);
    headFlashSprite.load("objects/mecha_golem/head.png", 24, 32);
    bodySprite.load("objects/mecha_golem/body.png");
    leftFootSprite.load("objects/mecha_golem/feet.png", 16, 11);
    rightFootSprite.load("objects/mecha_golem/feet.png", 16, 11);
    rightFootSprite.setFrame(1);
    armSprite.load("objects/mecha_golem/arm.png", 16, 16);
    armPartSprite.load("objects/mecha_golem/arm_part.png");

    headSprite.loadAnimationsFromFile("objects/mecha_golem/head.xml");
    armSprite.loadAnimationsFromFile("objects/mecha_golem/arm.xml");

    TA_Camera* camera = objectSet->getLinks().camera;
    headSprite.setCamera(camera);
    headFlashSprite.setCamera(camera);
    bodySprite.setCamera(camera);
    leftFootSprite.setCamera(camera);
    rightFootSprite.setCamera(camera);
    armSprite.setCamera(camera);
    armPartSprite.setCamera(camera);

    hitSound.load("sound/hit.ogg", TA_SOUND_CHANNEL_SFX3);

    position = {double(128 + TA::screenWidth - 61), 112};
    objectSet->getLinks().camera->setLockPosition({128, 0});
    hitboxVector.assign(HITBOX_MAX, HitboxVectorElement());
}

bool TA_MechaGolem::update()
{
    switch(state) {
        case STATE_IDLE:
            updateIdle();
            break;
        case STATE_WAIT:
            updateWait();
            break;
        case STATE_GO_LEFT:
        case STATE_GO_RIGHT:
            updateGo();
            break;
        case STATE_ARM_MOVE:
            updateArmMove();
            break;
        case STATE_ARM_MOVE_BACK:
            updateArmMoveBack();
            break;
        case STATE_PHASE_CHANGE:
            updatePhaseChange();
            break;
        case STATE_ARM_BITE1:
            updateArmBite1();
            break;
        case STATE_ARM_BITE2:
            updateArmBite2();
            break;
        case STATE_ARM_BITE3:
            updateArmBite3();
            break;
        case STATE_ARM_BITE4:
            updateArmBite4();
            break;
        default:
            break;
    }

    if(state != STATE_ARM_MOVE && state != STATE_ARM_MOVE_BACK && state != STATE_ARM_CIRCLE &&
        state != STATE_ARM_BITE1 && state != STATE_ARM_BITE2 && state != STATE_ARM_BITE3 && state != STATE_ARM_BITE4) {
        armPosition = position + TA_Point(-11, -42);
        armSprite.setAnimation("idle");
    }

    if(!secondPhase) {
        headSprite.setAnimation("idle");
    }

    updateDamage();
    updateHitboxes();
    return true;
}

void TA_MechaGolem::updateIdle()
{
    if(objectSet->getLinks().camera->isLocked()) {
        state = STATE_WAIT;
        objectSet->playBossMusic();
    }
}

void TA_MechaGolem::updateWait()
{
    if(health <= 8 && !secondPhase) {
        initPhaseChange();
        return;
    }

    timer += TA::elapsedTime;
    if(timer < waitTime) {
        return;
    }

    // TODO: check point to line distance here
    if(!secondPhase) {
        double distance = objectSet->getCharacterPosition().getDistance(position + TA_Point(-11, -42));
        if(distance < armMoveMaxDistance) {
            if(TA::random::next() % 3 == 0) {
                initGo();
            }
            else {
                initArmMove();
            }
        }
        else {
            initGo();
        }
    }
    else {
        if(TA::random::next() % 2 == 0) {
            initGo();
        }
        else {
            timer = 0;
            state = STATE_ARM_BITE1;
        }
    }
}

void TA_MechaGolem::initGo()
{
    timer = 0;
    startX = position.x;
    double cameraX = objectSet->getLinks().camera->getPosition().x;
    
    if(position.x < cameraX + goBorder) {
        state = STATE_GO_RIGHT;
    }
    else if(position.x > cameraX + TA::screenWidth - goBorder - bodySprite.getWidth()) {
        state = STATE_GO_LEFT;
    }
    else if(position.x + bodySprite.getWidth() / 2 < objectSet->getCharacterPosition().x) {
        state = (TA::random::next() % 4 == 0 ? STATE_GO_LEFT : STATE_GO_RIGHT);
    }
    else {
        state = (TA::random::next() % 4 == 0 ? STATE_GO_RIGHT : STATE_GO_LEFT);
    }

    if(state == STATE_GO_RIGHT) {
        headSprite.setAnimation("turn_back");
    }
}

void TA_MechaGolem::updateGo()
{
    int direction = (state == STATE_GO_LEFT ? -1 : 1);
    timer += TA::elapsedTime;

    if(timer > goTime) {
        if(state == STATE_GO_RIGHT) {
            headSprite.setAnimation("turn_forward");
        }
        position.x = startX + goDistance * direction;
        timer = 0;
        state = STATE_WAIT;
        return;
    }

    position.x = startX + goDistance * (timer / goTime) * direction;

    if(timer < goTime / 2) {
        double angle = (timer / (goTime / 2)) * TA::pi;
        double x = startX + (-cos(angle) + 1) / 2 * goDistance * direction;
        double y = sin(angle) * stepHeight;

        if(direction == -1) {
            leftFootSprite.setPosition(x + 5, position.y - 10 - y);
            rightFootSprite.setPosition(startX + 26, position.y - 10);
        }
        else {
            leftFootSprite.setPosition(startX + 5, position.y - 10);
            rightFootSprite.setPosition(x + 26, position.y - 10 - y);
        }
    }
    else {
        double angle = ((timer - goTime / 2) / (goTime / 2)) * TA::pi;
        double x = startX + (-cos(angle) + 1) / 2 * goDistance * direction;
        double y = sin(angle) * stepHeight;

        if(direction == -1) {
            leftFootSprite.setPosition((startX + goDistance * direction) + 5, position.y - 10);
            rightFootSprite.setPosition(x + 26, position.y - 10 - y);
        }
        else {
            leftFootSprite.setPosition(x + 5, position.y - 10 - y);
            rightFootSprite.setPosition((startX + goDistance * direction) + 26, position.y - 10);
        }
    }
}

void TA_MechaGolem::initArmMove()
{
    timer = 0;
    armTarget = getOptimalArmTarget();
    state = STATE_ARM_MOVE;
    armSprite.setAnimation("attack");
}

void TA_MechaGolem::updateArmMove()
{
    timer += TA::elapsedTime;
    if(timer > armMoveTime) {
        timer = 0;
        state = STATE_ARM_MOVE_BACK;
        return;
    }

    TA_Point startPosition = position + TA_Point(-11, -42);
    armPosition = startPosition + (armTarget - startPosition) * (timer / armMoveTime);
}

void TA_MechaGolem::updateArmMoveBack()
{
    timer += TA::elapsedTime;
    if(timer > armMoveTime) {
        timer = 0;
        state = STATE_WAIT;
        return;
    }

    TA_Point startPosition = position + TA_Point(-11, -42);
    armPosition = armTarget + (startPosition - armTarget) * (timer / armMoveTime);
}

TA_Point TA_MechaGolem::getOptimalArmTarget()
{
    std::vector<TA_Point> targets;
    targets.push_back(position + TA_Point(-11, -82));
    targets.push_back(position + TA_Point(-51, -42));
    targets.push_back(position + TA_Point(-43, -74));

    std::sort(targets.begin(), targets.end(), [&] (const auto &first, const auto &second) {
        TA_Point characterPosition = objectSet->getCharacterPosition();
        return (characterPosition - first).length() < (characterPosition - second).length();
    });

    return targets[0];
}

void TA_MechaGolem::initPhaseChange()
{
    timer = 0;
    state = STATE_PHASE_CHANGE;
    secondPhase = true;

    for(int delay = 0; delay < phaseChangeTime * 2 / 3; delay += phaseChangeExplosionInterval) {
        TA_Point explosionPosition = position + TA_Point(5, -56);
        explosionPosition.x += TA::random::next() % 8;
        explosionPosition.y += TA::random::next() % 16;
        objectSet->spawnObject<TA_Explosion>(explosionPosition, delay, TA_EXPLOSION_NEUTRAL);
    }
}

void TA_MechaGolem::updatePhaseChange()
{
    timer += TA::elapsedTime;
    if(timer > phaseChangeTime) {
        timer = 0;
        headSprite.setAnimation("idle2");
        state = STATE_WAIT;
        return;
    }

    if(timer > phaseChangeTime / 3) {
        headSprite.setAnimation("laugh");
    }
}

void TA_MechaGolem::updateArmBite1()
{
    timer += TA::elapsedTime;
    if(timer > armBite1Time) {
        timer = 0;
        state = STATE_ARM_BITE2;
        return;
    }

    armSprite.setAnimation("attack");
    TA_Point startPosition = position + TA_Point(-11, -42);
    TA_Point endPosition = position + TA_Point(-11, -82);
    armPosition = startPosition + (endPosition - startPosition) * (timer / armBite1Time);
}

void TA_MechaGolem::updateArmBite2()
{
    TA_Point startPosition = position + TA_Point(-11, -82);
    TA_Point endPosition = position + TA_Point(-32, -15);
    timer += TA::elapsedTime;

    if(timer > armBite2Time) {
        timer = 0;
        state = STATE_ARM_BITE3;
        armPosition = endPosition;
        objectSet->getLinks().camera->shake(24);

        TA_Point bombPosition;
        bombPosition.x = objectSet->getCharacterPosition().x - 8;
        bombPosition.y = -34;
        objectSet->spawnObject<TA_MechaGolemBomb>(bombPosition);

        return;
    }

    armSprite.setAnimation("bite");
    double angle = TA::pi / 2 + (timer / armBite2Time) * (timer / armBite2Time) * (TA::pi / 2);
    armPosition.x = startPosition.x + (endPosition.x - startPosition.x) * (-cos(angle));
    armPosition.y = endPosition.y + (endPosition.y - startPosition.y) * (-sin(angle));
}

void TA_MechaGolem::updateArmBite3()
{
    timer += TA::elapsedTime;
    if(timer > armBite3Time) {
        timer = 0;
        state = STATE_ARM_BITE4;
    }
}

void TA_MechaGolem::updateArmBite4()
{
    timer += TA::elapsedTime;
    if(timer > armBite4Time) {
        timer = 0;
        state = STATE_WAIT;
    }

    TA_Point startPosition = position + TA_Point(-32, -15);
    TA_Point endPosition = position + TA_Point(-11, -42);
    armPosition = startPosition + (endPosition - startPosition) * (timer / armBite4Time);
}

void TA_MechaGolem::updateDamage()
{
    if(invincibleTimer <= invincibleTime) {
        invincibleTimer += TA::elapsedTime;
        return;
    }

    if((objectSet->checkCollision(hitboxVector[HITBOX_WEAK].hitbox) & (TA_COLLISION_EXPLOSION_FIRST | TA_COLLISION_HAMMER)) == 0) {
        return;
    }

    hitSound.play();
    invincibleTimer = 0;
    health --;
}

void TA_MechaGolem::updateHitboxes()
{
    // TODO: add hitbox to block bombs from back
    hitboxVector[HITBOX_WALL_LEFT].hitbox.setRectangle(TA_Point(112, 0), TA_Point(128, 112));
    hitboxVector[HITBOX_WALL_RIGHT].hitbox.setRectangle(TA_Point(128 + TA::screenWidth, 0), TA_Point(144 + TA::screenWidth, 112));
    hitboxVector[HITBOX_WALL_LEFT].collisionType = hitboxVector[HITBOX_WALL_RIGHT].collisionType = (state == STATE_IDLE ? TA_COLLISION_TRANSPARENT : TA_COLLISION_SOLID);

    hitboxVector[HITBOX_BODY].hitbox.setRectangle(position + TA_Point(5, -44), position + TA_Point(52, -16));
    hitboxVector[HITBOX_WEAK].hitbox.setRectangle(position + TA_Point(8, -55), position + TA_Point(24, -42));
    hitboxVector[HITBOX_BODY].collisionType = hitboxVector[HITBOX_WEAK].collisionType = TA_COLLISION_DAMAGE;

    hitboxVector[HITBOX_ARM].hitbox.setRectangle(armPosition + TA_Point(4, 4), armPosition + TA_Point(12, 12));
    hitboxVector[HITBOX_ARM].collisionType = TA_COLLISION_DAMAGE;
}

void TA_MechaGolem::draw()
{
    headSprite.setPosition(position + TA_Point(5, -56));
    bodySprite.setPosition(position + TA_Point(0, -57));

    if(state != STATE_GO_LEFT && state != STATE_GO_RIGHT) {
        leftFootSprite.setPosition(position + TA_Point(5, -10));
        rightFootSprite.setPosition(position + TA_Point(26, -10));
    }

    headFlashSprite.setPosition(headSprite.getPosition());
    headFlashSprite.setFrame(headSprite.getCurrentFrame() + 5);

    bodySprite.draw();
    headSprite.draw();
    leftFootSprite.draw();
    rightFootSprite.draw();
    drawArm();

    if(invincibleTimer < damageFlashTime * 4 && int(invincibleTimer / damageFlashTime) % 2 == 0) {
        headFlashSprite.draw();
    }
}

void TA_MechaGolem::drawArm()
{
    armSprite.setPosition(armPosition);
    armSprite.draw();

    TA_Point startAdd = TA_Point(2, -26);
    TA_Point endAdd = (armPosition - position) + TA_Point(4, 8);

    for(double part : {0.25, 0.5, 0.75}) {
        TA_Point currentAdd = startAdd + (endAdd - startAdd) * part;
        currentAdd.x = round(currentAdd.x);
        currentAdd.y = round(currentAdd.y);
        armPartSprite.setPosition(position + currentAdd);
        armPartSprite.draw();
    }
}
