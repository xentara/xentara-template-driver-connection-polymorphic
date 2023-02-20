// Copyright (c) embedded ocean GmbH
#include "TemplateInputHandler.hpp"

#include "Attributes.hpp"

#include <xentara/data/DataType.hpp>
#include <xentara/data/ReadHandle.hpp>
#include <xentara/model/Attribute.hpp>
#include <xentara/utils/eh/currentErrorCode.hpp>
#include <xentara/utils/tools/Concepts.hpp>

namespace xentara::plugins::templateDriver
{
	
using namespace std::literals;

template <typename ValueType>
const model::Attribute TemplateInputHandler<ValueType>::kValueAttribute { model::Attribute::kValue, model::Attribute::Access::ReadOnly, staticDataType() };

template <typename ValueType>
auto TemplateInputHandler<ValueType>::read(std::chrono::system_clock::time_point timeStamp, ErrorSink &errorSink) -> void
{
	try
	{
		// Call the other read function, but catch exceptions.
		doRead(timeStamp);
	}
	catch (const std::exception &)
	{
		// Get the error from the current exception using this special utility function
		const auto error = utils::eh::currentErrorCode();
		// Handle the error
		handleReadError(timeStamp, error, errorSink);
	}
}

template <typename ValueType>
auto TemplateInputHandler<ValueType>::updateState(std::chrono::system_clock::time_point timeStamp, std::error_code error)
	-> void
{
	_state.update(timeStamp, error);
}

template <typename ValueType>
auto TemplateInputHandler<ValueType>::doRead(std::chrono::system_clock::time_point timeStamp) -> void
{
	/// @todo read the value
	ValueType value = {};

	/// @todo if the read function does not throw errors, but uses return types or internal handle state,
	// throw an std::system_error here on failure, or call handleReadError() directly.

	// The read was successful
	_state.update(timeStamp, value);

	/// @todo it may be advantageous to split this function up according to value type, either using explicit 
	/// template specialization, or using if constexpr().
	//
	// For example, this function could be split into doReadBoolean(), doReadInteger(), and doReadFloatingPoint() functions.
	// These functions could then be called like this:
	// 
	// if constexpr (std::same_as<ValueType, bool>)
	// {
	//     doReadBoolean(timeStamp);
	// }
	// else if constexpr (utils::Tools::Integral<ValueType>)
	// {
	//     doReadInteger(timeStamp);
	// }
	// else if constexpr (std::floating_point<ValueType>)
	// {
	//     doReadFloatingPoint(timeStamp);
	// }
	// 
	// To determine if a type is an integer type, you should use xentara::utils::Tools::Integral instead of std::integral,
	// because std::integral is true for *bool*, *char*, *wchar_t*, *char8_t*, *char16_t*, and *char32_t*, which is generally not desirable.
}

template <typename ValueType>
auto TemplateInputHandler<ValueType>::handleReadError(std::chrono::system_clock::time_point timeStamp, std::error_code error, ErrorSink &errorSink)
	-> void
{
	// Update our own state
	_state.update(timeStamp, error);
	// Notify the error sink
	errorSink.handleReadError(timeStamp, error);
}

template <typename ValueType>
constexpr auto TemplateInputHandler<ValueType>::staticDataType() -> const data::DataType &
{
	// Return the correct type
	if constexpr (std::same_as<ValueType, bool>)
	{
	    return data::DataType::kBoolean;
	}
	// To determine if a type is an integer type, we use xentara::utils::Tools::Integral instead of std::integral,
	// because std::integral is true for bool, char, wchar_t, char8_t, char16_t, and char32_t, which we don't want.
	else if constexpr (utils::tools::Integral<ValueType>)
	{
	    return data::DataType::kInteger;
	}
	else if constexpr (std::floating_point<ValueType>)
	{
	    return data::DataType::kFloatingPoint;
	}
}

template <typename ValueType>
auto TemplateInputHandler<ValueType>::dataType() const -> const data::DataType &
{
	return kValueAttribute.dataType();
}

template <typename ValueType>
auto TemplateInputHandler<ValueType>::resolveAttribute(std::string_view name) -> const model::Attribute *
{
	// Handle the value attribute separately
	if (name == kValueAttribute)
	{
		return &kValueAttribute;
	}

	// Check the state attributes
	if (auto attribute = _state.resolveAttribute(name))
	{
		return attribute;
	}

	return nullptr;
}

template <typename ValueType>
auto TemplateInputHandler<ValueType>::resolveEvent(std::string_view name, std::shared_ptr<void> parent) -> std::shared_ptr<process::Event>
{
	// Check the state events
	if (auto event = _state.resolveEvent(name, parent))
	{
		return event;
	}

	return nullptr;
}

template <typename ValueType>
auto TemplateInputHandler<ValueType>::readHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle>
{
	// Handle the value attribute separately
	if (attribute == kValueAttribute)
	{
		return _state.valueReadHandle();
	}
	
	// Check the state attributes
	if (auto handle = _state.readHandle(attribute))
	{
		return *handle;
	}

	return std::nullopt;
}

template <typename ValueType>
auto TemplateInputHandler<ValueType>::realize() -> void
{
	// Realize the state object
	_state.realize();
}

/// @class xentara::plugins::templateDriver::TemplateInputHandler
/// @todo change list of template instantiations to the supported types
template class TemplateInputHandler<bool>;
template class TemplateInputHandler<std::uint8_t>;
template class TemplateInputHandler<std::uint16_t>;
template class TemplateInputHandler<std::uint32_t>;
template class TemplateInputHandler<std::uint64_t>;
template class TemplateInputHandler<std::int8_t>;
template class TemplateInputHandler<std::int16_t>;
template class TemplateInputHandler<std::int32_t>;
template class TemplateInputHandler<std::int64_t>;
template class TemplateInputHandler<float>;
template class TemplateInputHandler<double>;

} // namespace xentara::plugins::templateDriver