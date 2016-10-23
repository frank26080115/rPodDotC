
// note: all integer units are 2x what the celcius value is. 20C is 40

#define BATTTEMP_ROOM_TEMP 40 // this means 20C
int16_t batttemp_history[BATTTEMP_HISTORY_SIZE][BATTTEMP_SENSOR_COUNT];
int batttemp_history_idx = 0;
double batttemp_filtered_history[BATTTEMP_FILTERED_HISTORY_SIZE][BATTTEMP_SENSOR_COUNT]; // TODO, make a trend line, to provide early warning, and to mask bad sensor reading

void batttemp_init()
{
	int i, j;
	for (i = 0; i < BATTTEMP_HISTORY_SIZE; i++) // for the entire history
	{
		for (j = 0; j < BATTTEMP_SENSOR_COUNT; j++) // for all sensors
		{
			batttemp_history[i][j] = BATTTEMP_ROOM_TEMP; // assume currently at room temperature
		}
	}
}

extern void batttemp_callback_alarm(char* alarms, uint16_t* temps);

// the caller of this function should already have the temperatures of every sensor in an array
// the job of this function is to detect if there is any reason to stop the test and/or issue a warning to the ground station
void batttemp_handleNewData(int16_t* all_temps)
{
	int i, j;
	char alarms[BATTTEMP_SENSOR_COUNT];
	char has_alarm = 0;

	// TODO: check if temp is obviously an error, perhaps correct it by using a trend line

	// copy into latest history, and process
	batttemp_history_idx = (batttemp_history_idx + 1) % BATTTEMP_HISTORY_SIZE; // increment with wrap-around/roll-over
	// note: using a rolling index like this, the purposes is to speed up the code by avoiding "shifting" the entire array
	for (i = 0; i < BATTTEMP_SENSOR_COUNT; i++) // for all sensors
	{
		char alert;
		batttemp_history[batttemp_history_idx][i] = all_temps[i]; // copy into latest history

		// check if consecutive samples are all over limit
		alert = 1;
		for (j = 0; j < BATTTEMP_HISTORY_SIZE; j++) // all samples in history of this particular sensor
		{
			if (batttemp_history[batttemp_history_idx][i] < BATTTEMP_THRESHOLD)
			{
				alert = 0; // one reading in the list is under threshold, thus we do not alarm
				break;
			}
		}
		alarms[i] = alert;
		if (alert != 0) {
			has_alarm = 1; // run the callback later if any has alarm
		}
	}

	// if we have any alarms
	if (has_alarm != 0) {
		batttemp_callback_alarm(alarms, all_temps);
		// TODO: batttemp_callback_alarm does not exist yet, somebody else must write this
		// it should shutdown the battery and send a message to the GUI
	}
}