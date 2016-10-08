void engine_ctrl()
{
	attitude_t attitude;
	hoverattitude_calculate(&nvm.pod_coords, &laser_readings, &attitude);
	float min_throttle = nvm.engine_min_throttle;

	float base_throttle = pid_calc(&nvm.pid_height, attitude.center_height, desired_z);
	float lr_diff = pid_calc(&nvm.pid_roll, attitude.roll, 0);
	float fb_diff = pid_calc(&nvm.pid_pitch, attitude.pitch, 0);

	mixer_state->base_throttle += base_throttle;
	mixer_state->lr_diff += lr_diff;
	mixer_state->fb_diff += fb_diff;

	// mixing
	float engine_fr = min_throttle + mixer_state->base_throttle + mixer_state->lr_diff + mixer_state->fb_diff;
	float engine_br = min_throttle + mixer_state->base_throttle + mixer_state->lr_diff - mixer_state->fb_diff;
	float engine_fl = min_throttle + mixer_state->base_throttle - mixer_state->lr_diff + mixer_state->fb_diff;
	float engine_bl = min_throttle + mixer_state->base_throttle - mixer_state->lr_diff - mixer_state->fb_diff;

	// TODO: round and normalize
	// TODO: send to DAC to ASI
}

void brake_ctrl()
{
	// we have three sources of data for "distance to end of tunnel":
	// * front laser
	// * contrast sensor
	// * accelerometer integration

	// use the minimum between front laser and contrast sensor, and if both of them fail, then use accelerometer integration

	// pos variables range is 0 to 100 for simplicity, 0 to 100 maps to the minimum and maximum of the stepper potentiometer readings
	float brake_pos = pid_calc(&nvm.pid_eddy_braking, current_speed, desired_speed);
	float gimbal_pos = pid_calc(&nvm.pid_gimbal_braking, current_speed, desired_speed);
	float pitch = attitude->pitch > 0 ? 0 : attitude->pitch;
	gimbal_pos -= pid_calc(&nvm.pid_gimbal_compensation, pitch, 0);

	// gimbal should not speed up the pod at all during braking
	gimbal_pos = gimbal_pos < 50 ? 50 : gimbal_pos;

	brake_pos = brake_pos < 0 ? 0 : brake_pos;
	gimbal_pos = gimbal_pos > 100 ? 100 : gimbal_pos;
	brake_pos = brake_pos > 100 ? 100 : brake_pos;
}

float percent_to_stepper_target(stepper_t* stepper, float percentage)
{
	float x = stepper->consts->adc_max - stepper->consts->adc_min;
	x *= percentage;
	x /= 100.0;
	x += stepper->consts->adc_min;
	x = x > stepper->consts->adc_max ? stepper->consts->adc_max : x;
	x = x < stepper->consts->adc_min ? stepper->consts->adc_min : x;
	return x;
}

int main()
{
	return 0;
}