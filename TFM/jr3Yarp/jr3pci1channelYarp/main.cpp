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
    yarp::os::Port port;
    port.open("/jr3:o");

    six_axis_array fm;
    force_array fs;
    int ret, fd;
    int f0,f1,f2,m0,m1,m2; // F-T from the sensor
    float fx, fy, fz, mx, my, mz; // Scaled F-T
    float xzmp, yzmp;

    if ((fd=open("/dev/jr3",O_RDWR)) < 0) {
        perror("Can't open device. No way to read force!");
    }
	
    ret=ioctl(fd,IOCTL0_JR3_GET_FULL_SCALES,&fs);
    printf("Full scales are %d %d %d %d %d %d\n",fs.f[0],fs.f[1],fs.f[2],fs.m[0],fs.m[1],fs.m[2]);
    ret=ioctl(fd,IOCTL0_JR3_ZEROOFFS);

while (1) {
    ret=ioctl(fd,IOCTL0_JR3_FILTER0,&fm);

    if (ret!=-1) {
        printf("Reading device ...");
        yarp::os::Bottle b;

        f0 = 100*fm.f[0]*fs.f[0]/16384;
        f1 = 100*fm.f[1]*fs.f[1]/16384;
        f2 = 100*fm.f[2]*fs.f[2]/16384;

        m0 = 10*fm.m[0]*fs.m[0]/16384;
        m1 = 10*fm.m[1]*fs.m[1]/16384;
        m2 = 10*fm.m[2]*fs.m[2]/16384;

        printf("Sensor data :\n");
        printf("[%d,%d,%d,%d,%d,%d]\n",f0,f1,f2,m0,m1,m2);
        // Other sensor data:
        //printf("%d\n",(short)fs.v[1]/16384);
        //printf("%d\n",(short)fs.v[2]/16384);

        fx = (float) f0/100;
        fy = (float) f1/100;
        fz = (float) f2/100;
        mx = (float) m0/10;
        my = (float) m1/10;
        mz = (float) m2/10;

        printf("F = [%f, %f, %f] N\n", fx,fy,fz);
        printf("M = [%f, %f, %f] N·m\n", mx,my,mz);

        /** ZMP equations: Single Support **/
        xzmp = -my / fz;
        yzmp = mx / fz;

        printf("ZMP = [ %f ,%f]\n", xzmp, yzmp);

        /** SEND DATA **/
        if (fz == 0.0) {
            printf ("Warning: No zmp data to send\n");
        } else {
            b.addDouble(xzmp);
            b.addDouble(yzmp);
            port.write(b);
            printf("Data sent\n");

        }


    } else perror("");

    //Sample time = 1ms
    usleep(100000); // delay in microseconds
}
    close(fd);
}

