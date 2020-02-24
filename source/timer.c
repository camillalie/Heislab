#include <time.h>
#include <stdio.h>

clock_t static start_time;

void timer_start(){
    start_time = clock();
}

int timer_less_than(int sec){
    if(((clock()-start_time)/CLOCKS_PER_SEC) >= sec){
        return 0;
    }
    return 1;
}