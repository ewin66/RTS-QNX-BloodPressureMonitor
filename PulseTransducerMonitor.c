/*

Eoin Wickens
Student Number: R00151204
Email: eoin.wickens@mycit.ie

This process monitors the Pulse Transducer Monitor and then sends the value to the HeartRateMonitor Process for analysis.
The Pulse transducer sensor is received from the RTDisplay for emulation purposes.

The pulse rate is specified as a sensor value in RTDisplay for the purpose of this project.

 This process will not start sending the readings to the HeartRateMonitor until it receives the RTSig3 signal.
*/

#include <stdio.h>
#include <stdlib.h>

#include <time.h>

#include <sys/netmgr.h>

#include <sys/neutrino.h>

#include "../.INCLUDE/qsm.h"          /* Required for using RTDisplay */

#include "MyMessages.h"

#include <sys/wait.h>

#include <signal.h>

#include "PIDArea.h"

#define MY_CYCLIC_PULSE _PULSE_CODE_MINAVAIL

#define RTsig1 SIGRTMIN
#define RTsig2 SIGRTMIN+1
#define RTsig3 SIGRTMIN+2

typedef union {

    struct _pulse pulse;

    /* your other message structures would go here too */

} my_message_t;

int start_variable = 0;

//This handler method handles the sival interrupts from the HeartRateMonitor process.
void myHandler(int sig, siginfo_t *info, void *other)
{

    if(sig == RTsig3){
        //sets start_var to 1 allowing the readings to be taken and sent back to the HR Monitor
        start_variable = 1;
    }
}
int main()

{
    ScreenInitialise(3, 1);

    SensorSetName(1, "Pulse-Transducer");

    int pulse_transducer = 0;

    struct sigevent event;

    struct itimerspec itime;

    timer_t timer_id;

    int chid;

    int rcvid;

    my_message_t msg;

    union sigval sval;

    pid_t lpid_list[8];

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

    chid = ChannelCreate(0);

    event.sigev_notify = SIGEV_PULSE;

    event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);

    event.sigev_priority = -1;

    event.sigev_code = MY_CYCLIC_PULSE;

    timer_create(CLOCK_REALTIME, &event, &timer_id);


    //Sensor is read cyclically every 0.01 second to ensure a high fidelity of the transducer readings

    itime.it_value.tv_nsec = 10000000;

    itime.it_interval.tv_nsec = 10000000;

    timer_settime(timer_id, 0, &itime, NULL);

    int last_sensor_read = 0;


    while (1) {
        /* Wait for a PULSE every period.         */

        rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);

        if (rcvid == 0) {

            /* Check to see if our CYCLIC pulse.      */

            if (msg.pulse.code == MY_CYCLIC_PULSE && start_variable == 1) {

                //Fetches the pulse_transducer value from the sensor which would be emulated with RTDisplay
                pulse_transducer = SensorGetValue(2);

                //When heartbeat is not happening the artery will still provide background vibration when under pressure
                //As such a pulse can be detected by a *heavy* beat against the transducer.
                //Assuming the transducer is mapped 1-100 then we will assume that a pulse is when the vibration is over 90.
                if(current_trans_reading > 90){
                    //Pulse is read
                    sval.sival_int = pulse_transducer;
                    sigqueue(lpid_list[3], RTsig1, sval);
                }
            }
        } /* else other pulses ... */
    } /* else other messages ... */
}

return 0;

}

