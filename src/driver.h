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

/*
 * Drivers add support for things like serial ports, TCP/IP sockets, etc.
 *
 * To keep implementation details for the drivers away from users of the
 * drivers there is a set of driver interfaces which are defined here.
 *
 * Every interface has a name and a structure that describes the interface
 * operations and callbacks. Operations are functions that the user of a driver
 * can invoke and callbacks are handlers for events that the driver will invoke
 * as needed. Operations and callbacks are all function pointers. The operation
 * pointers are filled out by the driver during instantiation and the callbacks
 * are filled out by the user to register event handlers.
 *
 * For example there is a structure named interface_stream and
 * another one named interface_rs232.
 *
 * Every interface that exists in the system also has a member in a structure
 * called interface. The member for each interface is a pointer to the structure
 * for that interface. When a driver is being setup it will allocate a interface
 * structure and populate the pointers for the interface that it implements then
 * return this pointer to what ever function is instantiating the driver.
 *
 * Support for an interface is defined as having a non-null pointer for the
 * interface in the interface structure. Support for an operation is indicated
 * by a non-full function pointer in the operations structure.
 *
 * This allows each driver to implement one or more interfaces
 *
 */

#ifndef SRC_DRIVER_H_
#define SRC_DRIVER_H_

#include <inttypes.h>
#include <stdbool.h>

#define DRIVER_LIFECYCLE(name) { .bootstrap = name##_lifecycle_bootstrap, .init = name##_lifecycle_init, .cleanup = name##_lifecycle_cleanup }
#define DRIVER_CALL_OP(driver, interface, operation, ...) driver->info->op.interface.operation(driver,##__VA_ARGS__)
#define DRIVER_CALL_CB(driver, interface, cbname, ...) if (driver->cb->interface.cbname != NULL) { driver->cb->interface.cbname(driver,##__VA_ARGS__); }

struct driver;
struct driver_info;

enum driver_status {
    driver_status_unstarted = 0,
    driver_status_starting,
    driver_status_started,
    driver_status_stopping,
    driver_status_stopped,
};

typedef void (*driver_cb)(struct driver *);
typedef void (*driver_stream_terminate_op)(struct driver *);
typedef uint32_t (*driver_stream_get_mask_op)(struct driver *);
typedef void (*driver_stream_set_mask_op)(struct driver *, uint32_t);
typedef void (*driver_stream_clear_mask_op)(struct driver *, uint32_t);

// these are events which indicate that something already happened
// so the names are passed tense adjectives
struct driver_stream_int_cb {
    driver_cb opened;
    driver_cb closed;
    driver_cb faulted;
    driver_cb received; // data has been read and is available
    driver_cb drained; // there is room in the output buffer
};

struct driver_stream_int_op {
    driver_stream_terminate_op terminate;
    // control notifications for events based on a bitmask
    driver_stream_get_mask_op get_mask;
    driver_stream_clear_mask_op clear_mask;
    driver_stream_set_mask_op set_mask;
};

struct driver_rs232_fc {
    bool rts;
    bool cts;
    bool dtr;
    bool dsr;
};

typedef void (*driver_rs232_fc_changed_cb)(struct driver *, struct driver_rs232_fc *);

struct driver_rs232_int_cb {
    driver_rs232_fc_changed_cb fc_changed;
};

struct driver_interface_op {
    const struct driver_stream_int_op stream;
};

struct driver_interface_cb {
    struct driver_stream_int_cb stream;
    struct driver_rs232_int_cb rs232;
};

typedef void (*driver_bootstrap_handler)(void);
typedef void (*driver_init_handler)(struct driver *, const char *);
typedef void (*driver_cleanup_handler)(struct driver *);

struct driver_lifecycle_op {
    driver_bootstrap_handler bootstrap;
    driver_init_handler init;
    driver_cleanup_handler cleanup;
};

struct driver_info {
    const char *name;
    // FIXME this should be private / opaque to the user
    const struct driver_lifecycle_op lifecycle;
    const struct driver_interface_op op;
};

struct driver {
    void *user;
    void *private;
    const struct driver_info *info;
    struct driver_interface_cb *cb;
    // FIXME this should be private / opaque to the user
    enum driver_status status;
};

struct driver * driver_create(const char *, const char *);
void driver_bootstrap(void);
void driver_destroy(struct driver *, void *notify_cb);
void driver_load_plugins(void);

const char * driver_get_status_string(enum driver_status status);
enum driver_status driver_get_status(struct driver *);
enum driver_status driver_set_status(struct driver *, enum driver_status);

#endif /* SRC_DRIVER_H_ */
