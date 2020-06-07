/*

* Contains the Message DataArea which will store the following values for the LCD Display.
Eoin Wickens
Student Number: R00151204
Email: eoin.wickens@mycit.ie

*/



#define MESS_SZ 30

struct DataArea

   {
   int systolic_pressure;
   int diastolic_pressure;
   int arterial_mean_pressure;
   int heartrate;
   int irregular_hr;
   };

#define DATA_AREA_SIZE sizeof(struct DataArea)
