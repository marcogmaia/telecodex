// Copyright (c) Maia

#pragma once

#include <memory>

namespace tc {

class JsonRpc {
 public:
 
  void Init();

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace tc
