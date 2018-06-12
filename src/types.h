/*
 * types.h
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

#ifndef SRC_TYPES_H_
#define SRC_TYPES_H_

#define UNUSED __attribute__((unused))
#define NORETURN __attribute__((noreturn))

// portable thread local storage - thanks SO!
// https://stackoverflow.com/questions/18298280/how-to-declare-a-variable-as-thread-local-portably
#ifdef __GNUC__
#define TLOCAL __thread
#elif __STDC_VERSION__ >= 201112L
#define TLOCAL _Thread_local
#elif defined(_MSC_VER)
#define TLOCAL __declspec(thread)
#else
#error Cannot define thread_local
#endif

#endif /* SRC_TYPES_H_ */
