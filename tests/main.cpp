#include <av/glfw/app.hpp>

using namespace av;

int main(int argc, char *argv[]) {
    GLFW_App({
        .window = {
            .title = "AVocado Testing Site",
            .width = 800, .height = 600
        }
    });
}
