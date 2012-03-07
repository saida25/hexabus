#include "ir_receiver.h"
#include <util/delay.h>
#include "sys/clock.h"
#include "sys/etimer.h" //contiki event timer library
#include "contiki.h"
#include "hexabus_config.h"
#include "value_broadcast.h"
#include "hexonoff.h"

#if IR_RECEIVER_DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static uint32_t ir_time = 0;
static uint32_t ir_time_since_last = 0;
static uint8_t ir_data[4] = {0,0,0,0};
static uint8_t ir_state = 0;
static uint8_t ir_edge_dir = 0;
static uint8_t ir_repeat = 0;
static uint8_t ir_bit = 0;
static uint8_t ir_byte = 0;
static uint8_t ir_last_data[4];

#if IR_SERVO_ENABLE
static uint8_t servo_time = 0;
static uint8_t servo_next = 150;
static uint8_t servo_pos = IR_SERVO_INITIAL_POS;
static uint8_t servo_active = 0;
static struct ctimer servo_timeout;
#endif

void ir_receiver_init() {

    //set_outputs(128);
    EICRA |= (1<<ISC21 );
    EIMSK |= (1<<INT2 );

    TCCR0A |= (1<<WGM01);
    TIMSK0 |= (1<<OCIE0A);
    OCR0A=COMP_VAL;
    TCCR0B |= ((1<<CS01));

    process_start(&ir_receiver_process, NULL);
#if IR_SERVO_ENABLE
    process_start(&servo_process, NULL);
#if IR_SERVO_HOLD_POSITION
    servo_active = 1;
#endif
#endif
    sei();
}

void ir_receiver_reset() {
    ir_data[0] = 0;
    ir_data[1] = 0;
    ir_data[2] = 0;
    ir_data[3] = 0;
    ir_state = 0;
    EICRA &= ~((1<<ISC21)|(1<<ISC20));
    EICRA |= (1<<ISC21);
    ir_edge_dir = 0;
}

uint32_t ir_get_last_command() {
#if IR_RECEIVER_RAW_MODE
    return *(uint32_t*) ir_last_data;
#else
    switch(*(uint32_t*) ir_last_data) {
        case IR0:
            return 0x1;
            break;
        case IR1:
            return 0x2;
            break;
        case IR2:
            return 0x4;
            break;
        case IR3:
            return 0x8;
            break;
        case IR4:
            return 0x10;
            break;
        case IR5:
            return 0x20;
            break;
        case IR6:
            return 0x40;
            break;
        case IR7:
            return 0x80;
            break;
        case IR8:
            return 0x100;
            break;
        case IR9:
            return 0x200;
            break;
        case IR10:
            return 0x400;
            break;
        case IR11:
            return 0x800;
            break;
        case IR12:
            return 0x1000;
            break;
        case IR13:
            return 0x2000;
            break;
        case IR14:
            return 0x4000;
            break;
        case IR15:
            return 0x8000;
            break;
        case IR16:
            return 0x10000;
            break;
        case IR17:
            return 0x20000;
            break;
        case IR18:
            return 0x40000;
            break;
        case IR19:
            return 0x80000;
            break;
        case IR20:
            return 0x100000;
            break;
        case IR21:
            return 0x200000;
            break;
        case IR22:
            return 0x400000;
            break;
        case IR23:
            return 0x800000;
            break;
        case IR24:
            return 0x1000000;
            break;
        case IR25:
            return 0x2000000;
            break;
        case IR26:
            return 0x4000000;
            break;
        case IR27:
            return 0x8000000;
            break;
        case IR28:
            return 0x10000000;
            break;
        case IR29:
            return 0x20000000;
            break;
        case IR30:
            return 0x40000000;
            break;
        case IR31:
            return 0x80000000;
            break;
        default:
            return 0;
    }
#endif
}

PROCESS(ir_receiver_process, "Listen for IR commands");

PROCESS_THREAD(ir_receiver_process, ev, data) {

    PROCESS_BEGIN();

    while(1) {
        PROCESS_WAIT_EVENT();

        if(ev == PROCESS_EVENT_POLL) {
            if(ir_repeat) {
                PRINTF("Got repeat signal %d \n", ir_time_since_last);
                ir_repeat = 0;
            } else {
                PRINTF("Got new command %d,%d,%d,%d!\n", ir_last_data[0],ir_last_data[1],ir_last_data[2],ir_last_data[3]);
            }
            broadcast_value(30);
        }
    }

    PROCESS_END();
}

#if IR_SERVO_ENABLE
PROCESS(servo_process, "Servos");
PROCESS_THREAD(servo_process, ev ,data) {

    PROCESS_BEGIN();

    
    while(1) {
        PROCESS_WAIT_EVENT();

        if(ev == PROCESS_EVENT_POLL) {
            if(servo_next == 150) {
                toggle_outputs(128);
                servo_time = 0;
                servo_next = servo_pos;
            } else {
                toggle_outputs(128);
                servo_time = 0;
                servo_next = 150;
            }
        }
        
    }


    PROCESS_END();
}

void servo_off() {
    servo_active = 0;
}

void set_servo(uint8_t pos) {
    servo_pos = pos;
    ctimer_set(&servo_timeout, CLOCK_SECOND*3, servo_off, NULL );
    servo_active = 1;
}

uint8_t get_servo() {
    return servo_pos;
}

#endif

ISR(INT2_vect) {
    //toggle_outputs(128);
    EIMSK &= ~(1<<INT2);

    ir_time_since_last = ir_time;
    ir_time = 0;
    TCNT0 = 0;

    switch(ir_state) {
        case 0:                  //Waiting for AGC burst
            if(!ir_edge_dir) {   // Beginning of possible burst
                EICRA |= ((1<<ISC21)|(1<<ISC20));
                ir_edge_dir = 1;
            } else {             // Possible end of burst
#if IR_SAMSUNG
                if( (ir_time_since_last>43)&&(ir_time_since_last<47) ) {
#else
                if( (ir_time_since_last>88)&&(ir_time_since_last<92) ) {
#endif
                    ir_state = 1;
                    EICRA &= ~((1<<ISC21)|(1<<ISC20));
                    EICRA |= (1<<ISC21);
                    ir_edge_dir = 0;
                } else {
                    ir_receiver_reset();
                }
            }
            break;

        case 1:                  //Waiting for AFC gap
            if( (ir_time_since_last>43)&&(ir_time_since_last<47) ) {

                ir_state = 2;
                ir_bit = 0;
                ir_byte = 0;

                EICRA |= ((1<<ISC21)|(1<<ISC20));
                ir_edge_dir = 1;

            } else if( (ir_time_since_last>20)&&(ir_time_since_last<25) ) {
                ir_repeat = 1;
                process_poll(&ir_receiver_process);
                ir_receiver_reset();
            } else {
                ir_receiver_reset();
            }
            break;
        case 2:              //Waiting for bits
            if(ir_edge_dir) {
                if( ir_byte > 3) {
                    memcpy(ir_last_data, ir_data, 4);
                    process_poll(&ir_receiver_process);
                    ir_receiver_reset();
                } else if( (ir_time_since_last>5)&&(ir_time_since_last<7) ) {
                    EICRA &= ~((1<<ISC21)|(1<<ISC20));
                    EICRA |= (1<<ISC21);
                    ir_edge_dir = 0;
                } else {
                    ir_receiver_reset();
                }
            } else {
                if( (ir_time_since_last>4)&&(ir_time_since_last<6) ) {
                    ir_bit++;
                    if(ir_bit > 7) {
                        ir_bit = 0;
                        ir_byte++;
                    }
                    EICRA |= ((1<<ISC21)|(1<<ISC20));
                    ir_edge_dir = 1;
                } else if( (ir_time_since_last>15)&&(ir_time_since_last<19)) {
                    ir_data[ir_byte] |= (1<<ir_bit);
                    ir_bit++;
                    if(ir_bit > 7) {
                        ir_bit = 0;
                        ir_byte++;
                    }
                    EICRA |= ((1<<ISC21)|(1<<ISC20));
                    ir_edge_dir = 1;
                } else {
                    ir_receiver_reset();
                }
            }
            break;

        default:
            break;
    }

    EIMSK |= (1<<INT2 );
}

ISR(TIMER0_COMPA_vect) {
    ir_time++;
#if IR_SERVO_ENABLE
    servo_time++;
    if(servo_time >= servo_next && servo_active) {
        process_poll(&servo_process);
    }
#endif
}

