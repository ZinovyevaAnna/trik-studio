#pragma once

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QSet>

#include "qrtext/core/lexer/token.h"
#include "qrtext/core/error.h"
#include "qrtext/core/lexer/tokenPatterns.h"

namespace qrtext {
namespace core {
namespace lexer {

/// Configurable lexer of something like Lua 5.3 based on regular expressions. Takes token patterns, provides
/// a list of tokens by given input string, list of lexer errors and a separate list of comments. Allows Unicode input
/// and lexeme definitions.
/// Requires that token redefinitions are sane, for example, keywords shall match the definition of identifiers.
/// Does syntax check on passed regexps (and reports errors as InternalError error type), but does not check whole set
/// of patterns for consistensy. Pattern match order is arbitrary, so there shall be no ambiquities in pattern
///  definitions (except between keywords and identifiers, keywords are processed separately). Does not add whitespaces
/// and newlines to output token stream, but does use them for connection and error recovery, so it is recommended to
/// not fiddle with them much.
/// In case of error skips symbols until next whitespace or newline and reports error.
/// Can be quite slow due to use of regexp matching with every regexp in patterns list, so do not use this lexer
/// on really large files.
///
/// Now lexer (with default token patterns) follows Lua 5.3 specification with following exceptions:
/// - long brackets are not supported, either for string literals or for comments.
template<typename TokenType>
class Lexer {
public:
	/// Constructor.
	/// @param patterns - object containing token patterns.
	/// @param errors - error stream, to which lexer errors shall be added.
	explicit Lexer(TokenPatterns<TokenType> const &patterns, QList<Error> &errors)
		: mPatterns(patterns), mErrors(errors)
	{
		// Doing syntax check of lexeme regexps and searching for whitespace and newline definitions, they will be needed
		// later for error recovery.
		for (TokenType const tokenType : mPatterns.allPatterns()) {
			QRegularExpression const &regExp = mPatterns.tokenPattern(tokenType);
			if (!regExp.isValid()) {
				qDebug() << "Invalid regexp: " + regExp.pattern();
				mErrors << Error(Connection(), "Invalid regexp: " + regExp.pattern()
						, ErrorType::lexicalError, Severity::internalError);
			} else {
				if (tokenType == TokenType::whitespace) {
					mWhitespaceRegexp = regExp;
				} else if (tokenType == TokenType::newline) {
					mNewLineRegexp = regExp;
				}
			}
		}
	}

	/// Tokenizes input string, returns list of detected tokens, list of errors and separate list of comments.
	QList<Token<TokenType>> tokenize(QString const &input)
	{
		mComments.clear();

		QList<Token<TokenType>> result;

		// Initializing connection.
		int absolutePosition = 0;
		int line = 0;
		int column = 0;

		// Scanning input string, trying to find longest match with regexp from a list of lexemes.
		while (absolutePosition < input.length()) {
			CandidateMatch bestMatch = findBestMatch(input, absolutePosition);

			if (bestMatch.match.hasMatch()) {
				int tokenEndLine = line;
				int tokenEndColumn = column;

				if (bestMatch.candidate != TokenType::whitespace
						&& bestMatch.candidate != TokenType::newline
						&& bestMatch.candidate != TokenType::comment)
				{
					// Determining connection of the lexeme. String is the only token that can span multiple lines so
					// special care is needed to maintain connection.
					if (bestMatch.candidate == TokenType::string) {
						QRegularExpressionMatchIterator matchIterator = mNewLineRegexp.globalMatch(
								bestMatch.match.captured());

						QRegularExpressionMatch match;

						while (matchIterator.hasNext()) {
							match = matchIterator.next();
							++tokenEndLine;
						}

						if (match.hasMatch()) {
							int relativeLastNewLineOffset = match.capturedEnd() - 1;
							int absoluteLastNewLineOffset = absolutePosition + relativeLastNewLineOffset;
							int absoluteTokenEnd = bestMatch.match.capturedEnd() - 1;
							tokenEndColumn = absoluteTokenEnd - absoluteLastNewLineOffset - 1;
						} else {
							tokenEndColumn += bestMatch.match.capturedLength() - 1;
						}
					} else {
						tokenEndColumn += bestMatch.match.capturedLength() - 1;
					}

					Range range(Connection(bestMatch.match.capturedStart(), line, column)
							, Connection(bestMatch.match.capturedEnd() - 1, tokenEndLine, tokenEndColumn));

					if (bestMatch.candidate == TokenType::identifier) {
						// Keyword is an identifier which is separate lexeme.
						bestMatch.candidate = checkForKeyword(bestMatch.match.captured());
					}

					result << Token<TokenType>(bestMatch.candidate, range, bestMatch.match.captured());
				} else if (bestMatch.candidate == TokenType::comment) {
					tokenEndColumn += bestMatch.match.capturedLength() - 1;
					Range range(Connection(bestMatch.match.capturedStart(), line, column)
							, Connection(bestMatch.match.capturedEnd() - 1, tokenEndLine, tokenEndColumn));

					mComments << Token<TokenType>(bestMatch.candidate, range, bestMatch.match.captured());
				}

				// Keeping connection updated.
				if (bestMatch.candidate == TokenType::newline) {
					++line;
					column = 0;
				} else if (bestMatch.candidate == TokenType::whitespace || bestMatch.candidate == TokenType::comment) {
					column += bestMatch.match.capturedLength();
				} else {
					line = tokenEndLine;
					column = tokenEndColumn + 1;
				}

				absolutePosition += bestMatch.match.capturedLength();
			} else {
				mErrors << Error({absolutePosition, line, column}
						, "Lexer error", ErrorType::lexicalError, Severity::error);

				// Panic mode: syncing on nearest whitespace or newline token.
				while (!mWhitespaceRegexp.match(input, absolutePosition
						, QRegularExpression::NormalMatch, QRegularExpression::AnchoredMatchOption).hasMatch()
						&& !mNewLineRegexp.match(input, absolutePosition
								, QRegularExpression::NormalMatch, QRegularExpression::AnchoredMatchOption).hasMatch()
						&& absolutePosition < input.length())
				{
					++absolutePosition;
					++column;
				}
			}
		}

		return result;
	}

	QList<Token<TokenType>> comments() const
	{
		return mComments;
	}

private:
	struct CandidateMatch {
		TokenType candidate;
		QRegularExpressionMatch match;
	};

	TokenType checkForKeyword(QString const &identifier) const
	{
		for (TokenType const keyword : mPatterns.allKeywords()) {
			if (mPatterns.keywordPattern(keyword) == identifier) {
				return keyword;
			}
		}

		return TokenType::identifier;
	}

	CandidateMatch findBestMatch(const QString &input, int const absolutePosition) const
	{
		TokenType candidate = TokenType::whitespace;
		QRegularExpressionMatch bestMatch;

		for (TokenType const token : mPatterns.allPatterns()) {
			QRegularExpression const &regExp = mPatterns.tokenPattern(token);

			QRegularExpressionMatch const &match = regExp.match(
					input
					, absolutePosition
					, QRegularExpression::NormalMatch
					, QRegularExpression::AnchoredMatchOption);

			if (match.hasMatch()) {
				if (match.capturedLength() > bestMatch.capturedLength()) {
					bestMatch = match;
					candidate = token;
				}
			}
		}

		return CandidateMatch{candidate, bestMatch};
	}

	TokenPatterns<TokenType> const mPatterns;

	QRegularExpression mWhitespaceRegexp;
	QRegularExpression mNewLineRegexp;

	QList<Error> &mErrors;
	QList<Token<TokenType>> mComments;
};

}
}
}
