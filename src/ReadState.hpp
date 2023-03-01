// Copyright (c) embedded ocean GmbH
#pragma once

#include "Attributes.hpp"

#include <xentara/data/Quality.hpp>
#include <xentara/data/ReadHandle.hpp>
#include <xentara/memory/ObjectBlock.hpp>
#include <xentara/process/Event.hpp>
#include <xentara/utils/eh/Failable.hpp>

#include <chrono>
#include <concepts>
#include <optional>
#include <memory>

namespace xentara::plugins::templateDriver
{

/// @brief State information for a read operation.
template <std::regular DataType>
class ReadState final
{
public:
	/// @brief Resolves an attribute that belong to this state.
	/// @note The value attribute is not resolved, as it may be writable as well, and thus shared with another object that takes
	/// care of the write direction.
	/// @param name The name of the attribute to resolve
	/// @return The attribute, or nullptr if we don't have an attribute with this name
	auto resolveAttribute(std::string_view name) -> const model::Attribute *;

	/// @brief Resolves an event.
	/// @param name The name of the event to resolve
	/// @param parent
	/// @parblock
	/// A shared pointer to the containing object.
	/// 
	/// The pointer is used in the aliasing constructor of std::shared_ptr when constructing the
	/// return value, so that the returned pointer will share ownership information with pointers to the parent object.
	/// @endparblock
	/// @return The event, or nullptr if we don't have an event with this name
	auto resolveEvent(std::string_view name, std::shared_ptr<void> parent) -> std::shared_ptr<process::Event>;

	/// @brief Creates a read-handle for an attribute that belong to this state.
	/// @note The value attribute is not handled, it must be gotten separately using valueReadHandle().
	/// @param attribute The attribute to create the handle for
	/// @return A read handle for the attribute, or std::nullopt if the attribute is unknown (including the value attribute)
	auto readHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle>;

	/// @brief Creates a read-handle for the value attribute
	/// @return A read handle to the value attribute
	auto valueReadHandle() const noexcept -> data::ReadHandle;

	/// @brief Realizes the state
	auto realize() -> void;

	/// @brief Updates the data and sends events
	/// @param timeStamp The update time stamp
	/// @param valueOrError This is a variant-like type that will hold either the new value, or an std::error_code object
	/// containing an read error
	auto update(std::chrono::system_clock::time_point timeStamp, const utils::eh::Failable<DataType> &valueOrError) -> void;

private:
	/// @brief This structure is used to represent the state inside the memory block
	struct State final
	{
		/// @brief The update time stamp
		std::chrono::system_clock::time_point _updateTime { std::chrono::system_clock::time_point::min() };
		/// @brief The current value
		DataType _value {};
		/// @brief The change time stamp
		std::chrono::system_clock::time_point _changeTime { std::chrono::system_clock::time_point::min() };
		/// @brief The quality of the value
		data::Quality _quality { data::Quality::Bad };
		/// @brief The error code when reading the value, or a default constructed std::error_code object for none.
		std::error_code _error { CustomError::NotConnected };
	};

	/// @brief A Xentara event that is fired when the value changes
	process::Event _valueChangedEvent { model::Attribute::kValue };
	/// @brief A Xentara event that is fired when the quality changes
	process::Event _qualityChangedEvent { model::Attribute::kQuality };

	/// @brief A summary event that is fired when anything changes
	process::Event _changedEvent { io::Direction::Input };

	/// @brief The data block that contains the state
	memory::ObjectBlock<State> _dataBlock;
};

/// @class xentara::plugins::templateDriver::ReadState
/// @todo change list of extern template statements to the supported types
extern template class ReadState<bool>;
extern template class ReadState<std::uint8_t>;
extern template class ReadState<std::uint16_t>;
extern template class ReadState<std::uint32_t>;
extern template class ReadState<std::uint64_t>;
extern template class ReadState<std::int8_t>;
extern template class ReadState<std::int16_t>;
extern template class ReadState<std::int32_t>;
extern template class ReadState<std::int64_t>;
extern template class ReadState<float>;
extern template class ReadState<double>;

} // namespace xentara::plugins::templateDriver