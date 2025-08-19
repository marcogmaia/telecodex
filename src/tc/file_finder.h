// Copyright (c) Maia

#pragma once

#include <string>
#include <vector>

#include <boost/asio/io_context.hpp>

namespace tc {

class IFileFinder {
 public:
  virtual ~IFileFinder() = default;

  virtual std::vector<std::string> GetFiles() = 0;
};

class FdFileFinder : public IFileFinder {
 public:
  std::vector<std::string> GetFiles() override;

 private:
  boost::asio::io_context io_context_;
};

}  // namespace tc
