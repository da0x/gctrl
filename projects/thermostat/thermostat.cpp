//
// This file is part of gctrl.
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

#include <gctrl.hpp>
using namespace gctrl;

#include <machine/thermostat.hpp>

int main() {
    std::signal(SIGTERM, gctrl::handle_signal);
    std::signal(SIGINT, gctrl::handle_signal);

    machine::thermostat thermostat;
    gctrl::timer timer;

    while (!terminate_requested) {
        timer.update();

        if (timer.timestep_1hz.seconds() >= 1.0) {
            thermostat.at_1hz(timer.timestep_1hz.seconds());
            timer.timestep_1hz.reset();
        }

        if (timer.timestep_10hz.milliseconds() >= 100.0) {
            thermostat.at_10hz(timer.timestep_10hz.seconds());
            timer.timestep_10hz.reset();
        }

        if (timer.timestep_100hz.milliseconds() >= 10.0) {
            thermostat.at_100hz(timer.timestep_100hz.seconds());
            timer.timestep_100hz.reset();
        }

        if (timer.timestep_1khz.milliseconds() >= 1.0) {
            thermostat.at_1khz(timer.timestep_1khz.seconds());
            timer.timestep_1khz.reset();
        }

        if (timer.timestep_10khz.microseconds() >= 100.0) {
            thermostat.at_10khz(timer.timestep_10khz.seconds());
            timer.timestep_10khz.reset();
        }

        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }

    std::cout << "Terminating gracefully..." << std::endl;
    return 0;
}
