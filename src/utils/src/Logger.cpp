#include "Logger.h"

#include <boost/log/core.hpp>
#include <boost/log/attributes/clock.hpp>
#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <iostream>
#include <iomanip>
#include <string_view>

// Global logger definitions
boost::log::sources::severity_logger<int> verbose(0); // Dominating output
boost::log::sources::severity_logger<int> debug(1);   // Follow what is happening
boost::log::sources::severity_logger<int> info(2);    // Should be informed about
boost::log::sources::severity_logger<int> warning(3); // Strange events
boost::log::sources::severity_logger<int> error(4);   // Recoverable errors
boost::log::sources::severity_logger<int> fatal(5);   // Unrecoverable errors

// Define attribute keywords
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", int)
BOOST_LOG_ATTRIBUTE_KEYWORD(thread_id, "ThreadID", boost::log::attributes::current_thread_id::value_type)

// Synchronous sink type (prevents log loss on exit and keeps order)
using text_sink = boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>;

// Deleter that does nothing (for std::cout shared_ptr)
struct NoDelete {
    void operator()(void*) {}
};

void init_log() {

    boost::shared_ptr<text_sink> sink = boost::make_shared<text_sink>();

    // Add std::cout to the sink backend
    boost::shared_ptr<std::ostream> stream{&std::cout, NoDelete{}};
    sink->locked_backend()->add_stream(stream);

    // Set filter (process all severities >= 0)
    sink->set_filter(severity >= 0);

    // Set custom formatter
    sink->set_formatter([/*message = "Message", severity = "Severity"*/](const boost::log::record_view &view, boost::log::formatting_ostream &os) {
        // Retrieve severity (int)
        auto log_level_val = view.attribute_values()["Severity"].extract<int>();
        int log_level = log_level_val ? log_level_val.get() : 0;

        std::string_view log_type;
        switch (log_level) {
            case 0: log_type = "verbose"; break;
            case 1: log_type = "debug";   break;
            case 2: log_type = "info";    break;
            case 3: log_type = "warning"; break;
            case 4: log_type = "error";   break;
            case 5: log_type = "fatal";   break;
            default: log_type = "unknown"; break;
        }

        // Get current time
        auto now = boost::posix_time::microsec_clock::local_time();
        
        // Format time with high precision
        std::ostringstream oss;
        boost::posix_time::time_facet* facet = new boost::posix_time::time_facet("%Y-%m-%d %H:%M:%S.%f");
        oss.imbue(std::locale(std::locale::classic(), facet));
        oss << now;

        // Retrieve message
        auto msg_val = view.attribute_values()["Message"].extract<std::string>();
        std::string msg_str = msg_val ? msg_val.get() : "";

        // Output formatted log
        os << "[" << oss.str() << "] [" << log_type << "] " << msg_str;
    });

    boost::log::core::get()->add_sink(sink);
    
    // Add common attributes like LineID, TimeStamp, ProcessID, ThreadID
    boost::log::add_common_attributes();
}

// Automatically call init_log() when the program starts
struct LogInitializer {
    LogInitializer() {
        init_log();
    }
};

static LogInitializer log_init;
