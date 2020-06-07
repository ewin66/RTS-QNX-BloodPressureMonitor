/*

Eoin Wickens
Student Number: R00151204
Email: eoin.wickens@mycit.ie

BPMonitor Process initiates the blood pressure monitor reading, it controls the cuff inflation and deflation of the pump process
through sigval interrupts to inflate/deflate by a fixed rate.

This process calculates the Systolic and Diastolic pressures based on the readings obtained from the BPTransducer monitor.
The arterial mean pressure (AMP) is also calculated here and is then sent to the LCD for display.


When this process reeives an RTSig3 interrupt from the TerminalInputProcess it will send an RTsig3 to the BPTransducerMonitorProcess
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
int current_pressure_reading = 0;
int systolic_pressure = 0;
int diastolic_pressure = 0;
int arterial_mean_pressure = 0;

int start_variable = 0;

//This handler method handles the sival interrupts from the BPTransducerMonitor process and the PumpProcess process.
void myHandler(int sig, siginfo_t *info, void *other)
{
    //if the sig is RTSig1 then it is an interrupt from the BPTransducerMonitor Process with an update of the transducer reading.
    if(sig == RTsig1){
        current_trans_reading = info->si_value.sival_int;
    }
    //If the sig is RTSig2 then the interrupt is coming from the PumpProcess with a return value for the current pressure of the pump/inflation bladder.
    if(sig == RTsig2){
        current_pressure_reading = info->si_value.sival_int;
    }

    if(sig == RTsig3){

        //Tells the BloodPressureTransducer to start readings.
        sval.sival_int = 0;
        sigqueue(6, RTsig3, sval);


        //Start variable being set initiates inflation of the bladder through the pump process. This is sent with sigvals further on in the code.
        start_variable = 1;
    }
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

    while (1)

    {
        //halts the process until RTSig3 Interrupt sets start_variable to start increasing pump pressure.
        while(start_variable == 0);

        //starts reading

        //Starts cuff inflation until the BPTransducer reading peaks goes down considerably from a high reading (difference of >60)


        //Pressure is then reduced by 4mg hh until the sensor reading from the BPTransducer is back between 50-100 (a High reading)
        //Systolic pressure is then taken immediately once the sensor goes high.
        if(systolic_pressure == 0 && last_trans_reading - current_trans_reading > 60){
            //sets the systolic pressure as the last_pressure reading before it cut off blood flow to brachial artery.
            systolic_pressure = current_pressure_reading;
        }


        //Once systolic pressure has been taken the pressure is reduced further by 4mg hh until the vibration lessens below threshold of 50
        //The diastolic pressure is then taken.
        if(systolic_pressure != 0 && diastolic_pressure == 0 && current_trans_reading < 50){
            diastolic_pressure = current_pressure_reading;
        }


        //If systolic pressure has not been taken (still set to 0) then increase the pressure to the bladder
        if(systolic_pressure == 0){
            sval.sival_int = 1;
            sigqueue(5, RTsig1, sval);
        }
            //If systolic pressure has been taken then we can decrease the pressure in the bladder through the pump by 4mm Hg
        else if(systolic_pressure != 0 && diastolic_pressure == 0){
            sval.sival_int = 0;
            sigqueue(5, RTsig1, sval);
        }


        //Calculations on the ArterialMeanPressure (AMP) can then be performed once the the previous two measurements have been completed
        if(systolic_pressure != 0 && diastolic_pressure != 0){
            arterial_mean_pressure = ((diastolic_pressure*2)*systolic_pressure)/3;


            //All three readings are then sent to the LCD Display via a shared data area.
            ptr->systolic_pressre = systolic_pressure;
            ptr->diastolic_pressure = diastolic_pressure;
            ptr->amp = arterial_mean_pressure;


            pause();
        }

        last_trans_reading = current_trans_reading;
        sleep(1);
    }

}

