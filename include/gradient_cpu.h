#include <vector>

struct Color {
    uint8_t r, g, b, a;
    Color() : r(0), g(0), b(0), a(255) {}
    Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
};

void generateGradientCPU(std::vector<Color>& framebuffer, int width, int height) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float t = static_cast<float>(x) / static_cast<float>(width);
            uint8_t red = static_cast<uint8_t>(t * 255);
            uint8_t green = static_cast<uint8_t>((1.0f - t) * 255);
            uint8_t blue = static_cast<uint8_t>(0.5f * 255);
            framebuffer[y * width + x] = Color(red, green, blue);
        }
    }
}