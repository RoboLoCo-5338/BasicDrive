#include "WPILib.h"

class Robot: public IterativeRobot
{
private:
	LiveWindow *lw;
	CANTalon *rightdrive1 = new CANTalon(1);
	CANTalon *rightdrive2 = new CANTalon(2);
	CANTalon *leftdrive1 = new CANTalon(3);
	CANTalon *leftdrive2 = new CANTalon(4);
	RobotDrive *drive;
	Joystick *controlstick = new Joystick(3);
	Joystick *leftjoystick = new Joystick(1);
	Joystick *rightjoystick = new Joystick(2);

	void RobotInit()
	{
		lw = LiveWindow::GetInstance();
		drive = new RobotDrive(leftdrive1, leftdrive2, rightdrive1, rightdrive2);
		drive.SetExpiration(20000);
		drive.SetSafetyEnabled(false);
	}

	void AutonomousInit()
	{

	}

	void AutonomousPeriodic()
	{

	}

	void TeleopInit()
	{
		driveTrain.SetExpiration(200000);
		driveTrain.SetSafetyEnabled(false);
	}

	void TeleopPeriodic()
	{
		float leftPower, rightPower;
		leftPower = leftjoystick->GetY();
		rightPower = rightjoystick->GetY();
		drive->RobotDrive(leftPower, rightPower, false);
	}

	void TestPeriodic()
	{
		lw->Run();
	}
};

START_ROBOT_CLASS(Robot);
