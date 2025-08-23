// Copyright (c) Maia

#include "tc/json_rpc.h"

#include <iostream>
#include <print>
#include <thread>

#include <boost/asio.hpp>

namespace tc {

// Creates a pair of anonymous pipe handles that support overlapped I/O.
void create_overlapped_pipe(HANDLE& hReadPipe, HANDLE& hWritePipe) {
  // 1. Generate a unique name for the named pipe.
  static int pipe_serial = 0;
  // This is not a real file, this is being stored in memory by the NPFS.
  std::string pipe_name = "\\\\.\\pipe\\anonymous_pipe.";
  pipe_name += std::to_string(GetCurrentProcessId());
  pipe_name += ".";
  pipe_name += std::to_string(pipe_serial++);

  // 2. Set up security attributes to allow handle inheritance if needed.
  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.bInheritHandle = FALSE;  // Set to TRUE if child processes need to inherit
  sa.lpSecurityDescriptor = NULL;

  // 3. Create the "server" end of the pipe with the overlapped flag.
  auto read_access = PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED;
  hReadPipe = CreateNamedPipeA(pipe_name.c_str(),  // Pipe name
                               read_access,  // Read access with overlapped mode
                               0,            // Pipe mode
                               1,            // Max instances
                               4096,         // Output buffer size
                               4096,         // Input buffer size
                               0,            // Default timeout
                               &sa           // Security attributes
  );

  if (hReadPipe == INVALID_HANDLE_VALUE) {
    throw std::runtime_error("CreateNamedPipeW failed for read handle.");
  }

  // 4. Create the "client" end of the pipe.
  hWritePipe =
      CreateFileA(pipe_name.c_str(),     // Pipe name
                  GENERIC_WRITE,         // Write access
                  0,                     // No sharing
                  &sa,                   // Security attributes
                  OPEN_EXISTING,         // Opens existing pipe
                  FILE_FLAG_OVERLAPPED,  // The crucial overlapped flag!
                  NULL                   // No template file
      );

  if (hWritePipe == INVALID_HANDLE_VALUE) {
    CloseHandle(hReadPipe);
    throw std::runtime_error("CreateFileW failed for write handle.");
  }
}

namespace detail {

std::string GetHeader(std::istream& istream) {
  std::string buffer(64, 0);
  istream.get(buffer.data(), buffer.size(), '\r');
  buffer.resize(istream.gcount());
  return buffer;
}

// Left trim: remove leading whitespace
void ltrim(std::string& s) {
  const auto it = std::find_if(s.begin(), s.end(), [](char c) {
    return !std::isspace(c, std::locale{});  // or use ASCII-only check
  });
  s.erase(s.begin(), it);
}

// Right trim: remove trailing whitespace
void rtrim(std::string& s) {
  const auto it = std::find_if(s.rbegin(), s.rend(), [](char c) {
    return !std::isspace(c, std::locale{});
  });
  s.erase(it.base(), s.end());
}

void trim(std::string& s) {
  ltrim(s);
  rtrim(s);
}

// [Line
// definition](https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap03.html#tag_03_206)
//
// - A sequence of zero or more non-<newline> characters plus a terminating
// <newline> character.
//
// NOTE: This function trims all the leading whitespaces and trailing spaces.
//
// Returns std::nullopt on failure (eof, fail, or shutdown requested)
std::optional<std::string> ReadNextContentLine(std::istream& input) {
  std::string line;

  // Guard: invalid stream state
  if (!input.good()) {
    return std::nullopt;
  }

  // Read next non-blank line (skipping whitespace)
  if (!std::getline(std::ws(input), line)) {
    // If we hit EOF and have a partial line, keep it
    if (!input.eof() || line.empty()) {
      return std::nullopt;
    }
    // Otherwise, we have a partial line at EOF — proceed to trim
  }

  trim(line);

  // Reject lines that are empty after trimming.
  if (line.empty()) {
    return std::nullopt;
  }

  return line;
}

// TODO: Create a new function to read the content length after parsing.
/// \see bool JSONTransport::readStandardMessage(std::string &JSON)

}  // namespace detail

class AsyncConsoleReader {
 public:
  // Your Asio logic will read from this fully asynchronous handle.
  boost::asio::windows::stream_handle readable_pipe;

  explicit AsyncConsoleReader(boost::asio::io_context& ioc)
      : readable_pipe(ioc) {
    // HANDLE read_handle;
    // HANDLE write_handle;

    // 1. Create a simple, anonymous pipe.
    // CreatePipe(&read_handle, &write_handle, NULL, 0);
    create_overlapped_pipe(read_handle_, write_handle_);

    // 2. Asio will safely read from the read-end of the pipe.
    readable_pipe.assign(read_handle_);

    // 3. Launch a dedicated thread to handle the blocking console reads.
    input_thread_ = std::jthread([this] {
      std::vector<char> buffer(4096);  // Larger buffer for efficiency

      // This thread's only job is to block here, waiting for user input.

      while (running_) {
        std::cin.read(buffer.data(), buffer.size());
        auto cnt = std::cin.gcount();
        if (cnt > 0) {
          DWORD bytes_written;
          if (!WriteFile(
                  write_handle_, buffer.data(), cnt, &bytes_written, nullptr)) {
            break;
          }
          std::print("cnt: {} - w {}\n", cnt, bytes_written);
        }
      }

      // Clean up the write handle when stdin closes.
      // Proper cleanup with error checking
      if (write_handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(write_handle_);
      }
      if (read_handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(read_handle_);
      }
    });
  }

  ~AsyncConsoleReader() {
    running_ = false;

    // Close write handle to unblock cin.read() if needed
    if (write_handle_ != INVALID_HANDLE_VALUE) {
      CloseHandle(write_handle_);
      write_handle_ = INVALID_HANDLE_VALUE;
    }
  }

 private:
  HANDLE read_handle_;
  HANDLE write_handle_;

  bool running_ = true;
  std::jthread input_thread_;
};

#ifdef BOOST_ASIO_WINDOWS
using NativeHandle = boost::asio::windows::stream_handle;
#else
using NativeHandle boost::asio::posix::stream_descriptor;
#endif

// void extracted() {
//   HANDLE new_stdin_handle =
//       CreateFileA("CONIN$",  // Special name for console input
//                   GENERIC_READ | GENERIC_WRITE,  // Must have read/write
//                   access FILE_SHARE_READ,               // Share mode NULL,
//                   // Security attributes OPEN_EXISTING,                 // It
//                   must already exist FILE_FLAG_OVERLAPPED,          // The
//                   overlapped flag! NULL                           // Template
//                   file
//       );

//   assert(new_stdin_handle != INVALID_HANDLE_VALUE);

//   if (!SetStdHandle(STD_INPUT_HANDLE, new_stdin_handle)) {
//     CloseHandle(new_stdin_handle);  // Clean up the handle we created
//   }
// }

// NativeHandle GetNativeInputHandle(boost::asio::io_context& ioc) {
// #ifdef BOOST_ASIO_WINDOWS
//   // This is necessary because the windows asio handles needs overlapped I/O.
//   // extracted();
//   return NativeHandle(ioc, GetStdHandle(STD_INPUT_HANDLE));
// #else
//   return NativeHandle(ioc, STDIN_FILENO);
// #endif
// }

struct JsonRpc::Impl {
  boost::asio::io_context ioc;
  AsyncConsoleReader cr{ioc};
  std::string buffer;
  // NativeHandle handle = GetNativeInputHandle(ioc);
  std::jthread task;
};

JsonRpc::JsonRpc()
    : impl_(std::make_unique<Impl>()) {}

JsonRpc::~JsonRpc() {
  impl_->ioc.stop();
};

void JsonRpc::Task() {
  impl_->buffer.clear();
  auto buffer = boost::asio::dynamic_buffer(impl_->buffer);

  boost::asio::async_read_until(impl_->cr.readable_pipe,
                                buffer,
                                "asd",
                                [this](std::error_code ec, int bytes_read) {
                                  auto text = std::string_view(
                                      impl_->buffer.data(), bytes_read);
                                  std::print("print: {}", text);

                                  Task();
                                });
}

void JsonRpc::Init() {
  boost::asio::executor_work_guard<boost::asio::any_io_executor> work{
      impl_->ioc.get_executor()};

  Task();

  impl_->task = std::jthread([this]() { impl_->ioc.run(); });
}

}  // namespace tc
