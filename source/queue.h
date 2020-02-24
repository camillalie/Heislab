#include "hardware.h"

void queue_set_order(int floor, HardwareOrder order);
int queue_order_above(int floor, HardwareMovement direction);
int queue_order_below(int floor, HardwareMovement direction);
int queue_order_at(int floor, HardwareMovement direction);
void queue_delete_element(int floor);
void queue_delete_all();