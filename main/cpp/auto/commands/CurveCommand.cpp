/*----------------------------------------------------------------------------*/
/* Copyright (c) 2017-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "../../../include/auto/commands/CurveCommand.h"
#include <math.h>
#include <cmath>

#define PI 3.141592653589 // TODO CHANGE, this is a mess

CurveCommand::CurveCommand(RobotModel *robot, double desiredRadius, double desiredAngle, bool turnLeft,
  NavXPIDSource* navXSource, TalonEncoderPIDSource* talonEncoderSource,
	AnglePIDOutput* anglePIDOutput, DistancePIDOutput* distancePIDOutput) : AutoCommand() { //using absolute angle, radius is inside wheel
  
  robot_ = robot;
  desiredRadius_ = desiredRadius;
  desiredAngle_ = desiredAngle;
  turnLeft_ = turnLeft;

  navXPIDSource_ = navXSource;
  talonEncoderPIDSource_ = talonEncoderSource;
  anglePIDOutput_ = anglePIDOutput;
  distancePIDOutput_ = distancePIDOutput;

  dPFacNet_ =  frc::Shuffleboard::GetTab("Private_Code_Input").Add("Curve dP", 0.8).GetEntry();
  dIFacNet_ =  frc::Shuffleboard::GetTab("Private_Code_Input").Add("Curve dI", 0.0).GetEntry();
  dDFacNet_ =  frc::Shuffleboard::GetTab("Private_Code_Input").Add("Curve dD", 0.2).GetEntry();

  tPFacNet_ =  frc::Shuffleboard::GetTab("Private_Code_Input").Add("Curve tP", 0.07).GetEntry();
  tIFacNet_ =  frc::Shuffleboard::GetTab("Private_Code_Input").Add("Curve tI", 0.0).GetEntry();
  tDFacNet_ =  frc::Shuffleboard::GetTab("Private_Code_Input").Add("Curve tD", 0.0).GetEntry();

  
  dOutputNet_ = frc::Shuffleboard::GetTab("Public_Display").Add("Curve dO", 0.0).GetEntry(); 
  tOutputNet_ = frc::Shuffleboard::GetTab("Public_Display").Add("Curve tO", 0.0).GetEntry(); 
  lOutputNet_ = frc::Shuffleboard::GetTab("Public_Display").Add("Curve lO", 0.0).GetEntry();
  rOutputNet_ = frc::Shuffleboard::GetTab("Public_Display").Add("Curve rO", 0.0).GetEntry();
  dErrorNet_ = frc::Shuffleboard::GetTab("Public_Display").Add("Curve dErr", 0.0).GetEntry(); 
  tErrorNet_ = frc::Shuffleboard::GetTab("Public_Display").Add("Curve tErr", 0.0).GetEntry(); 

  pidSourceNet_ = frc::Shuffleboard::GetTab("Public_Display").Add("Curve PID Get", 0.0).GetEntry(); 
}

CurveCommand::CurveCommand(RobotModel *robot, double desiredRadius, double desiredAngle,
  NavXPIDSource* navXSource, TalonEncoderPIDSource* talonEncoderSource,
	AnglePIDOutput* anglePIDOutput, DistancePIDOutput* distancePIDOutput) : AutoCommand() { //using absolute angle, radius is inside wheel
  
  robot_ = robot;
  desiredRadius_ = desiredRadius;
  desiredAngle_ = abs(desiredAngle);
  turnLeft_ = desiredAngle_<0;

  navXPIDSource_ = navXSource;
  talonEncoderPIDSource_ = talonEncoderSource;
  anglePIDOutput_ = anglePIDOutput;
  distancePIDOutput_ = distancePIDOutput;

  dPFacNet_ =  frc::Shuffleboard::GetTab("Private_Code_Input").Add("Curve dP", 0.8).GetEntry();
  dIFacNet_ =  frc::Shuffleboard::GetTab("Private_Code_Input").Add("Curve dI", 0.0).GetEntry();
  dDFacNet_ =  frc::Shuffleboard::GetTab("Private_Code_Input").Add("Curve dD", 0.2).GetEntry();

  tPFacNet_ =  frc::Shuffleboard::GetTab("Private_Code_Input").Add("Curve tP", 0.07).GetEntry();
  tIFacNet_ =  frc::Shuffleboard::GetTab("Private_Code_Input").Add("Curve tI", 0.0).GetEntry();
  tDFacNet_ =  frc::Shuffleboard::GetTab("Private_Code_Input").Add("Curve tD", 0.0).GetEntry();

  
  dOutputNet_ = frc::Shuffleboard::GetTab("Public_Display").Add("Curve dO", 0.0).GetEntry(); 
  tOutputNet_ = frc::Shuffleboard::GetTab("Public_Display").Add("Curve tO", 0.0).GetEntry(); 
  lOutputNet_ = frc::Shuffleboard::GetTab("Public_Display").Add("Curve lO", 0.0).GetEntry();
  rOutputNet_ = frc::Shuffleboard::GetTab("Public_Display").Add("Curve rO", 0.0).GetEntry();
  dErrorNet_ = frc::Shuffleboard::GetTab("Public_Display").Add("Curve dErr", 0.0).GetEntry(); 
  tErrorNet_ = frc::Shuffleboard::GetTab("Public_Display").Add("Curve tErr", 0.0).GetEntry(); 

  pidSourceNet_ = frc::Shuffleboard::GetTab("Public_Display").Add("Curve PID Get", 0.0).GetEntry(); 
}

void CurveCommand::Init(){

  initAngle_ = robot_->GetNavXYaw();

  curAngle_ = initAngle_;
  curPivDistance_ = 0.0;
  curDesiredAngle_ = curAngle_;

  //if(curAngle_>desiredAngle_) turnLeft_ = true;
  //else turnLeft_ = false;

  curAngleError_ = 0.0;

  robot_->SetTalonCoastMode();
  robot_->SetHighGear(); //TODO tune/fix
  robot_->ResetDriveEncoders(); //TODODODODODODOD FIXXXXXXXXXXXXXXXXXXXXXX

  LoadPIDValues();

  //navXPIDSource_ = new NavXPIDSource(robot_);
  //talonEncoderPIDSource_ = new TalonEncoderPIDSource(robot_);
  //anglePIDOutput_ = new AnglePIDOutput();
  //distancePIDOutput_ = new DistancePIDOutput();

  dPID_ = new PIDController(dPFac_, dIFac_, dDFac_, talonEncoderPIDSource_, distancePIDOutput_);
  tPID_ = new PIDController(tPFac_, tIFac_, tDFac_, navXPIDSource_, anglePIDOutput_);

  tPID_->SetPID(tPFac_, tIFac_, tDFac_);
	dPID_->SetPID(dPFac_, dIFac_, dDFac_);

  dPID_->SetSetpoint(desiredAngle_*desiredRadius_*PI/180);//2*PI*desiredRadius_/(360/desiredAngle_));
  tPID_->SetSetpoint(desiredAngle_);

  dPID_->SetAbsoluteTolerance(3.0/12.0); //this too U DUDE
  tPID_->SetAbsoluteTolerance(0.5); //HM TUNE TODODODODODOD

  tPID_->SetContinuous(true);
  tPID_->SetInputRange(-180, 180);
	dPID_->SetContinuous(false);
	//tPID_->SetInputRange(-180, 180);

  tPID_->SetOutputRange(-0.9, 0.9);
	dPID_->SetOutputRange(-0.9, 0.9); //MAKE VARIABLES TODO TODO     //adjust for 2019
	//tPID_->SetAbsoluteTolerance(0.3);	 //MAKE VARIABLES TODO TODO   //adjust for 2019

  dPID_->Enable();
  tPID_->Enable();

}

void CurveCommand::Reset(){
  robot_->SetDriveValues(RobotModel::kAllWheels, 0.0);
  
	// destroy angle PID
	if (tPID_ != NULL) {
		tPID_->Disable();
    tPID_->~PIDController();

		delete tPID_;

		tPID_ = NULL;

		printf("Reset Angle PID %f \n", robot_->GetNavXYaw());
	}

	// destroy distance PID
	if (dPID_ != NULL) {
		dPID_->Disable();
    dPID_->~PIDController();

		delete dPID_;

		dPID_ = NULL;
//		printf("Reset Distance PID");

	}
	isDone_ = true;
}

void CurveCommand::Update(double currTimeSec, double deltaTimeSec){ //TODO add timeout!

  pidSourceNet_.SetDouble(talonEncoderPIDSource_->PIDGet());

  if(dPID_->OnTarget() && tPID_->OnTarget()){ //TODO add timeout here, also TODO possible source of error if one done and one not?
    printf("%f Original Desired Distance: %f\n"
        "Final NavX Angle from PID Source: %f\n"
				"Final NavX Angle from robot: %f \n"
        "Final Distance from PID Source: %f\n",
				robot_->GetTime(), desiredAngle_*desiredRadius_*PI/180, navXPIDSource_->PIDGet(), robot_->GetNavXYaw(), talonEncoderPIDSource_->PIDGet());
    if(turnLeft_){
      printf("Final Distance from robot: %f\n", robot_->GetRightDistance());//robot_->GetLeftDistance()); /fixed inversion
    } else {
      printf("Final Distance from robot: %f\n", robot_->GetLeftDistance());
    }
		//Reset();
		isDone_ = true;
		robot_->SetDriveValues(RobotModel::kAllWheels, 0.0);
		printf("%f CurveCommand IS DONE \n", robot_->GetTime());
		//if (timeOut) {
		//	printf("%f FROM CURVE TIME OUT GO LEAVEEEEEE %f\n", robot_->GetTime(), timeDiff);
		//}
  } else {

    /* pid vs robot
    if(turnLeft_){
      curPivDistance_ = robot_->GetRightDistance();//robot_->GetLeftDistance();
    } else {
      curPivDistance_ = robot_->GetLeftDistance();
    }*/
    curPivDistance_ = talonEncoderPIDSource_->PIDGet();
    curDesiredAngle_ = CalcCurDesiredAngle(curPivDistance_);
    curAngleError_ = curDesiredAngle_ - curAngle_;

    tPID_->SetSetpoint(curDesiredAngle_);

    double dOutput = distancePIDOutput_->GetPIDOutput();
    double tOutput = anglePIDOutput_->GetPIDOutput();

    double lOutput;
    double rOutput;

    if(turnLeft_){
      lOutput = dOutput - tOutput; //TODO SKETCH
      rOutput = dOutput + tOutput;
    } else {
      lOutput = dOutput + tOutput; //TODO SKETCH
      rOutput = dOutput - tOutput;

    }
    
    //TODODODODO NEEDED OR IS THIS MESSING WITH THE PID????
    //power output checks
    if(lOutput > 1.0){
      rOutput = rOutput/lOutput;
      lOutput = 1.0;
    } else if (lOutput < -1.0){
      rOutput = rOutput/(-lOutput);
      lOutput = -1.0;
    }
    if(rOutput > 1.0) {
      lOutput = lOutput/rOutput;
      rOutput = 1.0;
    } else if (rOutput < -1.0) {
      lOutput = lOutput/(-rOutput);
      rOutput = -1.0;
    }

    robot_->SetDriveValues(-lOutput, rOutput);
    
    dOutputNet_.SetDouble(dOutput);
    tOutputNet_.SetDouble(tOutput);
    lOutputNet_.SetDouble(lOutput);
    rOutputNet_.SetDouble(rOutput);

    dErrorNet_.SetDouble(2*PI/(360/desiredAngle_) - talonEncoderPIDSource_->PIDGet());
    /*
    pid vs robot
    if(turnLeft_){
      dErrorNet_.SetDouble(2*PI/(360/desiredAngle_) - robot_->GetRightDistance());
    } else {
      dErrorNet_.SetDouble((2*PI/(360/desiredAngle_) - robot_->GetLeftDistance()));
    }
    */
    tErrorNet_.SetDouble(curAngleError_);
  }
}

double CurveCommand::CalcCurDesiredAngle(double curPivDistance){
  double rawAngle = (curPivDistance/desiredRadius_ *180/PI) + initAngle_; //TODO POSSIBLE ERROR WITH INIT ANGLE
  if(turnLeft_){ //CHECK LOGIC??? why is right negative makes no sense
    return -rawAngle;
  } else {
    return rawAngle;
  }
}

void CurveCommand::LoadPIDValues(){
  //TODO add to shuffleboard
  dPFac_ = dPFacNet_.GetDouble(0.8);
  dIFac_ = dIFacNet_.GetDouble(0.0);
  dDFac_ = dDFacNet_.GetDouble(0.0);

  tPFac_ = tPFacNet_.GetDouble(0.07);
  tIFac_ = tIFacNet_.GetDouble(0.0);
  tDFac_ = tDFacNet_.GetDouble(0.0);
}

bool CurveCommand::IsDone(){
  return isDone_;
}


CurveCommand::~CurveCommand(){
  /*dPID_->Disable();
  tPID_->Disable();

  dPID_->~PIDController();
  tPID_->~PIDController();*/
  Reset();

  dOutputNet_.Delete();
  tOutputNet_.Delete();
  lOutputNet_.Delete();
  rOutputNet_.Delete();
  dErrorNet_.Delete();
  tErrorNet_.Delete();

  dPFacNet_.Delete();
  dIFacNet_.Delete();
  dDFacNet_.Delete();

  tPFacNet_.Delete();
  tIFacNet_.Delete();
  tDFacNet_.Delete();

}