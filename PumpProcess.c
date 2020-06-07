/*

Eoin Wickens
Student Number: R00151204
Email: eoin.wickens@mycit.ie

This PumpProcess inflates and deflates the bladder cuff. This process requires input in the form of an interrupt from the BloodPressureMonitor
In order to know when to inflate and deflate. This is considered to be a passive process.

After each inflate or deflate mechanism the current pressure of the bladder is sent back to the BloodPressureMonitor so that the BPM can log the
Systolic and Diastolic pressures respectively.

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

union sigval sval;

pid_t lpid_list[8];

int current_pump_pressure = 0;

/* P1_pid = This must be set up to be the pid value of process P1???   */



//This handler method handles the sival interrupts from the BloodPressureMonitor. If the sival_int is 1
//Then the Pump (Actuator) should pump an additional 2 mm Hg of pressure into the bladder.

//if the sival_int is 0, then the pump should decrease pressure in the bladder by 4 mm Hg of pressure

//if the sival_int is 2, then the pump will just return the pressure of the bladder/pump without manipulating the pressure level.

void myHandler(int sig, siginfo_t *info, void *other)
{
    //Case for pumping an additional 2 mm Hg of pressure into the bladder
    if(info->si_value.sival_int == 1){
        current_pump_pressure += 2;

        //sends the current_pump_pressure back to the BloodPressureMonitorProcess
        sval.sival_int = current_pump_pressure;
    }

    //Case for reducing the pressure in the bladder by 4mm Hg of pressure.
    if(info->si_value.sival_int == 0) {

        //case to make sure pump doesn't go into negative pressure.
        if (current_pump_pressure > 3) {

            current_pump_pressure -= 4;
            //sends the current_pump_pressure back to the BloodPressureMonitorProcess
            sval.sival_int = current_pump_pressure;
        }
    }


    //Case for pumping an additional 2 mm Hg of pressure into the bladder
    if(info->si_value.sival_int == 2){
        //sends the current_pump_pressure back to the BloodPressureMonitorProcess
        sval.sival_int = current_pump_pressure;

    }

    //Sets the pump pressure to the new value
    ActuatorSetValue(1, current_pump_pressure);

    //sends the sigqueue interrupt back to the BloodPressureMonitor Process.
    //RTsig2 is used here sot that BloodPressureMonitor can correctly identify that this value is the pump pressure.
    sigqueue(4, RTsig2, sval);
}


int main(int argc, char *argv[]) {

    ScreenInitialise(3,1);


    //Setting the actuator name to PumpPressure
    ActuatorSetName(1, "PumpPressure");

    struct sigaction action;

    int retval;


    //**********************************************************************************
//  Setting up ability to modify message dataArea
    //**********************************************************************************

    int     fd;

    struct DataArea *ptr;
    /* Create the data area.                */
    fd = shm_open("/MyMessages", O_RDWR | O_EXCL, S_IRWXU);
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




    action.sa_handler = (void *)myHandler;
    sleep(1);
    sigfillset(&action.sa_mask);

    action.sa_flags = 0;

    if ((retval = sigaction(RTsig1,&action, NULL)) < 0)

    {

        fprintf(stderr, "Main(): cannot add alarm signal handler\n");

        exit(retval);

    }

    while (1)

    {

        pause();

    }

}

