# Vulkan Triangle Examples

一组使用 Vulkan API 的三角形渲染示例程序，包含从基础到进阶的多个示例。

## 功能

### 1. VulkanTriangle (基础示例)
- 使用 Vulkan 1.0+ API
- 绘制一个红色的静态三角形
- 使用 GLFW 创建窗口
- 包含完整的 Vulkan 初始化流程（实例、设备、交换链、管线等）

### 2. VulkanTriangleAnim (动画示例)
- 在基础示例上添加动画效果
- 三角形绕 Z 轴旋转（每秒 45 度）
- 使用 Uniform Buffer 传递变换矩阵
- 展示 Vulkan 中的时间动画实现

### 3. VulkanTriangleAnimTexture (纹理示例)
- 在动画示例基础上添加纹理贴图
- 使用 stb_image.h 加载 PNG 纹理图片
- 实现纹理采样器和描述符集
- 展示完整的纹理映射流程

## 项目结构

```
vulkan/
├── main.cpp                    # 基础示例主程序
├── main_anim.cpp               # 动画示例主程序
├── main_anim_texture.cpp       # 纹理示例主程序
├── triangle.vert               # 基础顶点着色器
├── triangle.frag               # 基础片段着色器
├── triangle_anim.vert          # 动画顶点着色器
├── triangle_anim.frag          # 动画片段着色器
├── triangle_texture.vert       # 纹理顶点着色器
├── triangle_texture.frag       # 纹理片段着色器
├── checkerboard_1024x1024.png  # 纹理图片
├── stb_image.h                 # 图片加载库
├── CMakeLists.txt              # CMake 构建配置
└── README.md                   # 项目说明文档
```

## 系统要求

### 必需依赖
- **CMake** 3.10+
- **Vulkan SDK** (包含 glslc 着色器编译器)
- **GLFW** 3.0+
- **glm** (OpenGL Mathematics)
- **C++17** 兼容编译器

### 安装依赖 (Ubuntu/Debian)
```bash
# 安装 Vulkan SDK
# 访问 https://vulkan.lunarg.com/ 下载

# 安装 GLFW 和 glm
sudo apt-get install libglfw3-dev libglm-dev

# 验证 Vulkan 安装
vulkaninfo
```

## 构建步骤

```bash
# 创建并进入 build 目录
mkdir build
cd build

# 配置 CMake
cmake ..

# 编译项目
make
```

编译完成后，build 目录将生成以下可执行文件：
- `VulkanTriangle` - 基础三角形示例
- `VulkanTriangleAnim` - 旋转三角形示例
- `VulkanTriangleAnimTexture` - 带纹理的旋转三角形示例

## 运行示例

### 基础三角形
```bash
cd build
./VulkanTriangle
```

### 旋转三角形
```bash
cd build
./VulkanTriangleAnim
```

### 带纹理的旋转三角形
```bash
cd build
./VulkanTriangleAnimTexture
```

**注意**: 纹理示例需要从当前目录加载 `checkerboard_1024x1024.png` 文件，请确保在 build 目录下运行，或确保纹理文件在当前工作目录。

## 技术细节

### Vulkan 初始化流程
1. 创建实例 (Instance)
2. 设置调试消息器 (Debug Messenger)
3. 创建窗口表面 (Surface)
4. 选择物理设备 (Physical Device)
5. 创建逻辑设备 (Logical Device)
6. 创建交换链 (Swap Chain)
7. 创建图像视图 (Image Views)
8. 创建渲染通道 (Render Pass)
9. 创建图形管线 (Graphics Pipeline)
10. 创建帧缓冲区 (Framebuffers)
11. 创建命令池 (Command Pool)
12. 创建资源 (顶点缓冲区、Uniform 缓冲区、纹理等)
13. 创建描述符集 (Descriptor Sets)
14. 创建命令缓冲区 (Command Buffers)
15. 创建同步对象 (Semaphores & Fences)

### 着色器
- 使用 GLSL 编写
- 通过 glslc 编译为 SPIR-V 字节码
- CMake 自动处理着色器编译

### 纹理加载
- 使用 stb_image.h 库加载 PNG 图片
- 自动转换为 RGBA 格式
- 创建 staging buffer 传输到 GPU
- 使用 VK_FORMAT_R8G8B8A8_SRGB 格式

## 验证层

项目在 Debug 模式下启用 Vulkan 验证层 (`VK_LAYER_KHRONOS_validation`)，用于：
- 检查 API 使用错误
- 输出性能和警告信息
- 帮助调试 Vulkan 应用

验证层会产生一些输出信息，这是正常现象。

## 常见问题

### "failed to create GLFW window"
- 确保系统支持 Vulkan
- 检查显卡驱动是否正确安装
- 运行 `vulkaninfo` 验证 Vulkan 支持

### "validation layers requested, but not available"
- 安装 Vulkan SDK 时确保选择了验证层
- 或者在代码中将 `enableValidationLayers` 设置为 `false`

### 纹理示例崩溃
- 确保 `checkerboard_1024x1024.png` 文件存在于运行目录
- 检查 stb_image.h 是否正确包含

## 参考资源

- [Vulkan Tutorial](https://vulkan-tutorial.com/) - 详细的 Vulkan 入门教程
- [Vulkan Specification](https://www.khronos.org/registry/vulkan/specs/) - Vulkan 官方规范
- [GLFW Documentation](https://www.glfw.org/documentation.html) - GLFW API 文档
- [stb_image](https://github.com/nothings/stb) - 单头图片加载库

## 许可证

本项目代码为学习目的，可自由使用和修改。
