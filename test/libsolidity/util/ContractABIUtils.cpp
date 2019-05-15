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

#include <test/libsolidity/util/ContractABIUtils.h>

#include <liblangutil/Common.h>

#include <boost/algorithm/string.hpp>

#include <fstream>
#include <memory>
#include <regex>
#include <stdexcept>

using namespace dev;
using namespace langutil;
using namespace solidity;
using namespace dev::solidity::test;
using namespace std;
using namespace soltest;

namespace
{

auto arraySize(string const& _arrayType) -> size_t
{
	auto leftBrack = _arrayType.find("[");
	auto rightBrack = _arrayType.find("]");

//	soltestAssert(leftBrack != string::npos && rightBrack != string::npos, "");

	string size = _arrayType.substr(leftBrack + 1, rightBrack - leftBrack - 1);
	return static_cast<size_t>(stoi(size));
}

}

dev::solidity::test::ParameterList ContractABIUtils::parametersFromJson(
	ErrorReporter& _errorReporter,
	Json::Value const& _contractABI,
	string const& _functionName
) const
{
	ParameterList abiParams;
	for (auto const& function: _contractABI)
		if (function["name"] == _functionName)
			for (auto const& output: function["outputs"])
			{
				cout << output << endl;
				string type = output["type"].asString();
				auto types = fromTypeName(output);
				if (types)
					for (auto const& type: types.get())
						abiParams.push_back(Parameter{bytes(), "", type, FormatInfo{}});
				else
				{
					_errorReporter.error(
						"Could not convert \"" + type +
						"\" to internal ABI type representation. Unable to update expectations."
					);
				}
			}

	return abiParams;
}

boost::optional<vector<ABIType>> ContractABIUtils::fromTypeName(Json::Value const& _functionOutput) const
{
	static regex s_boolType{"(bool)"};
	static regex s_uintType{"(uint\\d*)"};
	static regex s_intType{"(int\\d*)"};
	static regex s_bytesType{"(bytes\\d+)"};
	static regex s_dynBytesType{"(\\bbytes\\b)"};
	static regex s_stringType{"(string)"};
	static regex s_tupleType{"(tuple)"};

	static regex s_boolArrayType{"(bool)(\\[\\d+\\])"};
	static regex s_uintArrayType{"(uint\\d*)(\\[\\d+\\])"};
	static regex s_intArrayType{"(int\\d*)(\\[\\d+\\])"};
	static regex s_stringArrayType{"(string)(\\[\\d+\\])"};
	static regex s_tupleArrayType{"(tuple)(\\[\\d+\\])"};

	vector<ABIType> abiTypes;
	string type = _functionOutput["type"].asString();

	if (regex_match(type, s_boolType))
		abiTypes.push_back(ABIType{ABIType::Boolean, ABIType::AlignRight, 32});
	else if (regex_match(type, s_uintType))
		abiTypes.push_back(ABIType{ABIType::UnsignedDec, ABIType::AlignRight, 32});
	else if (regex_match(type, s_intType))
		abiTypes.push_back(ABIType{ABIType::SignedDec, ABIType::AlignRight, 32});
	else if (regex_match(type, s_bytesType))
		abiTypes.push_back(ABIType{ABIType::Hex, ABIType::AlignRight, 32});
	else if (regex_match(type, s_dynBytesType))
	{
		abiTypes.push_back(ABIType{ABIType::UnsignedDec, ABIType::AlignRight, 32});
		abiTypes.push_back(ABIType{ABIType::UnsignedDec, ABIType::AlignRight, 32});
		abiTypes.push_back(ABIType{ABIType::HexString, ABIType::AlignLeft, 32});
	}
	else if (regex_match(type, s_stringType))
	{
		abiTypes.push_back(ABIType{ABIType::Hex, ABIType::AlignRight, 32, ABIType::Pointer});
		abiTypes.push_back(ABIType{ABIType::UnsignedDec, ABIType::AlignRight, 32});
		abiTypes.push_back(ABIType{ABIType::String, ABIType::AlignLeft, 32});
	}
	else if (regex_match(type, s_tupleType))
	{
		for (auto const& component: _functionOutput["components"])
		{
			auto tupleComponentTypes = fromTypeName(component);
			if (tupleComponentTypes)
				for (auto const& abiType: tupleComponentTypes.get())
					abiTypes.push_back(abiType);
		}
	}
	else if (regex_match(type, s_boolArrayType))
	{
		for (size_t i = 0; i < arraySize(type); i++)
			abiTypes.push_back(ABIType{ABIType::Boolean, ABIType::AlignRight, 32});
	}
	else if (regex_match(type, s_uintArrayType))
	{
		for (size_t i = 0; i < arraySize(type); i++)
			abiTypes.push_back(ABIType{ABIType::UnsignedDec, ABIType::AlignRight, 32});
	}
	else if (regex_match(type, s_intArrayType))
	{
		for (size_t i = 0; i < arraySize(type); i++)
			abiTypes.push_back(ABIType{ABIType::SignedDec, ABIType::AlignRight, 32});
	}
	else if (regex_match(type, s_stringArrayType))
	{
		for (size_t i = 0; i < arraySize(type); i++)
		{
			abiTypes.push_back(ABIType{ABIType::Hex, ABIType::AlignRight, 32, ABIType::Pointer});
			abiTypes.push_back(ABIType{ABIType::UnsignedDec, ABIType::AlignRight, 32});
			abiTypes.push_back(ABIType{ABIType::String, ABIType::AlignLeft, 32});
		}
	}
	else if (regex_match(type, s_tupleArrayType))
	{
		for (size_t i = 0; i < arraySize(type); i++)
		{
			for (auto const& component: _functionOutput["components"])
			{
				auto tupleComponentTypes = fromTypeName(component);
				if (tupleComponentTypes)
					for (auto const& abiType: tupleComponentTypes.get())
						abiTypes.push_back(abiType);
			}
		}
	}
	else
		return boost::none;

	return boost::optional<vector<ABIType>>{abiTypes};
}
