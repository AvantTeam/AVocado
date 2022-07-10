#include <memory>

#include <av/log.hpp>
#include <av/glfw/app.hpp>
#include <av/glfw/assets.hpp>

using namespace av;

std::unique_ptr<Assets> assets;

struct Loader {
    static constexpr bool is_GL() noexcept {
        return false;
    }
    
    static void load(Assets &assets, const AssetDesc &desc, int &asset) {
        asset = 5;
    }
};

int main(int argc, char *argv[]) {
    return GLFW_Run({
        .init = [](const GLFW_Context &context) {
            assets.reset(new Assets({
                .window = context.window
            }));
            
            assets->load<int, Loader>({
                .path   = "path/to/asset",
                .loaded = [](Assets &assets, void *asset_ptr, void *) {
                    int asset = *static_cast<int *>(asset_ptr);
                    log("The loaded asset is: %d.", asset);
                }
            });
        },
        
        .dispose = [](const GLFW_Context &context) {
            assets.reset();
        },
        
        .render = [](const GLFW_Context &context) {
            
        },
        
        .window = {
            .title = "AVocado Testing Site",
            .width = 800, .height = 600
        }
    });
}
