/* Copyright 2013-2021 CyberTech Labs Ltd.
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

#include "structurizerNode.h"

#include <qrkernel/ids.h>

namespace generatorBase {

class SwitchStructurizerNode : public StructurizerNode
{
public:
	explicit SwitchStructurizerNode(const Vertex &id, const QVector<Vertex> &branches, QObject *parent);

	Type type() const;
	Vertex id() const;
	QMap<Vertex, StructurizerNode *> branches() const;
	QPair<Vertex, StructurizerNode *> defaultBranch() const;

	bool containsContinuation(const Vertex &id = Vertex()) const;
	void dropContinuations(const Vertex &id);
	int numberOfContinuation(const Vertex &id = Vertex()) const;

	void factorize(const Vertex &id, bool force = false);
	void derecursivate(const Vertex &id);
	void replaceContinuation(const Vertex &id, StructurizerNode *value);

	bool mergeConditionalBranches(const QSet<Vertex> &exits, const QSet<Vertex> &loopHeads);
	void transformDoWhile();

	ConditionTree *findAllContinuations(const Vertex &id) const;
	bool isEqual(StructurizerNode *other) const;
	StructurizerNode *clone() const;
	int numberOfConditionCalculating(const Vertex &id) const;

	void dropEmptyConditionals();

	bool hasBreakOnUpperLevel() const;

	bool isEmpty() const;

protected:
	const Vertex mId;
	QMap<Vertex, StructurizerNode *> mBranches;
	QPair<Vertex, StructurizerNode *> mDefaultBranch;
};
}
