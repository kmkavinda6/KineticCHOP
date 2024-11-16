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

#include "CPlusPlusCHOPExample.h"

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <assert.h>

#include <iostream>




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
	ginfo->timeslice = false;

	ginfo->inputMatchIndex = 0;
}

bool
CPlusPlusCHOPExample::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
{
	// If there is an input connected, we are going to match it's channel names etc
	// otherwise we'll specify our own.
	info->numChannels = inputs->getParInt("Mode") == 0 ? 9 : (inputs->getParInt("Mode") == 1 ? 10 : 62);
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

		np.name = "CalibrationminDMXOUT";
		np.label = "Motor Calibration Min DMXOUT";
		np.defaultValues[0] = 0.0;
		np.minSliders[0] = 0.0;
		np.maxSliders[0] = 255.0;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);

	}

	{
		OP_NumericParameter np;

		np.name = "CalibrationmaxDMXOUT";
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
void Motor::setChannel(int channel, int value) {
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

void KineticLight::setMotorChannel(int motorIndex, int channel, int value) {
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

