/*
 *
 * Eoin Wickens
 * Student Number: R00151204
 * Email: eoin.wickens@mycit.ie
 *
 * This will contain the PID data area for 7 processes
 *
*/

#define MESS_SZ 30

struct PIDArea

   {
	pid_t pid_list[8];
   };

//#define DATA_AREA_SIZE sizeof(struct PIDArea)
