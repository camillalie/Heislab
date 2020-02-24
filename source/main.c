#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "hardware.h"
#include "queue.h"
#include "timer.h"

typedef enum {
    STANDBY,
    DRIVING,
    OPEN_DOOR,
    EMERGENCY,
} State;

static State current_state = STANDBY;
static int current_floor;
static HardwareMovement current_direction;

void poll_order(){
    for(int f = 0; f<HARDWARE_NUMBER_OF_FLOORS; f++){
        for(HardwareOrder i = HARDWARE_ORDER_UP; i<= HARDWARE_ORDER_DOWN; i++){
            if(hardware_read_order(f,i)){
                queue_set_order(f,i);
                hardware_command_order_light(f,i, 1);
            }
        }
    }
}

int arrive_at_destination(int floor){
    int temporary_floor = hardware_read_floor_sensor(floor);
    if (temporary_floor){
        hardware_command_floor_indicator_on(floor);
        return 1;
    }
    return 0;   
}


static void clear_all_order_lights(){
    HardwareOrder order_types[3] = {
        HARDWARE_ORDER_UP,
        HARDWARE_ORDER_INSIDE,
        HARDWARE_ORDER_DOWN
    };

    for(int f = 0; f < HARDWARE_NUMBER_OF_FLOORS; f++){
        for(int i = 0; i < 3; i++){
            HardwareOrder type = order_types[i];
            hardware_command_order_light(f, type, 0);
        }
    }
}

static void sigint_handler(int sig){
    (void)(sig);
    printf("Terminating elevator\n");
    hardware_command_movement(HARDWARE_MOVEMENT_STOP);
    exit(0);
}

int emergency_stop(){
    if(hardware_read_stop_signal()){
        hardware_command_movement(HARDWARE_MOVEMENT_STOP);
        hardware_command_stop_light(1);
        current_state = EMERGENCY;
        return 1;
    }
    return 0;
}

void go_to_first_floor(){
    while (hardware_read_floor_sensor(0) != 1){ 
        hardware_command_movement(HARDWARE_MOVEMENT_DOWN)
    }
    hardware_command_movement(HARDWARE_MOVEMENT_STOP);
    current_floor = 0;
    current_state = STANDBY;
    current_direction = HARDWARE_MOVEMENT_DOWN;
}


int main(){
    int error = hardware_init();
    if(error != 0){
        fprintf(stderr, "Unable to initialize hardware\n");
        exit(1);
    }

    signal(SIGINT, sigint_handler);
    go_to_first_floor();
    while(1){
        switch (current_state)
        {
        case STANDBY:
            poll_order();
            if(emergency_stop()){
                break;
            }
            if (current_direction == HARDWARE_MOVEMENT_UP){ 
                if(queue_order_above()){
                    hardware_command_movement(HARDWARE_MOVEMENT_UP);
                    current_state = DRIVING;
                    break;
                }
                if(queue_order_below()){
                    hardware_command_movement(HARDWARE_MOVEMENT_DOWN);
                    current_direction = HARDWARE_MOVEMENT_DOWN;
                    current_state = DRIVING;
                    break;
                }
            }
            if (current_direction == HARDWARE_MOVEMENT_DOWN){
                if(queue_order_below()){
                    hardware_command_movement(HARDWARE_MOVEMENT_DOWN);
                    current_state = DRIVING;
                    break;
                }
                if(queue_order_above()){
                    hardware_command_movement(HARDWARE_MOVEMENT_UP);
                    current_direction = HARDWARE_MOVEMENT_UP;
                    current_state = DRIVING;
                    break;
                }
            }
            break;
        case DRIVING:
            poll_order();
            if(emergency_stop()){
                break;
            }
            for (int i = current_floor; i < HARDWARE_NUMBER_OF_FLOORS ; i ++){
                if (current_direction = HARDWARE_MOVEMENT_UP){
                    if (queue_order_at(i, HARDWARE_MOVEMENT_UP) && arrive_at_destination(i)){
                        hardware_command_movement(HARDWARE_MOVEMENT_STOP);
                        queue_delete_element(i);
                        hardware_command_order_light(0);
                        hardware_command_floor_indicator_on(i);
                        hardware_command_door_open(1);
                        timer_start();
                        current_floor = i;
                        current_state = OPEN_DOOR;
                        break;
                    }
                }
            }
            for (int i = current_floor; i <= 0; i--){
                if (current_direction == HARDWARE_MOVEMENT_DOWN){
                    if (queue_order_at(i,HARDWARE_MOVEMENT_DOWN) && arrive_at_destination(i)){
                        hardware_command_movement(HARDWARE_MOVEMENT_STOP);
                        queue_delete_element(i);
                        hardware_command_order_light(0);
                        hardware_command_floor_indicator_on(i);
                        hardware_command_door_open(1);
                        timer_start();
                        current_floor = i;
                        current_state = OPEN_DOOR;
                        break;
                    }
                } 
            }
            break;
        case OPEN_DOOR:
            poll_order();
            if(emergency_stop()){
                break;
            }
            if(hardware_read_obstruction_signal(){
                timer_start();
                 break; //er denne nødvendig?
            }
             if(timer_less_than(3)){
                hardware_command_door_open(0);  
                current_state = STANDBY;
                break;
            }
            break;
        case EMERGENCY: // Koffor funker ikkje denne?
            queue_delete_all();
            for(int f = 0; f<HARDWARE_NUMBER_OF_FLOORS; f++){
                for(HardwareOrder i = HARDWARE_ORDER_UP; i<= HARDWARE_ORDER_DOWN; i++){
                    hardware_command_order_light(f,i,0);
                }
            }
            if (hardware_read_stop_signal()== 0){
                current_state = STANDBY;
                hardware_command_stop_light(0);
                break;
            }
        break;
           default:
            break;
        }
    }
    return 0;
}