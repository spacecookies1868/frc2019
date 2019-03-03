/*----------------------------------------------------------------------------*/
/* Copyright (c) 2018 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once
#include "frc/WPILib.h" //TODO <>
#include <string>
#include <frc/shuffleboard/Shuffleboard.h>
#include <frc/shuffleboard/ShuffleboardTab.h>
//#include "frc/smartdashboard/SmartDashboard.h"
//#include "frc/Encoder.h"
#include <AHRS.h>
#include "ctre/Phoenix.h" //TODO <>
#include <networktables/NetworkTableEntry.h>
//#include "../ext/ini/ini.h"
//#include <TalonSRX.h>
//#include <VictorSPX.h>
//#include "CANTalon.h"
#include "rev/CANSparkMax.h"

class RobotModel {
 public:
  enum Wheels {kLeftWheels, kRightWheels, kAllWheels};
  enum Talons {kLeftMaster, kRightMaster};
  
  //constructor
  RobotModel();

  //Destructor
  ~RobotModel();

  void ResetTimer(); //TODO: implement ALL TIME
  double ModifyCurrent(int channel, double value);
  double CheckMotorCurrentOver(int channel, double power);
  void UpdateCurrent();
  double GetTime();

  double GetVoltage();
  double GetTotalCurrent();
  double GetTotalEnergy();
  double GetTotalPower();
  double GetCurrent(int channel);
  double GetCompressorCurrent();
  double GetRIOCurrent();
  double GetPressureSensorVal();

  void StopCompressor();
  void StartCompressor();

  void PrintState();

  //---------------DRIVE------------------
  WPI_TalonSRX *GetTalon(Talons talon); //TODO

  bool CollisionDetected();

  double GetWheelSpeed(Wheels wheel);

  void SetDriveValues(Wheels wheel, double value); //TODO
  void SetDriveValues(double leftValue, double rightValue);
  void SetTalonBrakeMode(); //TODO
  void SetTalonCoastMode(); //TODO

  void SetHighGear(); //TODO
  void SetLowGear(); //TODO
  bool IsHighGear();

  double GetLeftEncoderValue(); //TODO
  double GetRightEncoderValue(); //TODO

  double GetLeftDistance(); //TODO
  double GetRightDistance(); //TODO

  bool GetLeftEncoderStopped(); //TODO
  bool GetRightEncoderStopped(); //TODO

  void ResetDriveEncoders(); //TODO
  
  double GetNavXYaw(); //TODO
  double GetNavXPitch(); //TODO
  double GetNavXRoll(); //TODO
  
  void ZeroNavXYaw(); //TODO
  
  //---------------SUPERSTRUCTURE------------------
  void ResetGyro();
  void CalibrateGyro();
  double GetGyroAngle();
  int GetHabEncoderValue();
  
  void SetCargoIntakeOutput(double output);
  void SetCargoUnintakeOutput(double output);
  void SetCargoFlywheelOutput(double output);
  void SetHatchWristOutput(double output);
  void SetHatchIntakeWheelOutput(double output);
  void SetHabMotorOutput(double output);

  void SetCargoIntakeWrist(bool change);
  void SetHatchOuttake(bool change);
  void SetHatchBeak(bool change);

  double GetCargoFlywheelMotorOutput(double output);

  //blahslkdjfslkdjflksjdfklsj
  Encoder* GetCargoFlywheelEncoder();
  Victor* GetCargoFlywheelMotor();

  AnalogGyro* GetGyro();
  Victor* GetHatchWristMotor();

  enum GameMode{SANDSTORM, NORMAL_TELEOP};
  
  RobotModel::GameMode GetGameMode();
  double GetDPFac();
  double GetDIFac();
  double GetDDFac();

  double GetRPFac();
  double GetRIFac();
  double GetRDFac();

  double GetPivotPFac();
  double GetPivotIFac();
  double GetPivotDFac();

 private:

  double GetStaticFriction(double thrustValue);
  double HandleStaticFriction(double value, double thrustValue);
 
  GameMode currentGameMode_;

  frc::ShuffleboardTab &tab_;

  //Ini *pini_;
  double pivotPFac_, pivotIFac_, pivotDFac_; //TODO
	double driveDPFac_, driveDIFac_, driveDDFac_; //TODO

  double leftDriveOutput_, rightDriveOutput_;
  float last_world_linear_accel_x_, last_world_linear_accel_y_;
  double driveCurrentLimit_, intakeCurrentLimit_, totalCurrentLimit_, voltageFloor_, pressureFloor_, size_;
  double leftDriveACurrent_, leftDriveBCurrent_, leftDriveCCurrent_, rightDriveACurrent_, rightDriveBCurrent_, rightDriveCCurrent_,
		roboRIOCurrent_, compressorCurrent_, leftIntakeCurrent_, rightIntakeCurrent_;

  //Power Distribution
  bool lastOver_;
  double ratioAll_, ratioDrive_, ratioSuperstructure_;
  bool compressorOff_;

  int habRailsEncoderVal_;
  
  //bool cutSlaves_; //remove one slave from drive

  int autoPos_, autoMode_; //TODO
  std::string testMode_; //TODO

  WPI_TalonSRX *leftMaster_, *rightMaster_; //TODO this is weird, frc:: or not?
  WPI_VictorSPX *leftSlaveA_, *leftSlaveB_, *rightSlaveA_, *rightSlaveB_;

  Victor *cargoIntakeMotor_, *cargoFlywheelMotor_, *hatchIntakeWheelMotor_, *hatchWristMotor_;
  rev::CANSparkMax *habSparkMotor_;
  //rev::CANEncoder *habSparkMotor_;
  Encoder *cargoFlywheelEncoder_;
  DoubleSolenoid *cargoIntakeWristSolenoid_, *hatchBeakSolenoid_;
  Solenoid *hatchOuttakeSolenoid_;

  frc::Timer *timer_;
  AHRS *navX_; 
  AnalogGyro *gyro_; //gyro for hatch wrist
  int navXSpeed_; //in Hz

  frc::Compressor *compressor_; //TODO
  frc::Solenoid *gearShiftSolenoid_;
  frc::Encoder *leftDriveEncoder_, *rightDriveEncoder_; //TODO
  frc::PowerDistributionPanel *pdp_; //TODO

  bool highGear_, cargoWristEngaged_, hatchOuttakeEngaged_, hatchBeakEngaged_;

  nt::NetworkTableEntry jerkYNet_, jerkXNet_, leftDistanceNet_, rightDistanceNet_, yawNet_, pitchNet_, rollNet_, pressureNet_,
    dPFacNet_, dIFacNet_, dDFacNet_, rPFacNet_, rIFacNet_, rDFacNet_, pivotPFacNet_, pivotIFacNet_, pivotDFacNet_,
    ratioAllNet_, ratioDriveNet_, ratioSuperNet_, maxOutputNet_;
};
