/*----------------------------------------------------------------------------*/
/* Copyright (c) 2018 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "DriveController.h"
//#include <frc/WPILib.h>

DriveController::DriveController(RobotModel *robot, ControlBoard *humanControl) {

	robot_ = robot;
	humanControl_ = humanControl;

	//-----------------------NORMAL TELEOP-------------------------------------
	navXSource_ = new NavXPIDSource(robot_);
	talonEncoderSource_ = new TalonEncoderPIDSource(robot_);

    // Set sensitivity to 0
	thrustSensitivity_ = 0.0;
	rotateSensitivity_ = 0.0;
	quickTurnSensitivity_ = 0.0;

	LOW_THRUST_SENSITIVITY= 0.3; // TODO tune this
	LOW_ROTATE_SENSITIVITY = 0.7; // TODO tune this

	isDone_ = false;

	leftOutput = 0.0;
	rightOutput  = 0.0;

	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("Thrust z", thrustSensitivity_);
	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("Rotate z", rotateSensitivity_);
	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("Gear High?", humanControl_->GetHighGearDesired());
	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("Quick turn desired", humanControl_->GetQuickTurnDesired());
	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("Drive Mode Arcade Drive?", humanControl_->GetArcadeDriveDesired());
	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("Left motor output", leftOutput );
	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("Right motor output", rightOutput);
	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("Drive direction", GetDriveDirection());
	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("NavX angle", robot_->GetNavXYaw());
	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("Left drive distance", robot_->GetLeftDistance());
	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("Right drive distance", robot_->GetRightDistance());
	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("Left drive encoder value", robot_->GetLeftEncoderValue());
	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("Right drive encoder value", robot_->GetRightEncoderValue());
}

void DriveController::Reset() {
//	robot_->SetPercentVBusDriveMode(); check robotmodel

//	thrustSensitivity_ = robot_->pini_->getf("TELEOP DRIVING", "thrustSensitivity", 0.3);
//	rotateSensitivity_ = robot_->pini_->getf("TELEOP DRIVING", "rotateSensitivity", 0.5);
//	quickTurnSensitivity_ = robot_->pini_->getf("TELEOP DRIVING", "quickTurnSensitivity", 0.5);
//	check ini
}

void DriveController::Update(double currTimeSec, double deltaTimeSec) {
	//PrintDriveValues();

	switch (robot_->GetGameMode()) {
		case (RobotModel::NORMAL_TELEOP):
			//		robot->SetPercentVBusDriveMode(); old talon stuff
			double leftJoyY, leftJoyZ, rightJoyY, rightJoyX, rightJoyZ;

			// Takes joystick values
			leftJoyY = -humanControl_->GetJoystickValue(ControlBoard::kLeftJoy, ControlBoard::kY);	// was neg
			leftJoyZ = humanControl_->GetJoystickValue(ControlBoard::kLeftJoy, ControlBoard::kZ);
			rightJoyY = -humanControl_->GetJoystickValue(ControlBoard::kRightJoy, ControlBoard::kY);	// was neg
			rightJoyX = humanControl_->GetJoystickValue(ControlBoard::kRightJoy, ControlBoard::kX);
			rightJoyZ = humanControl_->GetJoystickValue(ControlBoard::kRightJoy, ControlBoard::kZ);

			// so leftJoyZ and rightJoyZ are from -1 to 1
			thrustSensitivity_ = (leftJoyZ + 1.0) / 2.0;
			rotateSensitivity_ = (rightJoyZ + 1.0) / 2.0;

			// Change gear
			if (humanControl_->GetHighGearDesired()) {
				robot_->SetHighGear();
			} else {
				robot_->SetLowGear();
			}
		
			// Checks quickturn or arcade drive
			if (humanControl_->GetQuickTurnDesired()) {
				QuickTurn(rightJoyX, 0.0);
			} else {
				if (humanControl_->GetArcadeDriveDesired()) {
					ArcadeDrive(rightJoyX, leftJoyY, thrustSensitivity_, rotateSensitivity_);
				} else {
					printf("Using Tank drive------------------------------------------------------------------------------------------------------- \n");
					TankDrive(leftJoyY, rightJoyY);
				}
			}
			break;
		case (RobotModel::SANDSTORM):
			printf("WARNING: DriveController initialized in sandstorm????");
		default:
			printf("ERROR: Game mode not reconized as either sandstorm or normal teleop.  In DriveController::Update()\n");
	}
}

void DriveController::ArcadeDrive(double myX, double myY, double thrustSensitivity, double rotateSensitivity) {

	double thrustValue = myY * GetDriveDirection();
	double rotateValue = myX;
//	double rotateValue = myX * GetDriveDirection(); // TODO fix if you want chloemode
	leftOutput = 0.0;
	rightOutput = 0.0;

	// Account for small joystick jostles (deadband)
	thrustValue = HandleDeadband(thrustValue, 0.1);	// 0.02 was too low
	rotateValue = HandleDeadband(rotateValue, 0.06);

	// Sensitivity adjustment
	rotateValue = GetCubicAdjustment(rotateValue, rotateSensitivity);
	rotateValue *= fabs(thrustValue);

	thrustValue = GetCubicAdjustment(thrustValue, thrustSensitivity);

	leftOutput = thrustValue + rotateValue;			// CHECK FOR COMP BOT
	rightOutput = thrustValue - rotateValue;

	// Make sure, output values are within range
	if (leftOutput > 1.0) {
		leftOutput = 1.0;
	} else if (rightOutput > 1.0) {
		rightOutput = 1.0;
	} else if (leftOutput < -1.0) {
		leftOutput = -1.0;
	} else if (rightOutput < -1.0) {
		rightOutput = -1.0;
	}

	robot_->SetDriveValues(RobotModel::kLeftWheels, -leftOutput); //TODO ARTEMIS FIX CHANGE INVERSION
	robot_->SetDriveValues(RobotModel::kRightWheels, rightOutput);

}

void DriveController::TankDrive(double myLeft, double myRight) {
	leftOutput = myLeft * GetDriveDirection();
	rightOutput = myRight * GetDriveDirection();

	robot_->SetDriveValues(RobotModel::kLeftWheels, -leftOutput); //TODO ARTEMIS FIX CHANGE INVERSION
	robot_->SetDriveValues(RobotModel::kRightWheels, rightOutput);
}

void DriveController::QuickTurn(double myRight, double turnConstant) {

	double rotateValue = GetCubicAdjustment(myRight, turnConstant);

	robot_->SetDriveValues(RobotModel::kLeftWheels, rotateValue);		// CHECK FOR COMP BOT
	robot_->SetDriveValues(RobotModel::kRightWheels, -rotateValue);		// CHECK FOR COMP BOT
}

int DriveController::GetDriveDirection() {
	if (humanControl_->GetReverseDriveDesired()) {
		return -1;
	} else {
		return 1;
	}
}

double DriveController::HandleDeadband(double value, double deadband) {
	if (fabs(value) < deadband) {
		return 0.0;
	} else {
		return value;
	}
}

// Rotation sensitivity: when z == 0 same output; when z==1, output^3
double DriveController::GetCubicAdjustment(double value, double adjustmentConstant) {
	return adjustmentConstant * std::pow(value, 3.0) + (1 - adjustmentConstant) * value;
}

bool DriveController::IsDone() {
	return isDone_;
}

/*
void DriveController::PrintDriveValues() {
	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("Drive direction", GetDriveDirection());
	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("NavX angle", robot_->GetNavXYaw());
	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("Left drive distance", robot_->GetLeftDistance());
	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("Right drive distance", robot_->GetRightDistance());
//	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("Left drive encoder value", robot_->leftMaster_->GetEncPosition());
//	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("Right drive encoder value", robot_->rightMaster_->GetEncPosition());
	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("Left drive encoder value", robot_->GetLeftEncoderValue());
	frc::Shuffleboard::GetTab("PRINTSSTUFFSYAYS").Add("Right drive encoder value", robot_->GetRightEncoderValue());
}
*/

DriveController::~DriveController() {
}
