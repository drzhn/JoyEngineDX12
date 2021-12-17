#ifndef TYPES_H
#define TYPES_H

#include "Utils/Assert.h"


namespace JoyEngine {
    struct Color {
        Color() {
            r = 0;
            g = 0;
            b = 0;
            a = 0;
        }

        Color(float r, float g, float b, float a) {
            ASSERT(r >= 0 && r <= 1);
            ASSERT(g >= 0 && g <= 1);
            ASSERT(b >= 0 && b <= 1);
            ASSERT(a >= 0 && a <= 1);
            this->r = r;
            this->g = g;
            this->b = b;
            this->a = a;
        }

        float r;
        float g;
        float b;
        float a;
    };
}
#endif //TYPES_H
