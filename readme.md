# GD32_clang_cmake_template

## Introduction

这是一个使用`vscode`作为IDE进行GD32嵌入式开发的模板，适合更喜欢使用vscode的开发者。
目前可以实现通过build生成hex和bin文件。

## 特点

editor:
- 使用`clangd`作为编辑器的lsp服务器,实现高效跳转和代码补全，以及更好的静态分析

compile & build:
- 使用`cmake`作为构建系统
- 使用`ninja`作为构建工具
- 使用`arm-none-eabi-gcc`作为编译器

## 环境搭建

### 需要的软件

- vscode
[vscode download](https://code.visualstudio.com/Download)
- cmake
官网安装
- ninja
- gcc-arm-none-eabi
[Arm GNU Toolchain Downlaods](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads/13-2-rel1)
- LLVM
[LLVM](https://llvm.org/)
- clang
安装`msys64`或者`LLVM`会自带`clang`

以上软件均需要添加到环境变量
### 需要的vscode拓展

- Clangd
- Clang-Format
- Cmake
- Cmake Tools
- CodeLLDB

### 需要更改的配置

#### setting.json

更改`clangd.exe`的地址
```json
"clangd.path": "C:/msys64/clang64/bin/clangd.exe"
```

#### .clangd

需要更改`clangd`的配置文件，添加头文件路径，不然会找不到标准头文件
```json
CompileFlags:
  Add: [
        "-IC:/code_configuration/arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-arm-none-eabi/arm-none-eabi/include/"
        ]   
```