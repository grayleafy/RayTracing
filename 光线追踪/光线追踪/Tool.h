#pragma once
#ifndef __Tool_h__
#define __Tool_h__

#include "time.h"
#include <stdlib.h>

void CPURandomInit() {
	srand(time(NULL));
}

float GetCPURandom() {
	return (float)rand() / (RAND_MAX + 1.0);
}


#endif



