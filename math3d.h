#ifndef _MATH3D_H_
#define _MATH3D_H_

#include "rpod_types.h"

typedef struct
{
	float a;
	float b;
	float c;
	float d;
}
plane_t;

typedef struct
{
	float x;
	float y;
}
point2d_t;

typedef struct
{
	float x;
	float y;
	float z;
}
point3d_t;

typedef struct
{
	float roll;
	float pitch;
	float center_height;
	float min_height;
	float corner_height[4];
}
attitude_t;

// we need 3 laser readings to the ground to determine roll, pitch, and center height
// we also need the positions of the corners just in case one engine is too close to the ground
typedef struct
{
	point3d_t laser_pos[3];
	point3d_t corner_pos[4];
}
pod_coords_t;

typedef struct
{
	point3d_t results[3];
}
laser_readings_t;

void hoverattitude_calculate(pod_coords_t* podcoords, laser_readings_t* readings, attitude_t* result);

#endif