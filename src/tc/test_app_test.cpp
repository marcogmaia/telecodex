// Copyright (c) Maia

#include <gtest/gtest.h>

#include <lsp/connection.h>
#include <lsp/io/stream.h>
#include <lsp/messagehandler.h>
#include <lsp/messages.h>
#include <lsp/process.h>
#include <boost/process.hpp>

namespace tc {

namespace {

class VectorStream : public lsp::io::Stream {
 public:
  VectorStream() = default;

  explicit VectorStream(const std::vector<char>& initialData)
      : buffer_(initialData) {}

  explicit VectorStream(std::vector<char>&& initialData)
      : buffer_(std::move(initialData)) {}

  void read(char* destination, std::size_t size) override {
    if (!destination) {
      throw std::invalid_argument("Destination buffer cannot be null.");
    }

    const std::size_t available = buffer_.size() - read_position_;
    const std::size_t bytes_to_read = std::min(size, available);

    if (bytes_to_read > 0) {
      std::copy(buffer_.begin() + read_position_,
                buffer_.begin() + read_position_ + bytes_to_read,
                destination);
    }

    if (size > bytes_to_read) {
      std::fill(destination + bytes_to_read, destination + size, Stream::Eof);
    }

    read_position_ += bytes_to_read;
  }

  void write(const char* source, std::size_t size) override {
    if (!source) {
      throw std::invalid_argument("Source buffer cannot be null.");
    }
    buffer_.insert(buffer_.end(), source, source + size);
  }

  const std::vector<char>& getBuffer() const {
    return buffer_;
  }

 private:
  std::vector<char> buffer_;
  std::size_t read_position_{0};
};

}  // namespace

TEST(TestApp, Test) {
  auto stream = VectorStream();
  auto connection = lsp::Connection(stream);

  auto message_handler = lsp::MessageHandler(connection);

  
}

}  // namespace tc
