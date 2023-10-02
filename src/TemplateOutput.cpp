// Copyright (c) embedded ocean GmbH
#include "TemplateOutput.hpp"

#include "AbstractTemplateOutputHandler.hpp"
#include "Tasks.hpp"
#include "TemplateOutputHandler.hpp"

#include <xentara/config/FallbackHandler.hpp>
#include <xentara/data/ReadHandle.hpp>
#include <xentara/data/WriteHandle.hpp>
#include <xentara/model/Attribute.hpp>
#include <xentara/model/ForEachAttributeFunction.hpp>
#include <xentara/model/ForEachEventFunction.hpp>
#include <xentara/model/ForEachTaskFunction.hpp>
#include <xentara/utils/json/decoder/Object.hpp>
#include <xentara/utils/json/decoder/Errors.hpp>

namespace xentara::plugins::templateDriver
{
	
using namespace std::literals;

auto TemplateOutput::load(utils::json::decoder::Object &jsonObject,
	config::Resolver &resolver,
	const config::FallbackHandler &fallbackHandler) -> void
{
	// Go through all the members of the JSON object that represents this object
	for (auto && [name, value] : jsonObject)
    {
		if (name == "dataType"sv)
		{
			// Create the handler
			_handler = createHandler(value);
		}
		/// @todo load custom configuration parameters
		else if (name == "TODO"sv)
		{
			/// @todo parse the value correctly
			auto todo = value.asNumber<std::uint64_t>();

			/// @todo check that the value is valid
			if (!"TODO")
			{
				/// @todo use an error message that tells the user exactly what is wrong
				utils::json::decoder::throwWithLocation(value, std::runtime_error("TODO is wrong with TODO parameter of template output"));
			}

			/// @todo set the appropriate member variables
		}
		else
		{
			// Pass any unknown parameters on to the fallback handler, which will load the built-in parameters ("id" and "uuid"),
			// and throw an exception if the key is unknown
            fallbackHandler(name, value);
		}
    }

	// Make sure that a data type was specified
	if (!_handler)
	{
		/// @todo replace "template output" with a more descriptive name
		utils::json::decoder::throwWithLocation(jsonObject, std::runtime_error("Missing data type in template output"));
	}
	/// @todo perform consistency and completeness checks
	if (!"TODO")
	{
		/// @todo use an error message that tells the user exactly what is wrong
		utils::json::decoder::throwWithLocation(jsonObject, std::runtime_error("TODO is wrong with template output"));
	}
}

auto TemplateOutput::createHandler(utils::json::decoder::Value &value) -> std::unique_ptr<AbstractTemplateOutputHandler>
{
	// Get the keyword from the value
	auto keyword = value.asString<std::string>();
	
	/// @todo use keywords that are appropriate to the I/O component
	if (keyword == "bool"sv)
	{
		return std::make_unique<TemplateOutputHandler<bool>>();
	}
	else if (keyword == "uint8"sv)
	{
		return std::make_unique<TemplateOutputHandler<std::uint8_t>>();
	}
	else if (keyword == "uint16"sv)
	{
		return std::make_unique<TemplateOutputHandler<std::uint16_t>>();
	}
	else if (keyword == "uint32"sv)
	{
		return std::make_unique<TemplateOutputHandler<std::uint32_t>>();
	}
	else if (keyword == "uint64"sv)
	{
		return std::make_unique<TemplateOutputHandler<std::uint64_t>>();
	}
	else if (keyword == "int8"sv)
	{
		return std::make_unique<TemplateOutputHandler<std::int8_t>>();
	}
	else if (keyword == "int16"sv)
	{
		return std::make_unique<TemplateOutputHandler<std::int16_t>>();
	}
	else if (keyword == "int32"sv)
	{
		return std::make_unique<TemplateOutputHandler<std::int32_t>>();
	}
	else if (keyword == "int64"sv)
	{
		return std::make_unique<TemplateOutputHandler<std::int64_t>>();
	}
	else if (keyword == "float32"sv)
	{
		return std::make_unique<TemplateOutputHandler<float>>();
	}
	else if (keyword == "float64"sv)
	{
		return std::make_unique<TemplateOutputHandler<double>>();
	}
	else if (keyword == "string"sv)
	{
		return std::make_unique<TemplateOutputHandler<std::string>>();
	}

	// The keyword is not known
	else
	{
		/// @todo replace "template output" with a more descriptive name
		utils::json::decoder::throwWithLocation(value, std::runtime_error("unknown data type in template output"));
	}

	return std::unique_ptr<AbstractTemplateOutputHandler>();
}

auto TemplateOutput::performReadTask(const process::ExecutionContext &context) -> void
{
	// tasks must not be executed before the configuration was loaded, so the handler should have been
	// created already.
	if (!_handler) [[unlikely]]
	{
		throw std::logic_error("internal error: \"read\" task of xentara::plugins::templateDriver::TemplateOutput executed before configuration has been loaded");
	}

	// Only perform the read only if the I/O component is connected
	if (!_ioComponent.get().connected())
	{
		return;
	}

	// Ask the handler to read the data
	_handler->read(context.scheduledTime(), *this);
}

auto TemplateOutput::performWriteTask(const process::ExecutionContext &context) -> void
{
	// tasks must not be executed before the configuration was loaded, so the handler should have been
	// created already.
	if (!_handler) [[unlikely]]
	{
		throw std::logic_error("internal error: \"write\" task of xentara::plugins::templateDriver::TemplateOutput executed before configuration has been loaded");
	}

	// Only perform the read only if the I/O component is connected
	if (!_ioComponent.get().connected())
	{
		return;
	}

	// Ask the handler to write the data
	_handler->write(context.scheduledTime(), *this);
}

auto TemplateOutput::dataType() const -> const data::DataType &
{
	// dataType() must not be called before the configuration was loaded, so the handler should have been
	// created already.
	if (!_handler) [[unlikely]]
	{
		throw std::logic_error("internal error: xentara::plugins::templateDriver::TemplateOutput::dataType() called before configuration has been loaded");
	}

	// Forward the request to the handler
	return _handler->dataType();
}

auto TemplateOutput::directions() const -> io::Directions
{
	return io::Direction::Input | io::Direction::Output;
}

auto TemplateOutput::forEachAttribute(const model::ForEachAttributeFunction &function) const -> bool
{
	// forEachAttribute() must not be called before the configuration was loaded, so the handler should have been
	// created already.
	if (!_handler) [[unlikely]]
	{
		throw std::logic_error("internal error: xentara::plugins::templateDriver::TemplateOutput::forEachAttribute() called before configuration has been loaded");
	}

	return
		// Handle the handler attributes
		_handler->forEachAttribute(function);

	/// @todo handle any additional attributes this class supports, including attributes inherited from the I/O component
}

auto TemplateOutput::forEachEvent(const model::ForEachEventFunction &function) -> bool
{
	// forEachAttribute() must not be called before the configuration was loaded, so the handler should have been
	// created already.
	if (!_handler) [[unlikely]]
	{
		throw std::logic_error("internal error: xentara::plugins::templateDriver::TemplateOutput::forEachEvent() called before configuration has been loaded");
	}

	return
		// Handle the handler events
		_handler->forEachEvent(function, sharedFromThis());

	/// @todo handle any additional events this class supports, including events inherited from the I/O component
}

auto TemplateOutput::forEachTask(const model::ForEachTaskFunction &function) -> bool
{
	// Handle all the tasks we support
	return
		function(tasks::kRead, sharedFromThis(&_readTask)) ||
		function(tasks::kWrite, sharedFromThis(&_writeTask));

	/// @todo handle any additional tasks this class supports
}

auto TemplateOutput::makeReadHandle(const model::Attribute &attribute) const noexcept -> std::optional<data::ReadHandle>
{
	// makeReadHandle() must not be called before the configuration was loaded, so the handler should have been
	// created already.
	if (!_handler) [[unlikely]]
	{
		// Don't throw an exception, because this function is noexcept
		return std::make_error_code(std::errc::invalid_argument);
	}

	// Handle the handler attributes
	if (auto handle = _handler->makeReadHandle(attribute))
	{
		return handle;
	}

	/// @todo handle any additional readable attributes this class supports, including attributes inherited from the I/O component

	return std::nullopt;
}

auto TemplateOutput::makeWriteHandle(const model::Attribute &attribute) noexcept -> std::optional<data::WriteHandle>
{
	// makeWriteHandle() must not be called before the configuration was loaded, so the handler should have been
	// created already.
	if (!_handler) [[unlikely]]
	{
		// Don't throw an exception, because this function is noexcept
		return std::make_error_code(std::errc::invalid_argument);
	}

	// Handle the handler attributes
	if (auto handle = _handler->makeWriteHandle(attribute, sharedFromThis()))
	{
		return handle;
	}

	/// @todo handle any additional writable attributes this class supports, including attributes inherited from the I/O component

	return std::nullopt;
}

auto TemplateOutput::realize() -> void
{
	// realize() must not be called before the configuration was loaded, so the handler should have been
	// created already.
	if (!_handler) [[unlikely]]
	{
		throw std::logic_error("internal error: xentara::plugins::templateDriver::TemplateOutput::realize() called before configuration has been loaded");
	}

	// Realize the handler
	_handler->realize();
}

auto TemplateOutput::ioComponentStateChanged(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void
{
	// ioComponentStateChanged() must not be called before the configuration was loaded, so the handler should have been
	// created already.
	if (!_handler) [[unlikely]]
	{
		throw std::logic_error("internal error: xentara::plugins::templateDriver::TemplateOutput::ioComponentStateChanged() called before configuration has been loaded");
	}

	// We cannot reset the error to Ok because we don't have a value. So we use the special custom error code instead.
	auto effectiveError = error ? error : CustomError::NoData;

	// Ask the handler to update its read state. We do not notify the I/O component, because that is who this message comes from in the first place.
	// Note: the write state is not updated, because the write state simply contains the last write error, which is unaffected
	// by I/O component errors.
	_handler->updateReadState(timeStamp, effectiveError);
}

auto TemplateOutput::handleReadError(std::chrono::system_clock::time_point timeStamp, std::error_code error)
	-> void
{
	// Just notify the I/O component. The handler will have updated its state already, before calling this function.
	_ioComponent.get().handleError(timeStamp, error, this);
}

auto TemplateOutput::handleWriteError(std::chrono::system_clock::time_point timeStamp, std::error_code error)
	-> void
{
	// Just notify the I/O component. The handler will have updated its state already, before calling this function.
	_ioComponent.get().handleError(timeStamp, error, this);
}

} // namespace xentara::plugins::templateDriver