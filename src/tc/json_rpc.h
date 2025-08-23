// Copyright (c) Maia

#pragma once

#include <memory>

namespace tc {

class JsonRpc {
 public:
  JsonRpc();
  ~JsonRpc();

  void Init();

  void Task();

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace tc
