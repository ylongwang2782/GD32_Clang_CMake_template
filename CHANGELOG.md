# Changelog

All notable changes to this project will be documented in this file. See [standard-version](https://github.com/conventional-changelog/standard-version) for commit guidelines.

## 1.1.0 (2025-06-10)


### 🐛 Bug Fixes

* 修复log verbose前缀缺失 ([f7e1e70](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/f7e1e70ab48c5fbd90aa8bdf832952cc76fb5839))
* 修复log函数格式化字符串过程中数据异常问题 ([b70c49e](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/b70c49edb8b2196eb82dbb4eefffc549c85625ec))
* 修复了任务中无法使用dma tx 的问题 ([e25d7fd](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/e25d7fd838fb1a8dbacecd7805fc6a0a376f8b44))
* 修复了消息队列报错的问题 ([fc18a31](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/fc18a31006522baef3d78a55bdbe012ff93311d2))
* 继续修复文件描述 ([d0120c9](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/d0120c9e697e5e566a2168724e35c8440ff1a343))


### ✨ Features

* add debug support ([5fe0590](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/5fe0590fc7718cdd1f9ddd0873c793a4584eb314))
* add GPIO initialization and UART4 interrupt handler ([86108e3](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/86108e3441389187b0a5154c76e3fbdbbbdac673))
* add SPI task for master and slave communication ([6459450](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/645945070ccff11efa70d0426b4f41f1b16581ed))
* add UWB packet builder and update UART baudrate ([bbbeba9](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/bbbeba9783e235f9f124763f8ff316bbdce33eca))
* **BSP:** add ADC single conversion class for GD32F4xx ([499c05e](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/499c05ec696afb6202de14d9e63fd7e4e1c29e21))
* **bsp:** 添加外部中断(EXTI)支持并优化SPI驱动 ([2817f6b](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/2817f6bfef2e28cc84333ac14f0c3db8f404eda8))
* **core:** add CX310 class and integrate it into UwbTask ([1ed2d32](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/1ed2d32c201cc53f66b43d08ca89e66505d2bccb))
* **dw1000:** 添加DW1000驱动支持并集成到主程序 ([a5b4254](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/a5b425493cbb8a81ce2a0f2d60433bd6c8525cf8))
* **uci:** add UCI class for UWB communication and update UART handling ([e261c58](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/e261c58fb69d5ee954462928dd7da6a705b773c2))
* **uwb:** 添加UWB模块核心功能实现 ([a37d634](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/a37d6348f86b3d9c5828080a07b1764337595255))
* 串口新增dma tx函数 ([49a3309](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/49a3309380ce1bc259aa8e6a28c0a77297d411f2))
* 新增log系统 ([33566f1](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/33566f17a0f4f2f2a89d9791fab5a3bdff2ff2cc))
* 新增printf重定向 ([0bcf02d](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/0bcf02db2add91f94c860d5aa1188b310a63f8d8))
* 新增下载 install功能 ([feed28c](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/feed28c96caa909d2522431e5c07aa3250eb2287))
* 新增多种日志功能 ([a445734](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/a44573466d4b5536e96f3ac6286dce807c693c31))
* 新增打印字符串函数 ([47e1c54](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/47e1c5496b39e3ac062678ffa921ed0a1da1d53e))
* 新增断言实现 ([12d3aaa](https://github.com/ylongwang2782/GD32_Clang_CMake_template/commit/12d3aaa92d6d0b93cd5d8b9b4c864e67142c107c))
