void engine_ctrl()
{
	attitude_t attitude;
	hoverattitude_calculate(&nvm.pod_coords, &laser_readings, &attitude);
	rpod_float_t min_throttle = nvm.engine_min_throttle;

	rpod_float_t base_throttle = pid_calc(&nvm.pid_height, attitude.center_height, desired_z);
	rpod_float_t lr_diff = pid_calc(&nvm.pid_roll, attitude.roll, 0);
	rpod_float_t fb_diff = pid_calc(&nvm.pid_pitch, attitude.pitch, 0);

	// mixing
	rpod_float_t engine_fr = min_throttle + base_throttle + lr_diff + fb_diff;
	rpod_float_t engine_br = min_throttle + base_throttle + lr_diff - fb_diff;
	rpod_float_t engine_fl = min_throttle + base_throttle - lr_diff + fb_diff;
	rpod_float_t engine_bl = min_throttle + base_throttle - lr_diff - fb_diff;

	// TODO: round and normalize
	// TODO: send to DAC to ASI
}

int main()
{
	return 0;
}