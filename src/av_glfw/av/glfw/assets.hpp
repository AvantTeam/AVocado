#ifndef AV_GLFW_ASSETS_HPP
#define AV_GLFW_ASSETS_HPP

#include <condition_variable>
#include <future>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <av/glfw/app.hpp>

namespace av {
    class Assets;
    
    struct AssetDesc {
        using LoadedCallback = std::add_pointer<void(Assets &, void *, void *)>::type;
        
        std::string path;
        
        void *loaded_userdata;
        LoadedCallback loaded;
    };
    
    class Assets {
        public:
        struct AssetsParam {
            GLFWwindow *window;
            int
                asset_threads = std::thread::hardware_concurrency(),
                gl_threads = 1;
        };
        
        private:
        struct AssetQueue {
            using LoadCallback = std::add_pointer<void(Assets &, const AssetDesc &, void *)>::type;
            
            AssetDesc desc;
            void *asset;
            
            LoadCallback load;
        };
        
        struct GLQueue {
            using LoadCallback = std::add_pointer<void(Assets &, const AssetDesc &, void *, void *, const GL &)>::type;
            
            AssetDesc desc;
            void *asset, *data;
            
            LoadCallback load;
        };
        
        template<typename T_Asset>
        struct AssetRegistry {
            static inline std::unordered_map<const Assets *, std::unordered_map<std::string, T_Asset>> all;
        };
        
        std::vector<std::future<void>> asset_threads, gl_threads;
        std::mutex registry_lock, asset_lock, gl_lock;
        std::condition_variable asset_cond, gl_cond;
        
        volatile bool terminate;
        std::vector<AssetQueue> asset_descs;
        std::vector<GLQueue> gl_descs;
        
        public:
        Assets():
            terminate(false),
            asset_threads(0), gl_threads(0) {}
        
        Assets(const AssetsParam &param):
            terminate(false),
            asset_threads(param.asset_threads), gl_threads(param.window ? param.gl_threads : 0) {
            for(int i = 0; i < param.asset_threads; i++) {
                asset_threads.emplace_back(std::move(std::async(std::launch::async, [this]() {
                    try {
                        while(true) {
                            AssetQueue desc;
                            
                            {
                                std::unique_lock<std::mutex> lock(asset_lock);
                                asset_cond.wait(lock, [this]() {
                                    return !asset_descs.empty() || terminate;
                                });
                                
                                if(terminate) return;
                                desc = std::move(asset_descs.front());
                                asset_descs.erase(asset_descs.begin());
                            }
                            
                            desc.load(*this, desc.desc, desc.asset);
                        }
                    } catch(std::exception &e) {
                        GLFW_Err(e.what());
                    }
                })));
            }
            
            if(param.window) {
                for(int i = 0; i < param.gl_threads; i++) {
                    gl_threads.emplace_back(std::move(std::async(std::launch::async, [this, param, i]() {
                        try {
                            GLFW_Context gl_context = std::move(GLFW_Context::create({
                                .title  = std::string("Assets GL Context ") + std::to_string(i),
                                .width  = 1,
                                .height = 1,
                                
                                .resizable = false,
                                .visible   = false,
                                .decorated = false
                            }));
                            
                            while(true) {
                                GLQueue desc;
                                
                                {
                                    std::unique_lock<std::mutex> lock(gl_lock);
                                    gl_cond.wait(lock, [this]() {
                                        return !gl_descs.empty() || terminate;
                                    });
                                    
                                    if(terminate) return;
                                    desc = std::move(gl_descs.front());
                                    gl_descs.erase(gl_descs.begin());
                                }
                                
                                desc.load(*this, desc.desc, desc.asset, desc.data, gl_context.gl);
                            }
                        } catch(std::exception &e) {
                            GLFW_Err(e.what());
                        }
                    })));
                }
            }
        }
        
        Assets(const Assets &) = delete;
        
        Assets(Assets &&) = delete;
        
        ~Assets() {
            terminate = true;
            asset_cond.notify_all();
            gl_cond.notify_all();
        }
        
        template<typename T_Asset, typename T_AssetLoader = T_Asset::LoaderType>
        void load(const AssetDesc &desc) {
            T_Asset *asset;
            
            {
                std::unique_lock<std::mutex> lock(registry_lock);
                
                auto &map = AssetRegistry<T_Asset>::all[this];
                if(map.contains(desc.path)) {
                    return;
                } else {
                    asset = &map[desc.path];
                }
            }
            
            {
                std::unique_lock<std::mutex> lock(asset_lock);
                asset_descs.emplace_back(std::move(AssetQueue{
                    .desc  = std::move(desc),
                    .asset = asset,
                    .load  = [](Assets &assets, const AssetDesc &desc, void *asset) {
                        if constexpr(T_AssetLoader::is_GL()) {
                            void *data;
                            T_AssetLoader::load(assets, desc, data);
                            
                            {
                                std::unique_lock<std::mutex> lock(assets.gl_lock);
                                assets.gl_descs.emplace_back(std::move(GLQueue{
                                    .desc  = std::move(desc),
                                    .asset = asset,
                                    .data  = data,
                                    .load  = [](Assets &assets, const AssetDesc &desc, void *asset, void *data, const GL &gl) {
                                        T_AssetLoader::load_GL(assets, desc, *static_cast<T_Asset *>(asset), data, gl);
                                        if(desc.loaded) desc.loaded(assets, asset, desc.loaded_userdata);
                                    }
                                }));
                            }
                            
                            assets.gl_cond.notify_one();
                        } else {
                            T_AssetLoader::load(assets, desc, *static_cast<T_Asset *>(asset));
                            if(desc.loaded) desc.loaded(assets, asset, desc.loaded_userdata);
                        }
                    }
                }));
            }
            
            asset_cond.notify_one();
        }
    };
}

#endif
