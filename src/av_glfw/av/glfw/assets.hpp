#ifndef AV_GLFW_ASSETS_HPP
#define AV_GLFW_ASSETS_HPP

#include <condition_variable>
#include <deque>
#include <future>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <av/gl.hpp>
#include <av/globals.hpp>
#include <av/glfw/glfw.hpp>
#include <av/glfw/globals_glfw.hpp>

namespace av {
    struct AssetsParam {
        int asset_threads = std::thread::hardware_concurrency(),
            gl_threads = 1;
    };
    
    struct AssetDesc {
        std::string path;
        Callback<void, void *> loaded;
    };
    
    namespace AssetLoader {
        template<typename T_AssetType>
        constexpr bool is_GL();
        
        template<typename T_AssetType>
        void load(const AssetDesc &, T_AssetType &);
        
        template<typename T_AssetType>
        void load_GL_init(const AssetDesc &, void **);
        
        template<typename T_AssetType>
        void load_GL(const AssetDesc &, T_AssetType &, void *);
    }
    
    namespace {
        struct AssetQueue {
            AssetDesc desc;
            Callback<void, const AssetDesc &> load;
        };
        
        struct GLQueue {
            AssetDesc desc;
            void *data;
            
            Callback<void, const AssetDesc &, void *> load;
        };
        
        struct LoadedCallback {
            Callback<void, void *> desc;
            Callback<void, const Callback<void, void *> &> loaded;
        };
        
        template<typename T_AssetType>
        struct Assets {
            static inline std::unordered_map<std::string, T_AssetType> assets;
            static inline std::unordered_set<std::string> loaded;
            static inline std::vector<LoadedCallback> delay_load;
            
            static inline std::mutex asset_lock, load_lock, delay_lock;
        };
        
        template<typename T_AssetType>
        bool has_asset(const std::string &name) {
            std::lock_guard<std::mutex> lock(Assets<T_AssetType>::asset_lock);
            return Assets<T_AssetType>::assets.contains(name);
        }
        
        template<typename T_AssetType>
        bool loaded_asset(const std::string &name) {
            std::lock_guard<std::mutex> lock(Assets<T_AssetType>::load_lock);
            return Assets<T_AssetType>::loaded.contains(name);
        }
        
        template<typename T_AssetType>
        T_AssetType *get_asset(const std::string &name) {
            std::lock_guard<std::mutex> lock(Assets<T_AssetType>::asset_lock);
            return &Assets<T_AssetType>::assets[name];
        }
        
        template<typename T_AssetType>
        void load_asset(const std::string &name) {
            std::lock_guard<std::mutex> lock(Assets<T_AssetType>::load_lock);
            Assets<T_AssetType>::loaded.emplace(std::move(name));
        }
        
        template<typename T_AssetType>
        void dispose() {
            {
                std::lock_guard<std::mutex> lock(Assets<T_AssetType>::asset_lock);
                Assets<T_AssetType>::assets.clear();
            }
            
            {
                std::lock_guard<std::mutex> lock(Assets<T_AssetType>::load_lock);
                Assets<T_AssetType>::loaded.clear();
            }
        }
        
        inline volatile bool terminate;
        
        inline std::vector<std::future<void>> asset_threads, gl_threads;
        inline std::deque<AssetQueue> asset_descs;
        inline std::deque<GLQueue> gl_descs;
        
        inline int loaded, to_load;
        
        inline std::mutex asset_lock, gl_lock, count_lock;
        inline std::condition_variable asset_cond, gl_cond;
        
        inline std::set<void(*)()> disposal;
    }
    
    void AV_init_assets(AssetsParam param) {
        terminate = false;
        
        asset_threads.reserve(param.asset_threads);
        gl_threads.reserve(param.gl_threads);
        
        for(int i = 0; i < param.asset_threads; i++) {
            asset_threads.emplace_back(std::move(std::async(std::launch::async, []() {
                try {
                    while(true) {
                        AssetQueue desc;
                        
                        {
                            std::unique_lock<std::mutex> lock(asset_lock);
                            asset_cond.wait(lock, []() {
                                return !asset_descs.empty() || terminate;
                            });
                            
                            if(terminate) return;
                            desc = std::move(asset_descs.front());
                            asset_descs.pop_front();
                        }
                        
                        desc.load(desc.desc);
                    }
                } catch(std::exception &e) {
                    AV_err(e.what());
                }
            })));
        }
        
        for(int i = 0; i < param.gl_threads; i++) {
            std::string title("Assets GL Context ");
            title += std::move(std::to_string(i));
            
            GLFWwindow *window = GLFW_Context::create_window({
                .title  = title,
                .width  = 1,
                .height = 1,
                
                .resizable = false,
                .visible   = false,
                .decorated = false,
                
                .share = static_cast<GLFWwindow *>(AV_window())
            });
            
            gl_threads.emplace_back(std::move(std::async(std::launch::async, [window, title, i]() {
                try {
                    AV_set_context(GLFW_Context(title, window));
                    while(true) {
                        GLQueue desc;
                        
                        {
                            std::unique_lock<std::mutex> lock(gl_lock);
                            gl_cond.wait(lock, []() {
                                return !gl_descs.empty() || terminate;
                            });
                            
                            if(terminate) break;
                            desc = std::move(gl_descs.front());
                            gl_descs.pop_front();
                        }
                        
                        desc.load(desc.desc, desc.data);
                    }
                } catch(std::exception &e) {
                    AV_reset_context();
                    AV_err(e.what());
                    return;
                }
                
                AV_reset_context();
            })));
        }
    }
    
    void AV_dispose_assets() {
        terminate = true;
        asset_cond.notify_all();
        gl_cond.notify_all();
        
        for(std::future<void> &f : asset_threads) f.wait();
        asset_threads.clear();
        
        for(std::future<void> &f : gl_threads) f.wait();
        gl_threads.clear();
        
        asset_descs.clear();
        gl_descs.clear();
        
        for(void(*dispose)() : disposal) dispose();
        disposal.clear();
    }
    
    template<typename T_AssetType>
    void AV_load_asset(const AssetDesc &desc) {
        disposal.emplace([]() { dispose<T_AssetType>(); });
        
        if(desc.loaded) {
            if(loaded_asset<T_AssetType>(desc.path)) {
                desc.loaded(get_asset<T_AssetType>(desc.path));
                return;
            } else if(has_asset<T_AssetType>(desc.path)) {
                std::lock_guard<std::mutex> lock(Assets<T_AssetType>::delay_lock);
                Assets<T_AssetType>::delay_load.emplace_back(std::move(LoadedCallback {
                    .desc   = desc.loaded,
                    .loaded = {[](void *asset, const Callback<void, void *> &loaded) {
                        loaded(asset);
                    }, get_asset<T_AssetType>(desc.path) }
                }));
                
                return;
            }
        } else if(has_asset<T_AssetType>(desc.path)) return;
        
        {
            std::lock_guard<std::mutex> lock(count_lock);
            to_load++;
        }
        
        T_AssetType *asset = get_asset<T_AssetType>(desc.path);
        
        {
            std::unique_lock<std::mutex> lock(asset_lock);
            asset_descs.emplace_back(std::move(AssetQueue {
                .desc = std::move(desc),
                .load = {[](void *asset, const AssetDesc &desc) {
                    if constexpr(AssetLoader::is_GL<T_AssetType>()) {
                        void *data;
                        AssetLoader::load_GL_init<T_AssetType>(desc, &data);
                        
                        {
                            std::unique_lock<std::mutex> lock(gl_lock);
                            gl_descs.emplace_back(std::move(GLQueue {
                                .desc = std::move(desc),
                                .data = data,
                                .load = {[](void *asset, const AssetDesc &desc, void *data) {
                                    AssetLoader::load_GL<T_AssetType>(desc, *static_cast<T_AssetType *>(asset), data);
                                    load_asset<T_AssetType>(desc.path);
                                    
                                    if(desc.loaded) desc.loaded(asset);
                                    
                                    {
                                        std::lock_guard<std::mutex> lock(Assets<T_AssetType>::delay_lock);
                                        for(LoadedCallback &loaded : Assets<T_AssetType>::delay_load) loaded.loaded(loaded.desc);
                                        
                                        Assets<T_AssetType>::delay_load.clear();
                                    }
                                    
                                    {
                                        std::lock_guard<std::mutex> lock(count_lock);
                                        loaded++;
                                    }
                                }, asset }
                            }));
                        }
                        
                        gl_cond.notify_one();
                    } else {
                        AssetLoader::load<T_AssetType>(desc, *static_cast<T_AssetType *>(asset));
                        load_asset<T_AssetType>(desc.path);
                        
                        if(desc.loaded) desc.loaded(asset);
                        
                        {
                            std::lock_guard<std::mutex> lock(Assets<T_AssetType>::delay_lock);
                            for(LoadedCallback &loaded : Assets<T_AssetType>::delay_load) loaded.loaded(loaded.desc);
                            
                            Assets<T_AssetType>::delay_load.clear();
                        }
                        
                        {
                            std::lock_guard<std::mutex> lock(count_lock);
                            loaded++;
                        }
                    }
                }, asset }
            }));
        }
        
        asset_cond.notify_one();
    }
    
    template<typename T_AssetType>
    T_AssetType &AV_get_asset(const std::string &name) {
        if(!loaded_asset<T_AssetType>(name)) throw std::runtime_error(std::string(typeid(T_AssetType).name()) + std::string(" with name '") + name + "' not loaded.");
        return *get_asset<T_AssetType>(name);
    }
    
    float AV_assets_progress() {
        std::lock_guard<std::mutex> lock(count_lock);
        return to_load == 0 ? 1.0f : (static_cast<float>(loaded) / static_cast<float>(to_load));
    }
    
    bool AV_assets_done() {
        std::lock_guard<std::mutex> lock(count_lock);
        return loaded >= to_load;
    }
}

#endif
