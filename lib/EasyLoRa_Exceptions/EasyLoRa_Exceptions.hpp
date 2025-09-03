#ifndef EASY_LORA_EXCEPTIONS_HEADER
#define EASY_LORA_EXCEPTIONS_HEADER

#include <exception>
#include <string>
#include <vector>
#include <cstdint>

class ResponseDontReceived : public std::exception {
public:

    [[nodiscard]]
    const char* what() const noexcept override {
        return "Response don't received";
    }
};

class AbnormalResponse : public std::exception {
public:
    explicit AbnormalResponse(const std::vector<uint8_t>& response)
    : errorMessage_m{ std::string{ "Abnormal response: " } + std::string(response.cbegin(), response.cend()) }
    {}

    [[nodiscard]]
    const char* what() const noexcept override {
        return errorMessage_m.data();
    }

private:
    std::string errorMessage_m;
};

class AbnormalRegister : public std::exception {
public:
    AbnormalRegister(std::string_view registerName, uint8_t value) 
    : errorMessage_m{ std::string("Abnormal register: ") + registerName.data() + " with value: " + std::to_string(value) }
    {}

    [[nodiscard]]
    const char* what() const noexcept override {
        return errorMessage_m.data();
    }

private:
    std::string errorMessage_m;

};

class DecodificationError : public std::exception {
public:
    [[nodiscard]]
    const char* what() const noexcept override {
        return "Unable to decode the data";
    }
};

class EncodificationError : public std::exception {
public:
    [[nodiscard]]
    const char* what() const noexcept override {
        return "Unable to encode the data";
    }
};

#endif // !EASY_LORA_EXCEPTIONS_HEADER