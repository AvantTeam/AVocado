#include <av/glfw/app.hpp>

using namespace av;

int main(int argc, char *argv[]) {
    return app_create({
        .init = []() {
            
        },
        
        .dispose = []() {
            
        },
        
        .render = []() {
            
        },
        
        .window = {
            .title = "AVocado Testing Site",
            .width = 800, .height = 600
        }
    });
}
