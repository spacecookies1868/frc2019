/*----------------------------------------------------------------------------*/
/* Copyright (c) 2018 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#ifndef SRC_AUTO_MODES_BLANKMODE_H_
#define SRC_AUTO_MODES_BLANKMODE_H_

#include "AutoMode.h"

class BlankMode : public AutoMode {
public:
	BlankMode(RobotModel *robot, ControlBoard *controlBoard);
	void CreateQueue(AutoMode::AutoPositions pos, AutoMode::HabLevel hablvl) override;
	void Init();
	virtual ~BlankMode();
};

#endif /* SRC_AUTO_MODES_BLANKMODE_H_ */