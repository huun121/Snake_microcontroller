/*
 * common.h
 *
 *  Created on: 12. 11. 2022
 *      Author: Pavel Hurdalek (xhurda01)
 */

#ifndef SOURCES_COMMON_H_
#define SOURCES_COMMON_H_

typedef struct point_t {
	int x;
	int y;
} point_t;

typedef enum {
  RIGHT,
  UP,
  LEFT,
  DOWN
} direction_t;

/*
 * Returns 1 if can change direction and 0 if can not
 */
int can_change_direction (direction_t current, direction_t next);

/**
 * Variable delay loop
 */
void delay(int t1, int t2);

#endif /* SOURCES_COMMON_H_ */
