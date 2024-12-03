# GD32_clang_cmake_template

## Introduction

这是一个使用vscode作为IDE进行GD32嵌入式开发的模板，适合更喜欢使用vscode的开发者。
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
- cmake
- ninja
- gcc-arm-none-eabi
- LLVM
- clang

### 需要的vscode拓展

- Clangd
- Clang-Format
- Cmake
- Cmake Tools
- CodeLLDB