#include "stringGenerator.h"

#include "ast/complexIdentifier.h"
#include "ast/text.h"

#include "generator/generatorForComplexIdentifierNode.h"

using namespace generationRules::generator;
using namespace simpleParser::ast;

QString StringGenerator::generatedString(
		const QSharedPointer<String> &stringNode
		, const GeneratorConfigurer &generatorConfigurer
		, ScopeInfo &scopeInfo)
{
	const auto identifierPart = qrtext::as<ComplexIdentifier>(stringNode->identifierPart());
	const auto generatedString = GeneratorForComplexIdentifierNode::generatedResult(identifierPart, generatorConfigurer
			, scopeInfo).toString();

	const auto textPart = qrtext::as<Text>(stringNode->textPart());
	const auto text = textPart->text();

	return generatedString + text;
}
