/*----------------------------------------------------------------------------*/
/* Copyright (c) 2017-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "CurveCommand.h"
#include <math.h>

#define PI 3.141592653589 //TODO CHANGE, this is a mess

CurveCommand::CurveCommand(RobotModel *robot, double desiredRadius, double desiredAngle) { //using absolute angle, radius is inside wheel
  robot_ = robot;
  desiredRadius_ = desiredRadius;
  desiredAngle_ = desiredAngle;

  dOutputNet_ = frc::Shuffleboard::GetTab("PRINTSTUFFSYAYS").Add("Curve dO", 0.0).GetEntry(); 
  tOutputNet_ = frc::Shuffleboard::GetTab("PRINTSTUFFSYAYS").Add("Curve tO", 0.0).GetEntry(); 
  dErrorNet_ = frc::Shuffleboard::GetTab("PRINTSTUFFSYAYS").Add("Curve dE", 0.0).GetEntry(); 
  tErrorNet_ = frc::Shuffleboard::GetTab("PRINTSTUFFSYAYS").Add("Curve tE", 0.0).GetEntry(); 

  dPFacNet_ =  frc::Shuffleboard::GetTab("Private_Code_Input").Add("Curve dP", 0.8).GetEntry();
  dIFacNet_ =  frc::Shuffleboard::GetTab("Private_Code_Input").Add("Curve dI", 0.0).GetEntry();
  dDFacNet_ =  frc::Shuffleboard::GetTab("Private_Code_Input").Add("Curve dD", 0.2).GetEntry();

  tPFacNet_ =  frc::Shuffleboard::GetTab("Private_Code_Input").Add("Curve tP", 0.8).GetEntry();
  tIFacNet_ =  frc::Shuffleboard::GetTab("Private_Code_Input").Add("Curve tI", 0.0).GetEntry();
  tDFacNet_ =  frc::Shuffleboard::GetTab("Private_Code_Input").Add("Curve tD", 0.2).GetEntry();
}

void CurveCommand::Init(){

  initAngle_ = robot_->GetNavXYaw();

  curAngle_ = initAngle_;
  curPivDistance_ = 0.0;
  curDesiredAngle_ = curAngle_;

  if(curAngle_>desiredAngle_) turnLeft_ = true;
  else turnLeft_ = false;

  curAngleError_ = 0.0;

  robot_->SetTalonCoastMode();
  robot_->SetHighGear(); //TODO tune/fix
  robot_->ResetDriveEncoders();

  LoadPIDValues();

  navXPIDSource_ = new NavXPIDSource(robot_);
  talonEncoderPIDSource_ = new TalonEncoderPIDSource(robot_);
  anglePIDOutput_ = new AnglePIDOutput();
  distancePIDOutput_ = new DistancePIDOutput();
  dPID_ = new PIDController(dPFac_, dIFac_, dDFac_, talonEncoderPIDSource_, distancePIDOutput_);
  tPID_ = new PIDController(tPFac_, tIFac_, tDFac_, navXPIDSource_, anglePIDOutput_);

  dPID_->SetSetpoint(2*PI*desiredRadius_/(360/desiredAngle_));
  tPID_->SetSetpoint(curAngleError_);

  tPID_->SetContinuous(true);
	//tPID_->SetInputRange(-180, 180);
	tPID_->SetOutputRange(-0.9, 0.9); //MAKE VARIABLES TODO TODO     //adjust for 2019
	tPID_->SetAbsoluteTolerance(0.3);	 //MAKE VARIABLES TODO TODO   //adjust for 2019

  dPID_->Enable();
  tPID_->Enable();

}

void CurveCommand::Update(){

  if(dPID_->OnTarget() && tPID_->OnTarget()){ //TODO add timeout here, also TODO possible source of error if one done and one not?
    printf("%f Final NavX Angle from PID Source: %f\n"
				"Final NavX Angle from robot: %f \n"
        "Final Distance from PID Source: %f\n",
				robot_->GetTime(), navXPIDSource_->PIDGet(), robot_->GetNavXYaw(), talonEncoderPIDSource_->PIDGet());
    if(turnLeft_){
      printf("Final Distance from robot: %f\n", robot_->GetLeftDistance());
    } else {
      printf("Final Distance from robot: %f\n", robot_->GetRightDistance());
    }
		//Reset();
		isDone_ = true;
		robot_->SetDriveValues(RobotModel::kLeftWheels, 0.0);
		robot_->SetDriveValues(RobotModel::kRightWheels, 0.0);
		printf("%f PIVOT IS DONE \n", robot_->GetTime());
		//if (timeOut) {
		//	printf("%f FROM CURVE TIME OUT GO LEAVEEEEEE %f\n", robot_->GetTime(), timeDiff);
		//}
  } else {

    if(turnLeft_){
      curPivDistance_ = robot_->GetLeftDistance();
    } else {
      curPivDistance_ = robot_->GetRightDistance();
    }
    curDesiredAngle_ = CalcCurDesiredAngle(curPivDistance_);
    curAngleError_ = curDesiredAngle_ - curAngle_;

    tPID_->SetSetpoint(curDesiredAngle_);

    double dOutput = distancePIDOutput_->GetPIDOutput();
    double tOutput = anglePIDOutput_->GetPIDOutput();

    if(turnLeft_){
      robot_->SetDriveValues(RobotModel::kLeftWheels, -dOutput);
      robot_->SetDriveValues(RobotModel::kRightWheels, -tOutput);
    } else {
      robot_->SetDriveValues(RobotModel::kRightWheels, -dOutput);
      robot_->SetDriveValues(RobotModel::kLeftWheels, -tOutput);
    }
    dOutputNet_.SetDouble(dOutput);
    tOutputNet_.SetDouble(tOutput);
    if(turnLeft_){
      dErrorNet_.SetDouble(2*PI/(360/desiredAngle_) - robot_->GetLeftDistance());
    } else {
      dErrorNet_.SetDouble((2*PI/(360/desiredAngle_) - robot_->GetRightDistance());
    }
    tErrorNet_.SetDouble(curAngleError_);
  }
}

double CurveCommand::CalcCurDesiredAngle(double curPivDistance){
  return curPivDistance/desiredRadius_ *180/PI + initAngle_;
}

void CurveCommand::LoadPIDValues(){
  //TODO add to shuffleboard
  dPFac_ = dPFacNet_.GetDouble(0.8);
  dIFac_ = dIFacNet_.GetDouble(0.0);
  dDFac_ = dDFacNet_.GetDouble(0.0);

  tPFac_ = tPFacNet_.GetDouble(0.8);
  tIFac_ = tIFacNet_.GetDouble(0.0);
  tDFac_ = tDFacNet_.GetDouble(0.0);
}