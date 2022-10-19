#include <algorithm>
#include "camera.h"

void TA_Camera::setFollowPosition(TA_Point *newFollowPosition)
{
    followPosition = newFollowPosition;
    position = *followPosition;
}

void TA_Camera::update()
{
    auto move = [&] (double current, double need) {
        if(current < need) {
            current = std::min(need, current + movementSpeed);
        }
        else {
            current = std::max(need, current - movementSpeed);
        }
        return current;
    };

    position.x = move(position.x, followPosition->x);
    position.y = move(position.y, followPosition->y);

    auto normalize = [&] (double current, double left, double right) {
        current = std::max(current, left);
        current = std::min(current, right);
        return current;
    };

    position.x = normalize(position.x, border.topLeft.x, border.bottomRight.x);
    position.y = normalize(position.y, border.topLeft.y, border.bottomRight.y);
}