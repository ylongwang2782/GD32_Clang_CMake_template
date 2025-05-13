#include <array>
#include <cstdint>

class UIDReader {
   public:
    // 获取UID值（静态方法）
    static uint32_t get() {
        static uint32_t value = [] {
            uint32_t* uid_address = reinterpret_cast<uint32_t*>(0x1FFF7A10);
            return *uid_address;
        }();
        return value;
    }

   private:
    // 禁止实例化
    UIDReader() = delete;
    ~UIDReader() = delete;
};
