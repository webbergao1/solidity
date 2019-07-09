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

#pragma once

#include <test/libsolidity/util/SoltestTypes.h>

#include <test/libsolidity/util/SoltestErrors.h>

#include <libdevcore/CommonData.h>

#include <json/json.h>

namespace dev
{
namespace solidity
{
namespace test
{

using ABITypes = std::vector<ABIType>;

/**
 * Utility class that aids conversions from contract ABI types stored in a
 * Json value to the internal ABIType representation of isoltest.
 */
class ContractABIUtils
{
public:
	/// Parses and translates Solidity's ABI types as Json string into
	/// a list of internal type representations of isoltest.
	/// Creates parameters from Contract ABI and is used to generate values for
	/// auto-correction during interactive update routine.
	static boost::optional<ParameterList> parametersFromJson(
		ErrorReporter& _errorReporter,
		Json::Value const& _contractABI,
		std::string const& _functionName
	);

	/// Overwrites types given by _inputParameters with ABI types if given
	/// ones do not match.
	static void overwriteWithABITypes(
		ErrorReporter& _errorReporter,
		ParameterList& _inputParameters,
		ParameterList const& _abiParameters
	);

	/// If parameter count does not match, take types defined by ABI, but only
	/// if the contract ABI is defined (needed for format tests where the actual
	/// result does not matter).
	static ParameterList preferredParameters(
		ErrorReporter& _errorReporter,
		ParameterList const& _inputParameters,
		ParameterList const& _abiParameters,
		bytes _bytes
	);

private:
	/// Parses and translates a single type and returns a list of
	/// internal type representations of isoltest.
	/// Types defined by the ABI will translate to ABITypes
	/// as follows:
	/// `bool` -> [`Boolean`]
	/// `uint` -> [`Unsigned`]
	/// `string` -> [`Unsigned`, `Unsigned`, `String`]
	/// `bytes` -> [`Unsigned`, `Unsigned`, `HexString`]
	/// ...
	static bool appendTypesFromName(
		Json::Value const& _functionOutput,
		ABITypes& _addressTypes,
		ABITypes& _valueTypes,
		ABITypes& _dynamicTypes,
		bool _isCompoundType = false
	);
};

}
}
}
