// Copyright (c) Maia

#include <istream>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

namespace tc {

///
/// \brief Writes a JSON message to stdout with the required length prefix.
///
/// \param message The JSON object to send.
///
void WriteMessage(const nlohmann::json& message);

///
/// \brief Writes a JSON-RPC 2.0 error response to stdout.
///
/// \param id The request ID. Can be null for certain errors like parse errors.
/// \param code The JSON-RPC error code.
/// \param message A string providing a short description of the error.
///
void WriteErrorResponse(const std::optional<nlohmann::json>& id,
                        int code,
                        const std::string& message);

///
/// \brief Reads a single length-prefixed JSON-RPC message from an input stream.
///
/// \param input The stream to read from (e.g., std::cin or std::stringstream).
/// \return An optional containing the parsed JSON object, or std::nullopt on
/// failure.
///
std::optional<nlohmann::json> ReadMessage(std::istream& input);

///
/// \brief Handles the 'initialize' request.
///
/// \param request The JSON request object.
///
void HandleInitialize(const nlohmann::json& request);

///
/// \brief Handles the 'queryFiles' request by streaming notifications.
///
/// \param request The JSON request object.
///
void HandleQueryFiles(const nlohmann::json& request);



}  // namespace tc
