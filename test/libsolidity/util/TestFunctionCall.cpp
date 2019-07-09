/*
	This file is part of solidity.
	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <test/libsolidity/util/TestFunctionCall.h>

#include <test/libsolidity/util/BytesUtils.h>
#include <test/libsolidity/util/ContractABIUtils.h>

#include <libdevcore/AnsiColorized.h>

#include <boost/algorithm/string/replace.hpp>
#include <boost/optional/optional.hpp>

#include <stdexcept>
#include <string>

using namespace dev;
using namespace solidity;
using namespace dev::solidity::test;
using namespace std;

string TestFunctionCall::format(
	ErrorReporter& _errorReporter,
	string const& _linePrefix,
	bool const _renderResult,
	bool const _highlight
) const
{
	using namespace soltest;
	using Token = soltest::Token;

	stringstream stream;

	bool highlight = !matchesExpectation() && _highlight;

	auto formatOutput = [&](bool const _singleLine)
	{
		string ws = " ";
		string arrow = formatToken(Token::Arrow);
		string colon = formatToken(Token::Colon);
		string comma = formatToken(Token::Comma);
		string comment = formatToken(Token::Comment);
		string ether = formatToken(Token::Ether);
		string newline = formatToken(Token::Newline);
		string failure = formatToken(Token::Failure);

		/// Formats the function signature. This is the same independent from the display-mode.
		stream << _linePrefix << newline << ws << m_call.signature;
		if (m_call.value > u256(0))
			stream << comma << ws << m_call.value << ws << ether;
		if (!m_call.arguments.rawBytes().empty())
		{
			string output = formatRawParameters(m_call.arguments.parameters, _linePrefix);
			stream << colon;
			if (_singleLine)
				stream << ws;
			stream << output;

		}

		/// Formats comments on the function parameters and the arrow taking
		/// the display-mode into account.
		if (_singleLine)
		{
			if (!m_call.arguments.comment.empty())
				stream << ws << comment << m_call.arguments.comment << comment;
			stream << ws << arrow << ws;
		}
		else
		{
			stream << endl << _linePrefix << newline << ws;
			if (!m_call.arguments.comment.empty())
			{
				 stream << comment << m_call.arguments.comment << comment;
				 stream << endl << _linePrefix << newline << ws;
			}
			stream << arrow << ws;
		}

		/// Format either the expected output or the actual result output
		string result;
		if (!_renderResult)
		{
			bool const isFailure = m_call.expectations.failure;
			result = isFailure ?
				failure :
				formatRawParameters(m_call.expectations.result);
			AnsiColorized(stream, highlight, {dev::formatting::RED_BACKGROUND}) << result;
		}
		else
		{
			bytes output = m_rawBytes;
			bool const isFailure = m_failure;
			result = isFailure ?
				failure :
				matchesExpectation() ?
					formatRawParameters(m_call.expectations.result) :
					formatBytesParameters(
						_errorReporter,
						output,
						m_call.signature,
						m_call.expectations.result,
						highlight
					);

			if (isFailure)
				AnsiColorized(stream, highlight, {dev::formatting::RED_BACKGROUND}) << result;
			else
				stream << result;
		}

		/// Format comments on expectations taking the display-mode into account.
		if (_singleLine)
		{
			if (!m_call.expectations.comment.empty())
				stream << ws << comment << m_call.expectations.comment << comment;
		}
		else
		{
			if (!m_call.expectations.comment.empty())
			{
				stream << endl << _linePrefix << newline << ws;
				stream << comment << m_call.expectations.comment << comment;
			}
		}
	};

	formatOutput(m_call.displayMode == FunctionCall::DisplayMode::SingleLine);
	return stream.str();
}

string TestFunctionCall::formatBytesParameters(
	ErrorReporter& _errorReporter,
	bytes const& _bytes,
	string const& _signature,
	dev::solidity::test::ParameterList const& _params,
	bool _highlight
) const
{
	using ParameterList = dev::solidity::test::ParameterList;

	stringstream os;
	string functionName{_signature.substr(0, _signature.find("("))};

	if (_bytes.empty())
		return {};

	stringstream bs;
	bs << endl << _bytes;
	_errorReporter.warning("Following bytes were returned by the call: " + bs.str());

	boost::optional<ParameterList> abiParams = ContractABIUtils::parametersFromJson(
		_errorReporter,
		m_contractABI,
		functionName
	);

	if (abiParams)
	{
		boost::optional<ParameterList> preferredParams = ContractABIUtils::preferredParameters(
			_errorReporter,
			_params,
			abiParams.get(),
			_bytes
		);

		if (preferredParams)
		{
			ContractABIUtils::overwriteWithABITypes(_errorReporter, preferredParams.get(), abiParams.get());
			os << BytesUtils::formatBytesRange(_bytes, preferredParams.get(), _highlight);
		}
	}
	else
	{
		ParameterList parameters;
		fill_n(
			back_inserter(parameters),
			_bytes.size() / 32,
			Parameter{bytes(), "", ABIType{ABIType::UnsignedDec}, FormatInfo{}}
		);
		os << BytesUtils::formatBytesRange(_bytes, parameters, _highlight);
	}
	return os.str();
}

string TestFunctionCall::formatRawParameters(
	dev::solidity::test::ParameterList const& _params,
	std::string const& _linePrefix
) const
{
	stringstream os;
	for (auto const& param: _params)
	{
		if (!param.rawString.empty())
		{
			if (param.format.newline)
				os << endl << _linePrefix << "// ";
			os << param.rawString;
			if (&param != &_params.back())
				os << ", ";
		}
	}
	return os.str();
}

void TestFunctionCall::reset()
{
	m_rawBytes = bytes{};
	m_failure = true;
}

bool TestFunctionCall::matchesExpectation() const
{
	return m_failure == m_call.expectations.failure && m_rawBytes == m_call.expectations.rawBytes();
}
