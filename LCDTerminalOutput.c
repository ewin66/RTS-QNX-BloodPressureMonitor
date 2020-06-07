/*
 * Eoin Wickens
 * Student Number: R00151204
 * Email: eoin.wickens@mycit.ie
 *
 * LCDTerminalOutput process displays all of the gathered readouts from the shared MyMessages DataArea.
 * This will only print out the results of the BloodPressureMonitor & HeartRateMonitor processes.
 *
 *
 * Semaphores are not needed due to this process only reading the messages from BloodPressureMonitor&HeartRateMonitor
 */



#include <errno.h>

#include <fcntl.h>

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>
#include "../.INCLUDE/qsm.h"          /* Required for using RTDisplay */
#include <sys/mman.h>

#include <sys/stat.h>

#include "MyMessages.h"
#include "PIDArea.h"

#define RTsig1 SIGRTMIN

int main(int argc, char *argv[]) {

    ScreenInitialise(3, 1);

    //**********************************************************************************
    // Reading the MyMessages
    //**********************************************************************************
    int fd;
    int count = 0;
    struct DataArea *ptr;
    pid_t lpid_list[8];

    /* Open the data area.                    */
    sleep(2);

    fd = shm_open("/MyMessages", O_RDWR, S_IRWXU);

    if (fd == -1) {
        printf("Error opening the data area: %s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Get a pointer to the data area.        */

    ptr = mmap(0, DATA_AREA_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);


    union sigval sval;
    sval.sival_int = 0;

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

    printf("\n\n*********************************");
    printf("LCDDisplay Initialized);
    printf("\n\n*********************************");

    while(1){
        sleep(5);
        //Checks to ensure that the Blood Pressure readings have come in since ArterialMeanPressure is the last
        //of the BloodPressureMonitorProcess readings to come, and that it is dependant on Systolic and Diastolic Pressures.

        if(ptr2->arterial_mean_pressure != 0){
            printf("\nSystolic Pressure Reading: %d", ptr2->systolic_pressure;
            printf("\nDiastolic Pressure Reading: %d", ptr2->diastolic_pressure;
            printf("\nArterial Mean Pressure: %d", ptr2->arterial_mean_pressure);
        }
        else{
            printf("\nBlood Pressure Results Pending..");
        }

        //Checks to ensure that the HeartRateMonitor process has returned a heartbeat before printing anything to the screen
        if(ptr2->heartrate != 0){
            printf("\nHeartRate Reading: %d", ptr2->heartrate);
        }

        if(ptr2->irregular_hr == 0){
            printf("No Irregular Heartbeat Detected");
        }
        else{
            printf("Warning! Irregular Heartbeat detected!");
        }
    }
}

