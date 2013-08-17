#include "controlFlowGeneratorBase.h"

using namespace qReal::robots::generators;

ControlFlowGeneratorBase::ControlFlowGeneratorBase(
		LogicalModelAssistInterface const &logicalModel
		, GraphicalModelAssistInterface const &graphicalModel
		, ErrorReporterInterface &errorReporter
		, GeneratorCustomizer const &customizer
		, Id const &diagramId
		, QObject *parent)
	: QObject(parent)
	, RobotsDiagramVisitor(logicalModel, customizer)
	, mModel(logicalModel)
	, mErrorReporter(errorReporter)
	, mCustomizer(customizer)
	, mDiagram(diagramId)
	, mValidator(logicalModel, graphicalModel, errorReporter, customizer, diagramId)
{
}

bool ControlFlowGeneratorBase::preGenerationCheck()
{
	return mValidator.validate();
}

ControlFlow *ControlFlowGeneratorBase::generate()
{
	if (!preGenerationCheck()) {
		return NULL;
	}

	ControlFlow *result = new ControlFlow;

	// This will start dfs on model graph with processig every block
	// in subclasses which must construct control flow in handlers
	startSearch(mValidator.initialNode());

	return result;
}

enums::semantics::Semantics ControlFlowGeneratorBase::semanticsOf(qReal::Id const &id) const
{
	return mCustomizer.semanticsOf(id.type());
}

QPair<qReal::Id, qReal::Id> ControlFlowGeneratorBase::ifBranchesFor(qReal::Id const &id) const
{
	return mValidator.ifBranchesFor(id);
}

QPair<qReal::Id, qReal::Id> ControlFlowGeneratorBase::loopBranchesFor(qReal::Id const &id) const
{
	return mValidator.loopBranchesFor(id);
}
