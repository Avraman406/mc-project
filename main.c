/*
 * File:   main.c
 * Author: Sai Teja
 * Title: CAR BLACK BOX
 * Created on 10 August, 2023, 8:11 AM
 */


#include <xc.h>
#include "clcd.h"
#include "main.h" 
#include "matrix_keypad.h"
#include "adc.h"                              
#include "i2c.h"
#include "ds1307.h"
#include "eeprom.h"
#include "uart.h"


void get_time(void);                          
unsigned char clock_reg[3];
unsigned char time[9];
unsigned char event[][3] = {"ON", "GR", "GN", "G1", "G2", "G3", "G4", "C ", "DL", "CL", "ST", "CP"};
unsigned char index = 0, speed, flag = 0;
unsigned char addr = 0x00;
unsigned char lap = 0, overflow_flag = 0;

void init_config() {
    init_adc();
    init_matrix_keypad();
    init_i2c();
    init_ds1307();
    init_clcd();
}

void main(void) {
    init_config();
    unsigned char key;
    clcd_print("TIME", LINE1(2));
    clcd_print("EVNT", LINE1(9));
    clcd_print("SP", LINE1(14));
    while (1) {
        get_time();
        clcd_print(time, LINE2(0));
        clcd_print(event[index], LINE2(10));
        speed = read_adc(CHANNEL4) / 10.33;
        clcd_putch(speed / 10 + 48, LINE2(14));
        clcd_putch(speed % 10 + 48, LINE2(15));
        key = read_switches(STATE_CHANGE);
        if (key == MK_SW1) {
            index = 7;
            flag = 0;
        }
        else if (key == MK_SW2) {
            if (index < 6) {
                flag = 0;
                index++;
            } else if (index == 7) {
                flag = 0;
                index = 2;
            }
        } else if (key == MK_SW3) {
            if (index > 1 && index < 7) {
                flag = 0;
                index--;
            } else if (index == 7) {
                flag = 0;
                index = 2;
            }
        } else
            flag = 1;
        if (flag == 0) {
            store_in_eeprom();
        }
        if (key == MK_SW5) {
            CLEAR_DISP_SCREEN;
            init_password();
        }
    }
    return;
}

void get_time(void) {
    clock_reg[0] = read_ds1307(HOUR_ADDR);
    clock_reg[1] = read_ds1307(MIN_ADDR);
    clock_reg[2] = read_ds1307(SEC_ADDR);

    if (clock_reg[0] & 0x40) {
        time[0] = '0' + ((clock_reg[0] >> 4) & 0x01);
        time[1] = '0' + (clock_reg[0] & 0x0F);
    } else {
        time[0] = '0' + ((clock_reg[0] >> 4) & 0x03);
        time[1] = '0' + (clock_reg[0] & 0x0F);
    }
    time[2] = ':';
    time[3] = '0' + ((clock_reg[1] >> 4) & 0x0F);
    time[4] = '0' + (clock_reg[1] & 0x0F);
    time[5] = ':';
    time[6] = '0' + ((clock_reg[2] >> 4) & 0x0F);
    time[7] = '0' + (clock_reg[2] & 0x0F);
    time[8] = '\0';
}

void store_in_eeprom(void) {
    CLEAR_DISP_SCREEN;
    unsigned char arr[10];
    arr[0] = time[0];
    arr[1] = time[1];
    arr[2] = time[3];
    arr[3] = time[4];
    arr[4] = time[6];
    arr[5] = time[7];
    arr[6] = event[index][0];
    arr[7] = event[index][1];
    arr[8] = speed / 10 + 48;
    arr[9] = speed % 10 + 48;
    for (int i = 0; i < 10; i++) {
        write_ext_eeprom(lap*10+i, arr[i]);
        clcd_putch(read_ext_eeprom(lap*10+i), LINE1(i));
    }
       if (lap == 9) {
        lap = 0;
        overflow_flag=1;
    } else {
        lap++;
    }
}