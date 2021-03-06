/* Copyright 2007-2015 QReal Research Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#include "speakerBlock.h"

using namespace trik::blocks::details;
using namespace kitBase::robotModel;

SpeakerBlock::SpeakerBlock(RobotModelInterface &robotModel, bool isPlayToneHz)
	: kitBase::blocksBase::common::DeviceBlock<robotModel::parts::TrikSpeaker>(robotModel),
	  mIsPlayToneHz(isPlayToneHz)
{
}

void SpeakerBlock::doJob(robotModel::parts::TrikSpeaker &speaker)
{
	if (mIsPlayToneHz) {
		auto duration = intProperty("Duration");
		speaker.play(duration);
	}
	else {
		const QString toSpeak = stringProperty("FileName");
		speaker.play(toSpeak);
	}

	emit done(mNextBlockId);
}
