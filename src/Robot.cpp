#include "WPILib.h"
#include "AHRS.h"
#include <memory> //if you're reading this, it's too late

using std::shared_ptr;

class Robot: public IterativeRobot, public PIDOutput {
private:
	// Robot CANTalon Layout
	// Talon SRX 1 - Right 1
	// Talon SRX 2 - Right 2
	// Talon SRX 3 - Left 1
	// Talon SRX 4 - Left 2
	// Talon SRX 5 - Intake Motor
	// Talon SRX 6 - Intake Lever
	// Talon SRX 7 - Winch
	// Talon SRX 8 - Shooter
	LiveWindow *lw = nullptr;

	AHRS *ahrs; // Gyroscope

	CANTalon *intake = new CANTalon(5); // Controller for intake

	CANTalon *intakeLever = new CANTalon(6); // Controller for intake lever
	CANTalon *leftdrive1 = new CANTalon(3); // Our controllers for our main drivetrain
	CANTalon *leftdrive2 = new CANTalon(4); // Two motors per side
	CANTalon *rightdrive2 = new CANTalon(1);
	CANTalon *rightdrive1 = new CANTalon(2);
	CANTalon *winch = new CANTalon(7);
	CANTalon *shooter = new CANTalon(8);

	// Create our drive train and assign the motor controllers
	RobotDrive *drive = new RobotDrive(leftdrive2, leftdrive1, rightdrive2,
			rightdrive1);

	// Create our inputs for our joysticks
	Joystick *controlstick = new Joystick(2);
	Joystick *leftjoystick = new Joystick(0);
	Joystick *rightjoystick = new Joystick(1);

	Compressor *comp = new Compressor();
	Solenoid *liftdown = new Solenoid(0);
	Solenoid *lift = new Solenoid(1);

	// Our power distribution panel object
	PowerDistributionPanel *pdp = new PowerDistributionPanel(0);

	// NavX PID controller that works with the gyroscope
	PIDController *turnController;

public:

	Robot() {

	}
private:
	SendableChooser chooser { };

	// Create default autonomous variables
	static const constexpr char* AUTO_SPEED = "Auto Speed (in [-1,1])";
	static const constexpr double AUTO_SPEED_DEFAULT = 0.5;
	static const constexpr char* AUTO_LENGTH = "Auto Length (seconds)";
	static const constexpr double AUTO_LENGTH_DEFAULT = 6;
	static const constexpr char* AUTO_INTAKE_SPEED = "Auto Intake Speed (in [-1,1])";
	static const constexpr double AUTO_INTAKE_SPEED_DEFAULT = 0.25;

	void RobotInit() override {
		lw = LiveWindow::GetInstance();
		drive->SetExpiration(20000);
		drive->SetSafetyEnabled(false);
		//Gyroscope stuff
		try {
			/* Communicate w/navX-MXP via the MXP SPI Bus.                                       */
			/* Alternatively:  I2C::Port::kMXP, SerialPort::Port::kMXP or SerialPort::Port::kUSB */
			/* See http://navx-mxp.kauailabs.com/guidance/selecting-an-interface/ for details.   */
			ahrs = new AHRS(SPI::Port::kMXP);
		} catch (std::exception ex) {
			std::string err_string = "Error instantiating navX-MXP:  ";
			err_string += ex.what();
			//DriverStation::ReportError(err_string.c_str());
		}

		if (ahrs) {
			LiveWindow::GetInstance()->AddSensor("IMU", "Gyro", ahrs);
			ahrs->ZeroYaw();

			// Kp	  Ki	 Kd		Kf    PIDSource PIDoutput
			turnController = new PIDController(0.015f, 0.003f, 0.100f, 0.00f,
					ahrs, this);
			turnController->SetInputRange(-180.0f, 180.0f);
			turnController->SetOutputRange(-1.0, 1.0);
			turnController->SetAbsoluteTolerance(2); //tolerance in degrees
			turnController->SetContinuous(true);
		}
		chooser.AddDefault("No Auto", new int(0));
		chooser.AddObject("GyroTest Auto", new int(1));
		chooser.AddObject("Spy Auto", new int(2));
		chooser.AddObject("Low Bar Auto", new int(3));
		chooser.AddObject("Straight Spy Auto", new int(4));
		chooser.AddObject("Adjustable Straight Auto", new int(5));
		SmartDashboard::PutNumber(AUTO_LENGTH, AUTO_LENGTH_DEFAULT);
		SmartDashboard::PutNumber(AUTO_SPEED, AUTO_SPEED_DEFAULT);
		SmartDashboard::PutNumber(AUTO_INTAKE_SPEED, AUTO_INTAKE_SPEED_DEFAULT);
		SmartDashboard::PutData("Auto Modes", &chooser);
		liftdown->Set(false);
		comp->Start();
	}
	double rotateRate = 0;
	void PIDWrite(float output) { // Implement PIDOutput
		rotateRate = output;
	}

	int currentState = 1;
	int autoSelected = 0;
	double autoSpeed = AUTO_SPEED_DEFAULT;
	double autoLength = AUTO_LENGTH_DEFAULT;
	double autoIntakeSpeed = AUTO_INTAKE_SPEED_DEFAULT;
	Timer *timer = new Timer();

	// Start auto mode
	void AutonomousInit() override {
		autoSelected = *((int*) chooser.GetSelected()); // autonomous mode chosen in dashboard
		currentState = 1;
		ahrs->ZeroYaw();
		ahrs->GetFusedHeading();
		autoLength = SmartDashboard::GetNumber(AUTO_LENGTH, AUTO_LENGTH_DEFAULT);
		autoSpeed = SmartDashboard::GetNumber(AUTO_SPEED, AUTO_SPEED_DEFAULT);
		autoIntakeSpeed = SmartDashboard::GetNumber(AUTO_INTAKE_SPEED, AUTO_INTAKE_SPEED_DEFAULT);
		liftdown->Set(false);
	}

	// Our autonomous
	void AutonomousPeriodic() override {
		UpdateDashboard();
		switch (autoSelected) {
		default:
		case 0:
			drive->TankDrive(0.0, 0.0);
			break;
		case 1:
			AutonomousGyroTurn();
			break;
		case 2:
			AutonomousSpy();
			break;
		case 3:
			AutonomousLowBar();
			break;
		case 4:
			AutonomousStraightSpy();
			break;
		case 5:
			AutonomousAdjustableStraight();
			break;
		}
	}

	void AutonomousGyroTurn() {
		switch (currentState) {
		case 1:

			timer->Reset();
			timer->Start();
			//State: Stopped
			//Transition: Driving State
			currentState = 2;
			break;
		case 2:
			//State: Driving
			//Stay in State until 2 seconds have passed--`
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
			drive->TankDrive(0.5 * rotateRate, -0.5 * rotateRate);
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
	void AutonomousStraightSpy() {
		switch (currentState) {
		case 1:
			timer->Reset();
			timer->Start();
			turnController->Reset();
			turnController->SetSetpoint(ahrs->GetYaw());
			turnController->Enable();
			currentState = 2;
			break;
		case 2:
			intakeLever->Set(0.25);
			if (timer->Get() >= 1) {
				intakeLever->Set(0);
				currentState = 3;
				timer->Reset();
				timer->Start();
			}
			break;
		case 3:
			drive->TankDrive(0.5, 0.5);
			if (timer->Get() >= 5) {
				drive->TankDrive(0.0, 0.0);
				currentState = 4;
				timer->Reset();
				timer->Start();
			}
			break;
		case 4:
			intake->Set(0.5);
			if (timer->Get() >= 2) {
				currentState = 5;
			}
			break;
		case 5:
			intake->Set(0.0);
			drive->TankDrive(0.0, 0.0);
			break;
		}
	}
	void AutonomousAdjustableStraight() {
		switch (currentState) {
		case 1:
			timer->Reset();
			timer->Start();
			turnController->Reset();
			turnController->SetSetpoint(ahrs->GetYaw());
			turnController->Enable();
			currentState = 2;
			break;
		case 2:
			intakeLever->Set(autoIntakeSpeed);
			if (timer->Get() >= 1) {
				intakeLever->Set(0);
				currentState = 3;
				timer->Reset();
				timer->Start();
			}
			break;
		case 3:
			drive->TankDrive(autoSpeed, autoSpeed);
			intakeLever->Set(-0.1);
			if (timer->Get() >= autoLength) {
				intakeLever->Set(0.0);
				drive->TankDrive(0.0, 0.0);
				currentState = 4;
				timer->Reset();
				timer->Start();
			}
			break;
		case 4:
			intake->Set(0.5);
			shooter->Set(-0.5);
			if (timer->Get() >= 2) {
				currentState = 5;
			}
			break;
		case 5:
			intake->Set(0.0);
			shooter->Set(0.0);
			drive->TankDrive(0.0, 0.0);
			break;
		}
	}
	void AutonomousSpy() {
//		Strategy 1 - start as spy with a boulder, score in lower goal. Starts with intake facing low goal
//		-------------------------------------------------------------------------------------------------------------------
		switch (currentState) {
		case 1:
			//		-State: stopped
			timer->Reset();
			timer->Start();
			ahrs->ZeroYaw();
			currentState = 2;
			break;

//		--transition: state Driving Forward
		case 2:
			//		-State: Driving Forward
			//		--wait until lined up with low goal
			//		--transition: State stopped
			drive->TankDrive(0.5, 0.5);
			if (timer->Get() >= 1) { // NEEDS TO BE SET
				//		-State: stopped
				//		--wait until stopped
				drive->TankDrive(0.0, 0.0);
				currentState = 3;
				timer->Reset();
				timer->Start();
			}
			break;
			//		--transition: State Shooting
		case 3:
//		-State: Shooting
//		--wait until shooting complete
			intake->Set(-.5);
			if (timer->Get() >= .7) { //Find Out Actual Time
				intake->Set(0);
				timer->Reset();
				timer->Start();
				currentState = 4;
			}
			break;
			//		--transition: State Backing Up
		case 4:
			//		-State: Backing Up
			//		--wait until off tower ramp
			drive->TankDrive(-0.5, -0.5);
			if (timer->Get() > 1) {
				drive->TankDrive(0.0, 0.0);
				ahrs->ZeroYaw();
				ahrs->Reset();
				currentState = 5;
				turnController->SetSetpoint(-65.5);
				turnController->Enable();
			}
			break;

//		--transition: Turning
		case 5:
			//		-State: Turning Left
			//		--wait until 65 degrees has been reached to line up with low bar
			drive->TankDrive(-0.5, 0.5);
			if (turnController->OnTarget()) {
				drive->TankDrive(0.0, 0.0);
				timer->Reset();
				timer->Start();
				currentState = 6;
			}
			break;
//		--transition: Backing Up
		case 6:
			//		-State backing Up
			//		--wait until near guard wall
			drive->TankDrive(-0.5, -0.5);
			if (timer->Get() >= 1) {
				drive->TankDrive(0.0, 0.0);
				ahrs->ZeroYaw();
				ahrs->Reset();
				currentState = 7;
				turnController->SetSetpoint(-24.5);
				turnController->Enable();
			}
			break;
//		--transition: Turn Left
		case 7:
//		-State: Turn Right
//		--wait until 25 degree turn has been made to line with low bar
			drive->TankDrive(-0.5, 0.5);
			if (turnController->OnTarget()) {
				drive->TankDrive(0.0, 0.0);
				timer->Reset();
				timer->Start();
				currentState = 8;
			}
			break;
//		--transition: Back Up
		case 8:
//		-State: Backing Up
//		--wait until backed through low bar
			drive->TankDrive(-0.5, -0.5);
			if (timer->Get() >= 1) { // NeedTo Update Value
				timer->Stop();
				currentState = 9;
			}
			break;
//		--transition: Stopped
		case 9:
//		-State: Stopped
			drive->TankDrive(0.0, 0.0);
			break;
		}
	}
	void AutonomousLowBar() {
//		Strategy 2 - start in a normal position lined up with low bar, go through low bars and score boulder in lower goal.
//		-------------------------------------------------------------------------------------------------------------------
//		-State: Stopped
//		--transition: state Drive Forward
//		-State: Driving Forowbar, and in position with tower
//		--transition: turn right
//		-State: Turning Right
//		--wait until 58.25 degrees
//		--transition: Drive Forward
//		-State: Driving Forward
//		--wait until lined up with tower low goal
//		--transition: Stopped
//		-State: Stopped
//		--transition: State Shooting
//		-State: Shooting
//		--wait until shooting complete
//		--transition State backing up
//		-State: Backing Up
//		--wait until backed up to be in line with low background
//		--transition: State turn
//		-State: Turning Left
//		--wait until 58.25 degrees (same as last turn)
//		--transition: Backing Up
//		-State: Backing Up
//		--wait until backed through lower goal
//		--transition: stopped
//		-State: Stopped
	}
	void TeleopInit() override {
		drive->SetExpiration(200000);
		drive->SetSafetyEnabled(false);
		liftdown->Set(false);
	}

	// The actual stuff
	void TeleopPeriodic() override {
		float leftPower, rightPower; // Get the values for the main drive train joystick controllers
		leftPower = -leftjoystick->GetY();
		rightPower = -rightjoystick->GetY();

		float multiplier; // TURBO mode
		if (rightjoystick->GetRawButton(1))
		{
			multiplier = 1;
		} else {
			multiplier = 0.5;
		}

		// wtf is a setpoint
		if (leftjoystick->GetRawButton(6)) {
			turnController->SetSetpoint(0);
			turnController->Reset();
			turnController->Enable();
			ahrs->ZeroYaw();
			//ahrs->Reset();
		}

		//Drive straight with one controller, else: drive with two controllers
		if (leftjoystick->GetRawButton(1)) {
			drive->TankDrive(leftPower * multiplier, leftPower * multiplier,
					false);
		} else if (leftjoystick->GetRawButton(2)) {
			drive->TankDrive(leftPower * multiplier + rotateRate,
					leftPower * multiplier + -rotateRate, false);
		} else {
			drive->TankDrive(leftPower * multiplier, rightPower * multiplier,
					false);
		}

		// That little flap at the bottom of the joystick
		float scaleIntake = (1 - (controlstick->GetThrottle() + 1) / 2);
		// Depending on the button, our intake will eat or shoot the ball
		if (controlstick->GetRawButton(1)) {
			intake->Set(-scaleIntake);
			shooter->Set(scaleIntake);
		} else if (controlstick->GetRawButton(2)) {
			intake->Set(scaleIntake);
			shooter->Set(-scaleIntake);
		} else {
			intake->Set(0);
			shooter->Set(0);
		}

		// Control the motor that lifts and descends the intake bar
		if (controlstick->GetRawButton(4)) {
			intakeLever->Set(.25);
		} else if (controlstick->GetRawButton(6)) {
			intakeLever->Set(-.25);
		} else if (controlstick->GetRawButton(5)){
			intakeLever->Set(-scaleIntake);
		} else if (controlstick->GetRawButton(3)) {
			intakeLever->Set(scaleIntake);
		} else {
			intakeLever->Set(0);
		}
		if (controlstick->GetRawButton(11)) {
			lift->Set(true);
			liftdown->Set(false);
		} else if (controlstick->GetRawButton(12)){
			lift->Set(false);
			liftdown->Set(true);
		} else if (controlstick->GetRawButton(7)) {
			liftdown->Set(false);
		}
		if (controlstick->GetRawButton(9)) {
			winch->Set(scaleIntake);
		} else if (controlstick->GetRawButton(10)) {
			winch->Set(-scaleIntake);
		} else {
			winch->Set(0);
		}
		UpdateDashboard();
	}

	void DisabledPeriodic() override {
		UpdateDashboard();
	}

	char i = 0;

	void UpdateDashboard() {
		float r = 0.00001 * i;
		SmartDashboard::PutNumber("State", currentState + r);
		SmartDashboard::PutNumber("PID Turn Error",
				turnController->GetError() + r);
		SmartDashboard::PutNumber("PID Target",
				turnController->GetSetpoint() + r);
//		SmartDashboard::PutBoolean("Straight", straight);
		SmartDashboard::PutData("test", turnController);
		SmartDashboard::PutNumber("Yaw:", ahrs->GetYaw() + r);
		SmartDashboard::PutNumber("Roll:", ahrs->GetRoll() + r);
		SmartDashboard::PutNumber("Pitch", ahrs->GetPitch() + r);
		SmartDashboard::PutNumber("Scissor 1", pdp->GetCurrent(1) + r);
		SmartDashboard::PutNumber("Scissor 2", pdp->GetCurrent(2) + r);
		SmartDashboard::PutNumber("Left Drive 1", pdp->GetCurrent(12) + r);
		SmartDashboard::PutNumber("Left Drive 2", pdp->GetCurrent(13) + r);
		SmartDashboard::PutNumber("Right Drive 1", pdp->GetCurrent(14) + r);
		SmartDashboard::PutNumber("Right Drive 2", pdp->GetCurrent(15) + r);
		SmartDashboard::PutNumber("Rotate Rate", rotateRate + r);
		i = (i + 1) % 2;
	}

	void TestPeriodic() override {
		lw->Run();
	}
}
;

START_ROBOT_CLASS(Robot);
