#include <Windows.h>

#include <iostream>
#include <thread>

#include <boost/asio.hpp>

#include "tc/json_rpc.h"

int main() {
  tc::JsonRpc rpc{};
  rpc.Init();
  std::this_thread::sleep_for(std::chrono::milliseconds(10000));
  return 0;
}
