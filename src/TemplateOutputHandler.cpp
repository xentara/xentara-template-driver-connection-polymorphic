// Copyright (c) embedded ocean GmbH
#include "TemplateOutputHandler.hpp"

#include "Attributes.hpp"

#include <xentara/data/DataType.hpp>
#include <xentara/data/ReadHandle.hpp>
#include <xentara/data/WriteHandle.hpp>
#include <xentara/model/Attribute.hpp>
#include <xentara/utils/eh/currentErrorCode.hpp>
#include <xentara/utils/tools/Concepts.hpp>

namespace xentara::plugins::templateDriver
{
	
using namespace std::literals;

template <typename ValueType>
const model::Attribute TemplateOutputHandler<ValueType>::kValueAttribute { model::Attribute::kValue, model::Attribute::Access::ReadWrite, staticDataType() };

template <typename ValueType>
auto TemplateOutputHandler<ValueType>::read(std::chrono::system_clock::time_point timeStamp, ErrorSink &errorSink) -> void
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
auto TemplateOutputHandler<ValueType>::updateReadState(std::chrono::system_clock::time_point timeStamp, std::error_code error)
	-> void
{
	_readState.update(timeStamp, error);
}

template <typename ValueType>
auto TemplateOutputHandler<ValueType>::doRead(std::chrono::system_clock::time_point timeStamp) -> void
{
	/// @todo read the value
	ValueType value = {};

	/// @todo if the read function does not throw errors, but uses return types or internal handle state,
	// throw an std::system_error here on failure, or call handleReadError() directly.

	// The read was successful
	_readState.update(timeStamp, value);

	/// @todo it may be advantageous to split this function up according to value type, either using explicit 
	/// template specialization, or using if constexpr().
	//
	// For example, this function could be split into doRealBoolean(), doRealInteger(), and doRealFloatingPoint() functions.
	// These functions could then be called like this:
	//
	// if constexpr (std::same_as<ValueType, bool>)
	// {
	//     doReadBoolean(timeStamp);
	// }
	// else if constexpr (utils::Tools::Integral<ValueType>)
	// {
	//     doRealInteger(timeStamp);
	// }
	// else if constexpr (std::floating_point<ValueType>)
	// {
	//     doRealFloatingPoint(timeStamp);
	// }
	//
	// To determine if a type is an integer type, you should use xentara::utils::Tools::Integral instead of std::integral,
	// because std::integral is true for bool, char, wchar_t, char8_t, char16_t, and char32_t, which is generally not desirable.
}

template <typename ValueType>
auto TemplateOutputHandler<ValueType>::handleReadError(std::chrono::system_clock::time_point timeStamp, std::error_code error, ErrorSink &errorSink)
	-> void
{
	// Update our own state
	_readState.update(timeStamp, error);
	// Notify the error sink
	errorSink.handleReadError(timeStamp, error);
}

template <typename ValueType>
constexpr auto TemplateOutputHandler<ValueType>::staticDataType() -> const data::DataType &
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
auto TemplateOutputHandler<ValueType>::write(std::chrono::system_clock::time_point timeStamp, ErrorSink &errorSink) -> void
{
	// Get the value
	auto pendingValue = _pendingOutputValue.dequeue();
	// If there was no pending value, just bail
	if (!pendingValue)
	{
		return;
	}

	try
	{
		// Call the other write function, but catch exceptions.
		doWrite(*pendingValue, timeStamp);
	}
	catch (const std::exception &)
	{
		// Get the error from the current exception using this special utility function
		const auto error = utils::eh::currentErrorCode();
		// Handle the error
		handleWriteError(timeStamp, error, errorSink);
	}
}

template <typename ValueType>
auto TemplateOutputHandler<ValueType>::updateWriteState(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void
{
	_writeState.update(timeStamp, error);
}

template <typename ValueType>
auto TemplateOutputHandler<ValueType>::doWrite(ValueType value, std::chrono::system_clock::time_point timeStamp) -> void
{
	/// @todo write the value

	/// @todo if the write function does not throw errors, but uses return types or internal handle state,
	// throw an std::system_error here on failure, or call _writeState.update() directly.

	// The write was successful
	_writeState.update(timeStamp, std::error_code());

	/// @todo it may be advantageous to split this function up according to value type, either using explicit 
	/// template specialization, or using if constexpr().
	//
	// For example, this function could be split into doWriteBoolean(), doWriteInteger(), and doWriteFloatingPoint() functions.
	// These functions could then be called like this:
	//
	// if constexpr (std::same_as<ValueType, bool>)
	// {
	//     doWriteBoolean(value, timeStamp);
	// }
	// else if constexpr (utils::Tools::Integral<ValueType>)
	// {
	//     doWriteInteger(value, timeStamp);
	// }
	// else if constexpr (std::floating_point<ValueType>)
	// {
	//     doWriteFloatingPoint(value, timeStamp);
	// }
	//
	// To determine if a type is an integer type, you should use xentara::utils::Tools::Integral instead of std::integral,
	// because std::integral is true for *bool*, *char*, *wchar_t*, *char8_t*, *char16_t*, and *char32_t*, which is generally not desirable.
}

template <typename ValueType>
auto TemplateOutputHandler<ValueType>::handleWriteError(std::chrono::system_clock::time_point timeStamp, std::error_code error, ErrorSink &errorSink)
	-> void
{
	// Update our own state
	_writeState.update(timeStamp, error);
	// Notify the error sink
	errorSink.handleWriteError(timeStamp, error);
}

template <typename ValueType>
auto TemplateOutputHandler<ValueType>::dataType() const -> const data::DataType &
{
	return kValueAttribute.dataType();
}

template <typename ValueType>
auto TemplateOutputHandler<ValueType>::resolveAttribute(std::u16string_view name) -> const model::Attribute *
{
	// Handle the value attribute separately
	if (name == kValueAttribute)
	{
		return &kValueAttribute;
	}

	// Check the read state attributes
	if (auto attribute = _readState.resolveAttribute(name))
	{
		return attribute;
	}
	// Check the write state attributes
	if (auto attribute = _writeState.resolveAttribute(name))
	{
		return attribute;
	}

	return nullptr;
}

template <typename ValueType>
auto TemplateOutputHandler<ValueType>::resolveEvent(std::u16string_view name, std::shared_ptr<void> parent) -> std::shared_ptr<process::Event>
{
	// Check the read state events
	if (auto event = _readState.resolveEvent(name, parent))
	{
		return event;
	}
	// Check the write state events
	if (auto event = _writeState.resolveEvent(name, parent))
	{
		return event;
	}

	return nullptr;
}

template <typename ValueType>
auto TemplateOutputHandler<ValueType>::readHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle>
{
	// Handle the value attribute separately
	if (attribute == kValueAttribute)
	{
		return _readState.valueReadHandle();
	}
	
	// Check the read state attributes
	if (auto handle = _readState.readHandle(attribute))
	{
		return *handle;
	}
	// Check the write state attributes
	if (auto handle = _writeState.readHandle(attribute))
	{
		return *handle;
	}

	return std::nullopt;
}

template <typename ValueType>
auto TemplateOutputHandler<ValueType>::writeHandle(const model::Attribute &attribute, std::shared_ptr<void> parent) noexcept -> std::optional<data::WriteHandle>
{
	// Handle the value attribute, which is the only writable attribute
	if (attribute == kValueAttribute)
	{
		// Make a shared pointer that refers to this handler
		std::shared_ptr<TemplateOutputHandler<ValueType>> sharedThis(parent, this);

		// This magic code creates a write handle of type ValueType that calls scheduleWrite() on sharedThis.
		// (There are two sets of braces needed here: one for data::WriteHandle, and one for std::optional)
		return {{ std::in_place_type<ValueType>, &TemplateOutputHandler<ValueType>::scheduleOutputValue, sharedThis }};
	}

	return std::nullopt;
}

template <typename ValueType>
auto TemplateOutputHandler<ValueType>::realize() -> void
{
	// Realize the state objects
	_readState.realize();
	_writeState.realize();
}

/// @class xentara::plugins::templateDriver::TemplateOutputHandler
/// @todo change list of template instantiations to the supported types
template class TemplateOutputHandler<bool>;
template class TemplateOutputHandler<std::uint8_t>;
template class TemplateOutputHandler<std::uint16_t>;
template class TemplateOutputHandler<std::uint32_t>;
template class TemplateOutputHandler<std::uint64_t>;
template class TemplateOutputHandler<std::int8_t>;
template class TemplateOutputHandler<std::int16_t>;
template class TemplateOutputHandler<std::int32_t>;
template class TemplateOutputHandler<std::int64_t>;
template class TemplateOutputHandler<float>;
template class TemplateOutputHandler<double>;

} // namespace xentara::plugins::templateDriver