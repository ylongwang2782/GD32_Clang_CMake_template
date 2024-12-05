# GD32_clang_cmake_template

## Introduction

这是一个完全脱离`keil`使用`vscode`作为IDE进行GD32嵌入式开发的模板，适合更喜欢使用vscode的开发者。
目前可以实现通过build生成hex和bin文件。

## 特点

editor:
- 使用`clangd`作为编辑器的lsp服务器,实现高效跳转和代码补全，以及更好的静态分析

compile & build:
- 使用`cmake`作为构建系统
- 使用`ninja`作为构建工具
- 使用`arm-none-eabi-gcc`作为编译器

os:
- 使用`freertos`作为操作系统

## 环境搭建

### 需要的软件

- [vscode](https://code.visualstudio.com/Download)
- [cmake](https://cmake.org/download/)
- [ninja](https://github.com/ninja-build/ninja/releases/tag/v1.12.1)
- gcc-arm-none-eabi
1. 下载[Arm GNU Toolchain Downlaods](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads/13-2-rel1)
2. 添加环境变量`arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-arm-none-eabi\bin`
- [LLVM](https://llvm.org/)
- clang
1. 安装`msys64`或者`LLVM`会自带`clang`
2. 添加环境变量
- [GD32 Embedded Builder](https://www.gd32mcu.com/cn/download)
GD32 Embedded Builder非常强大，其中包含了
1. 交叉编译工具链
2. .ld文件
3. .s文件
4. OpenOCD
5. OpenOCD会用到的cfg文件
- openocd-xpack
1. 先下载[GD32 Embedded Builder](https://www.gd32mcu.com/cn/download)
2. 添加`EmbeddedBuilder_v1.4.7.26843\Tools\OpenOCD\xpack-openocd-0.11.0-3\bin`到环境变量

以上软件均需要添加到环境变量
### 需要的vscode拓展

可以在`vscode`拓展中搜索`@recommended`以便快速安装推荐的拓展

### 需要更改的配置

#### setting.json

更改`clangd.exe`的地址
```json
"clangd.path": "C:/msys64/clang64/bin/clangd.exe"
```

更改clangd的编译器地址
```json
"--query-driver=C:/code_configuration/arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-arm-none-eabi/bin/arm-none-eabi-g++.exe"
```

#### FreeRTOS

FreeRTOS系统无需更改，可直接使用，支持c和cpp

## TODO

- [ ] 增加`C++`支持
- [ ] 支持在线调试
- [ ] 增加`printf`重定向