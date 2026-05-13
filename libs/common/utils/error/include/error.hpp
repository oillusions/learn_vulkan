#pragma once

#include <expected>
#include <format>
#include <memory>
#include <optional>
#include <source_location>
#include <string>

namespace detail {
struct ErrorRecord {
  using DetailType = std::string;
  std::string message;
  std::optional<DetailType> detail;
  std::source_location location;

  ErrorRecord(std::string message, std::optional<DetailType> detail,
              const std::source_location location) noexcept
      : message(std::move(message)), detail(std::move(detail)),
        location(location) {};

  bool has_detail() const { return detail.has_value(); }
};
} // namespace detail

class Error : public detail::ErrorRecord {
public:
  std::shared_ptr<const Error> cause;

  explicit Error(std::string message,
                 const std::source_location location =
                     std::source_location::current()) noexcept
      : ErrorRecord(std::move(message), std::nullopt, location),
        cause(nullptr) {};

  explicit Error(std::string message, DetailType detail,
                 const std::source_location location =
                     std::source_location::current()) noexcept
      : ErrorRecord(std::move(message), std::move(detail), location),
        cause(nullptr) {};

private:
  explicit Error(std::string message, std::optional<DetailType> detail,
                 const std::source_location location,
                 std::shared_ptr<const Error> cause) noexcept
      : ErrorRecord(std::move(message), std::move(detail), location),
        cause(std::move(cause)) {};

  explicit Error(ErrorRecord record,
                 std::shared_ptr<const Error> cause) noexcept
      : ErrorRecord(std::move(record)), cause(std::move(cause)) {};

  Error forward(this auto &&self, ErrorRecord record) noexcept {
    return Error{std::move(record), std::make_shared<const Error>(
                                        std::forward<decltype(self)>(self))};
  }

public:
  class FromFunctor : detail::ErrorRecord {
  public:
    FromFunctor(std::string message,
                const std::source_location location) noexcept
        : detail::ErrorRecord(std::move(message), std::nullopt, location) {};

    template <typename E> Error operator()(const E &e) noexcept;
  };

public:
  [[nodiscard]]
  Error forward(this auto &&self, std::string message,
                const std::source_location location =
                    std::source_location::current()) noexcept {
    return self.forward(
        ErrorRecord(std::move(message), std::nullopt, location));
  }
  [[nodiscard]]
  Error forward(this auto &&self, std::string message, DetailType detail,
                const std::source_location location =
                    std::source_location::current()) noexcept {
    return self.forward(
        ErrorRecord(std::move(message), std::move(detail), location));
  }

  static FromFunctor from(std::string message,
                          const std::source_location location =
                              std::source_location::current()) noexcept {
    return FromFunctor(std::move(message), location);
  }

private:
  class UnwrapFunctor : detail::ErrorRecord {
  public:
    UnwrapFunctor(std::string message,
                  const std::source_location location) noexcept
        : detail::ErrorRecord(std::move(message), std::nullopt, location) {};

    UnwrapFunctor(std::string message, DetailType detail,
                  const std::source_location location) noexcept
        : ErrorRecord(std::move(message), std::move(detail), location) {};

    template <typename T>
    [[nodiscard]]
    friend T operator|(std::expected<T, Error> expected,
                       const UnwrapFunctor &functor) {
      if (!expected) {
        throw std::move(expected.error()).forward(std::move(functor));
      }
      return std::move(expected).value();
    }
    friend void operator|(std::expected<void, Error> expected,
                          const UnwrapFunctor &functor) {
      if (!expected) {
        throw std::move(expected.error()).forward(std::move(functor));
      }
    }
  };

public:
  [[nodiscard]]
  static UnwrapFunctor unwrap(std::string message = "",
                              std::source_location location =
                                  std::source_location::current()) noexcept {
    return {message, location};
  }
  [[nodiscard]]
  static UnwrapFunctor unwrap(std::string message, DetailType detail,
                              std::source_location location =
                                  std::source_location::current()) noexcept {
    return {message, detail, location};
  }

private:
  static std::string tab_chain(const Error &error) {
    return error.cause->cause != nullptr ? "\n┠─" : "\n┖─";
  }

  static std::string tab_detail(const Error &error) {
    return error.cause != nullptr ? "\n┃ ┖─" : "\n  ┖─";
  }

  static std::string recursionChain(const Error &error) {
    if (error.cause) {
      if (error.has_detail()) {
        return format("[{}:{}]: {}{}{}", error.location.file_name(),
                      error.location.line(), error.message, tab_detail(error),
                      error.detail.value()) +
               tab_chain(error) + recursionChain(*error.cause);
      }
      return format("[{}:{}]: {}", error.location.file_name(),
                    error.location.line(), error.message) +
             tab_chain(error) + recursionChain(*error.cause);
    }

    if (error.has_detail()) {
      return format("[{}:{}]: {}{}{}", error.location.file_name(),
                    error.location.line(), error.message, tab_detail(error),
                    error.detail.value());
    }
    return format("[{}:{}]: {}", error.location.file_name(),
                  error.location.line(), error.message);
  }

public:
  [[nodiscard]]
  std::string string() const {
    return ": \n" + recursionChain(*this);
  }

  operator std::unexpected<Error>() const noexcept {
    return std::unexpected{*this};
  }

  template <typename T> operator std::expected<T, Error>() const noexcept {
    return std::unexpected{*this};
  }
};