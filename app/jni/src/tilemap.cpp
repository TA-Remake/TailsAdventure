#include <algorithm>
#include <sstream>
#include <SDL.h>
#include "tilemap.h"
#include "tinyxml2.h"
#include "error.h"
#include "tools.h"
#include "globals.h"
#include "character.h"

void TA_Tilemap::load(std::string filename)
{
    tinyxml2::XMLDocument xmlFile;
    xmlFile.Parse(readStringFromFile(filename).c_str());
    tinyxml2::XMLElement *xmlRoot = xmlFile.FirstChildElement("map");

    width = xmlRoot->IntAttribute("width");
    height = xmlRoot->IntAttribute("height");
    layerCount = xmlRoot->IntAttribute("nextlayerid") - 1;
    tileWidth = xmlRoot->FirstChildElement("tileset")->IntAttribute("tilewidth");
    tileHeight = xmlRoot->FirstChildElement("tileset")->IntAttribute("tileheight");
    int tileCount = xmlRoot->FirstChildElement("tileset")->IntAttribute("tilecount");

    tilemap.resize(layerCount);
    for(int layer = 0; layer < layerCount; layer ++) {
        tilemap[layer].resize(width);
        for(int pos = 0; pos < width; pos ++) {
            tilemap[layer][pos].resize(height);
        }
    }
    tileset.assign(tileCount, TA_Tile());

    for(int pos = 0; pos < tileCount; pos ++) {
        tileset[pos].animation = std::vector<int>{pos};
        tileset[pos].animationDelay = 1;
    }

    auto loadTileset = [&](tinyxml2::XMLElement *tilesetElement)
    {
        std::string textureFilename = filename;
        while(!textureFilename.empty() && textureFilename.back() != '/') {
            textureFilename.pop_back();
        }
        textureFilename += tilesetElement->FirstChildElement("image")->Attribute("source");
        texture.load(textureFilename);

        for(tinyxml2::XMLElement *tileElement = tilesetElement->FirstChildElement("tile");
            tileElement != nullptr; tileElement = tileElement->NextSiblingElement("tile"))
        {
            int tileId = tileElement->IntAttribute("id");
            if(tileElement->FirstChildElement("animation") != nullptr) {
                tileset[tileId].animation.clear();
                int delayMs = tileElement->FirstChildElement("animation")->FirstChildElement("frame")->IntAttribute("duration");
                tileset[tileId].animationDelay = int(delayMs * 60 / 1000 + 0.5);
                for(tinyxml2::XMLElement *frameElement = tileElement->FirstChildElement("animation")->FirstChildElement("frame");
                    frameElement != nullptr; frameElement = frameElement->NextSiblingElement("frame")) {
                    tileset[tileId].animation.push_back(frameElement->IntAttribute("tileid"));
                }
            }
            if(tileElement->FirstChildElement("objectgroup") != nullptr) {
                tinyxml2::XMLElement *object = tileElement->FirstChildElement("objectgroup")->FirstChildElement("object");
                std::stringstream pointStream;
                pointStream << object->FirstChildElement("polygon")->Attribute("points");
                TA_Point currentPoint, startPoint;
                startPoint.x = object->IntAttribute("x");
                startPoint.y = object->IntAttribute("y");
                char temp;
                while(pointStream >> currentPoint.x) {
                    pointStream >> temp >> currentPoint.y;
                    tileset[tileId].polygon.addVertex(currentPoint + startPoint);
                }
                if(object->FirstChildElement("properties") != nullptr) {
                    tileset[tileId].type = object->FirstChildElement("properties")->FirstChildElement("property")->IntAttribute("value");
                }
            }
        }
    };

    auto loadLayer = [&](tinyxml2::XMLElement *layerElement)
    {
        int layer = layerCount - layerElement->IntAttribute("id");
        std::stringstream mapStream;
        mapStream << layerElement->FirstChildElement("data")->GetText();
        for(int tileY = 0; tileY < height; tileY ++ ) {
            for(int tileX = 0; tileX < width; tileX ++) {
                int tile;
                char temp;
                mapStream >> tile;
                if(tileX != width - 1 || tileY != height - 1) {
                    mapStream >> temp;
                }
                tile --;
                tilemap[layer][tileX][tileY].tileIndex = tile;
                if(tile == -1) {
                    continue;
                }
                tilemap[layer][tileX][tileY].sprite.loadFromTexture(&texture, tileWidth, tileHeight);
                tilemap[layer][tileX][tileY].sprite.setPosition(tileX * tileWidth, tileY * tileHeight);
                tilemap[layer][tileX][tileY].sprite.setAnimation(TA_Animation(tileset[tile].animation, tileset[tile].animationDelay, -1));
            }
        }
    };

    loadTileset(xmlRoot->FirstChildElement("tileset"));
    tinyxml2::XMLElement *layerElement = xmlRoot->FirstChildElement("layer");
    for(int layer = 0; layer < layerCount; layer ++) {
        loadLayer(layerElement);
        layerElement = layerElement->NextSiblingElement("layer");
    }

    borderPolygons[0].setRectangle(TA_Point(0, -16), TA_Point(width * tileWidth, 0));
    borderPolygons[1].setRectangle(TA_Point(0, height * tileHeight), TA_Point(width * tileWidth, height * tileHeight + 16));
    borderPolygons[2].setRectangle(TA_Point(-16, 0), TA_Point(0, height * tileHeight));
    borderPolygons[3].setRectangle(TA_Point(width * tileWidth, 0), TA_Point(width * tileWidth + 16, height * tileHeight));
}

void TA_Tilemap::draw(int layer)
{
    for(int tileX = 0; tileX < width; tileX ++) {
        for(int tileY = 0; tileY < height; tileY ++) {
            if(tilemap[layer][tileX][tileY].tileIndex != -1) {
                tilemap[layer][tileX][tileY].sprite.draw();
            }
        }
    }
}

void TA_Tilemap::setCamera(TA_Camera *newCamera)
{
    newCamera->setBorder({TA_Point(0, 0), TA_Point(width * tileWidth - gScreenWidth, height * tileHeight - gScreenHeight)});
    for(int layer = 0; layer < layerCount; layer ++) {
        for(int tileX = 0; tileX < width; tileX ++) {
            for(int tileY = 0; tileY < height; tileY ++) {
                if(tilemap[layer][tileX][tileY].tileIndex != -1) {
                    tilemap[layer][tileX][tileY].sprite.setCamera(newCamera);
                }
            }
        }
    }
}

void TA_Tilemap::checkCollision(TA_Polygon polygon, int layer, int &flags, int top)
{
    int minX = 1e5, maxX = 0, minY = 1e5, maxY = 0;
    for(int pos = 0; pos < polygon.size(); pos ++) {
        TA_Point vertex = polygon.getVertex(pos);
        minX = std::min(minX, int(vertex.x / tileWidth));
        maxX = std::max(maxX, int(vertex.x / tileWidth));
        minY = std::min(minY, int(vertex.y / tileHeight));
        maxY = std::max(maxY, int(vertex.y / tileHeight));
    }
    auto normalize = [&](int &value, int left, int right) {
        value = std::max(value, left);
        value = std::min(value, right);
    };
    normalize(minX, 0, width);
    normalize(maxX, 0, width);
    normalize(minY, 0, height);
    normalize(maxY, 0, height);

    for(int tileX = minX; tileX <= maxX; tileX ++) {
        for(int tileY = minY; tileY <= maxY; tileY ++) {
            int tileId = tilemap[layer][tileX][tileY].tileIndex;
            if(tileId == -1 || tileset[tileId].polygon.empty()) {
                continue;
            }
            tileset[tileId].polygon.setPosition(TA_Point(tileX * tileWidth, tileY * tileHeight));
            if(polygon.intersects(tileset[tileId].polygon)) {
                flags |= (1 << tileset[tileId].type);
                int currentTop = (tileset[tileId].polygon.isRectangle() ? top : top - 16);
                if(tileset[tileId].type != 1 || tileY * tileHeight >= currentTop) {
                    flags |= 1;
                    return;
                }
            }
        }
    }

    for(int pos = 0; pos < 4; pos ++) {
        if(borderPolygons[pos].intersects(polygon)) {
            flags |= 1;
            return;
        }
    }
}
