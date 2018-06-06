/*
 * interface.h
 *
 *  Created on: Jun 4, 2018
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

#ifndef SRC_INTERFACE_H_
#define SRC_INTERFACE_H_

#define INTERFACE_CALL(interface, operation, ...) interface->op.operation(interface, __VA_ARGS__)
#define DRIVER_CALL(driver, operation, ...) driver->interface.op.operation(driver, __VA_ARGS__);

struct interface_stream {
    // these are events which indicate that something already happened
    // so the names are passed tense adjectives
    struct {
        void *opened;
        void *closed;
        void *faulted;
        void *received; // data has been read and is available
        void *drained; // there is room in the output buffer
    } cb;

    // these are operations that will do something so the names
    // are verbs or start with a verb
    struct {
        // FIXME Is this a constant pointer such that the pointer value
        // can not change? It is good to have users not able to change
        // the operation pointers - if this is a constant pointer then
        // does the value have to be defined when the struct is allocated?
        void const *terminate;
        // control notifications for events based on a bitmask
        void const *get_mask;
        void const *clear_mask;
        void const *set_mask;
    } op;
};

struct interface_message;

struct interface_rs232 {

};

struct interface {
    struct interface_stream *stream;
    struct interface_message *message;
    struct interface_rs232 *rs232;
};

struct driver {
    const char *name;
    void *private;
    struct interface interface;
};


#endif /* SRC_INTERFACE_H_ */
