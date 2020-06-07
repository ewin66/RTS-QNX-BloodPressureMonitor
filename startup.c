/*

Eoin Wickens
Student Number: R00151204
Email: eoin.wickens@mycit.ie

Startup creates 7 child processes and creates/initializes the shared PID data area to allow for interrupt based
Inter-process communication.

*/


#include <stdio.h>

#include <stdlib.h>

#include <sys/types.h>

#include <spawn.h>

#include <unistd.h>
#include <process.h>

#include <sys/neutrino.h>

#include <sys/wait.h>
#include "../.INCLUDE/qsm.h"            /* Required for setting up RTdisplay. */
#include "PIDArea.h"


#define RTsig1 SIGRTMIN

int main (int argc, char * argv[], char **envp)

   {
    ScreenInitialise(2, 1);

    int     fd;

   struct PIDArea *ptr;

   /* Call required for setting up RTdisplay. */
   char    *args1[] = {"LCDTerminalOutput", "-l", NULL};
   char    *args2[] = {"TerminalInput", "-l", "SecondParm", NULL};
   char    *args3[] = {"HeartRateMonitor", "-l", "SecondParm", NULL};
   char    *args4[] = {"BloodPressureMonitor", "-l", "SecondParm", NULL};
   char    *args5[] = {"PumpProcess", "-l", "SecondParm", NULL};
   char    *args6[] = {"BPTransducerMonitor", "-l", "SecondParm", NULL};
   char    *args7[] = {"PulseTransducerMonitor", "-l", "SecondParm", NULL};

   int     status1;
   int     status2;
   int     status3;
   int     status4;
   int     status5;
   int     status6;
   int     status7;

   pid_t   pid1;
   pid_t   pid2;
   pid_t   pid3;
   pid_t   pid4;
   pid_t   pid5;
   pid_t   pid6;
   pid_t   pid7;

   struct inheritance inherit1;
   struct inheritance inherit2;
   struct inheritance inherit3;
   struct inheritance inherit4;
   struct inheritance inherit5;
   struct inheritance inherit6;
   struct inheritance inherit7;

//**********************************************************************
//Creation of PIDArea DataArea
//**********************************************************************

   fd = shm_open("/PIDArea", O_RDWR | O_CREAT | O_EXCL, S_IRWXU);

   if (fd == -1) {

       printf("Error creating the shared memory object. %s.\n",

              strerror(errno));

       exit(EXIT_FAILURE);

   }
   /* Set the size data area.              */

   ftruncate(fd, DATA_AREA_SIZE);
   /* Make 'ptr' point into the data area. */

   ptr = mmap(0, DATA_AREA_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

//**********************************************************************
//Creation of MyMessages DataArea to house the integers being transmitted to LCDTerminalOutput
//**********************************************************************


    int     fd2;

    struct DataArea *ptr2;
    /* Create the data area.                */

    fd2 = shm_open("/MyMessages", O_RDWR | O_CREAT | O_EXCL, S_IRWXU);

    if (fd2 == -1) {

        printf("Error creating the shared memory object. %s.\n",

               strerror(errno));

        exit(EXIT_FAILURE);

    }
    /* Set the size data area.              */

    ftruncate(fd2, DATA_AREA_SIZE);
    /* Make 'ptr' point into the data area. */

    ptr2 = mmap(0, DATA_AREA_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd2, 0);



//**********************************************************************


   inherit1.flags = 0;
   inherit2.flags = 0;
   inherit3.flags = 0;
   inherit4.flags = 0;
   inherit5.flags = 0;
   inherit6.flags = 0;
   inherit7.flags = 0;


    //Initializes all of the readings in the DataArea to 0
    ptr2->systolic_pressure = 0;
    ptr2->diastolic_pressure = 0;
    ptr2->arterial_mean_pressure = 0;
    ptr2->heartrate = 0;
    ptr2->irregular_hr = 0;
   //Spawning Process 1-5.

   if ((pid1 = spawn(args1[0], 0, NULL, &inherit1, args1, envp)) == -1)

      perror("Process1: spawn() failed");

   else

      printf("Process1: spawned Process, pid ==> %d\n", pid1);


   if ((pid2 = spawn(args2[0], 0, NULL, &inherit2, args2, envp)) == -1)

      perror("Process2: spawn() failed");

   else

      printf("Process2: spawned Process, pid ==> %d\n", pid2);


   if ((pid3 = spawn(args3[0], 0, NULL, &inherit3, args3, envp)) == -1)

      perror("Process3: spawn() failed");

   else

      printf("Process3: spawned Process, pid ==> %d\n", pid3);



   if ((pid4 = spawn(args4[0], 0, NULL, &inherit4, args4, envp)) == -1)

      perror("Process4: spawn() failed");

   else

      printf("Process4: spawned Process, pid ==> %d\n", pid4);


   if ((pid5 = spawn(args5[0], 0, NULL, &inherit5, args5, envp)) == -1)

      perror("Process5: spawn() failed");

   else

      printf("Process5: spawned Process, pid ==> %d\n", pid5);

   if ((pid6 = spawn(args6[0], 0, NULL, &inherit6, args6, envp)) == -1)

       perror("Process6: spawn() failed");

   else

       printf("Process6: spawned Process, pid ==> %d\n", pid6);


   if ((pid7= spawn(args7[0], 0, NULL, &inherit7, args7, envp)) == -1)

       perror("Process7: spawn() failed");

   else

       printf("Process7: spawned Process, pid ==> %d\n", pid7);

     ptr->pid_list[1] = pid1;
     ptr->pid_list[2] = pid2;
     ptr->pid_list[3] = pid3;
     ptr->pid_list[4] = pid4;
     ptr->pid_list[5] = pid5;
     ptr->pid_list[6] = pid6;
     ptr->pid_list[7] = pid7;


   // Checking to see if process 1 has exited

   if (waitpid(pid1, &status1, WEXITED) <= 0)

      perror("Process1: Error with waiting???");

   else

      printf("Process1: Process pid=%d closed successfully and status==>%d\n",

             pid1, status1);

   // Checking to see if process 2 has exited

   if (waitpid(pid2, &status2, WEXITED) <= 0)

      perror("Process2: Error with waiting???");

   else

      printf("Process2: Process pid=%d closed successfully and status==>%d\n",

             pid2, status2);

   // Checking to see if process 3 has exited
   if (waitpid(pid3, &status3, WEXITED) <= 0)

      perror("Process3: Error with waiting???");

   else

      printf("Process3: Process pid=%d closed successfully and status==>%d\n",

             pid3, status3);
   // Checking to see if process 4 has exited

   if (waitpid(pid4, &status4, WEXITED) <= 0)

      perror("Process4: Error with waiting???");

   else

      printf("Process4: Process pid=%d closed successfully and status==>%d\n",

             pid4, status4);


   // Checking to see if process 5 has exited

   if (waitpid(pid5, &status5, WEXITED) <= 0)

      perror("Process5: Error with waiting???");

   else

      printf("Process5: Process pid=%d closed successfully and status==>%d\n",

             pid5, status5);

   if (waitpid(pid6, &status6, WEXITED) <= 0)

       perror("Process6: Error with waiting???");

   else

       printf("Process6: Process pid=%d closed successfully and status==>%d\n",

              pid6, status6);

   if (waitpid(pid7, &status7, WEXITED) <= 0)

       perror("Process7: Error with waiting???");

   else

       printf("Process7: Process pid=%7 closed successfully and status==>%d\n",

              pid7, status7);

       return(0);

   return(0);


}
