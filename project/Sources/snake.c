/*
 * snake.c
 *
 *  Created on: 12. 11. 2022
 *      Author: Pavel Hurdalek (xhurda01)
 */

#include "snake.h"

void snake_ctor (snake_t* snake) {
	snake->head = 0;
	snake->print_part = 0;
	snake->direction = RIGHT;
}

void snake_parts_init (snake_part_t parts[], int length) {
	for (int i = length - 1; i >= 0; i--) {
		parts[length - 1 - i].axis.x = 7 - i;
		parts[length - 1 - i].axis.y = 4;
	}
}

void snake_insert_parts (snake_t* snake, snake_part_t parts[], int length) {
	snake->print_part = &parts[0];

	for (int i = 0; i < length; i++) {
		snake->head = &parts[i];
		snake_part_t* p = snake->head;
		p->next = &parts[(i + 1) % length];
	}

	snake->head = &parts[length - 1];
}

void snake_get_point (snake_t* snake, point_t* point) {
	snake_part_t* print_part = snake->print_part;

	snake->print_part = print_part->next;

	point_t print_point = print_part->axis;

	point->x = print_point.x;
	point->y = print_point.y;
}

int snake_collision (snake_t* snake, int length) {
	snake_part_t* head = snake->head;
	snake_part_t* tmp = head->next;

	for (int i = 0; i < length - 1; i++) {
		if (tmp->axis.x == head->axis.x && tmp->axis.y == head->axis.y) {
			return 1;
		}
	}
	return 0;
}

int snake_move (snake_t* snake, int length) {
	snake_part_t* head = snake->head;

	// circular list -> head.next == tail
	snake_part_t* tail = head->next;

	point_t new_head = head->axis;

	switch (snake->direction) {
		case RIGHT:
			tail->axis.x = (new_head.x + 1) % 16;
			tail->axis.y = new_head.y;
			break;
		case UP:
			tail->axis.x = new_head.x;
			int y = (new_head.y - 1) % 8;
			tail->axis.y = y < 0 ? y + 8 : y;
			break;
		case LEFT:;
			int x = (new_head.x - 1) % 16;
			tail->axis.x = x < 0 ? x + 16 : x;
			tail->axis.y = new_head.y;
			break;
		case DOWN:
			tail->axis.x = new_head.x;
			tail->axis.y = (new_head.y + 1) % 8;
			break;

		// shouldn't be
		default:
			break;
	}
	snake->head = head->next;
	snake->print_part = snake->head;

	return snake_collision(snake, length);
}

void snake_change_direction (snake_t *snake, direction_t new_direction) {
	if (can_change_direction(snake->direction, new_direction)) {
		snake->direction = new_direction;
	}
}
