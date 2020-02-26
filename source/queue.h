/**
 * @file
 * @brief Controlls the order queue
 */

#include "hardware.h"

/**
 * @brief Add orders to queue
 * @param floor which floor there are added a command in. Tells what element in
 * array that should be high(1).
 * @param order which direction. Tells what array to put the order in.
 */
void queue_set_order(int floor, HardwareOrder order);

/** 
 * @brief checks if there is any order above.
 * @param floor Which floor we are in.
 * @return true(1) or false(0).
 */
int queue_order_above(int floor);

/**
 * @brief checks if there is any order below.
 * @param floor Which floor we are in.
 * @return true(1) or false(0).
 */
int queue_order_below(int floor);

/** 
 * @brief checks if there is a order at @p floor
 * @param floor Which floor we are in.
 * @param direction Which direction we are in.
 * @return true(1) or false(0).
 */
int queue_order_at(int floor, HardwareMovement direction);

/**
 * @brief deletes element from both arrays.
 * @param floor Which floor we are in.
 */
void queue_delete_element(int floor);

/**
 * @brief deletes all elements from array
 */
void queue_delete_all();
