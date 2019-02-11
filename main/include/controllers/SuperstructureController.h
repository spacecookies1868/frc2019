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
	SuperstructureController(RobotModel *myRobot, ControlBoard *myHumanControl);

	void Reset();

	void Update(double currTimeSec, double deltaTimeSec);

	void SetOutputs();

	void RefreshShuffleboard();

	void RefreshIni();

	virtual ~SuperstructureController();

	enum SuperstructureState {
		kInit, kIdle, kFlywheelCargo, kFlywheelRocket
	};

private:
	RobotModel *robot_;
	ControlBoard *humanControl_;

	//PID for cargo shooter into the cargo ship
	PIDController *cargoFlyPID_;
	//PID for cargo shooter into the rocket ship
	PIDController *rocketFlyPID_;

	double desiredFlywheelVelocCargo_, desiredFlywheelVelocRocket_, cargoIntakeOutput_, flywheelStartTime_;
	bool flywheelStarted_, cargoIntakeWristEngaged_, hatchOuttakeEngaged_, hatchPickUpEngaged_;

	uint32_t currState_;
	uint32_t nextState_;

	double cargoPFac_, cargoIFac_, cargoDFac_, rocketPFac_, rocketIFac_, rocketDFac_;
	
	nt::NetworkTableEntry rocketPNet_, rocketINet_, rocketDNet_, cargoPNet_, cargoINet_, cargoDNet_;
};

#endif /* SRC_CONTROLLERS_SUPERSTRUCTURECONTROLLER_H_ */