#pragma once
#include <cstdint>
class DeviceUID {
public:
    static uint32_t get() {
        static uint32_t value = []{
            uint32_t* uid_address = reinterpret_cast<uint32_t*>(0x1FFF7A10);
            return *uid_address;
        }();
        return value;
    }

private:
    DeviceUID() = delete;
    ~DeviceUID() = delete;
};
