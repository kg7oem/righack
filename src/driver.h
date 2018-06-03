/*
 * driver.h
 *
 *  Created on: Jun 1, 2018
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

#ifndef SRC_DRIVER_H_
#define SRC_DRIVER_H_

#include "vserial.h"

typedef void (*driver_init_handler)(VSERIAL *, const char *);
typedef void (*driver_cleanup_handler)(VSERIAL *);

struct driver_info {
    struct vserial_handlers vserial;
    driver_init_handler init;
    driver_cleanup_handler cleanup;
};

#endif /* SRC_DRIVER_H_ */
