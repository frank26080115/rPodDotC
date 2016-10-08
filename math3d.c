#include "math3d.h"
#include <math.h>
#include <stdlib.h>

void hoverattitude_calculate(pod_coords_t* podcoords, laser_readings_t* readings, attitude_t* result)
{
	plane_t ground;
	point3d_t vec1, vec2, cxprod;
	float d;
	int i;

	// calculate two vectors in the plane
	vec1.x = readings->results[0].x - readings->results[1].x;
	vec1.y = readings->results[0].y - readings->results[1].y;
	vec1.z = readings->results[0].z - readings->results[1].z;
	vec2.x = readings->results[1].x - readings->results[2].x;
	vec2.y = readings->results[1].y - readings->results[2].y;
	vec2.z = readings->results[1].z - readings->results[2].z;

	// calculate the cross product of the vectors
	// to get a vector normal to the plane
	cxprod.x = vec1.y * vec2.z - vec1.z * vec2.y;
	cxprod.x = vec1.z * vec2.x - vec1.x * vec2.z;
	cxprod.x = vec1.x * vec2.y - vec1.y * vec2.x;

	// the normal vector should be pointed in the +Z direction
	// tt affects which side of the plane has negative distances
	if (cxprod.z < 0) {
		cxprod.x *= -1;
		cxprod.y *= -1; 
		cxprod.z *= -1;
	}

	// plane in 3D: Ax + By + Cz + D = 0
	// A, B, C is the vector normal to the plane
	// use one of our original points to calculate D
	d = -1 * (cxprod.x*readings->results[0].x + cxprod.y*readings->results[0].y + cxprod.z*readings->results[0].z);

	ground.a = cxprod.x;
	ground.b = cxprod.y;
	ground.c = cxprod.z;
	ground.d = d;

	// calculate the height of each corner of the pod
	// track the lowest point
	result->min_height = 9999999;
	for (i = 0; i < 4; i++)
	{
		float tmp;
		point3d_t* corner = &podcoords->corner_pos[i];
		// calculate distance from a point to a plane
		tmp = result->corner_height[i] = (ground.a*corner->x + ground.b*corner->y + ground.c*corner->z + ground.d) / sqrt((double)(ground.a*ground.a + ground.b*ground.b + ground.c*ground.c));
		if (tmp < result->min_height) {
			result->min_height = tmp;
		}
	}

	result->center_height = -ground.z / ground.c;

	vect1.x = 1; vect1.y = 0; vect1.z = 0;
	result->roll = acos((double)((vec1.x*plane.a + vec1.y*plane.b + vec1.z*plane.c) / sqrt((double)(plane.a*plane.a + plane.b*plane.b + plane.c*plane.c))))*(180.0/M_PI);

	vect1.x = 0; vect1.y = 1; vect1.z = 0;
	result->pitch = acos((double)((vec1.x*plane.a + vec1.y*plane.b + vec1.z*plane.c) / sqrt((double)(plane.a*plane.a + plane.b*plane.b + plane.c*plane.c))))*(180.0/M_PI);
}