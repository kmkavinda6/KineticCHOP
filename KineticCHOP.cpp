/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include "KineticCHOP.h"

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <assert.h>

#include <iostream>
#include <array>
#include <stdexcept>

const double PI = 3.14159265358979323846;

// Helper function to convert degrees to radians
inline double degreesToRadians(double degrees) {
	return degrees * PI / 180.0;
}

// Custom clamp function for older C++ versions
template<typename T>
T clamp(T value, T min, T max) {
	return std::min(std::max(value, min), max);
}

// Helper function to convert height to DMX value
uint8_t heightToDMX(float height, float min_height, float max_height, float min_dmx, float max_dmx) {
	//if (min_height >= max_height) {
	//	throw std::invalid_argument("Minimum height should be less than maximum height.");
	//}
	//if (height < min_height || height > max_height) {
	//	throw std::out_of_range("Height is out of specified range.");
	//}

	float scaled_value = (height - min_height) / (max_height - min_height) * (max_dmx - min_dmx) + min_dmx;
	return static_cast<uint8_t>(clamp(scaled_value + 0.5f, min_dmx, max_dmx));
}

// Calculate motor heights based on roll, pitch, yaw
std::array<double, 3> calculateMotorHeights(double base_size, double roll_deg, double pitch_deg, double yaw_deg) {
	// Define motor positions in an equilateral triangle
	std::array<std::array<double, 3>, 3> motors = { {
		{0, 0, 0},                                           // Motor 1 (62CH)
		{base_size, 0, 0},                                   // Motor 2 (9CH)
		{base_size / 2, base_size * sqrt(3) / 2, 0}         // Motor 3 (9CH)
	} };

	// Calculate center of the triangle
	std::array<double, 3> center = {
		(motors[0][0] + motors[1][0] + motors[2][0]) / 3,
		(motors[0][1] + motors[1][1] + motors[2][1]) / 3,
		0
	};

	// Convert angles to radians
	double roll = degreesToRadians(roll_deg);
	double pitch = degreesToRadians(pitch_deg);
	double yaw = degreesToRadians(yaw_deg);

	// Rotation matrices
	std::array<std::array<double, 3>, 3> rollMatrix = { {
		{1, 0, 0},
		{0, cos(roll), -sin(roll)},
		{0, sin(roll), cos(roll)}
	} };

	std::array<std::array<double, 3>, 3> pitchMatrix = { {
		{cos(pitch), 0, sin(pitch)},
		{0, 1, 0},
		{-sin(pitch), 0, cos(pitch)}
	} };

	std::array<std::array<double, 3>, 3> yawMatrix = { {
		{cos(yaw), -sin(yaw), 0},
		{sin(yaw), cos(yaw), 0},
		{0, 0, 1}
	} };

	// Calculate heights for each motor
	std::array<double, 3> heights;
	for (int i = 0; i < 3; ++i) {
		double x = motors[i][0] - center[0];
		double y = motors[i][1] - center[1];
		double z = motors[i][2];

		// Apply transformations
		double x_yaw = yawMatrix[0][0] * x + yawMatrix[0][1] * y + yawMatrix[0][2] * z;
		double y_yaw = yawMatrix[1][0] * x + yawMatrix[1][1] * y + yawMatrix[1][2] * z;
		double z_yaw = yawMatrix[2][0] * x + yawMatrix[2][1] * y + yawMatrix[2][2] * z;

		double x_pitch = pitchMatrix[0][0] * x_yaw + pitchMatrix[0][1] * y_yaw + pitchMatrix[0][2] * z_yaw;
		double y_pitch = pitchMatrix[1][0] * x_yaw + pitchMatrix[1][1] * y_yaw + pitchMatrix[1][2] * z_yaw;
		double z_pitch = pitchMatrix[2][0] * x_yaw + pitchMatrix[2][1] * y_yaw + pitchMatrix[2][2] * z_yaw;

		double x_roll = rollMatrix[0][0] * x_pitch + rollMatrix[0][1] * y_pitch + rollMatrix[0][2] * z_pitch;
		double y_roll = rollMatrix[1][0] * x_pitch + rollMatrix[1][1] * y_pitch + rollMatrix[1][2] * z_pitch;
		double z_roll = rollMatrix[2][0] * x_pitch + rollMatrix[2][1] * y_pitch + rollMatrix[2][2] * z_pitch;

		heights[i] = z_roll + center[2];
	}
	return heights;
}


// These functions are basic C function, which the DLL loader can find
// much easier than finding a C++ Class.
// The DLLEXPORT prefix is needed so the compile exports these functions from the .dll
// you are creating
extern "C"
{

DLLEXPORT
void
FillCHOPPluginInfo(CHOP_PluginInfo *info)
{
	// Always set this to CHOPCPlusPlusAPIVersion.
	info->apiVersion = CHOPCPlusPlusAPIVersion;

	// The opType is the unique name for this CHOP. It must start with a 
	// capital A-Z character, and all the following characters must lower case
	// or numbers (a-z, 0-9)
	info->customOPInfo.opType->setString("Kineticlight");

	// The opLabel is the text that will show up in the OP Create Dialog
	info->customOPInfo.opLabel->setString("Kinetic Light");

	// Information about the author of this OP
	info->customOPInfo.authorName->setString("Kavinda Madhubhashana");
	info->customOPInfo.authorEmail->setString("kmkavinda6@gmail.com");

	// This CHOP can work with 4 inputs
	// The inputs are connected height, roll, pitch and yaw
	info->customOPInfo.minInputs = 4;

	// It can accept up to 5 input though, which changes it's behavior
	// The 5th input is speed 
	// The 6th input is Additional DMX values
	info->customOPInfo.maxInputs = 6;
}

DLLEXPORT
CHOP_CPlusPlusBase*
CreateCHOPInstance(const OP_NodeInfo* info)
{
	// Return a new instance of your class every time this is called.
	// It will be called once per CHOP that is using the .dll
	return new CPlusPlusCHOPExample(info);
}

DLLEXPORT
void
DestroyCHOPInstance(CHOP_CPlusPlusBase* instance)
{
	// Delete the instance here, this will be called when
	// Touch is shutting down, when the CHOP using that instance is deleted, or
	// if the CHOP loads a different DLL
	delete (CPlusPlusCHOPExample*)instance;
}

};


CPlusPlusCHOPExample::CPlusPlusCHOPExample(const OP_NodeInfo* info) : myNodeInfo(info)
{
	myExecuteCount = 0;
	myOffset = 0.0;

	// Initialize KineticLight with appropriate motor types
	kineticLight = std::make_unique<KineticLight>(
		new Motor62CH(),  // First motor (62CH)
		new Motor9CH(),   // Second motor (9CH)
		new Motor9CH()    // Third motor (9CH)
	);
}

CPlusPlusCHOPExample::~CPlusPlusCHOPExample()
{

}

void
CPlusPlusCHOPExample::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
	// This will cause the node to cook every frame
	ginfo->cookEveryFrameIfAsked = true;

	// Note: To disable timeslicing you'll need to turn this off, as well as ensure that
	// getOutputInfo() returns true, and likely also set the info->numSamples to how many
	// samples you want to generate for this CHOP. Otherwise it'll take on length of the
	// input CHOP, which may be timesliced.
	ginfo->timeslice = true;

	ginfo->inputMatchIndex = 0;
}

bool
CPlusPlusCHOPExample::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
{
	// If there is an input connected, we are going to match it's channel names etc
	// otherwise we'll specify our own.
	info->numChannels = 80;
	info->numSamples = 1;
	info->startIndex = 0;
	return true;
}

void
CPlusPlusCHOPExample::getChannelName(int32_t index, OP_String *name, const OP_Inputs* inputs, void* reserved1)
{
	char channelName[32];
	sprintf(channelName, "dmx%d", index + 1);
	name->setString(channelName);
}

void
CPlusPlusCHOPExample::execute(CHOP_Output* output,
							  const OP_Inputs* inputs,
							  void* reserved)
{
	myExecuteCount++;

	const OP_CHOPInput* heightInput = inputs->getInputCHOP(0);
	const OP_CHOPInput* rollInput = inputs->getInputCHOP(1);
	const OP_CHOPInput* pitchInput = inputs->getInputCHOP(2);
	const OP_CHOPInput* yawInput = inputs->getInputCHOP(3);
	const OP_CHOPInput* speedInput = inputs->getInputCHOP(4);
	const OP_CHOPInput* dmxInput = inputs->getInputCHOP(5);

	// Get parameters
	double baseSize = inputs->getParDouble("Basesize");
	double minHeight = inputs->getParDouble("Minheight");
	double maxHeight = inputs->getParDouble("Maxheight");

	// Get calibration parameters
	double calMinHeight = inputs->getParDouble("Calibrationminheight");
	double calMaxHeight = inputs->getParDouble("Calibrationmaxheight");
	double calMinDMX = inputs->getParDouble("Calibrationmindmxout");
	double calMaxDMX = inputs->getParDouble("Calibrationmaxdmxout");

	// Get current values from inputs
	double height = heightInput ? heightInput->getChannelData(0)[0] : minHeight;
	double roll = rollInput ? rollInput->getChannelData(0)[0] : 0.0;
	double pitch = pitchInput ? pitchInput->getChannelData(0)[0] : 0.0;
	double yaw = yawInput ? yawInput->getChannelData(0)[0] : 0.0;
	double speed = speedInput ? speedInput->getChannelData(0)[0] : 127.0;



	try {

		// Calculate motor heights
		std::array<double, 3> motorHeights = calculateMotorHeights(baseSize, roll, pitch, yaw);

		if (minHeight >= maxHeight) {
			throw std::invalid_argument("Minimum height should be less than maximum height.");
		}
		if ((motorHeights[0] < minHeight) && (motorHeights[1] < minHeight) && (motorHeights[2] < minHeight) || (motorHeights[0] > maxHeight) && (motorHeights[1] > maxHeight)&& (motorHeights[2] > maxHeight)) {
			throw std::out_of_range("Height is out of specified range.");
		}

		// Update motor values using KineticLight class
// First motor (62CH)
		kineticLight->setMotorChannel(1, 1, heightToDMX(motorHeights[0]+ height , calMinHeight, calMaxHeight, calMinDMX, calMaxDMX));
		kineticLight->setMotorChannel(1, 2, 0); // Fine-tuning
		kineticLight->setMotorChannel(1, 3, static_cast<uint8_t>(clamp(speed, 0.0, 255.0)));

		// Set additional DMX channels for first motor (lighting)
		for (int i = 4; i <= 62; i++) {
			int dmxValue = dmxInput && (i - 1) < dmxInput->numChannels ?
				static_cast<int>(clamp(dmxInput->getChannelData(i - 1)[0], 0.0f, 255.0f)) : 0;
			kineticLight->setMotorChannel(1, i, dmxValue);
		}

		// Second motor (9CH)
		kineticLight->setMotorChannel(2, 1, heightToDMX(motorHeights[1] + height, calMinHeight, calMaxHeight, calMinDMX, calMaxDMX));
		kineticLight->setMotorChannel(2, 2, 0); // Fine-tuning
		kineticLight->setMotorChannel(2, 3, static_cast<uint8_t>(clamp(speed, 0.0, 255.0)));

		// Third motor (9CH)
		kineticLight->setMotorChannel(3, 1, heightToDMX(motorHeights[2] + height, calMinHeight, calMaxHeight, calMinDMX, calMaxDMX));
		kineticLight->setMotorChannel(3, 2, 0); // Fine-tuning
		kineticLight->setMotorChannel(3, 3, static_cast<uint8_t>(clamp(speed, 0.0, 255.0)));


		// Copy values from KineticLight to output channels
		for (int i = 0; i < output->numChannels; i++) {
			float* channel = output->channels[i];
			if (i < 62) {
				// First motor channels (1-62)
				channel[0] = kineticLight->getMotor(1)->getChannel(i + 1);
			}
			else if (i >= 62 && i < 71) {
				// Second motor channels (63-71)
				channel[0] = kineticLight->getMotor(2)->getChannel(i - 61);
			}
			else if (i >= 71 && i < 80) {
				// Third motor channels (72-80)
				channel[0] = kineticLight->getMotor(3)->getChannel(i - 70);
			}
		}
	}
	catch (const std::exception& e) {
		// Handle errors by setting all channels to 0
		for (int i = 0; i < output->numChannels; i++) {
			output->channels[i][0] = 0;
		}
	}
}

int32_t
CPlusPlusCHOPExample::getNumInfoCHOPChans(void * reserved1)
{
	// We return the number of channel we want to output to any Info CHOP
	// connected to the CHOP. In this example we are just going to send one channel.
	return 2;
}

void
CPlusPlusCHOPExample::getInfoCHOPChan(int32_t index,
										OP_InfoCHOPChan* chan,
										void* reserved1)
{
	// This function will be called once for each channel we said we'd want to return
	// In this example it'll only be called once.

	if (index == 0)
	{
		chan->name->setString("executeCount");
		chan->value = (float)myExecuteCount;
	}

	if (index == 1)
	{
		chan->name->setString("offset");
		chan->value = (float)myOffset;
	}
}

bool		
CPlusPlusCHOPExample::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
{
	infoSize->rows = 2;
	infoSize->cols = 2;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void
CPlusPlusCHOPExample::getInfoDATEntries(int32_t index,
										int32_t nEntries,
										OP_InfoDATEntries* entries, 
										void* reserved1)
{
	char tempBuffer[4096];

	if (index == 0)
	{
		// Set the value for the first column
		entries->values[0]->setString("executeCount");

		// Set the value for the second column
#ifdef _WIN32
		sprintf_s(tempBuffer, "%d", myExecuteCount);
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%d", myExecuteCount);
#endif
		entries->values[1]->setString(tempBuffer);
	}

	if (index == 1)
	{
		// Set the value for the first column
		entries->values[0]->setString("offset");

		// Set the value for the second column
#ifdef _WIN32
		sprintf_s(tempBuffer, "%g", myOffset);
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString( tempBuffer);
	}
}

void
CPlusPlusCHOPExample::buildDynamicMenu(const OP_Inputs* inputs, OP_BuildDynamicMenuInfo* info, void* reserved1)
{

}

void
CPlusPlusCHOPExample::setupParameters(OP_ParameterManager* manager, void *reserved1)
{
	


	



	{
		OP_NumericParameter np;
		np.name = "Basesize";
		np.label = "Base Size";
		np.defaultValues[0] = 1.0;
		np.minSliders[0] = 0.1;
		np.maxSliders[0] = 5.0;
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	{
		OP_NumericParameter np;
		np.name = "Minheight";
		np.label = "Min Height";
		np.defaultValues[0] = 0.5;
		np.minSliders[0] = 0.0;
		np.maxSliders[0] = 1.0;
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	{
		OP_NumericParameter np;
		np.name = "Maxheight";
		np.label = "Max Height";
		np.defaultValues[0] = 3.0;
		np.minSliders[0] = 1.0;
		np.maxSliders[0] = 10.0;
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}


	// pitch
	{
		OP_NumericParameter np;

		np.name = "Minpitch";
		np.label = "Min Pitch";
		np.defaultValues[0] = -45.0;
		np.minSliders[0] = -90.0;
		np.maxSliders[0] = 90.0;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	{
		OP_NumericParameter np;

		np.name = "Maxpitch";
		np.label = "Max Pitch";
		np.defaultValues[0] = 45.0;
		np.minSliders[0] = -90.0;
		np.maxSliders[0] = 90.0;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// roll
	{
		OP_NumericParameter np;

		np.name = "Minroll";
		np.label = "Min Roll";
		np.defaultValues[0] = -45.0;
		np.minSliders[0] = -90.0;
		np.maxSliders[0] = 90.0;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	{
		OP_NumericParameter np;

		np.name = "Maxroll";
		np.label = "Max Roll";
		np.defaultValues[0] = 45.0;
		np.minSliders[0] = -90.0;
		np.maxSliders[0] = 90.0;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// yaw

	{
		OP_NumericParameter np;

		np.name = "Minyaw";
		np.label = "Min Yaw";
		np.defaultValues[0] = -45.0;
		np.minSliders[0] = -90.0;
		np.maxSliders[0] = 90.0;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	{
		OP_NumericParameter np;

		np.name = "Maxyaw";
		np.label = "Max Yaw";
		np.defaultValues[0] = 45.0;
		np.minSliders[0] = -90.0;
		np.maxSliders[0] = 90.0;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// need parameters for calibation of  max Hieght that motor can goes, min Height and 0 to 255 min DMXOUT and ,ax DMXOUT

	{
		OP_NumericParameter np;

		np.name = "Calibrationmaxheight";
		np.label = "Motor Calibration Max height";
		np.defaultValues[0] = 3.0;
		np.minSliders[0] = 1.0;
		np.maxSliders[0] = 10.0;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);

	}

	{
		OP_NumericParameter np;

		np.name = "Calibrationminheight";
		np.label = "Motor Calibration Min height";
		np.defaultValues[0] = 0.5;
		np.minSliders[0] = 0.0;
		np.maxSliders[0] = 1.0;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);

	}

	{
		OP_NumericParameter np;

		np.name = "Calibrationmindmxout";
		np.label = "Motor Calibration Min DMXOUT";
		np.defaultValues[0] = 0.0;
		np.minSliders[0] = 0.0;
		np.maxSliders[0] = 255.0;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);

	}

	{
		OP_NumericParameter np;

		np.name = "Calibrationmaxdmxout";
		np.label = "Motor Calibration Max DMXOUT";
		np.defaultValues[0] = 255.0;
		np.minSliders[0] = 0.0;
		np.maxSliders[0] = 255.0;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);

	}







}

void 
CPlusPlusCHOPExample::pulsePressed(const char* name, void* reserved1)
{
	if (!strcmp(name, "Reset"))
	{
		myOffset = 0.0;
	}
}


// Motor base class constructor
Motor::Motor(MotorType t) : type(t) {}

// Clamp DMX values to the 0-255 range
int Motor::clamp(int value) { return (value < 0) ? 0 : (value > 255) ? 255 : value; }

// Set a specific DMX channel's value
void Motor::setChannel(int channel, uint8_t value) {
	dmxChannels[channel] = clamp(value);
}

// Get a specific DMX channel's value
int Motor::getChannel(int channel) const {
	auto it = dmxChannels.find(channel);
	return (it != dmxChannels.end()) ? it->second : 0;
}

// Print all DMX channel values for the motor
void Motor::printStatus() const {
	std::cout << (type == NINE_CH ? "9CH" : type == TEN_CH ? "10CH" : "62CH") << " Motor - ";
	for (const auto& channel : dmxChannels) {
		std::cout << "CH" << channel.first << ": " << channel.second << " ";
	}
	std::cout << std::endl;
}

// 9CH Motor constructor initializes channels
Motor9CH::Motor9CH() : Motor(NINE_CH) {
	for (int ch = 1; ch <= 9; ++ch) dmxChannels[ch] = 0;
}

void Motor9CH::printStatus() const {
	std::cout << "9CH Motor - ";
	Motor::printStatus();
}

// 10CH Motor constructor initializes channels
Motor10CH::Motor10CH() : Motor(TEN_CH) {
	for (int ch = 1; ch <= 10; ++ch) dmxChannels[ch] = 0;
}

void Motor10CH::printStatus() const {
	std::cout << "10CH Motor - ";
	Motor::printStatus();
}

// 62CH Motor constructor initializes channels
Motor62CH::Motor62CH() : Motor(SIXTY_TWO_CH) {
	for (int ch = 1; ch <= 62; ++ch) dmxChannels[ch] = 0;
}

void Motor62CH::printStatus() const {
	std::cout << "62CH Motor - ";
	Motor::printStatus();
}


KineticLight::KineticLight(Motor* m1, Motor* m2, Motor* m3)
	: motor1(m1), motor2(m2), motor3(m3) {}

KineticLight::~KineticLight() {
	delete motor1;
	delete motor2;
	delete motor3;
}

void KineticLight::setMotorChannel(int motorIndex, int channel, uint8_t value) {
	if (motorIndex == 1) motor1->setChannel(channel, value);
	else if (motorIndex == 2) motor2->setChannel(channel, value);
	else if (motorIndex == 3) motor3->setChannel(channel, value);
}

void KineticLight::printStatus() const {
	std::cout << "Kinetic Light Status:" << std::endl;
	motor1->printStatus();
	motor2->printStatus();
	motor3->printStatus();
}

const Motor* KineticLight::getMotor(int index) const {
	switch (index) {
	case 1: return motor1;
	case 2: return motor2;
	case 3: return motor3;
	default: return nullptr;
	}
}