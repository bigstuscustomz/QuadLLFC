#include "response.h"
#include "../sys/tasks.h"
#include "state.h"
#include "../systick/systick.h"
#include "movement.h"

static struct response_controller_t _rc;
static struct task_t _response_task;

static int _response_is_on;

static float pid_gains_matrix[2][4][4];
/*
		{ 0,        PID_T_P, -PID_Z_P,  PID_Y_P },
		{ PID_T_P, 0,         PID_Z_P, PID_Y_P },
		{ 0,        -PID_T_P,-PID_Z_P,  PID_Y_P },
		{ -PID_T_P,  0,       PID_Z_P, PID_Y_P }
*/

static float pid_gains_d[4][4];
/*
		{ 0,        -PID_T_D, PID_Z_D,  PID_Y_D },
		{ -PID_T_D, 0,        -PID_Z_D, PID_Y_D },
		{ 0,        PID_T_D,  PID_Z_D,  PID_Y_D },
		{ PID_T_D,  0,        -PID_Z_D, PID_Y_D }
*/

static float pid_gains[2][AXIS_CNT];

void response_set_gains(int p_d) {
	pid_gains_matrix[p_d][0][0] = 0;
	pid_gains_matrix[p_d][0][1] = pid_gains[p_d][AxisPitch];
	pid_gains_matrix[p_d][0][2] = -pid_gains[p_d][AxisYaw];
	pid_gains_matrix[p_d][0][3] = pid_gains[p_d][AxisY];

	pid_gains_matrix[p_d][1][0] = pid_gains[p_d][AxisRoll];
	pid_gains_matrix[p_d][1][1] = 0;
	pid_gains_matrix[p_d][1][2] = pid_gains[p_d][AxisYaw];
	pid_gains_matrix[p_d][1][3] = pid_gains[p_d][AxisY];

	pid_gains_matrix[p_d][2][0] = 0;
	pid_gains_matrix[p_d][2][1] = -pid_gains[p_d][AxisPitch];
	pid_gains_matrix[p_d][2][2] = -pid_gains[p_d][AxisYaw];
	pid_gains_matrix[p_d][2][3] = pid_gains[p_d][AxisY];

	pid_gains_matrix[p_d][3][0] = -pid_gains[p_d][AxisRoll];
	pid_gains_matrix[p_d][3][1] = 0;
	pid_gains_matrix[p_d][3][2] = -pid_gains[p_d][AxisYaw];
	pid_gains_matrix[p_d][3][3] = pid_gains[p_d][AxisY];
}

void response_set_p_gain(int axis, float value) {
	pid_gains[0][axis] = value;
	response_set_gains(0);
}

void response_set_d_gain(int axis, float value) {
	pid_gains[1][axis] = value;
	response_set_gains(1);
}

float response_get_p_gain(int axis) {
	return pid_gains[0][axis];
}

float response_get_d_gain(int axis) {
	return pid_gains[1][axis];
}

static float response_state_last[AXIS_CNT];

/* This gets called every CFG_RESPONSE_UPDATE_MSECS */
void response_update(struct task_t *task)
{
	// Ability to turn off response system
	// Safety feature, DO NOT DISABLE!
	if(!_response_is_on)
		return;

	struct state_controller_t *sc;
	sc = stateControllerGet();

	// Figure out error
	float state_error[2][AXIS_CNT];
	float state_diff[AXIS_CNT];
	// P error
	stateSubtract(_rc.state_setpoint, sc->inertial_state, state_error[0]);
	// D error
	stateSubtract(sc->inertial_state, response_state_last, state_diff);
	// Convert to rad / s
	stateScale(state_diff, (1000 / CFG_RESPONSE_UPDATE_MSECS));
	stateSubtract(_rc.state_dt_setpoint, state_diff, state_error[1]);

	// Store state in last_state
	stateCopy(sc->inertial_state, response_state_last);

	float output[4];
	output[0] = 0;
	output[1] = 0;
	output[2] = 0;
	output[3] = 0;

	// Multiply gains for error P and D
	int i, j;
	for(i = 0;i < 2;i++) {
		for(j = 0;j < 4;j++) {
			output[j] += state_error[i][AxisRoll]*pid_gains_matrix[i][j][0] + state_error[i][AxisPitch]*pid_gains_matrix[i][j][1] +
			             state_error[i][AxisPitch]*pid_gains_matrix[i][j][2] + state_error[i][AxisY]*pid_gains_matrix[i][j][3];
		}
	}

	// Square outputs to account for motor input -> thrust conversion
	for(i = 0;i<4;i++)
		output[i] *= output[i];

	// output to motors
	motors_set(output);
}

void response_start(void)
{
	int i = 0;

	// Init _rc
	for(i = 0;i < AXIS_CNT;++i) {
		_rc.state_setpoint[i] = 0;
		_rc.state_dt_setpoint[i] = 0;
	}

	// Setup response update task
	_response_task.handler = response_update;
	_response_task.msecs = CFG_RESPONSE_UPDATE_MSECS;
	tasks_add_task(&_response_task);

	response_off();
}

struct response_controller_t *response_controller_get(void)
{
	return &_rc;
}

void response_on(void)
{
	_response_is_on = 1;
}

void response_off(void)
{
	_response_is_on = 0;
}
