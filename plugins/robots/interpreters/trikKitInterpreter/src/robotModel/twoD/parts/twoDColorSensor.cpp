#include "twoDColorSensor.h"

using namespace trikKitInterpreter::robotModel::twoD::parts;
using namespace interpreterBase::robotModel;

ColorSensor::ColorSensor(const DeviceInfo &info, const PortInfo &port)
	: robotModel::parts::TrikColorSensor(info, port)
{
}

void ColorSensor::init()
{
}

void ColorSensor::read()
{
	emit newData({0, 0, 0});
}
