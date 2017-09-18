#pragma once
#include <i2c2_device.hpp>

#define REGISTER_X					(1)
#define REGISTER_Y 					(3)
#define REGISTER_Z 					(5)
#define REGISTER_CONTROL_1			(0x2A)
#define I2C_ACCELERATION_ADDRESS 	(0x38)

class Accelerometer : private i2c2_device, public SingletonTemplate <Accelerometer>
{
private:

	// Constructor
	Accelerometer() : i2c2_device(I2CAddr_AccelerationSensor)
	{
		Initialize();
	}

	friend class SingletonTemplate <Accelerometer>;

public:

	// Initialize
	void 	Initialize();

	int16_t GetX();

	int16_t GetY();

	int16_t GetZ();

};

void Accelerometer::Initialize()
{
	// Setting into 100Hz active mode
	unsigned char active_mode_setting = (1 << 0) | (1 << 3) | (1 << 4);
	// Write the setting into the control register
	writeReg(REGISTER_CONTROL_1, active_mode_setting);
}

int16_t Accelerometer::GetX()
{
	return ((int16_t)get16BitRegister(REGISTER_X) / 16);
}

int16_t Accelerometer::GetY()
{
	return ((int16_t)get16BitRegister(REGISTER_Y) / 16);
}

int16_t Accelerometer::GetZ()
{
	return ((int16_t)get16BitRegister(REGISTER_Z) / 16);
}