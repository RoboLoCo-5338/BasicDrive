#include "WPILib.h"
#include "AHRS.h"
class Robot: public IterativeRobot, public PIDOutput {
private:
	LiveWindow *lw = nullptr;
	AHRS *ahrs;
	CANTalon *rightdrive2 = new CANTalon(3);
	CANTalon *rightdrive1 = new CANTalon(4);
	CANTalon *leftdrive1 = new CANTalon(1);
	CANTalon *leftdrive2 = new CANTalon(2);
	CANTalon *intake = new CANTalon(0);
	CANTalon *scissor1 = new CANTalon(5);
	CANTalon *scissor2 = new CANTalon(6);
	RobotDrive *drive = new RobotDrive(leftdrive2, leftdrive1, rightdrive2,
			rightdrive1);
	Joystick *controlstick = new Joystick(2);
	Joystick *leftjoystick = new Joystick(0);
	Joystick *rightjoystick = new Joystick(1);
	PowerDistributionPanel *pdp = new PowerDistributionPanel(0);

	PIDController *turnController;

public:
	Robot() {

	}
private:
	SendableChooser chooser { };

	void RobotInit() override {
		lw = LiveWindow::GetInstance();
		drive->SetExpiration(20000);
		drive->SetSafetyEnabled(false);
		try {
			/* Communicate w/navX-MXP via the MXP SPI Bus.                                       */
			/* Alternatively:  I2C::Port::kMXP, SerialPort::Port::kMXP or SerialPort::Port::kUSB */
			/* See http://navx-mxp.kauailabs.com/guidance/selecting-an-interface/ for details.   */
			ahrs = new AHRS(SPI::Port::kMXP);
		} catch (std::exception ex) {
			std::string err_string = "Error instantiating navX-MXP:  ";
			err_string += ex.what();
			DriverStation::ReportError(err_string.c_str());
		}

		if (ahrs) {
			LiveWindow::GetInstance()->AddSensor("IMU", "Gyro", ahrs);
			ahrs->ZeroYaw();
		}
		// Kp	  Ki	 Kd		Kf    PIDSource PIDoutput
		turnController = new PIDController(0.03f, 0.00f, 0.00f, 0.00f, ahrs,
				this);
		turnController->SetInputRange(-180.0f, 180.0f);
		turnController->SetOutputRange(-1.0, 1.0);
		turnController->SetAbsoluteTolerance(2); //tolerance in degrees
		turnController->SetContinuous(true);

		chooser.AddDefault("No Auto", new int(0));
		chooser.AddObject("Normal Auto", new int(1));
		SmartDashboard::PutData("Auto Modes", &chooser);
	}
	double rotateRate = 0;
	void PIDWrite(float output) { // Implement PIDOutput
		rotateRate = output;
	}

	int currentState = 1;
	int autoSelected = 0;
	Timer *timer;
	void AutonomousInit() override {
		autoSelected = *((int*) chooser.GetSelected());
		currentState = 1;
	}

	void AutonomousPeriodic() override {
		switch (autoSelected) {
		case 0:
			drive->TankDrive(0.0, 0.0);
			break;
		case 1:
			AutonomousNormal();
			break;
		}
	}

	void AutonomousNormal() {
		UpdateDashboard();
		switch (currentState) {
		case 1:
			timer = new Timer();
			timer->Reset();
			timer->Start();
			//State: Stopped
			//Transition: Driving State
			currentState = 2;
			break;
		case 2:
			//State: Driving
			//Stay in State until 2 seconds have passed
			//Transition: Gyroturn State
			drive->TankDrive(0.5, 0.5);
			if (timer->Get() >= 1) {
				drive->TankDrive(0.0, 0.0);
				ahrs->ZeroYaw();
				currentState = 3;
				turnController->SetSetpoint(90);
				turnController->Enable();
			}
			break;
		case 3:
			//State: Gyroturn
			//Stay in state until navx yaw has reached 90 degrees
			//Transition: Driving State
			drive->TankDrive(0.5*rotateRate,-0.5*rotateRate);
//			if (ahrs->GetYaw() >= 90) {
			if (turnController->OnTarget()) {
				drive->TankDrive(0.0, 0.0);
				currentState = 4;
				timer->Reset();
				timer->Start();
			}
			break;
		case 4:
			//State:Driving
			//Stay in state until 2 seconds have passed
			//Transition: State Stopped
			drive->TankDrive(0.5, 0.5);
			if (timer->Get() >= 1) {
				currentState = 5;
				timer->Stop();
			}
			break;
		case 5:
			//State: Stopped
			drive->TankDrive(0.0, 0.0);
			break;

		}

	}

	void TeleopInit() override {
		drive->SetExpiration(200000);
		drive->SetSafetyEnabled(false);
	}

	void TeleopPeriodic() override {
		float leftPower, rightPower;
		leftPower = -leftjoystick->GetY();
		rightPower = -rightjoystick->GetY();
		float multiplier;
		if (rightjoystick->GetRawButton(1)) {
			multiplier = 1;
		} else {
			multiplier = 0.5;
		}
		if (leftjoystick->GetRawButton(1)) {
			drive->TankDrive(leftPower * multiplier, leftPower * multiplier,
			false);
		} else {
			drive->TankDrive(leftPower * multiplier, rightPower * multiplier,
			false);
		}
		float scaleIntake = (controlstick->GetThrottle() + 1) / 2;
		if (controlstick->GetRawButton(1)) {
			intake->Set(-scaleIntake);
		} else if (controlstick->GetRawButton(2)) {
			intake->Set(scaleIntake);
		} else {
			intake->Set(0);
		}
		float scissorPower = controlstick->GetY();
		scissor1->Set(scissorPower);
		scissor2->Set(scissorPower);
		UpdateDashboard();
	}

	void DisabledPeriodic() override {
		UpdateDashboard();
	}

	char i = 0;
	void UpdateDashboard() {
		float r = 0.00001 * i;
		SmartDashboard::PutNumber("Yaw:", ahrs->GetYaw() + r);
		SmartDashboard::PutNumber("Roll:", ahrs->GetRoll() + r);
		SmartDashboard::PutNumber("Pitch", ahrs->GetPitch() + r);
		SmartDashboard::PutNumber("Scissor 1", pdp->GetCurrent(1) + r);
		SmartDashboard::PutNumber("Scissor 2", pdp->GetCurrent(2) + r);
		SmartDashboard::PutNumber("Left Drive 1", pdp->GetCurrent(12) + r);
		SmartDashboard::PutNumber("Left Drive 2", pdp->GetCurrent(13) + r);
		SmartDashboard::PutNumber("Right Drive 1", pdp->GetCurrent(14) + r);
		SmartDashboard::PutNumber("Right Drive 2", pdp->GetCurrent(15) + r);
		SmartDashboard::PutNumber("Rotate Rate", rotateRate);
		i = (i + 1) % 2;
	}

	void TestPeriodic() override {
		lw->Run();
	}
};

START_ROBOT_CLASS(Robot);
