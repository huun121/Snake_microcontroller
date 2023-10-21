/*
 * snake.h
 *
 *  Created on: 12. 11. 2022
 *      Author: Pavel Hurdalek (xhurda01)
 */

#ifndef SOURCES_SNAKE_H_
#define SOURCES_SNAKE_H_

#include "common.h"

typedef struct snake_part_t {
	point_t axis;
	struct snake_part_t* next;
} snake_part_t;

typedef struct snake_t {
	snake_part_t* head;
	snake_part_t* print_part;
	direction_t direction;
} snake_t;

void snake_ctor (snake_t* snake);

void snake_parts_init (snake_part_t parts[], int lenght);

void snake_insert_parts (snake_t* snake, snake_part_t parts[], int length);

void snake_get_point (snake_t* snake, point_t* point);

/**
 * Returns 1 if snake collides after move, else 0
 */
int snake_move (snake_t* snake, int length);

void snake_change_direction (snake_t *snake, direction_t new_direction);

#endif /* SOURCES_SNAKE_H_ */
