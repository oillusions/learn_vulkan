#include "error.hpp"

template <>
Error Error::FromFunctor::operator()(
    const std::string &description
) noexcept {
    return Error(
        this->message, 
        description, 
        this->location
    );
}
