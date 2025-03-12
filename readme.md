## 简介
本项目提供了一个完全脱离 Keil，使用 **VSCode** 作为 IDE 进行 **GD32** 嵌入式开发的模板。适合偏好使用 VSCode 进行开发的嵌入式工程师。通过本模板，您可以轻松实现代码的编译、烧录和在线调试。

## 主要功能
+ **Cmake:build**：构建固件
+ **Cmake:install**：烧录固件到设备
+ **Cortex-Debug**：对设备进行在线调试

## 项目特点
### 编辑器
+ **Clangd**：作为编辑器的 LSP 服务器，提供高效的代码跳转、补全和静态分析功能。

### 编译与构建
+ **CMake**：作为构建系统，支持跨平台开发。
+ **Ninja**：作为构建工具，提供快速的构建速度。
+ **arm-none-eabi-gcc**：作为编译器，支持 ARM 架构的嵌入式开发。

### 操作系统
+ **FreeRTOS**：作为实时操作系统，支持多任务调度。

## 工具下载
### 软件
+ [VSCode](https://code.visualstudio.com/Download)

### VSCode 扩展
以下是推荐的 VSCode 扩展列表，您可以通过以下方式安装：

1. **在线安装**：直接在 VSCode 扩展市场中搜索并安装。
2. **离线安装**：通过以下链接下载 `.vsix` 文件进行安装（如遇版本兼容问题，请调整插件版本或升级 VSCode）。

```json
{
    "recommendations": [
        "ms-vscode.cmake-tools",
        "cheshirekow.cmake-format",
        "llvm-vs-code-extensions.vscode-clangd",
        "xaver.clang-format",
        "marus25.cortex-debug",
        "mcu-debug.debug-tracker-vscode",
        "mcu-debug.memory-view",
        "mcu-debug.peripheral-viewer",
        "mcu-debug.rtos-views"
    ]
}
```

#### 扩展下载链接
+ [CMake Tools](https://marketplace.visualstudio.com/_apis/public/gallery/publishers/ms-vscode/vsextensions/cmake-tools/1.20.53/vspackage)
+ [Clang-Format](https://marketplace.visualstudio.com/_apis/public/gallery/publishers/xaver/vsextensions/clang-format/1.9.0/vspackage)
+ [Clangd](https://marketplace.visualstudio.com/_apis/public/gallery/publishers/llvm-vs-code-extensions/vsextensions/vscode-clangd/0.1.33/vspackage)
+ [CMake Format](https://marketplace.visualstudio.com/_apis/public/gallery/publishers/cheshirekow/vsextensions/cmake-format/0.6.11/vspackage)

#### Cortex-Debug 插件及其依赖
+ [Cortex-Debug](https://marketplace.visualstudio.com/_apis/public/gallery/publishers/marus25/vsextensions/cortex-debug/1.12.1/vspackage)
+ [Memory View](https://marketplace.visualstudio.com/_apis/public/gallery/publishers/mcu-debug/vsextensions/memory-view/0.0.25/vspackage)
+ [Peripheral Viewer](https://marketplace.visualstudio.com/_apis/public/gallery/publishers/mcu-debug/vsextensions/peripheral-viewer/1.4.6/vspackage)
+ [RTOS Views](https://marketplace.visualstudio.com/_apis/public/gallery/publishers/mcu-debug/vsextensions/rtos-views/0.0.7/vspackage)
+ [Debug Tracker](https://marketplace.visualstudio.com/_apis/public/gallery/publishers/mcu-debug/vsextensions/debug-tracker-vscode/0.0.15/vspackage)

### 环境工具
+ [CMake](https://cmake.org/download/)
+ [Ninja](https://github.com/ninja-build/ninja/releases/tag/v1.12.1)
+ [GD32 Embedded Builder](https://www.gd32mcu.com/data/documents/toolSoftware/GD32EmbeddedBuilder_v1.4.12.28625.7z)
+ [ARM GCC Toolchain](https://github.com/xpack-dev-tools/arm-none-eabi-gcc-xpack/releases/tag/v14.2.1-1.1)
+ [LLVM](https://releases.llvm.org/download.html)

## 环境搭建
### Windows
1. **添加用户环境变量路径**：
    - `your_path\Cmake\bin`
    - `your_path\ninja\`
    - `your_path\xpack-openocd-0.11.0-3\bin`
    - `your_path\arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-arm-none-eabi\bin`
    - `your_path\LLVM\bin`
2. **添加用户环境变量**：
    - `ARM_CXX_PATH = your_path/arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-arm-none-eabi/bin/arm-none-eabi-g++.exe`

### Ubuntu
1. **安装依赖**：
- **VSCode**：

```bash
sudo snap install --classic code
```

- **Ninja**：

```bash
sudo apt update
sudo apt install ninja-build
ninja --version
```

- **CMake**：

```bash
sudo apt update
sudo apt install cmake
cmake --version
```

- **GCC ARM Toolchain**：
   * 下载 [xPack GNU Arm Embedded GCC](https://github.com/xpack-dev-tools/arm-none-eabi-gcc-xpack/releases)
   * 添加路径到 `~/.bashrc`：

```bash
export PATH=$PATH:/path/to/xpack-arm-none-eabi-gcc/bin
```

 - **LLVM/Clang/Clangd/Clang-format**：
    * 下载 [LLVM](https://releases.llvm.org/download.html)
    * 添加路径到 `~/.bashrc`：

```bash
export PATH=$PATH:/path/to/LLVM/bin
```

 - **OpenOCD**：
    * 需要自行编译安装支持 GD 版本的 OpenOCD。

### MacOS
+ **TODO**：待补充。

## 配置 VSCode
### 指定 Clangd 的编译器
1. 将 `arm-none-eabi-g++` 的路径定义为环境变量 `ARM_CXX_PATH`。
2. 示例：
- **Ubuntu**：

```bash
export ARM_CXX_PATH="/path/to/xpack-arm-none-eabi-gcc/bin/arm-none-eabi-g++"
```

 - **Windows**：

```bash
setx /M ARM_CXX_PATH "C:/path/to/arm-gnu-toolchain/bin/arm-none-eabi-g++.exe"
```

3. 重启系统以使环境变量生效。

## 工程配置
### 配置 CMake
1. 在 VSCode 中选择 **Select a Kit**，选择 **Unspecified**。
2. 执行 **CMake: Delete Cache and Reconfigure**。

### 编译
+ **CMake: Build**：通过该指令编译固件，生成 `.elf`、`.bin`、`.hex` 文件。
    - 建议将该命令绑定到快捷键 `F7`。

### 烧录
+ **flash_by_install.cmake**：
  - `Jlink`选择`openocd_jlink.cfg`，并且使用[zadig](https://blog.csdn.net/K_O_R_K/article/details/120615059)切换`Jlink`为`WinUSB`模式。
  - `DapLink`选择`openocd_daplink.cfg`。
+ **CMake: Install**：通过该指令进行烧录，默认使用 **DapLink**。
    - 建议将该命令绑定到快捷键 `F8`。

---

通过本模板，您可以快速搭建基于 VSCode 的 GD32 开发环境，享受高效的开发体验。如有任何问题，欢迎提交 Issue 或 Pull Request。

