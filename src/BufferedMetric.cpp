#include <stdlib.h>
#include "BufferedMetric.h"

BufferedMetric::BufferedMetric(float minValueOffset) {
    this->minValueOffset = minValueOffset;
}

bool BufferedMetric::updateValue(float newValue) {
    if (abs(newValue - value) > minValueOffset) {
        value = newValue;
        return true;
    }

    return false;
}

float BufferedMetric::getValue() {
    return value;
}
