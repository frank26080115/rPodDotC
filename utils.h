#ifndef _UTILS_H_
#define _UTILS_H_

#define round_and_constrain(x, low, high) lround(x > high ? high : (x < low ? low : x))

#endif