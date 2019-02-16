/*----------------------------------------------------------------------------*/
/* Copyright (c) 2018 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#define SRC_AUTO_COMMANDS_OUTTAKEHATCHCOMMAND_H_
#define SRC_AUTO_COMMANDS_OUTTAKEHATCHCOMMAND_H_

#include <frc/WPILib.h> // needed?
#include "../AutoCommand.h"
#include "RobotModel.h" //maybe?

class OuttakeHatchCommand : public AutoCommand {
public:
	OuttakeHatchCommand(RobotModel *robot, bool hatchOUt);
	void Init();
	void Update(double currTimeSec, double deltaTimeSec);
	bool IsDone();
    void Reset();
    virtual ~OuttakeHatchCommand();

private:
	RobotModel *robot_;
	bool hatchOut_;
	bool isDone_;
};

//#endif /* SRC_AUTO_COMMANDS_OUTTAKEHATCHCOMMAND_H_ */