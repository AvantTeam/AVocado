#include <av/core/app.hpp>
#include <stdexcept>

using namespace av; // I don't care.

int main(int argc, char *argv[]) {
    class: public app_listener {
        public:
        void update(app &) override {
            throw std::runtime_error("Open't sesame.");
        }
    } listener;

    service::app::set();

    app &app = service::app::ref();
    app.add_listener(&listener);

    if(!app.init()) return 1;
    
    bool success = app.loop();
    service::app::reset();

    return success;
}
