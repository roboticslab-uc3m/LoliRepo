#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <cmath>

#include <yarp/os/all.h>
#include <yarp/dev/all.h>

#define PI  3.141592
#define TS 0.03
#define L   1.03 // Pendulum Longitude
#define g   9.81 // Gravity in m/s²

using namespace std;
