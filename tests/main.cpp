#include <av/core/app.hpp>
#include <stdexcept>

int main(int argc, char *argv[]) {
    class: public av::app_listener {
        public:
        void update(av::app &) override {
            throw std::runtime_error("Open't sesame.");
        }
    } listener;

    av::app app;
    app.add_listener(&listener);

    if(!app.init(av::app_config())) return 1;
    return !app.loop();
}
