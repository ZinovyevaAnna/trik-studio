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

#pragma once

#include "generatorBase/simpleGenerators/abstractSimpleGenerator.h"

#include "src/readableLabelManager.h"

namespace generatorBase {
namespace simple {

/// Generator for conditional construnctions
class SyntheticIfGenerator : public AbstractSimpleGenerator
{
public:
	SyntheticIfGenerator(const qrRepo::RepoApi &repo
		, GeneratorCustomizer &customizer
		, const QMap<qReal::Id, bool> &useVariable
		, bool elseIsEmpty
		, const QString syntheticCondition
		, const qReal::Id &id
		, bool needInverting
		, ReadableLabelManager &nameManager
		, QObject *parent = nullptr);

	QString generate() override;

private:
	const QMap<qReal::Id, bool> mUseVariable;
	const QString mSyntheticCondition;
	const bool mElseIsEmpty;
	const bool mNeedInverting;
	ReadableLabelManager &mNameManager;
};

}
}