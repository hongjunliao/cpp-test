/*!
 * This file is Part of cpp-test
 * @author: hongjun.liao<docici@126.com>
 */
#include "cpp_test.h"
#include "hp/hp_log.h"
#ifdef CPP_TEST_WITH_BOOST
#include <boost/asio.hpp>
#include <boost/asio/local/stream_protocol.hpp>

/////////////////////////////////////////////////////////////////////////////////////

int test_boost_asio_main(int argc, char **argv) {
	assert(argc >= 2);
	char const *path = argv[1];

	boost::system::error_code err;
	boost::asio::io_context io_context;
	boost::asio::io_service srv;
	boost::asio::local::stream_protocol::socket sock(io_context);
	sock.connect(boost::asio::local::stream_protocol::endpoint(path), err);
	if (err) {
		hp_log(stdout, "%s: '%s', %s\n", __FUNCTION__, path, err.message().c_str());
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////

//
// stream_client.cpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)

using boost::asio::local::stream_protocol;

enum { max_length = 1024 };

int test_boost_asio_main_1(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: stream_client <file>\n";
      return 1;
    }

    boost::asio::io_context io_context;

    stream_protocol::socket s(io_context);
    s.connect(stream_protocol::endpoint(argv[1]));

    using namespace std; // For strlen.
    std::cout << "Enter message: ";
    char request[max_length];
    std::cin.getline(request, max_length);
    size_t request_length = strlen(request);
    boost::asio::write(s, boost::asio::buffer(request, request_length));

    char reply[max_length];
    size_t reply_length = boost::asio::read(s,
        boost::asio::buffer(reply, request_length));
    std::cout << "Reply is: ";
    std::cout.write(reply, reply_length);
    std::cout << "\n";
  }
  catch (std::exception& e)
  {
    std::cerr << __FUNCTION__ << ": Exception: " << e.what() << "\n";
  }

  return 0;
}

#else // defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
# error Local sockets not available on this platform.
#endif // defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
#endif //#ifdef CPP_TEST_WITH_BOOST
