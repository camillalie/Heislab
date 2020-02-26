#ifndef TIMER_H
#define TIMER_H
/**
 * @file
 * @brief Contains timer functions for use in opening/closing the elevator door
 */

/**
 * @brief starts the timer.
 */
void timer_start();

/**
 * @brief Function checking if time passed is more than its treshold
 * @param sec Decides how many seconds function should last
 * @return 1 (true) if time passed is more than @p sec, 0 (false) else
 */
int timer_less_than(int sec);

#endif