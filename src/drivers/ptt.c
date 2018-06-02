/*
 * ptt.c
 *
 *  Created on: Jun 2, 2018
 *      Author: tyler
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <hamlib/rig.h>
#include <stdio.h>

#include "../driver.h"
#include "../util.h"

static void
ptt_init_handler(UNUSED VSERIAL *vserial) {
    printf("  Inside ptt_init_handler\n");
}

static void
ptt_cleanup_handler(UNUSED VSERIAL *vserial) {
    printf("  Inside ptt_cleanup_handler\n");
}

static void
ptt_control_line_handler(UNUSED VSERIAL *vserial, UNUSED struct vserial_control_line *control_line) {
    printf("  Inside ptt_control_line_handler\n");
}

struct driver_info *
ptt_driver_info(void) {
    struct driver_info info = {
            .init = ptt_init_handler,
            .cleanup = ptt_cleanup_handler,
    };

    info.vserial.control_line = ptt_control_line_handler;

    return util_memdup(&info, sizeof(info));
}
