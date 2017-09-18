#ifndef _BUFFERED_METRIC_H
#define _BUFFERED_METRIC_H

class BufferedMetric {
public:
    BufferedMetric(float minValueOffset);

    // Update the value if it matches the constructor constraints
    bool updateValue(float value);
    float getValue();

private:
    float minValueOffset;
    float value;
};

#endif