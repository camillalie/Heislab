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

static State current_state;
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

void poll_floor_sensors(){
    for(int f = 0; f < HARDWARE_NUMBER_OF_FLOORS; f++){
        if(hardware_read_floor_sensor(f)){
            current_floor = f;
            hardware_command_floor_indicator_on(f);
        }
    }
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
    while (hardware_read_floor_sensor(0) == 0){ 
        hardware_command_movement(HARDWARE_MOVEMENT_DOWN);
    }
    hardware_command_movement(HARDWARE_MOVEMENT_STOP);
    current_floor = 0;
    current_direction = HARDWARE_MOVEMENT_DOWN;
    hardware_command_floor_indicator_on(0);
    timer_start();
    current_state = OPEN_DOOR;
    hardware_command_door_open(1);
}

void turn_off_lights(int floor){
    for (HardwareOrder f = HARDWARE_ORDER_UP; f <= HARDWARE_ORDER_DOWN; f++){
        hardware_command_order_light(floor, f, 0);
    }
}


int main(){
    int error = hardware_init();
    if(error != 0){
        fprintf(stderr, "Unable to initialize hardware\n");
        exit(1);
    }

    signal(SIGINT, sigint_handler);
    clear_all_order_lights();
    go_to_first_floor();
    while(1){
        switch (current_state)
        {
        case STANDBY:
            if(emergency_stop()){
                break;
            }
            poll_order();
            if (current_direction == HARDWARE_MOVEMENT_UP){
                if (queue_order_above(current_floor)){
                    hardware_command_movement(HARDWARE_MOVEMENT_UP);
                    current_state = DRIVING;
                    break;
                }
                if (queue_order_below(current_floor)){
                    hardware_command_movement(HARDWARE_MOVEMENT_DOWN);
                    current_direction = HARDWARE_MOVEMENT_DOWN;
                    current_state = DRIVING;
                    break;
                }
                }
                if (current_direction == HARDWARE_MOVEMENT_DOWN){
                    if(queue_order_below(current_floor)){
                        hardware_command_movement(HARDWARE_MOVEMENT_DOWN);
                        current_state = DRIVING;
                        break;
                    }
                    if (queue_order_above(current_floor)) {
                        hardware_command_movement(HARDWARE_MOVEMENT_UP);
                        current_state = DRIVING;
                        current_direction = HARDWARE_MOVEMENT_UP;
                        break;
                    }
                }
            break;
        case DRIVING:
            if(emergency_stop()){
                break;
            }
            poll_order();
            poll_floor_sensors();
            if (queue_order_at(current_floor, HARDWARE_MOVEMENT_UP)){
                hardware_command_movement(HARDWARE_MOVEMENT_STOP);
                queue_delete_element(current_floor);
                turn_off_lights(current_floor);
                hardware_command_door_open(1);
                timer_start();
                current_state = OPEN_DOOR;
                break;
            }
            if (queue_order_at(current_floor,HARDWARE_MOVEMENT_DOWN)){
                hardware_command_movement(HARDWARE_MOVEMENT_STOP);
                queue_delete_element(current_floor);
                turn_off_lights(current_floor);
                hardware_command_door_open(1);
                timer_start();
                current_state = OPEN_DOOR;
                break;
            }

            break;
        case OPEN_DOOR:
            poll_order();
            if(emergency_stop()){
                break;
            }
            if(hardware_read_obstruction_signal()){
                timer_start();
                break;
            }
            if(timer_less_than(3)){
                hardware_command_door_open(0);  
                current_state = STANDBY;
                break;
            }
            break;
        case EMERGENCY: 
            queue_delete_all();
            for(int f = 0; f<HARDWARE_NUMBER_OF_FLOORS; f++){
                for(HardwareOrder i = HARDWARE_ORDER_UP; i<= HARDWARE_ORDER_DOWN; i++){
                    hardware_command_order_light(f,i,0);
                }
            }
            if(hardware_read_floor_sensor(current_floor)){
                hardware_command_door_open(1);
                if (hardware_read_stop_signal()== 0){
                    timer_start();
                    current_state = OPEN_DOOR;
                    hardware_command_stop_light(0);
                    break;
                }
            }          
            if (hardware_read_stop_signal()== 0){
                hardware_command_door_open(0);
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