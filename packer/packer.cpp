#include <av/bin_pack.hpp>
#include <av/io.hpp>
#include <av/log.hpp>
#include <av/time.hpp>
#include <av/graphics/2d/pixmap.hpp>
#include <cxxopts.hpp>

#include <filesystem>
#include <fstream>
#include <string>

int main(int argc, char *argv[]) {
    namespace fs = std::filesystem;

    av::time_manager time;
    time.update();

    float init_time = time.get();

    cxxopts::Options cmd(argv[0], "Pack your sprites in a directory recursively into large sprite atlas(es).");
    cmd.add_options("General usage")
        ("d,dir", "Specifies the root sprites directory.", cxxopts::value<std::string>())
        ("w,width", "Specifies the atlas page width.", cxxopts::value<int>()->default_value("4096"))
        ("h,height", "Specifies the atlas page height.", cxxopts::value<int>()->default_value("4096"))
        ("p,padding", "Specifies the padding for each sprite.", cxxopts::value<int>()->default_value("4"))
        ("f,flip", "Whether to flip sprite rectangles vertically.", cxxopts::value<bool>()->default_value("false"))
        ("q,quiet", "Outputs no logs.", cxxopts::value<bool>()->default_value("false"))
        ("help", "Print this message.");

    try {
        cxxopts::ParseResult result = cmd.parse(argc, argv);
        if(result.count("help") || result.arguments().size() == 0) {
            av::log::msg("AVocado sprite-packing tool.\n%s", cmd.help().c_str());
            return 0;
        }

        fs::path sprites_dir(result["dir"].as<std::string>());
        int bin_width = result["width"].as<int>();
        int bin_height = result["height"].as<int>();
        int padding = result["padding"].as<int>();
        bool flip = result["flip"].as<bool>();

        bool quiet = result["quiet"].as<bool>();
        if(!quiet) av::log::msg("Iterating through directories...");

        int total = 0;
        for(auto &f : fs::recursive_directory_iterator(sprites_dir)) {
            if(f.path().extension() == ".png") total++;
        }

        av::pixmap sprites[total];
        std::pair<std::string, av::rect_size<int>> infos[total];

        int i = 0;
        for(auto &f : fs::recursive_directory_iterator(sprites_dir)) {
            if(i >= total) break;
            if(f.path().extension() == ".png") {
                av::pixmap &sprite = sprites[i];
                sprite.load(f.path().string().c_str());
                if(flip) sprite.flip_y();

                std::string name = f.path().filename().string();
                name = name.substr(0, name.length() - 4);

                infos[i].first = name;
                infos[i].second = {sprite.get_width() + padding * 2, sprite.get_height() + padding * 2};
                i++;
            }
        }

        if(!quiet) {
            av::log::msg("Found %d sprites.", total);
            av::log::msg("Generating %dx%d sprite atlases...", bin_width, bin_height);
        }

        std::vector<av::bin_pack> bins;
        std::vector<av::pixmap> pages;
        std::vector<std::unordered_map<std::string, av::rect<int>>> regions;

        static constexpr int int_max = std::numeric_limits<int>::max();
        for(int i = 0; i < total;) {
            std::pair<int, int> rect_score(int_max, int_max);
            int best_rect, best_bin;
            bool all_fit = false;

            for(int j = 0; j < total; j++) {
                std::pair<int, int> bin_score(int_max, int_max), tmp_score(int_max, int_max);
                int best_bin_local;
                bool fits = false;

                for(int k = 0; k < bins.size(); k++) {
                    bins[k].score(infos[j].second.width, infos[j].second.height, tmp_score.first, tmp_score.second);
                    if(tmp_score.first != int_max && tmp_score.second != int_max) {
                        fits = true;
                    } else {
                        continue;
                    }

                    if(bin_score > tmp_score) {
                        bin_score = tmp_score;
                        best_bin_local = k;
                    }
                }

                if(fits) {
                    all_fit = true;
                    if(rect_score > bin_score) {
                        rect_score = bin_score;
                        best_bin = best_bin_local;
                        best_rect = j;
                    }
                }
            }

            if(all_fit) {
                av::rect<int> place = bins[best_bin].insert(infos[best_rect].second.width, infos[best_rect].second.height);
                place.x += padding;
                place.y += padding;
                place.width -= padding;
                place.height -= padding;

                pages[best_bin].draw_image(sprites[best_rect], place.x, place.y, false);
                infos[best_rect].second = {int_max, int_max};
                regions[best_bin].emplace(infos[best_rect].first, place);

                i++;
            } else {
                bins.emplace_back(bin_width, bin_height);
                pages.emplace_back(bin_width, bin_height);
                regions.emplace_back();
            }
        }

        if(!quiet) {
            av::log::msg("Generated %d sprite atlas%s.", pages.size(), pages.size() == 1 ? "" : "es");
            av::log::msg("Writing images and atlas data...");
        }

        //TODO fallback these into a separate version-based writer/reader.
        std::ofstream out("texture.atlas", std::ios::binary); // Open atlas writer.
        av::writes write(out);

        write.write<unsigned char>(1); // Write version.
        write.write(static_cast<unsigned char>(pages.size())); // Write page amount, up to 256.
        for(size_t i = 0; i < pages.size(); i++) {
            std::string page_name("texture");
            page_name.append(std::to_string(i)).append(".png");

            write.write(page_name); // Write page texture name.
            pages[i].write_to(page_name.c_str());

            std::unordered_map<std::string, av::rect<int>> &map = regions[i];

            write.write(static_cast<short>(map.size())); // Write regions amount, up to 65536.
            for(const auto &[name, region] : map) {
                write
                    .write(name)                                        // Write region name.
                    .write(static_cast<unsigned short>(region.x))       // Write region X position, up to 65536.
                    .write(static_cast<unsigned short>(region.y))       // Write region Y position, up to 65536.
                    .write(static_cast<unsigned short>(region.width))   // Write region width, up to 65536.
                    .write(static_cast<unsigned short>(region.height)); // Write region height, up to 65536.
            }
        }

        if(!quiet) {
            time.update();
            av::log::msg("Sprite packer has successfully packed the sprites, took %f seconds.", time.get() - init_time);
        }

        return 0;
    } catch(std::exception &e) {
        av::log::msg<av::log_level::error>(e.what());
        return 1;
    }
}
