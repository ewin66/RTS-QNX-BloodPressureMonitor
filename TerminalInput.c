/*

Eoin Wickens
Student Number: R00151204
Email: eoin.wickens@mycit.ie

This process retrieves input from the RTdisplay in the form of F1 and F2 interrupts.
And F1 press with initiate the device by sending RTSig3 interrupts to BloodPressureMonitor and HeartRateMonitor Proceses
These processes then initiate their own respective readings and wait for the feedback.

F2 will kill all processes before killing this process. Once all of these processes have been killed then the startup program will also exit.
This is a way of turning off the device.

*/


#include <errno.h>

#include <fcntl.h>

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <sys/mman.h>

#include <sys/stat.h>
#include "../.INCLUDE/qsm.h"          /* Required for using RTDisplay */
#include <signal.h>
#include "PIDArea.h"

#define RTsig1 SIGRTMIN

#define RTsig2 SIGRTMIN+1

#define RTsig3 SIGRTMIN+2

int last = 0;
union sigval sval;
pid_t lpid_list[8];

void myHandler(int sig, siginfo_t *info, void *other)

{

    switch (info->si_value.sival_int) {

        case 100: printf("F1 Received: Starting Readings!\n"); //Sends RTSig3 interrupts to BloodPressureMonitor Process and HeartRateMonitor Process
            sval.sival_int = 0;
            sigqueue(lpid_list[3], RTsig3, sval);
            sigqueue(lpid_list[3], RTsig3, sval);
            break;

        case 2000: printf("F2 Received: Killing all processes\n");


            //This will kill all processes before killing this process. Once all of these processes have been killed
            //Then the startup program will also exit.
            for(int i = 1; i<8; i++){
                if(i != 2)
                    kill(lpid_list[i], SIGKILL);
                if(i == 7){
                    kill(lpid_list[2], SIGKILL);
                }
            }

            break;

        default:

            printf("Test5: Unexpected signal of %d\n", info->si_value.sival_int);

    }

}

int main(int argc, char *argv[]) {

    struct sigaction action;

    int retval;
    char charchoice = '';
    int choice = 0;

    ScreenInitialise(3,1);

    pid_t lpid_list[8];

//**********************************************************************************
// Reading the PIDList from the PIDArea
//**********************************************************************************
    int     fd2;

    struct PIDArea *ptr2;
    /* Open the data area.                    */

    fd2 = shm_open("/PIDArea", O_RDWR, S_IRWXU);

    if (fd2 == -1) {

        printf("P2: Error opening the data area: %s\n",

               strerror(errno));

        exit(EXIT_FAILURE);

    }

    /* Get a pointer to the data area.        */

    ptr2 = mmap(0, DATA_AREA_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd2, 0);

    sleep(1);

    for(int i = 1; i<8; i++){

        lpid_list[i] = ptr2->pid_list[i];
    }

//**********************************************************************************


    action.sa_handler = (void *)myHandler;
    sleep(1);
    sigfillset(&action.sa_mask);

    action.sa_flags = 0;

    if ((retval = sigaction(RTsig1,&action, NULL)) < 0)

    {

        fprintf(stderr, "Main(): cannot add alarm signal handler\n");

        exit(retval);

    }

    printf("BloodPressureMonitor User Input Emulator Terminal\n");

    printf("\nPlease enter a keyboard input:\n");
    printf("\tF1: Initiate Readings\n");
    printf("\tF2: Exit Blood Pressure Monitor\n");

    //ets user input
    F1INTERRUPT(getpid(), 100);

    F2INTERRUPT(getpid(), 2000);

    while (1)

    {
        sleep(1); //Waits forever for interrupts
    }

}

