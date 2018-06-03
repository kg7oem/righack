/*
 * configfile.h
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

#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include <inttypes.h>
#include <stdbool.h>

void configfile_load(char *);

int configfile_count_vserial(void);
const char * configfile_get_vserial_name(int);
const char * configfile_get_vserial_driver(int);


const char * configfile_get_section_name(int);
const char * configfile_gets_section_key(const char *, const char *);
const char * configfile_rgets_section_key(const char *, const char *);
bool configfile_geti_section_key(const char *, const char *, int64_t *);
int64_t configfile_rgeti_section_key(const char *, const char *);


#endif /* SRC_CONFIG_H_ */
