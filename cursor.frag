

#version 330 core

#define PERIOD 1.0
#define BLINK_THRESHOLD 0.5

uniform float time;
uniform float last_stroke;
out vec4 diffuseColor;
void main() {
    float t = time - last_stroke;
    float threshold = float(t < BLINK_THRESHOLD);
    float blink = mod(floor(t / PERIOD), 2);
    diffuseColor = vec4(1.0);
}
