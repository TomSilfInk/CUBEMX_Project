#include "distance_angle_converter.h"

int distance_to_angle(uint32_t distance) {
    int angle;
    if (distance >= 25) {
        angle = 0;
    } else if (distance <= 5) {
        angle = 180;
    } else {
        angle = 180 - ((distance - 5) * 180) / 20;
    }
    return angle;
}