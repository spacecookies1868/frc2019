/*----------------------------------------------------------------------------*/
/* Copyright (c) 2018 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "../../include/controllers/SuperstructureController.h"

SuperstructureController::SuperstructureController(RobotModel *myRobot, ControlBoard *myHumanControl) {
    robot_ = myRobot;
	humanControl_ = myHumanControl;

	currState_ = kInit;
	nextState_ = kIdle;

    desiredFlywheelVelocCargo_ = 0.65; 
    desiredFlywheelVelocRocket_ = 0.25; 

    hatchWristNewAngle_ = true;

    cargoVelocNet_ = frc::Shuffleboard::GetTab("Operator_Input").Add("cargo veloc", 0.65).GetEntry(); //0.65
    cargoRocketVelocNet_ = frc::Shuffleboard::GetTab("Operator_Input").Add("rocket veloc", 0.25).GetEntry(); //0.25
    lightSensorNet_ = frc::Shuffleboard::GetTab("Operator_Input").Add("enable light sensor", true).WithWidget(BuiltInWidgets::kToggleSwitch).GetEntry();
    
    cargoIntakeOutput_ = 0.8; 

    flywheelStartTime_ = 0.0;
    flywheelStarted_ = false;

    cargoIntakeWristEngaged_ = false;
    hatchOuttakeEngaged_ = false;

  hookEngaged_ = false;
  timeEngagedCargoWrist_ = robot_->GetTime();

    //get PID vals for cargo ship
    cargoPFac_ = 0.8;
    cargoIFac_ = 0.0;
    cargoDFac_ = 0.2;

    //get PID vals for rocket ship
    rocketPFac_ = 0.8;
    rocketIFac_ = 0.0;
    rocketDFac_ = 0.2;

    //PID vals for hatch wrist
    hatchPDownFac_ = 0.00;
    hatchIDownFac_ = 0.0;
    hatchDDownFac_ = 0.02;

    hatchPUpFac_ = 0.00;
    hatchIUpFac_ = 0.0;
    hatchDUpFac_ = 0.02;

    cargoPNet_ = frc::Shuffleboard::GetTab("Operator_Input").Add("cargo P", 0.8).GetEntry();
    cargoINet_ = frc::Shuffleboard::GetTab("Operator_Input").Add("cargo I", 0.0).GetEntry();
    cargoDNet_ = frc::Shuffleboard::GetTab("Operator_Input").Add("cargo D", 0.2).GetEntry();

    rocketPNet_ = frc::Shuffleboard::GetTab("Operator_Input").Add("rocket P", 0.8).GetEntry();
    rocketINet_ = frc::Shuffleboard::GetTab("Operator_Input").Add("rocket I", 0.0).GetEntry();
    rocketDNet_ = frc::Shuffleboard::GetTab("Operator_Input").Add("rocket D", 0.2).GetEntry();

    hatchPDownNet_ = frc::Shuffleboard::GetTab("Operator_Input").Add("hatch Pd", 0.00).GetEntry(); //down pid
    hatchIDownNet_ = frc::Shuffleboard::GetTab("Operator_Input").Add("hatch Id", 0.0).GetEntry();
    hatchDDownNet_ = frc::Shuffleboard::GetTab("Operator_Input").Add("hatch Dd", 0.02).GetEntry();

    hatchPUpNet_ = frc::Shuffleboard::GetTab("Operator_Input").Add("hatch Pu", 0.00).GetEntry(); //up pid
    hatchIUpNet_ = frc::Shuffleboard::GetTab("Operator_Input").Add("hatch Iu", 0.0).GetEntry();
    hatchDUpNet_ = frc::Shuffleboard::GetTab("Operator_Input").Add("hatch Du", 0.02).GetEntry();

    hatchPowerNet_ = frc::Shuffleboard::GetTab("Operator_Input").Add("hatch POWER", 0.4).GetEntry();

    //shuffleboard PID values
    cargoFlyPID_  = new PIDController(cargoPFac_, cargoIFac_, cargoDFac_, robot_->GetCargoFlywheelEncoder(),
        robot_->GetCargoFlywheelMotor());
    cargoFlyPID_->SetSetpoint(desiredFlywheelVelocCargo_);  
    cargoFlyPID_->SetOutputRange(-1.0, 1.0); 
    cargoFlyPID_->SetAbsoluteTolerance(0.05); //TODO
    cargoFlyPID_->SetContinuous(false);

    rocketFlyPID_ = new PIDController(rocketPFac_, rocketIFac_, rocketDFac_, robot_->GetCargoFlywheelEncoder(),
        robot_->GetCargoFlywheelMotor());
    rocketFlyPID_->SetSetpoint(desiredFlywheelVelocRocket_);  
    rocketFlyPID_->SetOutputRange(-1.0, 1.0); 
    rocketFlyPID_->SetAbsoluteTolerance(0.05); //TODO
    rocketFlyPID_->SetContinuous(false);

    /**0
     * for hatch PID: currently 2 positions
     * 
     * down - stowed position to floor
     * up - floor to stowed position
    */

    //TODO INIT highgear disengaged whatever whatever (like last year's wrist engage asap)
}

void SuperstructureController::Reset() {
	currState_ = kInit;
	nextState_ = kInit;

    cargoFlyPID_->Reset();
    rocketFlyPID_->Reset();

    RefreshShuffleboard();

    robot_->SetCargoIntakeOutput(0.0);
    robot_->SetHatchIntakeWheelOutput(0.0);
}

void SuperstructureController::Update(double currTimeSec, double deltaTimeSec) {

    SetOutputs(); 

	switch(currState_) {
        case kInit:
            cargoFlyPID_->Reset();
            cargoFlyPID_->Disable();
            
            rocketFlyPID_->Reset();
            rocketFlyPID_->Disable();

            robot_->SetCargoIntakeOutput(0.0); 
            robot_->SetCargoFlywheelOutput(0.0);
            robot_->SetHatchIntakeWheelOutput(0.0);

            nextState_ = kIdle;
        case kIdle:
            nextState_ = kIdle;
            RefreshShuffleboard();

            // NOT SURE IF THIS IS NEEDED, gyro was supposed to be intrigrated into this initially
            if (humanControl_->GetHatchWristDownDesired()) { 
			    // printf("hatch intake wrist to floor\n");
                robot_->SetHatchWristOutput(-hatchPowerNet_.GetDouble(0.4));
            } else if (humanControl_->GetHatchWristUpDesired()) { 
			    robot_->SetHatchWristOutput(hatchPowerNet_.GetDouble(0.4));
		    } else { //otherwise, keep in past 90 degree point
			    robot_->SetHatchWristOutput(0.0);
            }

            //CARGO CODE

            //note: combined wrist and intake/unintake (so if wrist down and not unintaking, auto intake + no two controllers on same motor)
            if(humanControl_->GetCargoIntakeWristDesired()){
                printf("wrist down\n");
                hookEngaged_ = false;
                robot_->SetCargoIntakeWrist(true);
                
                if(!humanControl_->GetCargoUnintakeDesired()){
                    if (!CargoInIntake() || !lightSensorNet_.GetBoolean(true)) {
                       robot_->SetCargoIntakeOutput(cargoIntakeOutput_);
                    } else {
                        robot_->SetCargoIntakeOutput(0.0);
                    }
                } else {
                    robot_->SetCargoUnintakeOutput(cargoIntakeOutput_);
                }
            } else {
                 if(!hookEngaged_){
                    timeEngagedCargoWrist_ = robot_->GetTime();
                }
                hookEngaged_ = true;
                robot_->SetCargoIntakeWrist(false);
                if(humanControl_->GetCargoIntakeDesired()){ 
                    robot_->SetCargoIntakeOutput(cargoIntakeOutput_);
                } else if (humanControl_->GetCargoUnintakeDesired()){
                    robot_->SetCargoUnintakeOutput(cargoIntakeOutput_);
                } else {
                    robot_->SetCargoIntakeOutput(0.0);
                }
            }    
            if(hookEngaged_ && currTimeSec-timeEngagedCargoWrist_ > 2){
                robot_->DisengageHook(); //closes hook
                //printf("disengage hook\n");
            } else {
                robot_->EngageHook();
                //printf("engage hook\n");
            } 

            if (humanControl_->GetCargoFlywheelDesiredRocket()){ // Note: check if less power first, don't want accident more power
               robot_->SetHatchBeak(true);
               robot_->SetCargoFlywheelOutput(RatioFlywheel(desiredFlywheelVelocRocket_)); 
               printf("flywheel speed is following %f\n", RatioFlywheel(desiredFlywheelVelocRocket_));
            } else if(humanControl_->GetCargoFlywheelDesired()){ // flywheel for cargo ship
                printf("cargo shooting into cargo ship\n");
                robot_->SetHatchBeak(false);
                robot_->SetCargoFlywheelOutput(RatioFlywheel(desiredFlywheelVelocCargo_)); 
                printf("flywheel speed is following %f\n", RatioFlywheel(desiredFlywheelVelocRocket_));
            } else if (humanControl_->GetCargoFlywheelUnintakeDesired()) {
                printf("unintaking the flywheel\n");
                robot_->SetHatchBeak(true);
                robot_->SetCargoFlywheelOutput(-desiredFlywheelVelocRocket_);
            } else {
                robot_->SetCargoFlywheelOutput(0.0);

                //so beak is not messed up if multiple activated at a time
                if(humanControl_->GetHatchOuttakeDesired()){ //TODO different state: time delay
                    robot_->SetHatchBeak(true);
                    robot_->SetHatchOuttake(true);
                } else if(humanControl_->GetHatchBeakDesired()){
                    robot_->SetHatchBeak(true);
                } else {
                    robot_->SetHatchBeak(false);
                    robot_->SetHatchOuttake(false);
                }
            }

            if(humanControl_->GetHabBrakeDesired() || humanControl_->GetHabBrakeLevel2Desired()){ 
                //&& !humanControl_->GetTestDesired() && !humanControl_->GetTest3Desired()
                robot_->SetHabBrake(false);
                printf("hab brake not on\n");
            } else {
                robot_->SetHabBrake(true);
                // printf("hab brake activate\n");
            }

            //TODO SUPER SKETCH 2 messages random everywhere see robot

            if(humanControl_->GetHighGearDesired()){
                robot_->SetHighGear();
                // printf("High Gear \n");
            } else {
                robot_->SetLowGear();
                // printf("Low Gear \n");
            }

            break;
        default:
            printf("WARNING: State not found in SuperstructureController::Update()\n");
    }
	currState_ = nextState_;
}

void SuperstructureController::SetOutputs() {
    cargoFlyPID_->SetSetpoint(desiredFlywheelVelocCargo_);
    rocketFlyPID_->SetSetpoint(desiredFlywheelVelocRocket_);
}

void SuperstructureController::HabEncoderTest() {
    currHabEncoderVal_ = robot_->GetHabEncoderValue();
    printf("Hab Encoder Value is the follwoing %f\n", currHabEncoderVal_);
}

void SuperstructureController::LightSensorTest(){
    currLightSensorStatus_ = robot_->GetLightSensorStatus();
    //printf("Light Sensor Value is the following %d\n", currLightSensorStatus_);
}

void SuperstructureController::RefreshShuffleboard() {
    desiredFlywheelVelocCargo_ = cargoVelocNet_.GetDouble(0.65);
    desiredFlywheelVelocRocket_ = cargoRocketVelocNet_.GetDouble(0.25);

    cargoPFac_ = cargoPNet_.GetDouble(0.8);
    cargoIFac_ = cargoINet_.GetDouble(0.0);
    cargoDFac_ = cargoDNet_.GetDouble(0.2);

    rocketPFac_ = rocketPNet_.GetDouble(0.8);
    rocketIFac_ = rocketINet_.GetDouble(0.0);
    rocketDFac_ = rocketDNet_.GetDouble(0.2);
    
    hatchPUpFac_ = hatchPUpNet_.GetDouble(0.00);
    hatchIUpFac_ = hatchIUpNet_.GetDouble(0.0);
    hatchDUpFac_ = hatchDUpNet_.GetDouble(0.02);

    hatchPDownFac_ = hatchPDownNet_.GetDouble(0.00);
    hatchIDownFac_ = hatchIDownNet_.GetDouble(0.0);
    hatchDDownFac_ = hatchDDownNet_.GetDouble(0.02);
}

void SuperstructureController::HatchWristControllerUpdate(double newAngle_, double pFac_, double iFac_, double dFac_) { 
    hatchWristPID_->SetPID(pFac_, iFac_, dFac_); 
    hatchWristPID_->SetSetpoint(newAngle_);
}

bool SuperstructureController::CargoInIntake(){
    currLightSensorStatus_ = robot_->GetLightSensorStatus();
    //printf("Light Sensor Value is the following %d\n", currLightSensorStatus_);
    return currLightSensorStatus_;
}

double SuperstructureController::RatioFlywheel(double value){
    double ratioFlywheelOutput = 12.5/robot_->GetVoltage()*value;
    if(ratioFlywheelOutput > 1){
        ratioFlywheelOutput = 1;
    } else if(ratioFlywheelOutput < -1){
        ratioFlywheelOutput = -1;
    }
    return ratioFlywheelOutput;
}

void SuperstructureController::RefreshIni() {

}

SuperstructureController::~SuperstructureController() {
}