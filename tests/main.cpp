#include <av/log.hpp>
#include <av/glfw/app.hpp>

using namespace av;

struct Loader {
    static constexpr bool is_GL() noexcept {
        return false;
    }
    
    static void load(Assets &assets, const Assets::AssetDesc &desc, int &asset) {
        asset = 5;
    }
};

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
