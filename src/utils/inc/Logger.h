#pragma once

#include <boost/log/common.hpp>
#include <boost/log/sources/severity_logger.hpp>

// Extern declared global logger instances
extern boost::log::sources::severity_logger<int> verbose;
extern boost::log::sources::severity_logger<int> debug;
extern boost::log::sources::severity_logger<int> info;
extern boost::log::sources::severity_logger<int> warning;
extern boost::log::sources::severity_logger<int> error;
extern boost::log::sources::severity_logger<int> fatal;

// Initialization function
void init_log();