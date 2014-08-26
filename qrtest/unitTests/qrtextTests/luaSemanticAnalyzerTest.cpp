#include "luaSemanticAnalyzerTest.h"

#include "qrtext/core/types/any.h"

#include "qrtext/lua/ast/assignment.h"
#include "qrtext/lua/ast/block.h"
#include "qrtext/lua/ast/functionCall.h"
#include "qrtext/lua/ast/unaryMinus.h"

#include "qrtext/lua/types/integer.h"
#include "qrtext/lua/types/float.h"
#include "qrtext/lua/types/string.h"

#include "gtest/gtest.h"

using namespace qrTest;
using namespace qrtext;
using namespace qrtext::lua;
using namespace qrtext::lua::details;
using namespace qrtext::lua::types;

void LuaSemanticAnalyzerTest::SetUp()
{
	mAnalyzer.reset(new LuaSemanticAnalyzer(mErrors));
	mParser.reset(new LuaParser(mErrors));
	mLexer.reset(new LuaLexer(mErrors));
}

QSharedPointer<qrtext::core::ast::Node> LuaSemanticAnalyzerTest::parse(QString const &code)
{
	return mParser->parse(mLexer->tokenize(code));
}

TEST_F(LuaSemanticAnalyzerTest, sanityCheck)
{
	auto tree = parse("123");
	mAnalyzer->analyze(tree);
	EXPECT_TRUE(mErrors.empty());
	EXPECT_TRUE(mAnalyzer->type(tree)->is<Integer>());
	EXPECT_TRUE(mAnalyzer->type(tree)->is<Number>());
}

TEST_F(LuaSemanticAnalyzerTest, assignment)
{
	auto tree = parse("a = 123");

	auto variable = as<ast::Assignment>(tree)->variable();
	auto value = as<ast::Assignment>(tree)->value();

	mAnalyzer->analyze(tree);

	EXPECT_TRUE(mErrors.empty());
	EXPECT_TRUE(mAnalyzer->type(variable)->is<types::Integer>());
	EXPECT_TRUE(mAnalyzer->type(value)->is<types::Integer>());
}

TEST_F(LuaSemanticAnalyzerTest, unaryOperator)
{
	auto tree = parse("-123");

	mAnalyzer->analyze(tree);

	EXPECT_TRUE(mErrors.empty());
	EXPECT_TRUE(mAnalyzer->type(as<ast::UnaryMinus>(tree))->is<types::Integer>());
}

TEST_F(LuaSemanticAnalyzerTest, propagation)
{
	auto tree = parse("a = -123; b = a");

	mAnalyzer->analyze(tree);

	EXPECT_TRUE(mErrors.empty());

	auto block = as<ast::Block>(tree);
	auto firstAssignment = as<ast::Assignment>(block->children()[0]);
	auto secondAssignment = as<ast::Assignment>(block->children()[1]);

	auto firstVariable = firstAssignment->variable();
	auto firstValue = firstAssignment->value();

	EXPECT_TRUE(mAnalyzer->type(firstVariable)->is<types::Integer>());
	EXPECT_TRUE(mAnalyzer->type(firstValue)->is<types::Integer>());

	auto secondVariable = secondAssignment->variable();
	auto secondValue = secondAssignment->value();

	EXPECT_TRUE(mAnalyzer->type(secondVariable)->is<types::Integer>());
	EXPECT_TRUE(mAnalyzer->type(secondValue)->is<types::Integer>());
}

TEST_F(LuaSemanticAnalyzerTest, integerFloatCoercion)
{
	auto tree = parse("a = -123; a = 1.0");

	mAnalyzer->analyze(tree);

	EXPECT_TRUE(mErrors.isEmpty());

	auto block = as<ast::Block>(tree);
	auto firstAssignment = as<ast::Assignment>(block->children()[0]);
	auto secondAssignment = as<ast::Assignment>(block->children()[1]);

	auto firstVariable = firstAssignment->variable();
	auto firstValue = firstAssignment->value();

	EXPECT_TRUE(mAnalyzer->type(firstVariable)->is<types::Float>());
	EXPECT_TRUE(mAnalyzer->type(firstValue)->is<types::Integer>());

	auto secondVariable = secondAssignment->variable();
	auto secondValue = secondAssignment->value();

	EXPECT_TRUE(mAnalyzer->type(secondVariable)->is<types::Float>());
	EXPECT_TRUE(mAnalyzer->type(secondValue)->is<types::Float>());
}

TEST_F(LuaSemanticAnalyzerTest, functionReturnType)
{
	auto tree = parse("a = f(1)");

	mAnalyzer->addIntrinsicFunction("f", QSharedPointer<types::Function>(new types::Function(
			QSharedPointer<core::types::TypeExpression>(new types::Float()),
			{QSharedPointer<core::types::TypeExpression>(new types::Integer())}
			)));

	mAnalyzer->analyze(tree);

	EXPECT_TRUE(mErrors.isEmpty());

	auto assignment = as<ast::Assignment>(tree);

	auto a = assignment->variable();
	auto f = assignment->value();

	EXPECT_TRUE(mAnalyzer->type(a)->is<types::Float>());
	EXPECT_TRUE(mAnalyzer->type(f)->is<types::Float>());
}

TEST_F(LuaSemanticAnalyzerTest, functionParameters)
{
	auto tree = parse("a = f(b, c)");

	mAnalyzer->addIntrinsicFunction("f", QSharedPointer<types::Function>(new types::Function(
			QSharedPointer<core::types::TypeExpression>(new types::Float()),
			{QSharedPointer<core::types::TypeExpression>(new types::Integer())
					, QSharedPointer<core::types::TypeExpression>(new types::String())}
			)));

	mAnalyzer->analyze(tree);

	EXPECT_TRUE(mErrors.isEmpty());

	auto assignment = as<ast::Assignment>(tree);

	auto f = as<ast::FunctionCall>(assignment->value());

	auto b = f->arguments()[0];
	auto c = f->arguments()[1];

	EXPECT_TRUE(mAnalyzer->type(b)->is<types::Integer>());
	EXPECT_TRUE(mAnalyzer->type(c)->is<types::String>());
}

TEST_F(LuaSemanticAnalyzerTest, invalidFunctionParameters)
{
	auto tree = parse("a = f(0.5, 'a')");

	mAnalyzer->addIntrinsicFunction("f", QSharedPointer<types::Function>(new types::Function(
			QSharedPointer<core::types::TypeExpression>(new types::Float()),
			{QSharedPointer<core::types::TypeExpression>(new types::Integer())
					, QSharedPointer<core::types::TypeExpression>(new types::String())}
			)));

	mAnalyzer->analyze(tree);

	ASSERT_FALSE(mErrors.isEmpty());
}

TEST_F(LuaSemanticAnalyzerTest, invalidCoercion)
{
	auto tree = parse("a = f(b, c); b = 0.5");

	mAnalyzer->addIntrinsicFunction("f", QSharedPointer<types::Function>(new types::Function(
			QSharedPointer<core::types::TypeExpression>(new types::Float()),
			{QSharedPointer<core::types::TypeExpression>(new types::Integer())
					, QSharedPointer<core::types::TypeExpression>(new types::String())}
			)));

	mAnalyzer->analyze(tree);

	EXPECT_FALSE(mErrors.isEmpty());

	auto block = as<ast::Block>(tree);
	auto firstAssignment = as<ast::Assignment>(block->children()[0]);

	auto a = firstAssignment->variable();
	auto f = as<ast::FunctionCall>(firstAssignment->value());

	EXPECT_TRUE(mAnalyzer->type(a)->is<types::Float>());

	auto b = f->arguments()[0];
	auto c = f->arguments()[1];

	EXPECT_TRUE(mAnalyzer->type(b)->is<qrtext::core::types::Any>());
	EXPECT_TRUE(mAnalyzer->type(c)->is<types::String>());
}
