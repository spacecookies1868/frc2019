/*----------------------------------------------------------------------------*/
/* Copyright (c) 2018 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#ifndef SRC_CONTROLLERS_SUPERSTRUCTURECONTROLLER_H_
#define SRC_CONTROLLERS_SUPERSTRUCTURECONTROLLER_H_

#include "RobotModel.h"
#include "ControlBoard.h"


class SuperstructureController {
public:
	// initializes all variables
	// takes in RobotModel and ControlBoard
	SuperstructureController(RobotModel *myRobot, ControlBoard *myHumanControl);

	void Reset();

	void Update(double currTimeSec, double deltaTimeSec);

	void SetOutputs();

	void HabEncoderTest();

	bool CargoInIntake();

	void LightSensorTest();

	void HatchWristControllerUpdate(double newAngle_, double pFac_, double iFac_, double dFac_);

	double RatioFlywheel(double value);

	void RefreshShuffleboard();

	void RefreshIni();

	virtual ~SuperstructureController();

	enum SuperstructureState {
		kInit, kIdle
	};

private:
	RobotModel *robot_;
	ControlBoard *humanControl_;

	// PID for cargo shooter into the cargo ship
	PIDController *cargoFlyPID_;
	// PID for cargo shooter into the rocket ship
	PIDController *rocketFlyPID_;
	// PID for hatch wrist motor controller
	PIDController *hatchWristPID_;

	double desiredFlywheelVelocCargo_, desiredFlywheelVelocRocket_, cargoIntakeOutput_, flywheelStartTime_;
	double ratioFlywheelOutput_;
	bool flywheelStarted_, cargoIntakeWristEngaged_, hatchOuttakeEngaged_, hatchWristNewAngle_;
	double currHabEncoderVal_;
	int habRailsEncoderVal_;
	bool currLightSensorStatus_;

	double timeEngagedCargoWrist_;

	bool hookEngaged_;

	uint32_t currState_;
	uint32_t nextState_;

	double cargoPFac_, cargoIFac_, cargoDFac_, rocketPFac_, rocketIFac_, rocketDFac_, 
		hatchPDownFac_, hatchIDownFac_, hatchDDownFac_, hatchPUpFac_, hatchIUpFac_, hatchDUpFac_;
	
	nt::NetworkTableEntry rocketPNet_, rocketINet_, rocketDNet_, cargoPNet_, cargoINet_, cargoDNet_,
		cargoVelocNet_, cargoRocketVelocNet_, hatchPDownNet_, hatchIDownNet_, hatchDDownNet_, 
		hatchPUpNet_, hatchIUpNet_, hatchDUpNet_, hatchPowerNet_, lightSensorNet_;

};

#endif /* SRC_CONTROLLERS_SUPERSTRUCTURECONTROLLER_H_ */