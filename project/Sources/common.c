/*
 * common.c
 *
 *  Created on: 12. 11. 2022
 *      Author: Pavel Hurdalek (xhurda01)
 */

#include "common.h"

int can_change_direction (direction_t current, direction_t next)
{
	return (current + next) % 2;
}

void delay(int t1, int t2)
{
	int i, j;

	for(i=0; i<t1; i++) {
		for(j=0; j<t2; j++);
	}
}




