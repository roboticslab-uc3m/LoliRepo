/*******************************************************
 * main.cpp
 * @Brief: Program for reading forces and torques form two jr3 F/T sensors using jr3pcidriver.
 *      The information read from the sensors is written in a YARP port.
 * @Author: Loli Pinel
 * @Date: September 2015
 *******************************************************/


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/ioctl.h>

#include <yarp/os/all.h>

#include "jr3pci-ioctl.h"

int main(void) {

    yarp::os::Network yarp;
    yarp::os::Port port0;
    yarp::os::Port port1;

    port0.open("/jr3ch0:o");
    port1.open("/jr3ch1:o");

    six_axis_array fm0, fm1;
    force_array fs0, fs1;
    int ret, fd;
    int f0[3], m0[3]; // F-T from the sensor 0 in Newton
    int f1[3], m1[3]; // F-T from the sensor 1 in Newton
    float fx0, fy0, fz0, mx0, my0, mz0; // Scaled F-T from sensor 0 in dN*m (0.1N*m)
    float fx1, fy1, fz1, mx1, my1, mz1; // Scaled F-T from sensor 1 in dN*m (0.1N*m)

    if ((fd=open("/dev/jr3",O_RDWR)) < 0) {
        perror("Can't open device. No way to read force!");
    }

    ret=ioctl(fd,IOCTL0_JR3_GET_FULL_SCALES,&fs0);
    printf("Full scales of Sensor 0 are %d %d %d %d %d %d\n",fs0.f[0],fs0.f[1],fs0.f[2],fs0.m[0],fs0.m[1],fs0.m[2]);
    ret=ioctl(fd,IOCTL1_JR3_GET_FULL_SCALES,&fs1);
    printf("Full scales of Sensor 1 are: %d %d %d %d %d %d\n", fs1.f[0],fs1.f[1],fs1.f[2],fs1.m[0],fs1.m[1],fs1.m[2]);
    ret=ioctl(fd,IOCTL0_JR3_ZEROOFFS);
    ret=ioctl(fd,IOCTL1_JR3_ZEROOFFS);


    while (1) {
        ret=ioctl(fd,IOCTL0_JR3_FILTER0,&fm0);
        ret=ioctl(fd,IOCTL1_JR3_FILTER0,&fm1);

        if (ret!=-1) {
            yarp::os::Bottle b0;
            yarp::os::Bottle b1;

            printf("Reading device ...\n");

            // -------- SENSOR 0 ------------ //
            printf("Sensor 0 : [ ");

            f0[0] = 100*fm0.f[0]*fs0.f[0]/16384;
            f0[1]= 100*fm0.f[1]*fs0.f[1]/16384;
            f0[2] = 100*fm0.f[2]*fs0.f[2]/16384;
            m0[0] = 10*fm0.m[0]*fs0.m[0]/16384;
            m0[1] = 10*fm0.m[1]*fs0.m[1]/16384;
            m0[2] = 10*fm0.m[2]*fs0.m[2]/16384;

            printf("Sensor 0 data :");
            printf("[%d,%d,%d,%d,%d,%d]\n",f00,f01,f02,m00,m01,m02);
            // Other sensor data
            //printf("%d\n",(short)fs0.v[1]/16384);
            //printf("%d\n",(short)fs0.v[2]/16384);

            fx0 = (float) f00/100;
            fy0 = (float) f01/100;
            fz0 = (float) f02/100;
            mx0 = (float) m00/100;
            my0 = (float) m01/100;
            mz0 = (float) m02/100;

            printf("F0 = [%f, %f, %f] N\n", fx0,fy0,fz0);
            printf("M0 = [%f, %f, %f] N·m\n", mx0,my0,mz0);

            // -------- SENSOR 1 ------------ //
            f1[0] = 100*fm1.f[0]*fs1.f[0]/16384;
            f1[1] = 100*fm1.f[1]*fs1.f[1]/16384;
            f1[2] = 100*fm1.f[2]*fs1.f[2]/16384;
            m1[0] = 10*fm1.m[0]*fs1.m[0]/16384;
            m1[1] = 10*fm1.m[1]*fs1.m[1]/16384;
            m1[2] = 10*fm1.m[2]*fs1.m[2]/16384;

            printf("Sensor 1 data :");
            printf("[%d,%d,%d,%d,%d,%d]\n",f10,f11,f12,m10,m11,m12);
            // Other sensor data
            //printf("%d\n",(short)fs1.v[1]/16384);
            //printf("%d\n",(short)fs1.v[2]/16384);

            fx1 = (float) f10/100;
            fy1 = (float) f11/100;
            fz1 = (float) f12/100;
            mx1 = (float) m10/100;
            my1 = (float) m11/100;
            mz1 = (float) m12/100;

            printf("F1 = [%f, %f, %f] N\n", fx1,fy1,fz1);
            printf("M1 = [%f, %f, %f] N·m\n", mx1,my1,mz1);


            /** SEND DATA **/
            b0.addDouble(fx0);
            b0.addDouble(fy0);
            b0.addDouble(fz0);
            b0.addDouble(mx0);
            b0.addDouble(my0);
            b0.addDouble(mz0);

            b1.addDouble(fx1);
            b1.addDouble(fy1);
            b1.addDouble(fz1);
            b1.addDouble(mx1);
            b1.addDouble(my1);
            b1.addDouble(mz1);

            port0.write(b0);
            port1.write(b1);

        } else perror("");

        //Sample time = 1ms
        usleep(100000); // delay in microseconds
    }
    close(fd);
}

