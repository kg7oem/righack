/*
 * config.h
 *
 *  Created on: Jun 1, 2018
 *      Author: tyler
 */

#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

void config_load(char *);

int config_count_vserial(void);
const char * config_get_vserial_name(int);

#endif /* SRC_CONFIG_H_ */
