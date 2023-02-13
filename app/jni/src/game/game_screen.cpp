#include "game_screen.h"

void TA_GameScreen::init()
{
    links.character = &character;
    links.tilemap = &tilemap;
    links.camera = &camera;
    links.objectSet = &objectSet;

    character.load(links);
    tilemap.load("maps/pf/pf1.tmx");
    tilemap.setCamera(&camera);
    objectSet.setCollisionTilemap(&tilemap);
    hud.load(links);
    objectSet.setCamera(&camera);
    objectSet.setCharacterHitbox(character.getHitbox());

    objectSet.spawnBreakableBlock(TA_Point(1120, 256));
    objectSet.spawnBreakableBlock(TA_Point(1120, 272));
    objectSet.spawnBreakableBlock(TA_Point(1120, 288));
    objectSet.spawnBreakableBlock(TA_Point(1232, 336));
    objectSet.spawnBreakableBlock(TA_Point(1248, 336));
    objectSet.spawnBreakableBlock(TA_Point(1296, 336));
    objectSet.spawnBreakableBlock(TA_Point(1312, 336));
    objectSet.spawnBreakableBlock(TA_Point(1264, 384), true);
    objectSet.spawnBreakableBlock(TA_Point(1280, 384), true);
    objectSet.spawnWalker(TA_Point(544, 272), 40, true);
    objectSet.spawnWalker(TA_Point(1072, 272), 90, true);
    objectSet.spawnWalker(TA_Point(1216, 128), 60, true);
    objectSet.spawnWalker(TA_Point(1216, 32), 60, true);
    objectSet.spawnWalker(TA_Point(1708, 144), 0, false);
    objectSet.spawnWalker(TA_Point(1804, 256), 0, true);
    objectSet.spawnHoverPod(TA_Point(384, 160), 272, true);
    objectSet.spawnHoverPod(TA_Point(752, 144), 208, true);
    objectSet.spawnHoverPod(TA_Point(1520, 48), 208, true);
}

TA_ScreenState TA_GameScreen::update()
{
    character.handleInput();
    objectSet.update();
    character.update();
    camera.update(character.isOnGround());

    tilemap.draw(0);
    tilemap.draw(1);
    objectSet.draw(0);
    character.draw();
    objectSet.draw(1);
    tilemap.draw(2);
    character.drawControls();
    hud.draw();

    if(character.gameOver()) {
        return TA_SCREENSTATE_TITLE;
    }
    return TA_SCREENSTATE_CURRENT;
}

void TA_GameScreen::quit()
{

}
