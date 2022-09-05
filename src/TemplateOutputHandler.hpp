// Copyright (c) embedded ocean GmbH
#pragma once

#include "AbstractTemplateOutputHandler.hpp"
#include "ReadState.hpp"
#include "WriteState.hpp"
#include "SingleValueQueue.hpp"

#include <xentara/model/Attribute.hpp>

namespace xentara::plugins::templateDriver
{

using namespace std::literals;

/// @brief Data type specific functionality for TemplateOutput.
///
/// @todo rename this class to something more descriptive
///
/// @todo split this class into several classes for different value types or classes of value type, if necessary.
/// For example, this class could be split into TemplateBooleanOutputHandler, TemplateIntegerOutputHandler,
/// and TemplateFloatingPointOutputHandler classes.
template <typename ValueType>
class TemplateOutputHandler final : public AbstractTemplateOutputHandler
{
public:
	/// @name Virtual Overrides for AbstractTemplateOutputHandler
	/// @{

	auto dataType() const -> const data::DataType & final;

	auto resolveAttribute(std::u16string_view name) -> const model::Attribute * final;

	auto resolveEvent(std::u16string_view name, std::shared_ptr<void> parent) -> std::shared_ptr<process::Event> final;

	auto readHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle> final;

	auto writeHandle(const model::Attribute &attribute, std::shared_ptr<void> parent) noexcept -> std::optional<data::WriteHandle> final;

	auto realize() -> void final;
		
	auto read(std::chrono::system_clock::time_point timeStamp, ErrorSink &errorSink) -> void final;
	
	auto updateReadState(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void final;

	auto write(std::chrono::system_clock::time_point timeStamp, ErrorSink &errorSink) -> void final;	

	auto updateWriteState(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void final;

	///@}
	
	/// @brief A Xentara attribute containing the current value.
	/// @note This is a member of this class rather than of the attributes namespace, because the access flags
	/// and type may differ from class to class
	static const model::Attribute kValueAttribute;

private:
	/// @brief The actual implementation of read(), which may throw exceptions on error.
	auto doRead(std::chrono::system_clock::time_point timeStamp) -> void;
	/// @brief Handles a read error
	auto handleReadError(std::chrono::system_clock::time_point timeStamp, std::error_code error, ErrorSink &errorSink) -> void;

	/// @brief The actual implementation of write(), which may throw exceptions on error.
	auto doWrite(ValueType value, std::chrono::system_clock::time_point timeStamp) -> void;	
	/// @brief Handles a write error
	auto handleWriteError(std::chrono::system_clock::time_point timeStamp, std::error_code error, ErrorSink &errorSink) -> void;

	/// @brief Determines the correct data type based on the *ValueType* template parameter
	///
	/// This function returns the same value as dataType(), but is static and constexpr.
	static constexpr auto staticDataType() -> const data::DataType &;

	/// @brief Schedules a value to be written.
	///
	/// This function is called by the value write handle.
	auto scheduleOutputValue(ValueType value) noexcept
	{
		_pendingOutputValue.enqueue(value);
	}

	/// @brief The read state
	ReadState<ValueType> _readState;
	/// @brief The write state
	WriteState _writeState;

	/// @brief The queue for the pending output value
	SingleValueQueue<ValueType> _pendingOutputValue;
};

/// @class xentara::plugins::templateDriver::TemplateOutputHandler
/// @todo change list of extern template statements to the supported types
extern template class TemplateOutputHandler<bool>;
extern template class TemplateOutputHandler<std::uint8_t>;
extern template class TemplateOutputHandler<std::uint16_t>;
extern template class TemplateOutputHandler<std::uint32_t>;
extern template class TemplateOutputHandler<std::uint64_t>;
extern template class TemplateOutputHandler<std::int8_t>;
extern template class TemplateOutputHandler<std::int16_t>;
extern template class TemplateOutputHandler<std::int32_t>;
extern template class TemplateOutputHandler<std::int64_t>;
extern template class TemplateOutputHandler<float>;
extern template class TemplateOutputHandler<double>;

} // namespace xentara::plugins::templateDriver