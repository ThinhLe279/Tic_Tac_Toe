/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*					WIN32 PORT & LINUX PORT
*                          (c) Copyright 2004, Werner.Zimmermann@fht-esslingen.de
*                 (Similar to Example 1 of the 80x86 Real Mode port by Jean J. Labrosse)
*                                           All Rights Reserved
** *****************************************************************************************************
*		Examination file
* *****************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define  TASK_STK_SIZE                 512       /* Size of each task's stacks (# of WORDs)            */
#define  N_TASKS                        3       /* Number of identical tasks                          */


#define PLAYER_1    0x01
#define PLAYER_2    0x02
#define DRAW    0x04

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK        TaskStk[N_TASKS][TASK_STK_SIZE];        /* Tasks stacks                                  */
OS_STK        TaskStartStk[TASK_STK_SIZE];
OS_STK        CheckWinSTK[TASK_STK_SIZE];

OS_FLAG_GRP* Game_Logic;
OS_EVENT* Mail_Box;

char board[9] = { '.','.', '.', '.', '.','.', '.', '.', '.', };


/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void TaskStart(void* pdata);

void Draw_board(void* pdata);

void Player1(void* pdata);

void Player2(void* pdata);

void Check_Win(void* pdata);


/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/

int  main(void)
{

    INT8U err;



    PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    Game_Logic = OSFlagCreate(0, &err);

    Mail_Box = OSMboxCreate((void*)0);

    OSTaskCreate(TaskStart, (void*)0, &TaskStartStk[TASK_STK_SIZE - 1], 0);

    OSStart();                                             /* Start multitasking                       */

    return 0;
}


/*
*********************************************************************************************************
*                                              STARTUP TASK
*********************************************************************************************************
*/
void  TaskStart(void* pdata)
{
    INT16U key;


    pdata = pdata;                                         /* Prevent compiler warning                 */

    OSTaskCreate(Draw_board, (void*)0, &TaskStk[0][TASK_STK_SIZE - 1], 1);

    OSTaskCreate(Player1, (void*)0, &TaskStk[1][TASK_STK_SIZE - 1], 2);

    OSTaskCreate(Player2, (void*)0, &TaskStk[2][TASK_STK_SIZE - 1], 3);

    OSTaskCreate(Check_Win, (void*)0, &CheckWinSTK[TASK_STK_SIZE - 1], 4);



    for (;;) {

        if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                exit(0);  	                           /* End program                              */
            }
        }
        OSTimeDlyHMSM(0, 0, 1, 0);                         /* Wait one second                          */
    }
}

/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/

void Draw_board(void* pdata) {

    char draw[9];

    char line_1[15],
        line_2[15],
        line_3[15];

    INT8U err, err2;
    int* msg;
    int check = 0;


    while (1) {

        OSFlagPend(Game_Logic, DRAW, OS_FLAG_WAIT_CLR_ANY, 0, &err);

        for (int i = 0; i < 9; i++) {
            draw[i] = board[i];
        }

        sprintf(line_1, " %c | %c | %c ", draw[0], draw[1], draw[2]);
        PC_DispStr(5, 5, (INT8U*)line_1, DISP_FGND_BLACK + DISP_BGND_GRAY);
        PC_DispStr(5, 6, (INT8U*)"-----------", DISP_FGND_BLACK + DISP_BGND_GRAY);

        sprintf(line_2, " %c | %c | %c ", draw[3], draw[4], draw[5]);
        PC_DispStr(5, 7, (INT8U*)line_2, DISP_FGND_BLACK + DISP_BGND_GRAY);
        PC_DispStr(5, 8, (INT8U*)"-----------", DISP_FGND_BLACK + DISP_BGND_GRAY);

        sprintf(line_3, " %c | %c | %c ", draw[6], draw[7], draw[8]);
        PC_DispStr(5, 9, (INT8U*)line_3, DISP_FGND_BLACK + DISP_BGND_GRAY);

        msg = (int*)OSMboxPend(Mail_Box, 0, &err2);

        check = *msg;

        if (check == -1) {
            OSFlagPost(Game_Logic, PLAYER_1, OS_FLAG_SET, &err);
            OSTimeDlyHMSM(0, 0, 0, 200);
        }

        else {

            if (check == 1) {
                PC_DispStr(5, 10, (INT8U*)"Player 1 won!", DISP_FGND_BLACK + DISP_BGND_GRAY);
            }

            else if (check == 2) {
                PC_DispStr(5, 10, (INT8U*)"Player 2 won!", DISP_FGND_BLACK + DISP_BGND_GRAY);
            }

            else if(check == 0)  PC_DispStr(5, 10, (INT8U*)"No winner!", DISP_FGND_BLACK + DISP_BGND_GRAY);

            OSTaskDel(1);
            OSTaskDel(2);
            OSTaskDel(3);
            OSTaskDel(4);
            //OSTimeDlyHMSM(0, 0, 0, 200);
        }


        OSTimeDlyHMSM(0, 0, 1, 0);


    }

}

void Player1(void* pdata) {

    INT8U err;
    int ran = 0;
    srand(time(NULL));

    while (1) {

        OSFlagPend(Game_Logic, PLAYER_1, OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME, 0, &err);

        while (board[ran] != '.') {
            ran = rand() % 9;
        }
        board[ran] = 'X';

        OSFlagPost(Game_Logic, PLAYER_2, OS_FLAG_SET, &err);

        OSTimeDlyHMSM(0, 0, 1, 0);

    }

}

void Player2(void* pdata) {

    INT8U err;
    int ran = 0;
    srand(time(NULL));

    while (1) {

        OSFlagPend(Game_Logic, PLAYER_2, OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME, 0, &err);

        while (board[ran] != '.') {
            ran = rand() % 9;
        }
        board[ran] = 'O';

        OSFlagPost(Game_Logic, DRAW, OS_FLAG_SET, &err);

        OSTimeDlyHMSM(0, 0, 1, 0);

    }
}

void Check_Win(void* pdata) {

    int result = 0;

    while (1) {

        if (board[0] == board[1] && board[1] == board[2] && board[0] != '.') {
            if (board[0] == 'X') {
                result = 1;
                OSMboxPost(Mail_Box, (void*)&result);
            }
            else {
                result = 2; OSMboxPost(Mail_Box, (void*)&result);
            }
        }

        else if (board[3] == board[4] && board[4] == board[5] && board[3] != '.') {
            if (board[3] == 'X') {
                result = 1;
                OSMboxPost(Mail_Box, (void*)&result);
            }
            else {
                result = 2; OSMboxPost(Mail_Box, (void*)&result);
            }
        }

        else if (board[6] == board[7] && board[7] == board[8] && board[6] != '.') {
            if (board[6] == 'X') {
                result = 1;
                OSMboxPost(Mail_Box, (void*)&result);
            }
            else {
                result = 2; OSMboxPost(Mail_Box, (void*)&result);
            }
        }

        else if (board[0] == board[3] && board[3] == board[6] && board[0] != '.') {
            if (board[0] == 'X') {
                result = 1;
                OSMboxPost(Mail_Box, (void*)&result);
            }
            else {
                result = 2; OSMboxPost(Mail_Box, (void*)&result);
            }
        }

        else if (board[1] == board[4] && board[4] == board[7] && board[1] != '.') {
            if (board[1] == 'X') {
                result = 1;
                OSMboxPost(Mail_Box, (void*)&result);
            }
            else {
                result = 2; OSMboxPost(Mail_Box, (void*)&result);
            }
        }

        else if (board[2] == board[5] && board[5] == board[8] && board[2] != '.') {
            if (board[2] == 'X') {
                result = 1;
                OSMboxPost(Mail_Box, (void*)&result);
            }
            else {
                result = 2; OSMboxPost(Mail_Box, (void*)&result);
            }
        }

        else if (board[0] == board[4] && board[4] == board[8] && board[0] != '.') {
            if (board[0] == 'X') {
                result = 1;
                OSMboxPost(Mail_Box, (void*)&result);
            }
            else {
                result = 2; OSMboxPost(Mail_Box, (void*)&result);
            }
        }

        else if (board[2] == board[4] && board[4] == board[6] && board[2] != '.') {
            if (board[2] == 'X') {
                result = 1;
                OSMboxPost(Mail_Box, (void*)&result);
            }
            else {
                result = 2; OSMboxPost(Mail_Box, (void*)&result);
            }
        }

        else if (board[0] != '.' && board[1] != '.' && board[2] != '.' && board[3] != '.' &&
            board[4] != '.' && board[5] != '.' && board[6] != '.' && board[7] != '.' && board[8] != '.') {

            result = 0;
            OSMboxPost(Mail_Box, (void*)&result);
        }

        else {

            result = -1;
            OSMboxPost(Mail_Box, (void*)&result);
        }

        OSTimeDlyHMSM(0, 0, 1, 0);
    }

}


/*
*********************************************************************************************************
*                                      NON-TASK FUNCTIONS
*********************************************************************************************************
*/
