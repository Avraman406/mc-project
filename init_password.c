#include <xc.h>
#include "main.h"
#include "adc.h"
#include "clcd.h"
#include "matrix_keypad.h"
#include "i2c.h"
#include "ds1307.h"
#include "eeprom.h"
#include "uart.h"

void get_time(void);
unsigned char str[5];
extern unsigned char time[9];
static int count = 0;
unsigned long int delay = 0;
unsigned char flag = 0;
extern unsigned char overflow_flag=0;
unsigned char clear_flag=0;
unsigned char lap = 0;
unsigned char index,prev_index = 100; 
unsigned char speed;
unsigned long l=0;
unsigned int i=0, j=0;
unsigned char key=0;
unsigned char time_flag=0;
unsigned char n_pswd[4] = {}; 
unsigned char r_pswd[4] = {}; 
unsigned char c_pswd[4];
unsigned char pass_flag=0;
unsigned char addr=0x00;

void check_matrix_keypad(void) {
    unsigned char key;
    static unsigned int i;
    key = read_switches(STATE_CHANGE);

    if (key == MK_SW11 && count < 4) {
        str[count] = '0';
        clcd_putch('*', LINE2(count));
        count++;
    }
    if (key == MK_SW12 && count < 4) {
        str[count] = '1';
        clcd_putch('*', LINE2(count));
        count++;
    } else if (count == 4) {
        str[count] = '\0';
    }

}

void wrong_password(int attempt) {
    CLEAR_DISP_SCREEN;

    while (delay++ <= 2000) {
        clcd_print("x Try Again", LINE1(0));
        clcd_putch('0' + attempt, LINE2(0));
        clcd_print("attempt left", LINE2(2));
    }

}

int _strcmp(const char* str1, const char* str2) {
    int i = 0;
    while (str1[i] != '\0') {
        if (str1[i] != str2[i]) {
            return 1;
        }
        i++;
    }
    return 0;
}

void init_password(void) {

    for (int k = 0; k < 4; k++) {
        c_pswd[k] = n_pswd[k];
      //  write_ext_eeprom(200 + k, '0'); 
    }
    check_matrix_keypad();
    static int attempt = 3;
    unsigned char i = 0;
   

    char password[5] = "0000";
     for (int i = 0;i < 4;i++)
    {
        password[i]=read_ext_eeprom(200+i);
    }
    while (1) {
        clcd_print(" ENTER PASSWORD", LINE1(0));
        if (count < 4) {
            check_matrix_keypad();
            for (int i = 2000; i > 0; i++);
        } else {
            CLEAR_DISP_SCREEN;
            if (_strcmp(password, str) == 1) {
                attempt--;
                if (attempt > 0) {
                    wrong_password(attempt);
                    delay = 0;
                    count = 0;
                    CLEAR_DISP_SCREEN;
                } else {
                    CLEAR_DISP_SCREEN;
                    clcd_print("You are blocked ", LINE1(0));
                    clcd_print("Wait...", LINE2(0));
                    clcd_print("Sec", LINE2(12));
                    for (int i = 120; i > 0; i--) {
                        clcd_putch(i % 10 + 48, LINE2(10));
                        clcd_putch(i / 10 % 10 + 48, LINE2(9));
                        clcd_putch(i / 100 + 48, LINE2(8));
                        for (unsigned long int j = 100000; j > 0; j--);
                    }
                    CLEAR_DISP_SCREEN;
                    attempt = 3;
                    count = 0;
                }

            } else {
                CLEAR_DISP_SCREEN;
                attempt = 3;
                    count = 0;
                unsigned char index = 0;
                unsigned char log[][17] = {"View Log        ", "Clear Log       ", "Download Log    ", "Set Time        ", "Change Pwd       "};
                unsigned char star_flag = 0, count;
                unsigned int time = 0, time1 = 0;
                 
                while (1) {
                    if (read_switches(LEVEL_CHANGE) == MK_SW11) {

                        if (time++ >= 2000) {
                            time = 0;
                            count = star_flag + index;
                            if (count == 1) {
                                clear_log();
                            } else if (count == 2) {
                                download_log();
                            } else if (count == 3) {
                                set_time();
                            } else if (count == 4) {
                                change_pwd();
                            } else {
                                view_log();
                            }
                        }
                    } else if (time < 2000 && time != 0) {

                        time = 0;
                        if (star_flag == 1) {
                            star_flag = 0;
                        } else {
                            if (index > 0)
                                index--;
                        }
                    } else if (read_switches(LEVEL_CHANGE) == MK_SW12) {
                        if (time1++ >= 2000) {
                            time1 = 0;
                            break;
                        }
                    } else if (time1 < 2000 && time1 != 0) {
                        time1 = 0;
                        if (star_flag == 0) {
                            star_flag = 1;
                        } else {
                            if (index < 3) {
                                index++;
                            }
                        }
                    }


                    if (star_flag == 0) {
                        clcd_print("*", LINE1(0));
                        clcd_print(" ", LINE2(0));
                    } else {
                        clcd_print("*", LINE2(0));
                        clcd_print(" ", LINE1(0));
                    }
                    clcd_print(log[index], LINE1(2));
                    clcd_print(log[index + 1], LINE2(2));
                }
                CLEAR_DISP_SCREEN;
                clcd_print("  TIME   EVNT SP", LINE1(0));
                break;
            }

        }


    }
}

void view_log(void) {
    CLEAR_DISP_SCREEN;
    clcd_print("Logs", LINE1(0));
    unsigned char count = 0, i = 0,a,start_index=0;
    unsigned int time = 0, time1 = 0;
     if(overflow_flag == 1){
       a=9;
       start_index=lap;
    }
    else
    {
        start_index=0;
        a=lap;
    }
        
    while(1) {
        clcd_putch((i % 10) + 48, LINE2(0));
        clcd_putch(read_ext_eeprom((count+start_index)%10 * 10 + 0), LINE2(2));
        clcd_putch(read_ext_eeprom((count+start_index)%10 * 10 + 1), LINE2(3));
        clcd_putch(':', LINE2(4));
        clcd_putch(read_ext_eeprom((count+start_index)%10 * 10 + 2), LINE2(5));
        clcd_putch(read_ext_eeprom((count+start_index)%10 * 10 + 3), LINE2(6));
        clcd_putch(':', LINE2(7));
        clcd_putch(read_ext_eeprom((count+start_index)%10 * 10 + 4), LINE2(8));
        clcd_putch(read_ext_eeprom((count+start_index)%10 * 10 + 5), LINE2(9));
        clcd_putch(' ', LINE2(10));
        clcd_putch(read_ext_eeprom((count+start_index)%10 * 10 + 6), LINE2(11));
        clcd_putch(read_ext_eeprom((count+start_index)%10 * 10 + 7), LINE2(12));
        clcd_putch(' ', LINE2(13));
        clcd_putch(read_ext_eeprom((count+start_index)%10 * 10 + 8), LINE2(14));
        clcd_putch(read_ext_eeprom((count+start_index)%10 * 10 + 9), LINE2(15));
        unsigned key = read_switches(LEVEL_CHANGE);
        if (key == MK_SW12) {
            if (time++ >= 200) {
                time = 0;
                CLEAR_DISP_SCREEN;
                break;
            }
        } else if (time < 200 && time != 0) {
            time = 0;
            if (count < a) {
                i++;
                count++;
            }
        }
        if (key == MK_SW11) {
            if (time1++ > 200) {
                time1 = 0;
            }
        } else if (time1 < 200 && time1 != 0) {
            time1 = 0;
            if (count > 0) {
                i--;
                count--;
            }
        }
    }
}
void clear_log(void) {
    CLEAR_DISP_SCREEN;
    get_time();
    char arr[10] = {};
    lap = 0;
    overflow_flag = 0;
    speed = read_adc(CHANNEL4) / 10.33;
    arr[0] = time[0];
    arr[1] = time[1];
    arr[2] = time[3];
    arr[3] = time[4];
    arr[4] = time[6];
    arr[5] = time[7];
    arr[6] = 'C';
    arr[7] = 'L';
    arr[8] = speed / 10 + 48;
    arr[9] = speed % 10 + 48;
    for (int i = 0; i < 10; i++) {
        write_ext_eeprom(lap * 10 + i, arr[i]);
        clcd_putch(read_ext_eeprom(lap * 10 + i), LINE1(i));
    }
    if (lap == 9) {
        lap = 0;
    } else {
        lap++;
    }
    clcd_print("Logs cleared", LINE1(2));
    clcd_print("successfully", LINE2(2));
}

void download_log(void) {
    init_uart();
    CLEAR_DISP_SCREEN;
    get_time();
    char arr[10] = {};
     speed = read_adc(CHANNEL4) / 10.33;
    arr[0] = time[0];
    arr[1] = time[1];
    arr[2] = time[3];
    arr[3] = time[4];
    arr[4] = time[6];
    arr[5] = time[7];
    arr[6] = 'D';
    arr[7] = 'L';
    arr[8] = speed / 10 + 48;
    arr[9] = speed % 10 + 48;
    for (int i = 0; i < 10; i++) {
        write_ext_eeprom(lap * 10 + i, arr[i]);
        clcd_putch(read_ext_eeprom(lap * 10 + i), LINE1(i));
    }
    if (lap == 9) {
        lap = 0;
    } else {
        lap++;
    }
    unsigned char start_index = 0, i = 0,a;
    if(overflow_flag == 1){
       a=10;
       start_index=lap;
    }
    else
    {
        start_index=0;
        a=lap;
    }
        
    puts("Logs:\n\r");
    puts("\n\r");
    for(i=0;i<a;i++){
        putch((i % 10) + 48);
        putch(' ');
        putch(read_ext_eeprom((i+start_index)%10 * 10 + 0));
        putch(read_ext_eeprom((i+start_index)%10 * 10 + 1));
        putch(':');
        putch(read_ext_eeprom((i+start_index)%10 * 10 + 2));
        putch(read_ext_eeprom((i+start_index)%10 * 10 + 3));
        putch(':');
        putch(read_ext_eeprom((i+start_index)%10 * 10 + 4));
        putch(read_ext_eeprom((i+start_index)%10 * 10 + 5));
        putch(' ');
        putch(read_ext_eeprom((i+start_index)%10 * 10 + 6));
        putch(read_ext_eeprom((i+start_index)%10 * 10 + 7));
        putch(' ');
        putch(read_ext_eeprom((i+start_index)%10 * 10 + 8));
        putch(read_ext_eeprom((i+start_index)%10 * 10 + 9));
        puts("\n\r");
              if(i==10){
            CLEAR_DISP_SCREEN;
            
       
            break;
        }
    }
   
}

void set_time(){
     if (index >= 0 && index < 7) {
        prev_index = index; 
    }
    clear_flag = 0;
    index = 10;
    get_time();
   // store_in_eeprom();

    i = 0;
    int delay = 0;
    char b = 0;
    char hr, min, sec;
    CLEAR_DISP_SCREEN;
    clcd_print(time, LINE2(0)); 
    while (1) {
        clcd_print("Time (HH:MM:SS) ", LINE1(0));      
        hr = (time[0] - 48)*10 + (time[1] - 48);
        min = (time[3] - 48)*10 + (time[4] - 48);
        sec = (time[6] - 48)*10 + (time[7] - 48);
        if (l++ < 300) {
            clcd_putch(time[b], LINE2(b));
            clcd_putch(time[b + 1], LINE2(b + 1));
        } else if (l < 600)
            clcd_print("  ", LINE2(b));
        else
            l = 0;
        char pre_key = key;
        key = read_switches(LEVEL_CHANGE);
        if (key != 0xff) { 
            if (delay++ == 2000) { 
                delay = 0;
                if (key == 11) {
                    

                    hr = time[0] - 48;
                    hr = hr << 4;
                    hr = time[1] - 48 + hr;
                    write_ds1307(HOUR_ADDR, hr);

                    min = time[3] - 48;
                    min = min << 4;
                    min = time[4] - 48 + min;
                    write_ds1307(MIN_ADDR, min);

                    sec = time[3] - 48;
                    sec = sec << 4;
                    sec = time[4] - 48 + sec;
                    write_ds1307(SEC_ADDR, sec);
                    CLEAR_DISP_SCREEN;
                    flag = 2; 
                    break;
                } else if (key == 12) {
                    
                    CLEAR_DISP_SCREEN;
                    flag = 2;
                    break;
                }
            }
        } else if (delay < 2000 && delay > 0) {
            delay = 0;
            if (pre_key == 11) {
                if (time_flag == 0) { 
                    if (hr >= 0 && hr < 23) {
                        hr++;
                        time[0] = hr / 10 + 48;
                        time[1] = hr % 10 + 48;
                    } else {
                        hr = 0;
                        time[0] = '0';
                        time[1] = '0';
                    }
                } else if (time_flag == 1) { 
                    if (min >= 0 && min < 59) {
                        min++;
                        time[3] = min / 10 + 48;
                        time[4] = min % 10 + 48;
                    } else {
                        min = 0;
                        time[3] = '0';
                        time[4] = '0';
                    }
                } else if (time_flag == 2) { 
                    if (sec >= 0 && sec < 59) {
                        sec++;
                        time[6] = sec / 10 + 48;
                        time[7] = sec % 10 + 48;
                    } else {
                        sec = 0;
                        time[6] = '0';
                        time[7] = '0';
                    }
                }
            } else if (pre_key == 12) {
                if (time_flag >= 0 && time_flag < 3) {
                    time_flag++;

                                  
                } else {
                    time_flag = 0;
                }
                if (time_flag == 0) {
                    clcd_putch(time[b], LINE2(b));
                    clcd_putch(time[b + 1], LINE2(b + 1));
                    b = 0;
                } else if (time_flag == 1) {
                    clcd_putch(time[b], LINE2(b));
                    clcd_putch(time[b + 1], LINE2(b + 1));
                    b = 3;
                } else if (time_flag == 2) {
                    clcd_putch(time[b], LINE2(b));
                    clcd_putch(time[b + 1], LINE2(b + 1));
                    b = 6;
                }
            }
        }
    }
}
void store_pwd(char *n_pswd) {
    for (int k = 0; k < 4; k++) {
        c_pswd[k] = n_pswd[k];
        write_ext_eeprom(200 + k, n_pswd[k]); 
    }
} 
void change_pwd() {
    if (index >= 0 && index  <=6) {
        prev_index = index; 
    }
    clear_flag = 0; 
    index = 10; 
    get_time(); 
   
    int j = 0;
    int x = 0;
    int y = 6;
    while (1) {
        if (i < 4) {
            clcd_print("  New password  ", LINE1(0));
            clcd_print("      ", LINE2(0));
            clcd_print("      ", LINE2(10));
            if (l++ < 300)
                clcd_putch('_', LINE2(j));
            else if (l < 600)
                clcd_putch(' ', LINE2(j));
            else
                l = 0;
        }
        
        key = read_switches(STATE_CHANGE);
        
        if (key != ALL_RELEASED && i < 4) {
            if (key == 11) {
                clcd_putch('*', LINE2(j++));
                n_pswd[i] = '0';
                i++;
            } else if (key == 12) {
                clcd_putch('*', LINE2(j++));
                n_pswd[i] = '1';
                i++;
            }
        } else if (i == 4) {
            if (l++ < 300)
                clcd_putch('_', LINE2(y));
            else if (l < 600)
                clcd_putch(' ', LINE2(y));
            else
                l = 0;
            clcd_print("reenter password", LINE1(0));
            clcd_print("      ", LINE2(0));
            clcd_print("      ", LINE2(10));
            if (key != ALL_RELEASED && x < 4) {
                if (key == 11) {
                    clcd_putch('*', LINE2(y++));
                    r_pswd[x] = '0';
                    x++;
                } else if (key == 12) {
                    clcd_putch('*', LINE2(y++));
                    r_pswd[x] = '1';
                    x++;
                }
            }
            if (x == 4) {
                for (int k = 0; k < 4; k++) {
                    if (n_pswd[k] != r_pswd[k]) {
                        pass_flag = 1;
                        break;
                    } else {
                        pass_flag = 0;
                        store_pwd(n_pswd);
                    }
                }
                if (pass_flag == 0) {
                    CLEAR_DISP_SCREEN;
                    clcd_print("Change password ", LINE1(0));
                    clcd_print("   successful   ", LINE2(0));

                    clear_flag = 0;
                    index = 11;
                    get_time();
                  


                    for (int i = 1000; i--;)
                        for (int j = 3000; j--;);
                    CLEAR_DISP_SCREEN;
                    flag = 2; 
                    break;
                } else {
                    
                    CLEAR_DISP_SCREEN;
                    clcd_print("Change password ", LINE1(0));
                    clcd_print("   failure   ", LINE2(0));
                    for (int i = 1000; i--;)
                        for (int j = 3000; j--;);
                    flag = 2; 
                    break;
                }
            }
        }
    }
}



           