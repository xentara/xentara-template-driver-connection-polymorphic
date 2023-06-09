// Copyright (c) embedded ocean GmbH
#include "ReadState.hpp"

#include "Attributes.hpp"

#include <xentara/memory/memoryResources.hpp>
#include <xentara/memory/WriteSentinel.hpp>

namespace xentara::plugins::templateDriver
{

template <std::regular DataType>
auto ReadState<DataType>::forEachAttribute(const model::ForEachAttributeFunction &function) const -> bool
{
	// Handle all the attributes we support
	return
		function(model::Attribute::kUpdateTime) ||
		function(model::Attribute::kChangeTime) ||
		function(model::Attribute::kQuality) ||
		function(attributes::kError);
}

template <std::regular DataType>
auto ReadState<DataType>::forEachEvent(const model::ForEachEventFunction &function, std::shared_ptr<void> parent) -> bool
{
	// Handle all the events we support
	return
		function(model::Attribute::kValue, std::shared_ptr<process::Event>(parent, &_valueChangedEvent)) ||
		function(model::Attribute::kQuality, std::shared_ptr<process::Event>(parent, &_qualityChangedEvent)) ||
		function(process::Event::kChanged, std::shared_ptr<process::Event>(parent, &_changedEvent));
}

template <std::regular DataType>
auto ReadState<DataType>::makeReadHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle>
{
	// Try each readable attribute
	if (attribute == model::Attribute::kUpdateTime)
	{
		return _dataBlock.member(&State::_updateTime);
	}
	else if (attribute == model::Attribute::kChangeTime)
	{
		return _dataBlock.member(&State::_changeTime);
	}
	else if (attribute == model::Attribute::kQuality)
	{
		return _dataBlock.member(&State::_quality);
	}
	else if (attribute == attributes::kError)
	{
		return _dataBlock.member(&State::_error);
	}

	return std::nullopt;
}

template <std::regular DataType>
auto ReadState<DataType>::valueReadHandle() const noexcept -> data::ReadHandle
{
	return _dataBlock.member(&State::_value);
}

template <std::regular DataType>
auto ReadState<DataType>::realize() -> void
{
	// Create the data block
	_dataBlock.create(memory::memoryResources::data());
}

template <std::regular DataType>
auto ReadState<DataType>::update(std::chrono::system_clock::time_point timeStamp, const utils::eh::expected<DataType, std::error_code> &valueOrError) -> void
{
	// Make a write sentinel
	memory::WriteSentinel sentinel { _dataBlock };
	auto &state = *sentinel;
	const auto &oldState = sentinel.oldValue();

	state._updateTime = timeStamp;

	// See if we have a value
	if (valueOrError)
	{
		// Set the value
		state._value = *valueOrError;

		// Reset the error
		state._quality = data::Quality::Good;
		state._error = {};
	}
	// We don't have a value, but an error
	else
	{
		// Reset the value to a default constructed value
		state._value = {};

		// Set the error
		state._quality = data::Quality::Bad;
		state._error = valueOrError.error();
	}

	// Detect changes
	const auto valueChanged = state._value != oldState._value;
	const auto qualityChanged = state._quality != oldState._quality;
	const auto errorChanged = state._error != oldState._error;
	const auto changed = valueChanged || qualityChanged || errorChanged;

	// Update the change time, if necessary. We always need to write the change time, even if it is the same as before,
	// because the memory resource might use swap-in.
	state._changeTime = changed ? timeStamp : oldState._changeTime;

	// Commit the data before sending the events
	sentinel.commit();

	// Fire the correct events
	if (valueChanged)
	{
		_valueChangedEvent.fire();
	}
	if (qualityChanged)
	{
		_qualityChangedEvent.fire();
	}
	if (changed)
	{
		_changedEvent.fire();
	}
}

/// @class xentara::plugins::templateDriver::ReadState
/// @todo change list of template instantiations to the supported types
template class ReadState<bool>;
template class ReadState<std::uint8_t>;
template class ReadState<std::uint16_t>;
template class ReadState<std::uint32_t>;
template class ReadState<std::uint64_t>;
template class ReadState<std::int8_t>;
template class ReadState<std::int16_t>;
template class ReadState<std::int32_t>;
template class ReadState<std::int64_t>;
template class ReadState<float>;
template class ReadState<double>;
template class ReadState<std::string>;

} // namespace xentara::plugins::templateDriver