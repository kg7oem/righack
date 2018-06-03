/*
 * guts.h
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

#ifndef SRC_GUTS_H_
#define SRC_GUTS_H_

#include "types.h"

enum exit_value {
    exit_ok = 0,
    exit_fatal = 1,
    exit_args = 10,
    exit_nomem = 100,
};

void guts_exit(enum exit_value);

#endif /* SRC_GUTS_H_ */
