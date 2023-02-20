// Copyright (c) embedded ocean GmbH
#pragma once

#include "AbstractTemplateInputHandler.hpp"
#include "ReadState.hpp"

#include <xentara/model/Attribute.hpp>

namespace xentara::plugins::templateDriver
{

using namespace std::literals;

// Data type specific functionality for TemplateInput.
// 
/// @todo rename this class to something more descriptive
// 
/// @todo split this class into several classes for different value types or classes of value type, if necessary.
// For example, this class could be split into TemplateBooleanInputHandler, TemplateIntegerInputHandler,
// and TemplateFloatingPointInputHandler classes.
template <typename ValueType>
class TemplateInputHandler final : public AbstractTemplateInputHandler
{
public:
	/// @name Virtual Overrides for AbstractTemplateInputHandler
	/// @{

	auto dataType() const -> const data::DataType & final;

	auto resolveAttribute(std::string_view name) -> const model::Attribute * final;

	auto resolveEvent(std::string_view name, std::shared_ptr<void> parent) -> std::shared_ptr<process::Event> final;

	auto readHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle> final;

	auto realize() -> void final;
		
	auto read(std::chrono::system_clock::time_point timeStamp, ErrorSink &errorSink) -> void final;

	auto updateState(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void final;
	
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

	/// @brief Determines the correct data type based on the *ValueType* template parameter
	///
	/// This function returns the same value as dataType(), but is static and constexpr.
	static constexpr auto staticDataType() -> const data::DataType &;

	/// @brief The state
	ReadState<ValueType> _state;
};

/// @class xentara::plugins::templateDriver::TemplateInputHandler
/// @todo change list of extern template statements to the supported types
extern template class TemplateInputHandler<bool>;
extern template class TemplateInputHandler<std::uint8_t>;
extern template class TemplateInputHandler<std::uint16_t>;
extern template class TemplateInputHandler<std::uint32_t>;
extern template class TemplateInputHandler<std::uint64_t>;
extern template class TemplateInputHandler<std::int8_t>;
extern template class TemplateInputHandler<std::int16_t>;
extern template class TemplateInputHandler<std::int32_t>;
extern template class TemplateInputHandler<std::int64_t>;
extern template class TemplateInputHandler<float>;
extern template class TemplateInputHandler<double>;

} // namespace xentara::plugins::templateDriver