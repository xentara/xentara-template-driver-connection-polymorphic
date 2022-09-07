// Copyright (c) embedded ocean GmbH
#include "TemplateInput.hpp"

#include "AbstractTemplateInputHandler.hpp"
#include "TemplateInputHandler.hpp"

#include <xentara/data/ReadHandle.hpp>
#include <xentara/process/ExecutionContext.hpp>
#include <xentara/utils/json/decoder/Object.hpp>
#include <xentara/utils/json/decoder/Errors.hpp>

namespace xentara::plugins::templateDriver
{
	
using namespace std::literals;

TemplateInput::Class TemplateInput::Class::_instance;

auto TemplateInput::loadConfig(const ConfigIntializer &initializer,
		utils::json::decoder::Object &jsonObject,
		config::Resolver &resolver,
		const FallbackConfigHandler &fallbackHandler) -> void
{
	// Get a reference that allows us to modify our own config attributes
    auto &&configAttributes = initializer[Class::instance().configHandle()];

	// Go through all the members of the JSON object that represents this object
	for (auto && [name, value] : jsonObject)
    {
		if (name == u8"dataType"sv)
		{
			// Create the handler
			_handler = createHandler(value);
		}
		/// @todo load custom configuration parameters
		else if (name == u8"TODO"sv)
		{
			/// @todo parse the value correctly
			auto todo = value.asNumber<std::uint64_t>();

			/// @todo check that the value is valid
			if (!"TODO")
			{
				/// @todo use an error message that tells the user exactly what is wrong
				utils::json::decoder::throwWithLocation(value, std::runtime_error("TODO is wrong with TODO parameter of template input"));
			}

			/// @todo set the appropriate member variables, and update configAttributes accordingly (if necessary) 
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
		/// @todo replace "template input" with a more descriptive name
		utils::json::decoder::throwWithLocation(jsonObject, std::runtime_error("Missing data type in template input"));
	}
	/// @todo perform consistency and completeness checks
	if (!"TODO")
	{
		/// @todo use an error message that tells the user exactly what is wrong
		utils::json::decoder::throwWithLocation(jsonObject, std::runtime_error("TODO is wrong with template input"));
	}
}

auto TemplateInput::createHandler(utils::json::decoder::Value &value) -> std::unique_ptr<AbstractTemplateInputHandler>
{
	// Get the keyword from the value
	auto keyword = value.asString<std::u8string>();
	
	/// @todo use keywords that are appropriate to the I/O component
	if (keyword == u8"bool"sv)
	{
		return std::make_unique<TemplateInputHandler<bool>>();
	}
	else if (keyword == u8"uint8"sv)
	{
		return std::make_unique<TemplateInputHandler<std::uint8_t>>();
	}
	else if (keyword == u8"uint16"sv)
	{
		return std::make_unique<TemplateInputHandler<std::uint16_t>>();
	}
	else if (keyword == u8"uint32"sv)
	{
		return std::make_unique<TemplateInputHandler<std::uint32_t>>();
	}
	else if (keyword == u8"uint64"sv)
	{
		return std::make_unique<TemplateInputHandler<std::uint64_t>>();
	}
	else if (keyword == u8"int8"sv)
	{
		return std::make_unique<TemplateInputHandler<std::int8_t>>();
	}
	else if (keyword == u8"int16"sv)
	{
		return std::make_unique<TemplateInputHandler<std::int16_t>>();
	}
	else if (keyword == u8"int32"sv)
	{
		return std::make_unique<TemplateInputHandler<std::int32_t>>();
	}
	else if (keyword == u8"int64"sv)
	{
		return std::make_unique<TemplateInputHandler<std::int64_t>>();
	}
	else if (keyword == u8"float32"sv)
	{
		return std::make_unique<TemplateInputHandler<float>>();
	}
	else if (keyword == u8"float64"sv)
	{
		return std::make_unique<TemplateInputHandler<double>>();
	}

	// The keyword is not known
	else
	{
		/// @todo replace "template input" with a more descriptive name
		utils::json::decoder::throwWithLocation(value, std::runtime_error("unknown data type in template input"));
	}

	return std::unique_ptr<AbstractTemplateInputHandler>();
}

auto TemplateInput::performReadTask(const process::ExecutionContext &context) -> void
{
	// tasks must not be executed before the configuration was loaded, so the handler should have been
	// created already.
	if (!_handler)
	{
		throw std::logic_error("internal error: \"read\" task of xentara::plugins::templateDriver::TemplateInput executed before configuration has been loaded");
	}

	// Only perform the reconnect if we are supposed to be connected in the first place
	if (!_ioComponent.get().connected())
	{
		return;
	}

	// Ask the handler to read the data
	_handler->read(context.scheduledTime(), *this);
}

auto TemplateInput::dataType() const -> const data::DataType &
{
	// dataType() must not be called before the configuration was loaded, so the handler should have been
	// created already.
	if (!_handler)
	{
		throw std::logic_error("internal error: xentara::plugins::templateDriver::TemplateInput::dataType() called before configuration has been loaded");
	}

	// Forward the request to the handler
	return _handler->dataType();
}

auto TemplateInput::directions() const -> io::Directions
{
	return io::Direction::Input;
}

auto TemplateInput::resolveAttribute(std::u16string_view name) -> const model::Attribute *
{
	// resolveAttribute() must not be called before the configuration was loaded, so the handler should have been
	// created already.
	if (!_handler)
	{
		throw std::logic_error("internal error: xentara::plugins::templateDriver::TemplateInput::resolveAttribute() called before configuration has been loaded");
	}

	// Check the handler attributes
	if (auto attribute = _handler->resolveAttribute(name))
	{
		return attribute;
	}

	/// @todo add any additional attributes this class supports, including attributes inherited from the I/O component

	return nullptr;
}

auto TemplateInput::resolveTask(std::u16string_view name) -> std::shared_ptr<process::Task>
{
	if (name == u"read"sv)
	{
		return std::shared_ptr<process::Task>(sharedFromThis(), &_readTask);
	}

	/// @todo add any additional tasks this class supports

	return nullptr;
}

auto TemplateInput::resolveEvent(std::u16string_view name) -> std::shared_ptr<process::Event>
{
	// resolveAttribute() must not be called before the configuration was loaded, so the handler should have been
	// created already.
	if (!_handler)
	{
		throw std::logic_error("internal error: xentara::plugins::templateDriver::TemplateInput::resolveEvent() called before configuration has been loaded");
	}

	// Check the handler events
	if (auto event = _handler->resolveEvent(name, sharedFromThis()))
	{
		return event;
	}

	/// @todo add any additional events this class supports, including events inherited from the I/O component

	return nullptr;
}

auto TemplateInput::readHandle(const model::Attribute &attribute) const noexcept -> data::ReadHandle
{
	// readHandle() must not be called before the configuration was loaded, so the handler should have been
	// created already.
	if (!_handler)
	{
		// Don't throw an exception, because this function is noexcept
		return std::make_error_code(std::errc::invalid_argument);
	}

	// Check the handler attributes
	if (auto handle = _handler->readHandle(attribute))
	{
		return *handle;
	}

	/// @todo add any additional readable attributes this class supports, including attributes inherited from the I/O component

	return data::ReadHandle::Error::Unknown;
}

auto TemplateInput::realize() -> void
{
	// realize() must not be called before the configuration was loaded, so the handler should have been
	// created already.
	if (!_handler)
	{
		throw std::logic_error("internal error: xentara::plugins::templateDriver::TemplateInput::realize() called before configuration has been loaded");
	}

	// Realize the handler
	_handler->realize();
}

auto TemplateInput::ioComponentStateChanged(std::chrono::system_clock::time_point timeStamp, std::error_code error) -> void
{
	// ioComponentStateChanged() must not be called before the configuration was loaded, so the handler should have been
	// created already.
	if (!_handler)
	{
		throw std::logic_error("internal error: xentara::plugins::templateDriver::TemplateInput::ioComponentStateChanged() called before configuration has been loaded");
	}

	// We cannot reset the error to Ok because we don't have a value. So we use the special custom error code instead.
	auto effectiveError = error ? error : CustomError::NoData;

	// Ask the handler to update its state. We do not notify the I/O component, because that is who this message comes from in the first place.
	_handler->updateState(timeStamp, effectiveError);
}

auto TemplateInput::handleReadError(std::chrono::system_clock::time_point timeStamp, std::error_code error)
	-> void
{
	// Just notify the I/O component. The handler will have updated its state already, before calling this function.
	_ioComponent.get().handleError(timeStamp, error, this);
}

} // namespace xentara::plugins::templateDriver