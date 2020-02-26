#include "queue.h"

static int order_up[HARDWARE_NUMBER_OF_FLOORS];

static int order_down[HARDWARE_NUMBER_OF_FLOORS];

void queue_set_order(int floor, HardwareOrder order){
    if (order == HARDWARE_ORDER_INSIDE){
        order_up[floor] = 1;
        order_down[floor] = 1;
    }
    if (order == HARDWARE_ORDER_UP){
        order_up[floor] = 1;
    }
    if (order == HARDWARE_ORDER_DOWN){
        order_down[floor] = 1;
    }
}

int queue_order_above(int floor){
    for (int f = floor; f < HARDWARE_NUMBER_OF_FLOORS; f++){
        if (order_up[f] || order_down[f]){
            return 1;
        }
    }
    return 0;
}

int queue_order_below(int floor){
    for (int f = floor; f>= 0; f--){
        if (order_up[f] || order_down[f]){
            return 1;
        }
    }
    return 0;
}

int queue_order_at(int floor, HardwareMovement direction){
    if (direction == HARDWARE_MOVEMENT_UP){
        return order_up[floor];
    }
    if (direction == HARDWARE_MOVEMENT_DOWN){
        return order_down[floor];
    }
    return 0;
}

void queue_delete_element(int floor){
    order_up[floor]=0;
    order_down[floor]=0;
}

void queue_delete_all(){
    for (int i = 0; i < HARDWARE_NUMBER_OF_FLOORS; i++){
        order_up[i]= 0;
        order_down[i]=0;
    }
}
