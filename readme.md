# GD32_clang_cmake_template

## Introduction

这是一个完全脱离`keil`使用`vscode`作为IDE进行GD32嵌入式开发的模板，适合更喜欢使用vscode的开发者。
- `Cmake:build`：构建固件
- `Cmake:install`：烧录固件到设备
- `Cortex-Debug`：对设备进行调试

## 特点

Editor:
- 使用`clangd`作为编辑器的lsp服务器,实现高效跳转和代码补全，以及更好的静态分析

Compile & Build:
- 使用`cmake`作为构建系统
- 使用`ninja`作为构建工具
- 使用`arm-none-eabi-gcc`作为编译器

os:
- 使用`freertos`作为操作系统

## 环境搭建

### 配置环境变量

针对不同平台不同设备，工具链地址不同，因此需要根据不同平台不同设备配置环境变量，将以下变量以及对应的地址添加到环境变量中

1. ARM_TOOLCHAIN_GPP
2. CLANGD_PATH
3. CROSS_COMPILE_TOOLCHAIN
4. OPENOCD_SCRIPTS_PATH


例如windows下环境变量为

```bash
set ARM_TOOLCHAIN_GPP=C:/code_configuration/arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-arm-none-eabi/bin/arm-none-eabi-g++.exe
set CLANGD_PATH=C:/msys64/clang64/bin/clangd.exe
set CROSS_COMPILE_TOOLCHAIN=C:/code_configuration/EmbeddedBuilder_v1.4.7.26843/Tools/GNU Tools ARM Embedded/xpack-arm-none-eabi-gcc/9.2.1-1.1
set OPENOCD_SCRIPTS_PATH=C:/code_configuration/EmbeddedBuilder_v1.4.7.26843/Tools/OpenOCD/xpack-openocd-0.11.0-3/scripts/
```

### 需要的软件

- [vscode](https://code.visualstudio.com/Download)
- [cmake](https://cmake.org/download/)
1. 添加环境变量`cmake\bin`
- [ninja](https://github.com/ninja-build/ninja/releases/tag/v1.12.1)
1. 添加环境变量`ninja\`
- [GD32 Embedded Builder](https://www.gd32mcu.com/cn/download)
1. 添加环境变量`EmbeddedBuilder_v1.4.7.26843\Tools\OpenOCD\xpack-openocd-0.11.0-3\bin`
2. 添加环境变量`EmbeddedBuilder_v1.4.7.26843\Tools\GNU Tools ARM Embedded\xpack-arm-none-eabi-gcc\9.2.1-1.1\bin`

可选安装项：
- clang(optional)
- [LLVM](https://llvm.org/)(optional）

以上软件均需要添加到环境变量
### 需要的vscode拓展

可以在`vscode`拓展中搜索`@recommended`以便快速安装推荐的拓展

### 需要更改的配置

#### cmake-tools-kits.json

`Edit User-Local Cmake Kits`中添加

```json
  {
    "name": "GD32F4xx_none_compiler",
    "toolchainFile": "D:\\project\\GD32_Clang_CMake_template\\CMake\\arm-none-eabi-gcc.cmake",
    "isTrusted": true,
    "preferredGenerator": {
      "name": "MinGW Makefiles"
    }
  }
```

其中toolchainFile更改为工程下的`.cmake`文件地址（应该可以使用相对地址）

#### FreeRTOS

FreeRTOS系统无需更改，可直接使用，支持c和cpp

## TODO

- [ ]考虑增加环境库
- [ ]如何实现跨系统兼容