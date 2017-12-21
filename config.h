#ifndef CONFIG_H
#define CONFIG_H

//------------------------- PROJECT CONFIG ------------------//

//#define USE_OPENGL
#define HIGH_PERF

//------------------------- RECEIVE COMMANDS ----------------//

#define ARD_LOG 255

#define ARD_PID1_INPUT 1
#define ARD_PID1_OUTPUT 2
#define ARD_PID1_SETPOINT 3

#define ARD_PID2_INPUT 4
#define ARD_PID2_OUTPUT 5
#define ARD_PID2_SETPOINT 6

#define ARD_PID3_INPUT 7
#define ARD_PID3_OUTPUT 8
#define ARD_PID3_SETPOINT 9

//skip 10 -> it's newline
#define ARD_PID1_KP 11
#define ARD_PID1_KI 12
#define ARD_PID1_KD 13

#define ARD_PID2_KP 14
#define ARD_PID2_KI 15
#define ARD_PID2_KD 16

#define ARD_PID3_KP 17
#define ARD_PID3_KI 18
#define ARD_PID3_KD 19

#define ARD_NORMAL_LOOP_TIME 20
#define ARD_SERIAL_LOOP_TIME 21

//------------------------- TRANSMIT COMMANDS ---------------//
#define CUTE_PID1_KP 1
#define CUTE_PID1_KI 2
#define CUTE_PID1_KD 3

#define CUTE_PID2_KP 4
#define CUTE_PID2_KI 5
#define CUTE_PID2_KD 6

#define CUTE_PID3_KP 7
#define CUTE_PID3_KI 8
#define CUTE_PID3_KD 9

//avoid 10
#define CUTE_PID1_SETP 11
#define CUTE_PID2_SETP 12
#define CUTE_PID3_SETP 13

#define CUTE_P1_PRNT_ON 20
#define CUTE_P1_PRNT_OFF 21
#define CUTE_P2_PRNT_ON 22
#define CUTE_P2_PRNT_OFF 23
#define CUTE_P3_PRNT_ON 24
#define CUTE_P3_PRNT_OFF 25

#define CUTE_GIRO_TO_MOT_ON 26
#define CUTE_GIRO_TO_MOT_OFF 27
#define CUTE_GET_ALL_PID_CFGS 28

#define CUTE_SAVE_TO_EEPROM 30
#define CUTE_GET_UP 31

#define CUTE_CYCLE_TIME_PRINTS_ON 32
#define CUTE_CYCLE_TIME_PRINTS_OFF 33

//------------------------- SCALING -----------------------//
#define SCALE_PID1_INPUT 40
#define SCALE_PID1_OUTPUT 255
#define SCALE_PID1_SETPOINT 40

#define SCALE_PID2_INPUT 40
#define SCALE_PID2_OUTPUT 255
#define SCALE_PID2_SETPOINT 40

#define SCALE_PID3_INPUT 120
#define SCALE_PID3_OUTPUT 40
#define SCALE_PID3_SETPOINT 120

#endif // CONFIG_H
