#ifndef AV_BINPACK_HPP
#define AV_BINPACK_HPP

#include "math.hpp"

#include <vector>

namespace av {
    /**
     * @brief Max rects bin pack, ported from https://github.com/juj/RectangleBinPack/blob/master/MaxRectsBinPack.h.
     * Reformatted and stripped, leaving only Best-Short-Side-Fit heuristic in usage.
     */
    class bin_pack {
        int bin_width;
        int bin_height;

        size_t new_free_rects_last_size;
        std::vector<rect<int>> new_free_rects;
        std::vector<rect<int>> used_rects;
        std::vector<rect<int>> free_rects;

        public:
        /** @brief Default constructor. Call `init(int, int)` afterwards. */
        bin_pack(): bin_width(0), bin_height(0) {}

        /** @brief Instantiates a bin of the given size. */
        bin_pack(int width, int height) {
            init(width, height);
        }

        /**
         * @brief Initializes the packer to an empty bin of width x height units. Call whenever you need to restart with a
         * new bin.
         * @param width The bin width.
         * @param height The bin height.
         */
        void init(int width, int height) {
            bin_width = width;
            bin_height = height;

            rect<int> n;
            n.width = width;
            n.height = height;

            used_rects.clear();
            free_rects.clear();
            free_rects.push_back(n);
        }

        /**
         * @brief Inserts the given list of rectangles in an offline/batch mode, possibly rotated.
         *
         * @param rects The list of rectangles to insert. This vector will be destroyed in the process.
         * @param dst [out] This list will contain the packed rectangles. The indices will not correspond to that of rects.
         */
        void insert(std::vector<rect_size<int>> &rects, std::vector<rect<int>> &dst) {
            dst.clear();

            while(rects.size() > 0) {
                int best_score1 = std::numeric_limits<int>::max();
                int best_score2 = std::numeric_limits<int>::max();
                int best_index = -1;
                rect<int> best_node;

                for(size_t i = 0; i < rects.size(); ++i) {
                    int score1;
                    int score2;
                    rect<int> new_node = score(rects[i].width, rects[i].height, score1, score2);

                    if(score1 < best_score1 || (score1 == best_score1 && score2 < best_score2)) {
                        best_score1 = score1;
                        best_score2 = score2;
                        best_node = new_node;
                        best_index = i;
                    }
                }

                if(best_index == -1) return;

                place(best_node);
                dst.push_back(best_node);
                rects[best_index] = rects.back();
                rects.pop_back();
            }
        }
        /**
         * @brief Inserts a single rectangle into the bin, possibly rotated.
         *
         * @param width The rectangle width.
         * @param height The rectangle height.
         * @return The inserted rectangle.
         */
        rect<int> insert(int width, int height) {
            // Unused in this function. We don't need to know the score after finding the position.
            int score1 = std::numeric_limits<int>::max();
            int score2 = std::numeric_limits<int>::max();
            rect<int> new_node = find_pos(width, height, score1, score2);

            if(new_node.height == 0) return new_node;

            place(new_node);
            return new_node;
        }

        /** @return The ratio of used surface area to the total bin area. */
        double occupancy() const {
            size_t used_area = 0;
            for(size_t i = 0; i < used_rects.size(); ++i) {
                const rect<int> &r = used_rects[i];
                used_area += static_cast<size_t>(r.width) * r.height;
            }

            return static_cast<double>(used_area) / (static_cast<size_t>(bin_width) * bin_height);
        }

        /**
         * @brief Computes the placement score for placing the given rectangle with the given method.
         *
         * @param width The rectangle width.
         * @param height The rectangle height.
         * @param score1 [out] The primary placement score will be outputted here.
         * @param score2 [out] The secondary placement score will be outputted here. This is used to break ties.
         * @return The struct that identifies where the rectangle would be placed if it were placed.
         */
        rect<int> score(int width, int height, int &score1, int &score2) const {
            score1 = std::numeric_limits<int>::max();
            score2 = std::numeric_limits<int>::max();
            rect<int> new_node = find_pos(width, height, score1, score2);

            if(new_node.height == 0) {
                score1 = std::numeric_limits<int>::max();
                score2 = std::numeric_limits<int>::max();
            }

            return new_node;
        }

        private:
        void place(const rect<int> &node) {
            for(size_t i = 0; i < free_rects.size();) {
                if(split_free_node(free_rects[i], node)) {
                    free_rects[i] = free_rects.back();
                    free_rects.pop_back();
                } else {
                    ++i;
                }
            }

            prune_free_list();
            used_rects.push_back(node);
        }

        rect<int> find_pos(int width, int height, int &best_short_fit, int &best_long_fit) const {
            rect<int> best_node{};

            best_short_fit = std::numeric_limits<int>::max();
            best_long_fit = std::numeric_limits<int>::max();

            for(size_t i = 0; i < free_rects.size(); ++i) {
                const rect<int> &r = free_rects[i];

                // Try to place the rectangle in upright orientation.
                if(r.width >= width && r.height >= height) {
                    int leftover_hor = abs(r.width - width);
                    int leftover_ver = abs(r.height - height);
                    int short_fit = min(leftover_hor, leftover_ver);
                    int long_fit = max(leftover_hor, leftover_ver);

                    if(short_fit < best_short_fit || (short_fit == best_short_fit && long_fit < best_long_fit)) {
                        best_node.x = r.x;
                        best_node.y = r.y;
                        best_node.width = width;
                        best_node.height = height;
                        best_short_fit = short_fit;
                        best_long_fit = long_fit;
                    }
                }
            }

            return best_node;
        }

        void insert_new(const rect<int> &new_rect) {
            if(new_rect.width == 0 || new_rect.height == 0) return;

            for(size_t i = 0; i < new_free_rects_last_size;) {
                // Is this new free rectangle already accounted for?
                if(new_rect.contained_in(new_free_rects[i])) return;

                // Does this new free rectangle obsolete a previous new free rectangle?
                if(new_free_rects[i].contained_in(new_rect)) {
                    // Remove i'th new free rectangle, but do so by retaining the order of the older vs newest free rectangles
                    // that we may still be placing in calling function `split_free_node()`.
                    new_free_rects[i] = new_free_rects[--new_free_rects_last_size];
                    new_free_rects[new_free_rects_last_size] = new_free_rects.back();
                    new_free_rects.pop_back();
                } else {
                    ++i;
                }
            }

            new_free_rects.push_back(new_rect);
        }

        /** @return `true` if the free node was split, `false` otherwise. */
        bool split_free_node(const rect<int> &free, const rect<int> &used) {
            // Test with SAT if the rectangles even intersect.
            if(
                used.x >= free.x + free.width || used.x + used.width <= free.x ||
                used.y >= free.y + free.height || used.y + used.height <= free.y
                ) return false;

            // We add up to four new free rectangles to the free rectangles list below. None of these four newly added free
            // rectangles can overlap any other three, so keep a mark of them to avoid testing them against each other.
            new_free_rects_last_size = new_free_rects.size();

            if(used.x < free.x + free.width && used.x + used.width > free.x) {
                // New node at the top side of the used node.
                if(used.y > free.y && used.y < free.y + free.height) {
                    rect<int> new_node = free;
                    new_node.height = used.y - new_node.y;

                    insert_new(new_node);
                }

                // New node at the bottom side of the used node.
                if(used.y + used.height < free.y + free.height) {
                    rect<int> new_node = free;
                    new_node.y = used.y + used.height;
                    new_node.height = free.y + free.height - (used.y + used.height);

                    insert_new(new_node);
                }
            }

            if(used.y < free.y + free.height && used.y + used.height > free.y) {
                // New node at the left side of the used node.
                if(used.x > free.x && used.x < free.x + free.width) {
                    rect<int> new_node = free;
                    new_node.width = used.x - new_node.x;

                    insert_new(new_node);
                }

                // New node at the right side of the used node.
                if(used.x + used.width < free.x + free.width) {
                    rect<int> new_node = free;
                    new_node.x = used.x + used.width;
                    new_node.width = free.x + free.width - (used.x + used.width);

                    insert_new(new_node);
                }
            }

            return true;
        }

        /** Goes through the free rectangle list and removes any redundant entries. */
        void prune_free_list() {
            // Test all newly introduced free rectangles against old free rectangles.
            for(size_t i = 0; i < free_rects.size(); ++i) {
                for(size_t j = 0; j < new_free_rects.size();) {
                    if(new_free_rects[j].contained_in(free_rects[i])) {
                        new_free_rects[j] = new_free_rects.back();
                        new_free_rects.pop_back();
                    } else {
                        ++j;
                    }
                }
            }

            // Merge new and old free rectangles to the group of old free rectangles.
            free_rects.insert(free_rects.end(), new_free_rects.begin(), new_free_rects.end());
            new_free_rects.clear();
        }
    };
}

#endif // !AV_BINPACK_HPP
