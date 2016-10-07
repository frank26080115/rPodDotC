char stepper_calibrate_all(stepper_t* steppers[], int count)
{
	stepper_t* stepper;
	int i;
	char canExit;
	systmr_t timestamp;

	TRACE("stepper_calibrate_all");
	TRACE("checking for hardware failure");
	for (i = 0; i < count; i++)
	{
		stepper = steppers[i];
		if (stepper->isMinHit(stepper) && stepper->isMaxHit(stepper)) {
			FATAL_ERROR("stepper motor %d has both limit switches hit", stepper->id);
			return 0;
		}
	}

	TRACE("moving to minimum limit switch");
	timestamp = systmr_nowMillis();
	canExit = 0;
	while (canExit == 0 && (systmr_nowMillis() - timestamp) < 30000)
	{
		canExit = 1;
		for (i = 0; i < count; i++)
		{
			stepper = steppers[i];
			if (stepper->isMinHit(stepper) == 0)
			{
				stepper->moveRelative(-1);
				canExit = 0;
				stepper_cal_step_delay();
			}
		}
	}

	if (canExit == 0)
	{
		FATAL_ERROR("stepper motors unable to reach minimum switch");
		return 0;
	}

	TRACE("freeing from minimum limit switch");
	timestamp = systmr_nowMillis();
	canExit = 0;
	while (canExit == 0 && (systmr_nowMillis() - timestamp) < 30000)
	{
		canExit = 1;
		for (i = 0; i < count; i++)
		{
			stepper = steppers[i];
			if (stepper->isMinHit(stepper))
			{
				stepper->moveRelative(1);
				canExit = 0;
				stepper_cal_step_delay();
			}
			else
			{
				stepper->position = 0;
				stepper->consts.adcMin = stepper->getAdc(stepper);
			}
		}
	}

	if (canExit == 0)
	{
		FATAL_ERROR("stepper motors unable to free from minimum switch");
		return 0;
	}

	TRACE("moving to maximum limit switch");
	timestamp = systmr_nowMillis();
	canExit = 0;
	while (canExit == 0 && (systmr_nowMillis() - timestamp) < 30000)
	{
		canExit = 1;
		for (i = 0; i < count; i++)
		{
			stepper = steppers[i];
			if (stepper->isMaxHit(stepper) == 0)
			{
				stepper->moveRelative(1);
				canExit = 0;
				stepper_cal_step_delay();
			}
		}
	}

	if (canExit == 0)
	{
		FATAL_ERROR("stepper motors unable to reach maximum switch");
		return 0;
	}

	TRACE("freeing from maximum limit switch");
	timestamp = systmr_nowMillis();
	canExit = 0;
	while (canExit == 0 && (systmr_nowMillis() - timestamp) < 30000)
	{
		canExit = 1;
		for (i = 0; i < count; i++)
		{
			stepper = steppers[i];
			if (stepper->isMaxHit(stepper))
			{
				stepper->moveRelative(-1);
				canExit = 0;
				stepper_cal_step_delay();
			}
			else
			{
				stepper->consts.endpoint_max = stepper->position;
				stepper->consts.adcMax = stepper->getAdc(stepper);
			}
		}
	}

	if (canExit == 0)
	{
		FATAL_ERROR("stepper motors unable to free from maximum switch");
		return 0;
	}

	TRACE("moving to middle");
	timestamp = systmr_nowMillis();
	canExit = 0;
	while (canExit == 0 && (systmr_nowMillis() - timestamp) < 30000)
	{
		canExit = 1;
		for (i = 0; i < count; i++)
		{
			stepper = steppers[i];
			stepper->consts.endpoint_mid = stepper->consts.endpoint_max / 2;
			if (stepper->position > stepper->consts.endpoint_mid)
			{
				stepper->moveRelative(stepper, -1);
				canExit = 0;
				stepper_cal_step_delay();
			}
		}
	}

	if (canExit == 0)
	{
		FATAL_ERROR("stepper motors unable to move to middle");
		return 0;
	}

	TRACE("getting middle ADC value for steppers");
	for (i = 0; i < count; i++)
	{
		stepper = steppers[i];
		stepper->consts.adcMid = stepper->getAdc(stepper);
		// TODO: averaging
	}

	return 1;
}
