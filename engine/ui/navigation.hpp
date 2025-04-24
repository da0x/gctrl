//
// General Controls Engine
//
// Copyright (C) 2024 Daher Alfawares
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//
// Contact Information: www.gctrl.org
//

#pragma once

#include <deque>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include "database/list.hpp"

namespace ui {

    namespace navigation {

        constexpr int max_history_size = 200;

        enum class mode {
            edit,
            run
        };

        enum class type {
            machine,
            controller,
            driver,
            function,
            element,
            port
        };

        struct entry {
            int type;
            int index;
            int mode;

            bool operator==(const entry& other) const {
                return type == other.type && index == other.index && mode == other.mode;
            }
        };

        class history {
        public:
            history() {
                push({ 0, -1, 0 });
            }

            void push(const entry& entry) {
                if (!history_deque.empty() && history_deque.back() == entry) {
                    return;
                }
                if (!history_deque.empty() && history_deque.back().type == entry.type) {
                    if (history_deque.back().index == -1 && entry.index != -1) {
                        history_deque.back() = entry;
                        forward_stack.clear();
                        return;
                    }
                    else if (entry.index == -1 && history_deque.back().index != -1) {
                        return;
                    }
                }
                if (history_deque.size() == max_history_size) {
                    history_deque.pop_front();
                }
                history_deque.push_back(entry);
                forward_stack.clear();
            }

            void delete_entry(int type, int index, int mode) {
                auto remove_and_adjust = [&](std::deque<entry>& entries) {
                    entries.erase(std::remove_if(entries.begin(), entries.end(), [=](const entry& entry) {
                        return entry.type == type && entry.index == index && entry.mode == mode;
                        }), entries.end());

                    for (auto& entry : entries) {
                        if (entry.type == type && entry.index > index) {
                            entry.index -= 1;
                        }
                    }
                    };

                remove_and_adjust(history_deque);
                remove_and_adjust(forward_stack);
            }

            entry most_recent_for_type(int type) const {
                for (auto it = history_deque.rbegin(); it != history_deque.rend(); ++it) {
                    if (it->type == type && it->index != -1) {
                        return *it;
                    }
                }
                return { type, -1, 0 };
            }

            entry most_recent_for_mode(int mode) const {
                for (auto it = history_deque.rbegin(); it != history_deque.rend(); ++it) {
                    if (it->mode == mode && it->index != -1) {
                        return *it;
                    }
                }
                return { mode, -1, 0 };
            }

            bool back(entry& entry) {
                if (history_deque.size() <= 1) return false;
                forward_stack.push_back(history_deque.back());
                history_deque.pop_back();
                entry = history_deque.back();
                return true;
            }

            bool forward(entry& entry) {
                if (forward_stack.empty()) return false;
                history_deque.push_back(forward_stack.back());
                entry = forward_stack.back();
                forward_stack.pop_back();
                return true;
            }

            bool can_go_back() const {
                return history_deque.size() > 1;
            }

            bool can_go_forward() const {
                return !forward_stack.empty();
            }

            void clear() {
                history_deque.clear();
                forward_stack.clear();
            }

            void save(const std::filesystem::path& engine_path) const {
                std::filesystem::path user_path = engine_path;
                user_path += ".user";

                nlohmann::json data;
                for (const auto& entry : history_deque) {
                    data.push_back({ {"type", entry.type}, {"index", entry.index}, {"mode", entry.mode} });
                }

                std::ofstream out_file(user_path);
                if (out_file.is_open()) {
                    out_file << data.dump(4);
                    out_file.close();
                }
            }

            void load(const std::filesystem::path& engine_path) {
                std::filesystem::path user_path = engine_path;
                user_path += ".user";

                std::ifstream in_file(user_path);
                if (in_file.is_open()) {
                    try {
                        nlohmann::json data;
                        in_file >> data;
                        clear();
                        for (const auto& item : data) {
                            history_deque.push_back({
                                item.value("type", 0),
                                item.value("index", -1),
                                item.value("mode", 0)
                                });
                        }
                    }
                    catch (const nlohmann::json::exception& e) {
                        ui::cout << "User settings failed to load: " << e.what() << ui::endl;
                    }
                    in_file.close();
                }
            }

            const entry& current() const { return history_deque.back(); }

        private:
            std::deque<entry> history_deque;
            std::deque<entry> forward_stack;
        };



        inline ui::navigation::history navigation_history;

        inline void load(const std::filesystem::path& engine_path) {
            navigation_history.load(engine_path);
        }

        inline void save(const std::filesystem::path& engine_path) {
            navigation_history.save(engine_path);
        }

        inline void push(const entry& entry) {
            navigation_history.push(entry);
        }

        inline void delete_entry(int type, int index, int mode) {
            navigation_history.delete_entry(type, index, mode);
        }

        inline const entry& current_entry() {
            return navigation_history.current();
        }

        inline bool can_go_back() {
            return navigation_history.can_go_back();
        }

        inline bool can_go_forward() {
            return navigation_history.can_go_forward();
        }

        inline bool back(entry& e) {
            return navigation_history.back(e);
        }

        inline bool forward(entry& e) {
            return navigation_history.forward(e);
        }


        inline entry most_recent_for_type(int type) {
            return navigation_history.most_recent_for_type(type);
        }

        inline entry most_recent_for_mode(int mode) {
            return navigation_history.most_recent_for_mode(mode);
        }

    }

} // namespace ui
