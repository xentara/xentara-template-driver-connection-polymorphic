// Copyright (c) embedded ocean GmbH
#pragma once

#include "Attributes.hpp"
#include "CustomError.hpp"

#include <xentara/memory/Array.hpp>
#include <xentara/memory/ObjectBlock.hpp>
#include <xentara/model/ElementCategory.hpp>
#include <xentara/process/Event.hpp>
#include <xentara/process/Task.hpp>
#include <xentara/skill/Element.hpp>
#include <xentara/skill/EnableSharedFromThis.hpp>
#include <xentara/utils/core/Uuid.hpp>
#include <xentara/utils/tools/Unique.hpp>

#include <string_view>
#include <functional>
#include <forward_list>

namespace xentara::plugins::templateDriver
{

using namespace std::literals;

/// @brief A class representing a specific type of I/O component.
/// @todo rename this class to something more descriptive
class TemplateIoComponent final : public skill::Element, public skill::EnableSharedFromThis<TemplateIoComponent>
{
public:
	/// @brief The class object containing meta-information about this element type
	/// @todo change class name
	/// @todo assign a unique UUID
	/// @todo change display name
	using Class = ConcreteClass<"TemplateIoComponent",
		"deadbeef-dead-beef-dead-beefdeadbeef"_uuid,
		"template driver I/O component">;

	/// @brief A handle used to access the I/O component
	/// @todo implement a proper handle
	class Handle final : private utils::tools::Unique
	{
	public:
		/// @brief determines of the I/O component is connected
		explicit operator bool() const noexcept
		{
			/// @todo return the actual state
			return false;
		}
	};

	/// @brief Interface for objects that want to be notified of errors
	class ErrorSink
	{
	public:
		/// @brief Virtual destructor
		/// @note The destructor is pure virtual (= 0) to ensure that this class will remain abstract, even if we should remove all
		/// other pure virtual functions later. This is not necessary, of course, but prevents the abstract class from becoming
		/// instantiable by accident as a result of refactoring.
		virtual ~ErrorSink() = 0;

		/// @brief Called on error, or on success.
		///
		/// This function is called in three instances, with different values for the *error* parameter:
		///
		/// Call reason                               | Value of the *error* parameter
		/// :---------------------------------------- | :------------------------------------------
		/// A connection was successfully established | a default constructed std::error_code object
		/// A connection was gracefully closed        | CustomError::NotConnected
		/// The connection was lost unexpectedly      | an appropriate error code
		///
		/// @todo give this a more descriptive name, e.g. "deviceStateChanged"
		virtual auto ioComponentStateChanged(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void = 0;
	};

	/// @brief Adds an error sink
	auto addErrorSink(std::reference_wrapper<ErrorSink> sink)
	{
		_errorSinks.push_front(sink);
	}

	/// @brief Request that the I/O component be connected.
	///
	/// Each call to this function must be balanced by a call to requestDisconnect().
	/// 
	/// If this is the first request, then the connection will be attempted, and the function will not return until
	/// the connection has been successfully established, or has failed. In either case, error sinks will be notified,
	/// so any error sinks calling this must be prepared to have ioComponentStateChanged() called from within this function.
	auto requestConnect(std::chrono::system_clock::time_point timeStamp) noexcept -> void;

	/// @brief Request that the I/O component be disconnected.
	///
	/// Each call to this function must balance a corresponding call to requestConnect().
	/// 
	/// If this is the last request, then the connection will be closed, and the function will not return until
	/// the connection has been terminated. All error sinks will be notified with error code CustomError::NotConnected,
	/// so any error sinks calling this must be prepared to have ioComponentStateChanged() called from within this function.
	auto requestDisconnect(std::chrono::system_clock::time_point timeStamp) noexcept -> void;
	
	/// @brief Notifies the I/O component that an error was detected from outside, e.g. when reading or writing a data point.
	/// 
	/// If this error affects the I/O component as a whole, error sinks will be notified. If the sender is an error sink itself,
	/// and does not whish to be notified, but intends to handle the error itself instead, it can pass a pointer to itself as the sender parameter. 
	auto handleError(std::chrono::system_clock::time_point timeStamp, std::error_code error, const ErrorSink *sender = nullptr) noexcept -> void;

	/// @brief Checks whether the I/O component is up
	auto connected() const -> bool
	{
		return bool(_handle);
	}

	/// @brief Returns a handle to the I/O component
	auto handle() const -> const Handle &
	{
		return _handle;
	}

	/// @name Virtual Overrides for skill::Element
	/// @{

	auto createChildElement(const skill::Element::Class &elementClass, skill::ElementFactory &factory)
		-> std::shared_ptr<skill::Element> final;

	auto forEachAttribute(const model::ForEachAttributeFunction &function) const -> bool final;
	
	auto forEachEvent(const model::ForEachEventFunction &function) -> bool final;

	auto forEachTask(const model::ForEachTaskFunction &function) -> bool final;

	auto makeReadHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle> final;

	auto category() const noexcept -> model::ElementCategory final
	{
		return model::ElementCategory::Device;
	}

	/// @}

private:
	/// @brief This structure represents the current state of the I/O component
	struct State
	{
		/// @brief The state of the I/O component
		bool _deviceState { false };
		/// @brief The last time the component was connected or disconnected
		std::chrono::system_clock::time_point _connectionTime { std::chrono::system_clock::time_point::min() };
		/// @brief The error code when connecting, or a default constructed std::error_code object for none.
		std::error_code _error { CustomError::NotConnected };
	};

	/// @brief This class providing callbacks for the Xentara scheduler for the "reconnect" task
	class ReconnectTask final : public process::Task
	{
	public:
		/// @brief This constuctor attached the task to its target
		ReconnectTask(std::reference_wrapper<TemplateIoComponent> target) : _target(target)
		{
		}

		/// @name Virtual Overrides for process::Task
		/// @{

		auto stages() const -> Stages final
		{
			return Stage::PreOperational | Stage::Operational | Stage::PostOperational;
		}

		auto preparePreOperational(const process::ExecutionContext &context) -> Status final;

		auto preOperational(const process::ExecutionContext &context) -> Status final;

		auto operational(const process::ExecutionContext &context) -> void final;

		auto preparePostOperational(const process::ExecutionContext &context) -> Status final;

		auto postOperational(const process::ExecutionContext &context) -> Status final;

		auto finishPostOperational(const process::ExecutionContext &context) -> void final;

		/// @}

	private:
		/// @brief A reference to the target element
		std::reference_wrapper<TemplateIoComponent> _target;
	};
	
	/// @brief This function is called by the "reconnect" task.
	///
	/// This function attempts to reconnect any disconnected I/O components.
	auto performReconnectTask(const process::ExecutionContext &context) -> void;

	/// @brief Attempts to establish a connection to the I/O component and updates the state accordingly.
	///
	/// This function will notify error sinks if anything changes.
	auto connect(std::chrono::system_clock::time_point timeStamp) -> void;

	/// @brief Terminates the connection to the I/O component and updates the state accordingly.
	///
	/// This function will notify error sinks if anything changes.
	auto disconnect(std::chrono::system_clock::time_point timeStamp) -> void;

	/// @brief Updates the state and sends events
	auto updateState(std::chrono::system_clock::time_point timeStamp, std::error_code error, const ErrorSink *excludeErrorSink = nullptr) -> void;

	/// @brief Checks whether an error is the result of a lost connection
	static auto isConnectionError(std::error_code error) noexcept -> bool;

	/// @name Virtual Overrides for skill::Element
	/// @{

	auto load(utils::json::decoder::Object &jsonObject, config::Context &context) -> void final;

	auto realize() -> void final;

	/// @}

	/// @brief A Xentara event that is raised when the connection is established
	process::Event _connectedEvent;
	/// @brief A Xentara event that is raised when the connection is closed or lost
	process::Event _disconnectedEvent;

	/// @brief The "reconnect" task
	ReconnectTask _reconnectTask { *this };

	/// @brief A list of objects that want to be notified of errors
	std::forward_list<std::reference_wrapper<ErrorSink>> _errorSinks;

	/// @brief The number of people who would like this component to be connected
	std::atomic<std::size_t> _connectionRequestCount { 0 };

	/// @brief A handle to the I/O component
	Handle _handle;
	/// @brief The last error we encountered.
	/// 
	/// May have the following values:
	/// - If the connection is open, this will be a default constructed std::error_code object
	/// - If the connection was closed gracefully, this will be CustomError::NotConnected;
	/// - Otherwise, this will contain an appropriate error code
	std::error_code _lastError { CustomError::NotConnected };

	/// @brief The data block that contains the state
	memory::ObjectBlock<State> _stateDataBlock;
};

inline TemplateIoComponent::ErrorSink::~ErrorSink() = default;

} // namespace xentara::plugins::templateDriver