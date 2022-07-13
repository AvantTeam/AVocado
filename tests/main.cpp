#include <av/glfw/app.hpp>

using namespace av;

int main(int argc, char *argv[]) {
    return AV_run({
        .init = {[](void *) {
            
        }},
        
        .dispose = {[](void *) {
            
        }},
        
        .render = {[](void *) {
            
        }},
        
        .window = {
            .title = "AVocado Testing Site",
            .width = 800, .height = 600
        }
    });
}
