// Copyright (c) embedded ocean GmbH
#include "WriteState.hpp"

#include "Attributes.hpp"
#include "Events.hpp"

#include <xentara/memory/memoryResources.hpp>
#include <xentara/memory/WriteSentinel.hpp>

#include <string_view>

namespace xentara::plugins::templateDriver
{

using namespace std::literals;

auto WriteState::forEachAttribute(const model::ForEachAttributeFunction &function) const -> bool
{
	// Handle all the attributes we support
	return
		function(model::Attribute::kWriteTime) ||
		function(attributes::kWriteError);
}

auto WriteState::forEachEvent(const model::ForEachEventFunction &function, std::shared_ptr<void> parent) -> bool
{
	// Handle all the events we support
	return
		function(events::kWritten, std::shared_ptr<process::Event>(parent, &_writtenEvent)) ||
		function(attributes::kWriteError, std::shared_ptr<process::Event>(parent, &_writeErrorEvent));
}

auto WriteState::makeReadHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle>
{
	// Try each readable attribute
	if (attribute == model::Attribute::kWriteTime)
	{
		return _dataBlock.member(&State::_writeTime);
	}
	else if (attribute == attributes::kWriteError)
	{
		return _dataBlock.member(&State::_writeError);
	}

	return std::nullopt;
}

auto WriteState::realize() -> void
{
	// Create the data block
	_dataBlock.create(memory::memoryResources::data());
}

auto WriteState::update(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void
{
	// Make a write sentinel
	memory::WriteSentinel sentinel { _dataBlock };
	auto &state = *sentinel;

	// Update the state
	state._writeTime = timeStamp;
	state._writeError = error;
	// Commit the data before sending the event
	sentinel.commit();

	// Fire the correct event
	if (!error)
	{
		_writtenEvent.fire();
	}
	else
	{
		_writeErrorEvent.fire();
	}
}

} // namespace xentara::plugins::templateDriver