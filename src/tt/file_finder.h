// Copyright (c) Maia

#pragma once

#include <string>
#include <vector>

#include <boost/asio/io_context.hpp>

namespace tt {

class IFileFinder {
 public:
  virtual ~IFileFinder() = default;

  virtual std::vector<std::string> GetFiles() = 0;
  virtual std::vector<std::string> GetFilesFromDir(std::string root_dir) = 0;
};

class FdFileFinder : public IFileFinder {
 public:
  std::vector<std::string> GetFiles() override;

  std::vector<std::string> GetFilesFromDir(std::string root_dir) override;

 private:
  boost::asio::io_context io_context_;
};

}  // namespace tt
