#pragma once
#include <array>
#include <cstdint>

class UIDReader {
   public:
    // 存储 UID 数据的成员变量
    uint64_t value;    // 直接使用 uint32_t 存储 UID
    // 获取单例实例
    static UIDReader& getInstance() {
        static UIDReader instance;    // 静态实例，第一次调用时创建
        return instance;
    }

   private:
    // 私有构造函数，确保只能通过 `getInstance()` 访问实例
    UIDReader() { readUID(); }

    // 禁止拷贝构造函数和赋值运算符
    UIDReader(const UIDReader&) = delete;
    UIDReader& operator=(const UIDReader&) = delete;

    // 从硬件地址读取 UID 数据
    void readUID() {
        uint64_t* uid_address = reinterpret_cast<uint64_t*>(0x1FFF7A10);
        value = *uid_address;    // 直接赋值给 value
    }
};
