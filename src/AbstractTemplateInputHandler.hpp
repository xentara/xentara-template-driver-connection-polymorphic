// Copyright (c) embedded ocean GmbH
#pragma once

#include <xentara/data/DataType.hpp>
#include <xentara/data/ReadHandle.hpp>
#include <xentara/model/Attribute.hpp>
#include <xentara/process/Event.hpp>

#include <string_view>
#include <memory>
#include <chrono>
#include <optional>

namespace xentara::plugins::templateDriver
{

using namespace std::literals;

// Base class for data type specific functionality for TemplateInput.
/// @todo rename this class to something more descriptive
class AbstractTemplateInputHandler
{
public:
	/// @brief Interface for objects that want to be notified of read errors
	class ErrorSink
	{
	public:
		/// @brief Virtual destructor
		/// @note The destructor is pure virtual (= 0) to ensure that this class will remain abstract, even if we should remove all
		/// other pure virtual functions later. This is not necessary, of course, but prefents the abstract class from becoming
		/// instantiable by accident as a result of refactoring.
		virtual ~ErrorSink() = 0;

		/// @brief Called when a read error occurs
		virtual auto handleReadError(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void = 0;
	};

	/// @brief Virtual destructor
	/// @note The destructor is pure virtual (= 0) to ensure that this class will remain abstract, even if we should remove all
	/// other pure virtual functions later. This is not necessary, of course, but prefents the abstract class from becoming
	/// instantiable by accident as a result of refactoring.
	virtual ~AbstractTemplateInputHandler() = 0;

	/// @brief Returns the data type
	virtual auto dataType() const -> const data::DataType & = 0;

	/// @brief Resolves an attribute that belong to this state.
	/// @param name The name of the attribute to resolve
	/// @return The attribute, or nullptr if we don't have an attribute with this name
	virtual auto resolveAttribute(std::string_view name) -> const model::Attribute * = 0;

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
	virtual auto resolveEvent(std::string_view name, std::shared_ptr<void> parent) -> std::shared_ptr<process::Event> = 0;

	/// @brief Creates a read-handle for an attribute that belong to this state.
	/// @param attribute The attribute to create the handle for
	/// @return A read handle for the attribute, or std::nullopt if the attribute is unknown
	virtual auto readHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle> = 0;

	/// @brief Realizes the handler
	virtual auto realize() -> void = 0;
		
	/// @brief Attempts to read the data from the I/O component and updates the handler accordingly.
	virtual auto read(std::chrono::system_clock::time_point timeStamp, ErrorSink &errorSink) -> void = 0;
	/// @brief Updates the state without specifying a value
	virtual auto updateState(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void = 0;
};

inline AbstractTemplateInputHandler::~AbstractTemplateInputHandler() = default;

inline AbstractTemplateInputHandler::ErrorSink::~ErrorSink() = default;

} // namespace xentara::plugins::templateDriver