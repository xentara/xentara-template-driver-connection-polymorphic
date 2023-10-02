// Copyright (c) embedded ocean GmbH
#pragma once

#include "TemplateIoComponent.hpp"
#include "ReadState.hpp"
#include "WriteState.hpp"
#include "ReadTask.hpp"
#include "SingleValueQueue.hpp"
#include "WriteTask.hpp"
#include "AbstractTemplateOutputHandler.hpp"

#include <xentara/process/Task.hpp>
#include <xentara/skill/DataPoint.hpp>
#include <xentara/skill/EnableSharedFromThis.hpp>
#include <xentara/utils/json/decoder/Value.hpp>

#include <functional>
#include <string_view>

namespace xentara::plugins::templateDriver
{

using namespace std::literals;

/// @brief A class representing a specific type of output.
/// @todo rename this class to something more descriptive
class TemplateOutput final :
	public skill::DataPoint,
	public TemplateIoComponent::ErrorSink,
	public AbstractTemplateOutputHandler::ErrorSink,
	public skill::EnableSharedFromThis<TemplateOutput>
{
public:
	/// @brief The class object containing meta-information about this element type
	/// @todo change class name
	/// @todo assign a unique UUID
	using Class = ConcreteClass<"TemplateOutput", "deadbeef-dead-beef-dead-beefdeadbeef"_uuid>;

	/// @brief This constructor attaches the output to its I/O component
	TemplateOutput(std::reference_wrapper<TemplateIoComponent> ioComponent) :
		_ioComponent(ioComponent)
	{
		ioComponent.get().addErrorSink(*this);
	}
	
	/// @name Virtual Overrides for skill::DataPoint
	/// @{

	auto dataType() const -> const data::DataType & final;

	auto directions() const -> io::Directions final;

	auto forEachAttribute(const model::ForEachAttributeFunction &function) const -> bool final;
	
	auto forEachEvent(const model::ForEachEventFunction &function) -> bool final;

	auto forEachTask(const model::ForEachTaskFunction &function) -> bool final;

	auto makeReadHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle> final;

	auto makeWriteHandle(const model::Attribute &attribute) noexcept -> std::optional<data::WriteHandle> final;

	auto realize() -> void final;

	/// @}

	/// @name Virtual Overrides for TemplateIoComponent::ErrorSink
	/// @{

	auto ioComponentStateChanged(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void final;

	/// @}

	/// @name Virtual Overrides for AbstractTemplateOutputHandler::ErrorSink
	/// @{

	auto handleReadError(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void final;

	auto handleWriteError(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void final;

	/// @}

protected:
	/// @name Virtual Overrides for skill::DataPoint
	/// @{

	auto load(utils::json::decoder::Object &jsonObject,
		config::Resolver &resolver,
		const config::FallbackHandler &fallbackHandler) -> void final;

	/// @}

private:
	// The tasks need access to out private member functions
	friend class ReadTask<TemplateOutput>;
	friend class WriteTask<TemplateOutput>;

	/// @brief Creates an output handler based on a configuration value
	auto createHandler(utils::json::decoder::Value &value) -> std::unique_ptr<AbstractTemplateOutputHandler>;

	/// @brief This function is forwarded to the I/O component.
	auto requestConnect(std::chrono::system_clock::time_point timeStamp) noexcept -> void
	{
		_ioComponent.get().requestConnect(timeStamp);
	}

	/// @brief This function is forwarded to the I/O component.
	auto requestDisconnect(std::chrono::system_clock::time_point timeStamp) noexcept -> void
	{
		_ioComponent.get().requestDisconnect(timeStamp);
	}

	/// @brief This function is called by the "read" task.
	///
	/// This function attempts to read the value if the I/O component is up.
	auto performReadTask(const process::ExecutionContext &context) -> void;
	/// @brief This function is called by the "write" task.
	///
	/// This function attempts to write the value if the I/O component is up.
	auto performWriteTask(const process::ExecutionContext &context) -> void;

	/// @brief Invalidates any read data
	auto invalidateData(std::chrono::system_clock::time_point timeStamp) -> void;

	/// @brief The I/O component this output belongs to
	/// @todo give this a more descriptive name, e.g. "_device"
	std::reference_wrapper<TemplateIoComponent> _ioComponent;

	/// @brief The handler for data type specific functionality, or nullptr, if the data type hans not been loaded yet
	std::unique_ptr<AbstractTemplateOutputHandler> _handler;

	/// @brief The "read" task
	ReadTask<TemplateOutput> _readTask { *this };
	/// @brief The "write" task
	WriteTask<TemplateOutput> _writeTask { *this };
};

} // namespace xentara::plugins::templateDriver