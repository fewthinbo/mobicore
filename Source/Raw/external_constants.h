#pragma once
//#ifdef PLATFORM_FREEBSD
//#include <signal.h>
//#include <sys/types.h>
//#include <sys/socket.h>
//#endif

#include <boost/asio.hpp>
//#include <boost/asio/ip/tcp.hpp>
//#include <boost/system/error_code.hpp>

namespace network {
	using tcp = boost::asio::ip::tcp;
	using error_code = boost::system::error_code;
	using socket = tcp::socket;
	using io_context = boost::asio::io_context;
}