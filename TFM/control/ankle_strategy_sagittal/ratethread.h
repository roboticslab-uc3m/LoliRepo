#ifndef _ratethread_H_
#define _ratethread_H_

#include "LIPM2d.h"

static FILE *fp;

yarp::os::Port port0;
yarp::os::Port port1;
yarp::dev::IPositionControl *posRightLeg;
yarp::dev::IPositionControl *posLeftLeg;
yarp::dev::IVelocityControl *velRightLeg;
yarp::dev::IVelocityControl *velLeftLeg;

class MyRateThread : public yarp::os::RateThread
{
public:
    MyRateThread() : yarp::os::RateThread(TS*1000.0){
        n = 1;
        e = 0.0255; // distance from the ground to sensor center in metres
        sum = 0.0;
        offs_x = 0.0;
        offs_y = 0.0;
    	X = 0.0;
        angle_x = 0.0;
        angle_x_pre = 0.0;
	}
    void run(){
        printf("----------\n Running\n");
        _dt = n*TS;
        if (n <= 300){ref = 0.0;}
        else if (n >= 300 && n <= 330){ref = (0.01/30)*n - 0.1;}
        else {ref = ref;}
        //ref = 0.0;
        getInitialTime();
        readFTSensor();
        zmpComp();
        evaluateModel();
        setJoints();
        printData();
        saveToFile();
        n++;
        getCurrentTime();
        double t = curr_time - init_loop;
        printf("LoopTime = %f\n",t);
    }
    void getInitialTime()
    {
        init_loop = yarp::os::Time::now();
    }
    void getCurrentTime(){
        curr_time = yarp::os::Time::now();
    }

    void readFTSensor(){
        yarp::os::Bottle b0;
        yarp::os::Bottle b1;
        port0.read(b0);
        port1.read(b1);

        _fx0 = b0.get(0).asDouble();
        _fy0 = b0.get(1).asDouble();
        _fz0 = b0.get(2).asDouble();
        _mx0 = b0.get(3).asDouble();
        _my0 = b0.get(4).asDouble();

        _fx1 = b1.get(0).asDouble();
        _fy1 = b1.get(1).asDouble();
        _fz1 = b1.get(2).asDouble();
        _mx1 = b1.get(3).asDouble();
        _my1 = b1.get(4).asDouble();
    }

    void zmpComp(){
        /** ZMP Equations : Double Support **/
        _xzmp0 = -(_my0 + e*_fx0) / _fz0;
        _yzmp0 = (_mx0 + e*_fy0) / _fz0;

        _xzmp1 = -(_my1 + e*_fx1) /_fz1;
        _yzmp1 = (_mx1 + e*_fy1) /_fz1;

        _xzmp = (_xzmp0 * _fz0 + _xzmp1 * _fz1) / (_fz0 + _fz1);
        _yzmp = (_yzmp0 * _fz0 + _yzmp1 * _fz1) / (_fz0 + _fz1);



        // OFFSET
        if (n >=1 && n < 50){
            sum = _xzmp + sum;
            offs_x = sum / n;
            printf("offs = %f\n", offs_x);
        }

        X  = (_xzmp - offs_x);
        _yzmp = _yzmp - offs_y;

        if ((_xzmp != _xzmp) || (_yzmp != _yzmp)){
            printf ("Warning: No zmp data\n");
        }
    }

    void evaluateModel(){
        /** EVALUACION MODELO **/
        _eval_x.model(X, ref);

        // Angle sent to ankle joints
        angle_x = -(_eval_x.y-(4.3948*pow(_eval_x.y,2))+0.23*_eval_x.y)/0.0135;
        angVel = 0.22*((angle_x - angle_x_pre)/TS);
        angle_x_pre = angle_x;
    }

    void setJoints(){
        /** Position control **/
//        posRightLeg->positionMove(4, angle_x); // position in degrees
//        posLeftLeg->positionMove(4, angle_x);

        /** Velocity control **/
        velRightLeg->velocityMove(4, angVel); // velocity in degrees per second
        velLeftLeg->velocityMove(4, angVel);
    }
    void printData(){
        cout << "t = " << _dt << endl;
        cout << "ZMP = [" << X << ", " << _yzmp << "]" << endl;
//        cout << "Azmp = " << _eval_x._zmp_error << endl;
//        cout << "x_model = " << _eval_x.y << endl;
//        cout << "x1[0] = " << _eval_x._x1[0] << endl;
//        cout << "Ud = " << _eval_x._u_ref << endl;
//        cout << "u = " << _eval_x._u << endl;
//        cout << "angle_x = " << angle_x << endl;
//        cout << "angle_x_pre = " << angle_x_pre << endl;
        printf("angle_x = %.15f\n", angle_x);
        cout << "angVel = " << angVel << endl;

    }
    void saveToFile()
    {
        fprintf(fp,"\n%d", n);
        fprintf(fp,",%.4f", _dt);
        fprintf(fp,",%.15f", X);
        fprintf(fp,",%.15f", _eval_x.y);
        fprintf(fp,",%.15f", _eval_x._zmp_error);
        fprintf(fp,",%.15f", _eval_x._zmp_ref);
        fprintf(fp,",%10f", _eval_x._u);
	fprintf(fp,",%10f", _eval_x._x1[0]);
	fprintf(fp,",%10f", _eval_x._x2[0]);
        //fprintf(fp,",%f", angle_x);
        fprintf(fp,",%f", angVel);
    }


private:
    int n;
    LIPM2d _eval_x;

    float _fx0, _fy0, _fz0, _mx0, _my0; // F-T from sensor 0
    float _fx1, _fy1, _fz1, _mx1, _my1; // F-T from sensor 1

    float e; // distance [m] between ground and sensor center
    float offs_x; // zmp offset in initial time.
    float offs_y;
    float sum;
    float _xzmp0, _yzmp0; // ZMP sensor 0
    float _xzmp1, _yzmp1; // ZMP sensor 1
    float _xzmp; // Global x_ZMP
    float _yzmp; // Global y_ZMP
    float X;
    float angle_x, angle_x_pre;
    float angle_y;
    float angVel; //Angular velocity in degrees per second

    double init_time, init_loop, curr_time, _dt;

    float ref;
};

#endif //_ratethread_H_
