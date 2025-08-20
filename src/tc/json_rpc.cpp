// Copyright (c) Maia

#include "tc/json_rpc.h"

#include <boost/asio.hpp>

namespace tc {

auto GetNativeInputHandle(boost::asio::io_context& ioc) {
#ifdef BOOST_ASIO_WINDOWS
  return boost::asio::windows::stream_handle(ioc,
                                             GetStdHandle(STD_INPUT_HANDLE));
#else
  return boost::asio::posix::stream_descriptor(ioc, STDIN_FILENO);
#endif
}

void JsonRpc::Init() {}

}  // namespace tc
