/*
 * driver.h
 *
 *  Created on: Jun 1, 2018
 *      Author: tyler
 */

#ifndef SRC_DRIVER_H_
#define SRC_DRIVER_H_

#include "vserial.h"

typedef void (*driver_init_handler)(VSERIAL *);
typedef void (*driver_cleanup_handler)(VSERIAL *);

struct driver_info {
    struct vserial_handlers vserial;
    driver_init_handler init;
    driver_cleanup_handler cleanup;
};

#endif /* SRC_DRIVER_H_ */
