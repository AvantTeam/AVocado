#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>

namespace av {
    struct rect_size {
        int width;
        int height;
    };

    struct rect {
        int x;
        int y;
        int width;
        int height;
        
        bool contained_in(const rect &other) const {
            return
                x >= other.x && y >= other.y &&
                x + width <= other.x + other.width &&
                y + height <= other.y + other.height;
        }
    };

    int common_length(int i1start, int i1end, int i2start, int i2end) {
        if(i1end < i2start || i2end < i1start) return 0;
        return std::min(i1end, i2end) - std::max(i1start, i2start);
    }

    class max_rects_bin_pack {
        public:
        max_rects_bin_pack(): bin_width(0), bin_height(0) {}
 
        /**
         * @brief Instantiates a bin of the given size.
         * @param allow_flip Specifies whether the packing algorithm is allowed to rotate the input rectangles by 90 degrees
         * to consider a better placement.
         */
        max_rects_bin_pack(int width, int height, bool allow_flip = true) {
            init(width, height, allow_flip);
        }

        /**
         * @brief Initializes the packer to an empty bin of width x height units. Call whenever you need to restart with a
         * new bin.
         * @param width The bin width;
         * @param height The bin height;
         * @param allow_flip Specifies whether the packing algorithm is allowed to rotate the input rectangles by 90 degrees
         * to consider a better placement.
         */
        void init(int width, int height, bool allow_flip = true) {
            allow_flip = allow_flip;
            bin_width = width;
            bin_height = height;

            rect n;
            n.x = 0;
            n.y = 0;
            n.width = width;
            n.height = height;

            used_rects.clear();
            free_rects.clear();
            free_rects.push_back(n);
        }

        /** @brief Specifies the different heuristic rules that can be used when deciding where to place a new rectangle. */
        enum class heuristic {
            /** @brief Positions the rectangle against the short side of a free rectangle into which it fits the best */
            best_short_side_fit,
            /** @brief Positions the rectangle against the long side of a free rectangle into which it fits the best. */
            best_long_side_fit,
            /** @brief Positions the rectangle into the smallest free rect into which it fits. */
            best_area_fit,
            /** @brief Does the Tetris placement. */
            bottom_left_rule,
            /** @brief Chooses the placement where the rectangle touches other rects as much as possible. */
            contact_point_rule
        };
        
        /**
         * @brief Inserts the given list of rectangles in an offline/batch mode, possibly rotated.
         * 
         * @param rects The list of rectangles to insert. This vector will be destroyed in the process.
         * @param dst [out] This list will contain the packed rectangles. The indices will not correspond to that of rects.
         * @param method The rectangle placement rule to use when packing.
         */
        void insert(std::vector<rect_size> &rects, std::vector<rect> &dst, heuristic method) {
            dst.clear();

            while(rects.size() > 0) {
                int best_score1 = std::numeric_limits<int>::max();
                int best_score2 = std::numeric_limits<int>::max();
                int best_index = -1;
                rect best_node;

                for(size_t i = 0; i < rects.size(); ++i) {
                    int score1;
                    int score2;
                    rect new_node = score(rects[i].width, rects[i].height, method, score1, score2);

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
         * Inserts a single rectangle into the bin, possibly rotated.
         * 
         * @param width The rectangle width.
         * @param height The rectangle height.
         * @param method The packing method. See `heuristic` documentation for details.
         * @return The inserted rectangle.
         */
        rect insert(int width, int height, heuristic method) {
            rect new_node;

            // Unused in this function. We don't need to know the score after finding the position.
            int score1 = std::numeric_limits<int>::max();
            int score2 = std::numeric_limits<int>::max();
            switch(method) {
                case heuristic::best_short_side_fit: new_node = find_pos_short_side(width, height, score1, score2); break;
                case heuristic::bottom_left_rule: new_node = find_pos_bottom_left(width, height, score1, score2); break;
                case heuristic::contact_point_rule: new_node = find_pos_contact_point(width, height, score1); break;
                case heuristic::best_long_side_fit: new_node = find_pos_long_side(width, height, score2, score1); break;
                case heuristic::best_area_fit: new_node = find_pos_best_area(width, height, score1, score2); break;
            }

            if(new_node.height == 0) return new_node;

            place(new_node);
            return new_node;
        }

        /** @return The ratio of used surface area to the total bin area. */
        double occupancy() const {
            uint64_t used_area = 0;
            for(size_t i = 0; i < used_rects.size(); ++i) {
                const rect &r = used_rects[i];
                used_area += r.width * r.height;
            }

            return (double)used_area / ((uint64_t)bin_width * bin_height);
        }

        private:
        int bin_width;
        int bin_height;

        bool allow_flip;

        size_t new_free_rects_last_size;
        std::vector<rect> new_free_rects;
        std::vector<rect> used_rects;
        std::vector<rect> free_rects;

        /**
         * @brief Computes the placement score for placing the given rectangle with the given method.
         * 
         * @param width The rectangle width.
         * @param height The rectangle height.
         * @param method The packing method. See `heuristic` documentation for details.
         * @param score1 [out] The primary placement score will be outputted here.
         * @param score2 [out] The secondary placement score will be outputted here. This is used to break ties.
         * @return The struct that identifies where the rectangle would be placed if it were placed.
         */
        rect score(int width, int height, heuristic method, int &score1, int &score2) const {
            rect new_node;
            score1 = std::numeric_limits<int>::max();
            score2 = std::numeric_limits<int>::max();
            switch(method) {
                case heuristic::best_short_side_fit: new_node = find_pos_short_side(width, height, score1, score2); break;
                case heuristic::bottom_left_rule: new_node = find_pos_bottom_left(width, height, score1, score2); break;
                case heuristic::contact_point_rule: {
                    new_node = find_pos_contact_point(width, height, score1);
                    score1 = -score1; // Reverse since we are minimizing, but for contact point score bigger is better.

                } break;

                case heuristic::best_long_side_fit: new_node = find_pos_long_side(width, height, score2, score1); break;
                case heuristic::best_area_fit: new_node = find_pos_best_area(width, height, score1, score2); break;
            }

            // Cannot fit the current rectangle.
            if(new_node.height == 0) {
                score1 = std::numeric_limits<int>::max();
                score2 = std::numeric_limits<int>::max();
            }

            return new_node;
        }

        void place(const rect &node) {
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

        int contact_point_score_node(int x, int y, int width, int height) const {
            int score = 0;

            if(x == 0 || x + width == bin_width) score += height;
            if(y == 0 || y + height == bin_height) score += width;

            for(size_t i = 0; i < used_rects.size(); ++i) {
                const rect &r = used_rects[i];
                if(r.x == x + width || r.x + r.width == x) score += common_length(r.y, r.y + r.height, y, y + height);
                if(r.y == y + height || r.y + r.height == y) score += common_length(r.x, r.x + r.width, x, x + width);
            }

            return score;
        }

        rect find_pos_bottom_left(int width, int height, int &best_y, int &best_x) const {
            rect best_node = {};

            best_y = std::numeric_limits<int>::max();
            best_x = std::numeric_limits<int>::max();

            for(size_t i = 0; i < free_rects.size(); ++i) {
                const rect &r = used_rects[i];

                // Try to place the rectangle in upright (non-flipped) orientation.
                if(r.width >= width && r.height >= height) {
                    int top_side_y = r.y + height;
                    if(top_side_y < best_y || (top_side_y == best_y && r.x < best_x)) {
                        best_node.x = r.x;
                        best_node.y = r.y;
                        best_node.width = width;
                        best_node.height = height;
                        best_y = top_side_y;
                        best_x = r.x;
                    }
                }

                if(allow_flip && r.width >= height && r.height >= width) {
                    int top_side_y = r.y + width;
                    if(top_side_y < best_y || (top_side_y == best_y && r.x < best_x)) {
                        best_node.x = r.x;
                        best_node.y = r.y;
                        best_node.width = height;
                        best_node.height = width;
                        best_y = top_side_y;
                        best_x = r.x;
                    }
                }
            }

            return best_node;
        }

        rect find_pos_short_side(int width, int height, int &best_short_fit, int &best_long_fit) const {
            rect best_node = {};

            best_short_fit = std::numeric_limits<int>::max();
            best_long_fit = std::numeric_limits<int>::max();

            for(size_t i = 0; i < free_rects.size(); ++i) {
                const rect &r = used_rects[i];

                // Try to place the rectangle in upright (non-flipped) orientation.
                if(r.width >= width && r.height >= height) {
                    int leftover_hor = std::abs(r.width - width);
                    int leftover_ver = std::abs(r.height - height);
                    int short_fit = std::min(leftover_hor, leftover_ver);
                    int long_fit = std::max(leftover_hor, leftover_ver);

                    if(short_fit < best_short_fit || (short_fit == best_short_fit && long_fit < best_long_fit)) {
                        best_node.x = r.x;
                        best_node.y = r.y;
                        best_node.width = width;
                        best_node.height = height;
                        best_short_fit = short_fit;
                        best_long_fit = long_fit;
                    }
                }

                if(allow_flip && r.width >= height && r.height >= width) {
                    int leftover_hor_f = std::abs(r.width - height);
                    int leftover_ver_f = std::abs(r.height - width);
                    int short_fit_f = std::min(leftover_hor_f, leftover_ver_f);
                    int long_fit_f = std::max(leftover_hor_f, leftover_ver_f);

                    if(short_fit_f < best_short_fit || (short_fit_f == best_short_fit && long_fit_f < best_long_fit)) {
                        best_node.x = r.x;
                        best_node.y = r.y;
                        best_node.width = height;
                        best_node.height = width;
                        best_short_fit = short_fit_f;
                        best_long_fit = long_fit_f;
                    }
                }
            }

            return best_node;
        }

        rect find_pos_long_side(int width, int height, int &best_short_fit, int &best_long_fit) const {
            rect best_node = {};

            best_short_fit = std::numeric_limits<int>::max();
            best_long_fit = std::numeric_limits<int>::max();

            for(size_t i = 0; i < free_rects.size(); ++i) {
                const rect &r = used_rects[i];

                // Try to place the rectangle in upright (non-flipped) orientation.
                if(r.width >= width && r.height >= height) {
                    int leftover_hor = std::abs(r.width - width);
                    int leftover_ver = std::abs(r.height - height);
                    int short_fit = std::min(leftover_hor, leftover_ver);
                    int long_fit = std::max(leftover_hor, leftover_ver);

                    if(long_fit < best_long_fit || (long_fit == best_long_fit && short_fit < best_short_fit)) {
                        best_node.x = r.x;
                        best_node.y = r.y;
                        best_node.width = width;
                        best_node.height = height;
                        best_short_fit = short_fit;
                        best_long_fit = long_fit;
                    }
                }

                if(allow_flip && r.width >= height && r.height >= width) {
                    int leftover_hor_f = std::abs(r.width - height);
                    int leftover_ver_f = std::abs(r.height - width);
                    int short_fit_f = std::min(leftover_hor_f, leftover_ver_f);
                    int long_fit_f = std::max(leftover_hor_f, leftover_ver_f);

                    if(long_fit_f < best_long_fit || (long_fit_f == best_long_fit && short_fit_f < best_short_fit)) {
                        best_node.x = r.x;
                        best_node.y = r.y;
                        best_node.width = height;
                        best_node.height = width;
                        best_short_fit = short_fit_f;
                        best_long_fit = long_fit_f;
                    }
                }
            }

            return best_node;
        }

        rect find_pos_best_area(int width, int height, int &best_area_fit, int &best_short_fit) const {
            rect best_node = {};

            best_area_fit = std::numeric_limits<int>::max();
            best_short_fit = std::numeric_limits<int>::max();

            for(size_t i = 0; i < free_rects.size(); ++i) {
                const rect &r = used_rects[i];
                int area_fit = r.width * r.height - width * height;

                // Try to place the rectangle in upright (non-flipped) orientation.
                if(r.width >= width && r.height >= height) {
                    int leftover_hor = std::abs(r.width - width);
                    int leftover_ver = std::abs(r.height - height);
                    int short_fit = std::min(leftover_hor, leftover_ver);

                    if(area_fit < best_area_fit || (area_fit == best_area_fit && short_fit < best_short_fit)) {
                        best_node.x = r.x;
                        best_node.y = r.y;
                        best_node.width = width;
                        best_node.height = height;
                        best_short_fit = short_fit;
                        best_area_fit = area_fit;
                    }
                }

                if(allow_flip && r.width >= height && r.height >= width) {
                    int leftover_hor_f = std::abs(r.width - height);
                    int leftover_ver_f = std::abs(r.height - width);
                    int short_fit_f = std::min(leftover_hor_f, leftover_ver_f);

                    if(area_fit < best_area_fit || (area_fit == best_area_fit && short_fit_f < best_short_fit)) {
                        best_node.x = r.x;
                        best_node.y = r.y;
                        best_node.width = height;
                        best_node.height = width;
                        best_short_fit = short_fit_f;
                        best_area_fit = area_fit;
                    }
                }
            }

            return best_node;
        }

        rect find_pos_contact_point(int width, int height, int &best_score) const {
            rect best_node = {};

            best_score = -1;

            for(size_t i = 0; i < free_rects.size(); ++i) {
                const rect &r = used_rects[i];

                // Try to place the rectangle in upright (non-flipped) orientation.
                if(r.width >= width && r.height >= height) {
                    int score = contact_point_score_node(r.x, r.y, width, height);
                    if(score > best_score) {
                        best_node.x = r.x;
                        best_node.y = r.y;
                        best_node.width = width;
                        best_node.height = height;
                        best_score = score;
                    }
                }

                if(allow_flip && r.width >= height && r.height >= width) {
                    int score = contact_point_score_node(r.x, r.y, height, width);
                    if(score > best_score) {
                        best_node.x = r.x;
                        best_node.y = r.y;
                        best_node.width = height;
                        best_node.height = width;
                        best_score = score;
                    }
                }
            }

            return best_node;
        }

        void insert_new(const rect &new_rect) {
            assert(new_rect.width > 0);
            assert(new_rect.height > 0);

            for(size_t i = 0; i < new_free_rects_last_size;) {
                // Is this new free rectangle already accounted for?
                if(new_rect.contained_in(new_free_rects[i])) return;

                // Does this new free rectangle obsolete a previous new free rectangle?
                if(new_free_rects[i].contained_in(new_rect)) {
                    // Remove i'th new free rectangle, but do so by retaining the order of the older vs newest free
                    // rectangles that we may still be placing in calling function split_free_node().
                    new_free_rects[i] = new_free_rects[--new_free_rects_last_size];
                    new_free_rects[new_free_rects_last_size] = new_free_rects.back();
                    new_free_rects.pop_back();
                } else {
                    ++i;
                }
            }

            new_free_rects.push_back(new_rect);
        }

        /** @return True if the free node was split. */
        bool split_free_node(const rect &free, const rect &used) {
            // Test with SAT if the rectangles even intersect.
            if(
                used.x >= free.x + free.width || used.x + used.width <= free.x ||
                used.y >= free.y + free.height || used.y + used.height <= free.y
            ) return false;

            // We add up to four new free rectangles to the free rectangles list below. None of these four newly added
            // free rectangles can overlap any other three, so keep a mark of them to avoid testing them against each
            // other.
            new_free_rects_last_size = new_free_rects.size();

            if(used.x < free.x + free.width && used.x + used.width > free.x) {
                // New node at the top side of the used node.
                if(used.y > free.y && used.y < free.y + free.height) {
                    rect new_node = free;
                    new_node.height = used.y - new_node.y;
                    insert_new(new_node);
                }

                // New node at the bottom side of the used node.
                if(used.y + used.height < free.y + free.height) {
                    rect new_node = free;
                    new_node.y = used.y + used.height;
                    new_node.height = free.y + free.height - (used.y + used.height);
                    insert_new(new_node);
                }
            }

            if(used.y < free.y + free.height && used.y + used.height > free.y) {
                // New node at the left side of the used node.
                if(used.x > free.x && used.x < free.x + free.width) {
                    rect new_node = free;
                    new_node.width = used.x - new_node.x;
                    insert_new(new_node);
                }

                // New node at the right side of the used node.
                if(used.x + used.width < free.x + free.width) {
                    rect new_node = free;
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
                        // The old free rectangles can never be contained in any of the new free rectangles (the new
                        // free rectangles keep shrinking in size)
                        assert(!free_rects[i].contained_in(new_free_rects[j]));

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

int main(int argc, char *argv[]) {

}
