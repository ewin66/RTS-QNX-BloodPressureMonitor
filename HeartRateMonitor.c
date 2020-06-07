/*

Eoin Wickens
Student Number: R00151204
Email: eoin.wickens@mycit.ie

HeartRateMonitor Process takes the PulseTransducer readings and calculates the heartbeat in beats per minute.
 Heart rates are received as sigval interrupts from the PulseTransducerMonitor process.

Heart rate is calculated here by taking the current time of the previous beat and the current time of the current beat
and subtracting them, then dividing 60 by the result.

An irregular heartbeat is calculated through deductive reasoning, where if the time elapsed between beats is
less than 0.3 then either their heartrate is over 200bpm or it is an irregular heartbeat.

The heartrate and irregular heartbeat values are then added to the MyMessages dataarea.

When this process receives an RTSig3 interrupt from the TerminalInputProcess it will send an RTsig3 to the PulseTransducerMonitor Process
which tells it to start sending back telemetry data.

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

#include <math.h>

#include <time.h>

#include "MyMessages.h"

#define RTsig1 SIGRTMIN

#define RTsig2 SIGRTMIN+1

#define RTsig3 SIGRTMIN+2


union sigval sval;

//PID List for interproc communication
pid_t lpid_list[8];


//Instantiates the current transducer reading for the bp transducer val.
int current_trans_reading = 0;
int last_trans_reading = 0;

int est_heart_rate = 0;
int time_between_heart_rate = 0;

//Declares QNX clock variables to record the time between pulses
clock_t time, last_time;
long time_elapsed = 0;
long last_time_elapsed = 0;
int heart_rate = 0;
int irregular_hb = 0;

//This handler method handles the sival interrupts from the PulseTranducerMonitor Process
void myHandler(int sig, siginfo_t *info, void *other)
{
    //if the sig is RTSig1 then it is an interrupt from the PulseTranducerMonitor Process with an update of the transducer reading.
    if(sig == RTsig1){
        current_trans_reading = info->si_value.sival_int;
        time = clock();
    }

    // If the program receives an RTsig3 interrupt then it will start readings.
    if(sig == RTsig3){

        //Tells the PulseTransducer to start readings.
        sval.sival_int = 0;
        sigqueue(7, RTsig3, sval);
    }

    //Accounts for the first pulse to be recorded before calculations are performed.
    if(last_time != 0){
        time_elapsed = ((last_time - time) / CLOCKS_PER_SEC));

        // Calculates Heart Rate
        heart_rate = 60/time_elapsed;

        // 0.3 equates to 200bpm - If your heart rate exceeds this your heart has exploded or you have an irregular heartbeat
        if(last_time_elapsed - time_elapsed < 0.3){
            irregular_hb = 1;
        }

        //Adds the values to the shared data area MyMessages.
        ptr->heartrate = heart_rate;
        ptr->irregular_hr = irregular_hb;

    }
    last_time = time;
    last_time_elapsed = time_elapsed;
}

int main(int argc, char *argv[]) {

    struct sigaction action;

    int retval;

    ScreenInitialise(3,1);

    action.sa_handler = (void *)myHandler;
    sleep(1);
    sigfillset(&action.sa_mask);

    action.sa_flags = 0;

    //**********************************************************************************
    //  MyMessages
    //**********************************************************************************
    int     fd;
    struct DataArea *ptr;
    /* Create the data area.                */
    fd = shm_open("/myDataArea", O_RDWR | O_EXCL, S_IRWXU);

    if (fd == -1) {
        printf("Error opening the shared memory object. %s.\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    /* Set the size data area.              */
    ftruncate(fd, DATA_AREA_SIZE);

    /* Make 'ptr' point into the data area. */
    ptr = mmap(0, DATA_AREA_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    //**********************************************************************************
    // Reading the PIDList from the PIDArea
    //**********************************************************************************

    int     fd2;

    struct PIDArea *ptr2;
    /* Open the data area.                    */

    fd2 = shm_open("/PIDArea", O_RDWR, S_IRWXU);

    if (fd2 == -1) {

        printf("Error opening the data area: %s\n",

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


    if ((retval = sigaction(RTsig1,&action, NULL)) < 0)

    {

        fprintf(stderr, "Main(): cannot add alarm signal handler\n");

        exit(retval);

    }

    //Sleeps for 10 seconds to allow the Pulse interrupts to start coming through before any calculations are performed.
    sleep(10);


    while (1){
        pause();
    }

}

