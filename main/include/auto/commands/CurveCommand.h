/*----------------------------------------------------------------------------*/
/* Copyright (c) 2017-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#include <Commands/Command.h>
#include <RobotModel.h>
#include <PIDInputSource.h>
#include <PIDOutputSource.h>

class CurveCommand : public frc::Command {
 public:
  CurveCommand(RobotModel *robot, double desiredRadius, double desiredAngle);
  void Init();
  void Update();

 private:
  double CalcCurDesiredAngle(double curPivDistance);
  void LoadPIDValues();


  NavXPIDSource *navXPIDSource_;
  TalonEncoderPIDSource *talonEncoderPIDSource_;
  AnglePIDOutput *anglePIDOutput_;
  DistancePIDOutput *distancePIDOutput_;
  PIDController *dPID_, *tPID_;

  double dPFac_, dIFac_, dDFac_;
  double tPFac_, tIFac_, tDFac_;

  RobotModel *robot_;
  double initAngle_;
  double desiredRadius_, desiredAngle_;
  double curPivDistance_, curDesiredAngle_, curAngle_;

  double curAngleError_;

  bool turnLeft_;

  bool isDone_;

  NetworkTableEntry dOutputNet_, tOutputNet_, dErrorNet_, tErrorNet_, dPFacNet_, dIFacNet_, dDFacNet_, tPFacNet_, tIFacNet_, tDFacNet_;
};
