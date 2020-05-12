/*
 * version.h
 *
 *  Created on: Sep 17, 2018
 *      Author: meir
 */

#ifndef SRC_VERSION_H_
#define SRC_VERSION_H_

#define REDISGEARS_VERSION_MAJOR 1
#define REDISGEARS_VERSION_MINOR 0
#define REDISGEARS_VERSION_PATCH 0

#define STR1(a) #a
#define STR(e) STR1(e)

#define REDISGEARS_MODULE_VERSION \
  (REDISGEARS_VERSION_MAJOR * 10000 + REDISGEARS_VERSION_MINOR * 100 + REDISGEARS_VERSION_PATCH)

#define REDISGEARS_VERSION_STR STR(REDISGEARS_VERSION_MAJOR) "." STR(REDISGEARS_VERSION_MINOR) "." STR(REDISGEARS_VERSION_PATCH)

/* API versions. */
#define REDISMODULE_APIVER_1 1

#define REDISGEARS_DATATYPE_VERSION 1
#define REDISGEARS_DATATYPE_NAME "GEARS_DT0"

#define REDISGEARS_MODULE_NAME "rg"

#endif /* SRC_VERSION_H_ */
