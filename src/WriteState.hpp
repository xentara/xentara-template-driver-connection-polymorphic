// Copyright (c) embedded ocean GmbH
#pragma once

#include "Attributes.hpp"

#include <xentara/data/ReadHandle.hpp>
#include <xentara/memory/ObjectBlock.hpp>
#include <xentara/process/Event.hpp>

#include <chrono>
#include <concepts>
#include <optional>
#include <memory>

namespace xentara::plugins::templateDriver
{

/// @brief State information for a write operation.
class WriteState final
{
public:
	/// @brief Resolves an attribute that belong to this state.
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
	/// @param attribute The attribute to create the handle for
	/// @return A read handle for the attribute, or std::nullopt if the attribute is unknown
	auto readHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle>;

	/// @brief Realizes the state
	auto realize() -> void;

	/// @brief Updates the data and sends events
	/// @param timeStamp The update time stamp
	/// @param error The error code, or a default constructed std::error_code object if no error occurred
	auto update(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void;

private:
	/// @brief This structure is used to represent the state inside the memory block
	struct State final
	{
		/// @brief The last time the value was written (successfully or not)
		std::chrono::system_clock::time_point _writeTime { std::chrono::system_clock::time_point::min() };
		/// @brief The error code when writing the value, or 0 for none.
		/// @note The error is initialized to 0, because it is not an error if the value was never written.
		attributes::ErrorCode _writeError { 0 };
	};

	/// @brief A Xentara event that is fired when the value was successfully written
	process::Event _writtenEvent { io::Direction::Output };
	/// @brief A Xentara event that is fired when a write error occurred
	process::Event _writeErrorEvent { io::Direction::Output };

	/// @brief The data block that contains the state
	memory::ObjectBlock<State> _dataBlock;
};

} // namespace xentara::plugins::templateDriver