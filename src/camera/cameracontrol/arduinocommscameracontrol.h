/*
* Copyright I3D Robotics Ltd, 2020
* Author: Ben Knight (bknight@i3drobotics.com)
*/

#ifndef ARDUINOCOMMSCAMERACONTROL_H
#define ARDUINOCOMMSCAMERACONTROL_H

#include "abstractarduinocoms.h"

//!  Arduino comms camera control
/*!
  Commuicate with arduino for camera control via serial (QSerialPort)
*/
class ArduinoCommsCameraControl : public AbstractArduinoComs
{
public:
    explicit ArduinoCommsCameraControl(QObject *parent = 0) :
        AbstractArduinoComs(parent)
    {}

    void updateFPS(int fps);
};

#endif // ARDUINOCOMMSCAMERACONTROL_H
