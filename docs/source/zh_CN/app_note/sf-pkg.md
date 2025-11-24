# SF-PKG 使用指南

与 [RT-PKG](./rt-pkg.md) 类似，SF-PKG 也是一个包管理器。但是与 RT-PKG 不同，SF-PKG 底层采用了 [Conan](https://conan.io/) 作为包管理工具，支持更丰富的包管理功能，方便用户创建、上传和下载驱动包。并且支持语义化版本管理，方便用户进行版本控制和依赖管理。

## 安装包

接下来的介绍都假定你已经进入对应的有效 SDK 工程中，并且已经执行过 `.\export.ps1` 脚本初始化环境。

### 添加依赖（sf-pkg-add-dep）

执行添加依赖命令：

```bash
sdk.py sf-pkg-add-dep
```

![添加依赖](assert\sf-pkg-add-dep.png)

执行成功后，会在 `project` 目录下生成 `conanfile.txt` 文件：

![conanfile.txt 文件](assert\conanfile_txt.png)

接下来，我们需要手动编辑 `conanfile.txt` 文件，添加需要使用的包。例如，添加 SHT30 驱动包：

```txt
[requires]
sht30/0.0.4@caisong123

[generators]
SConsDeps
KconfigDeps
```

### 配置格式说明

`[requires]` 项中的内容格式必须正确，格式为：

```
包名/版本号@用户名
```

- `包名`：Conan 包名称
- `版本号`：需要使用的版本
- `用户名`：包的发布者用户名

### 搜索可用的包

如果不确定包名或版本号，可以使用以下命令搜索：

```bash
conan search "包名关键字/*" -r=artifactory
```

#### 示例：搜索 SHT30 相关的包

```bash
conan search "sht30/*" -r=artifactory
```

![搜索包](assert\conan_search.png)

### 安装依赖（sf-pkg-install）

#### 执行安装命令

在工程的 `project` 目录下，执行安装命令：

```bash
sdk.py sf-pkg-install
```

![安装依赖](assert\sf-pkg-install.png)

#### 查看安装结果

安装成功后，会在 `project` 目录下生成 `sf-pkgs` 文件夹，其中包含：

- 驱动源码
- Conan 生成的脚本文件

![安装的文件](assert\installed_files.png)

### 使用驱动

完成依赖安装后，即可直接使用驱动进行开发：

- 可以直接编译和下载
- include 头文件时无需填写绝对路径，Conan 会自动处理路径配置

#### Kconfig 配置注意事项

- `menuconfig` 会自动整合 `sf-pkgs` 文件夹下的所有 `Kconfig` 文件
- 最终这些配置项会出现在 `menuconfig` 的 **SiFli External Components** 菜单中

## 上传自己的包

经过上面的学习，我们已经了解了如何使用 sf-pkg 下载和使用包。接下来，我们将介绍如何创建并上传自己的包到 sf-pkg 服务器，以便其他用户也能使用你的驱动包。

### 获取访问令牌

#### 登录网站

访问网站：<https://packages.sifli.com/zh>

使用 GitHub 账号登录，用户名即为 GitHub 用户名。

```{note}
目前必须使用 GitHub 账号登录，其他方式暂不支持。并且用户名为你的 GitHub 用户名，唯一的区别就是使用的全小写。
```

![登录网站](assert\log_in_to_the_website.png)

#### 进入个人中心

登录成功后，点击 **Profile** 进入个人中心。

![进入个人中心](assert\enter_profile.png)

#### 申请令牌

在个人中心页面申请访问令牌（Token）。

![创建令牌](assert\create_token.png)

#### 保存令牌

获取到的令牌请妥善保存，后续用于 `sdk.py sf-pkg-login` 命令。

![获取令牌](assert\get_token.png)

### 登录 sf-pkg

#### 执行登录命令

在终端中输入以下命令进行登录：

```bash
sdk.py sf-pkg-login -n github 用户名 -t 获取的 token
```

![登录成功](assert\sdk-pkg-login.png)

```{note}
需要注意的是，每台电脑只需要登录一次即可，登录信息会保存在本地。
```

### 创建包配置（sf-pkg-new）

#### 准备驱动文件夹

1. 新建文件夹，将需要打包的驱动代码放入该文件夹
2. 使用 VS Code 终端进入该驱动文件夹：

```bash
cd 文件夹路径
```

#### 执行创建命令
在终端中输入以下命令：

```bash
sdk.py sf-pkg-new --name 包名
```

![创建包配置](assert\sf-pkg-new.png)

#### 查看生成的配置文件

命令执行成功后，驱动文件夹下会自动生成两个配置文件：
- `conandata.yml` - 数据配置文件
- `conanfile.py` - 包配方文件

![驱动文件夹结构](assert\driver_folder.png)

#### 1.3.4 打包方式说明

Conan 支持两种打包方式：

| 打包方式     | 说明                                                                                       |
| ------------ | ------------------------------------------------------------------------------------------ |
| **本地打包** | 直接打包本地源文件                                                                         |
| **远程打包** | 通过 `conandata.yml` 配置远程 URL、版本号和 SHA256，在 `conanfile.py` 中配置相应的打包方式 |

以下以**本地打包 SHT30 驱动**为例进行说明。

###  配置 conanfile.py

####  完整配置示例

下面是一个详细注释的 `conanfile.py` 配置文件示例：
``` python
from conan import ConanFile
from conan.tools.files import copy, get
import os

class Sht30Recipe(ConanFile):
    """
    SHT30 驱动的 Conan 配方类
    用于定义如何打包和分发 SHT30 温湿度传感器驱动
    """
    
    # ==================== 基本信息 ====================
    name = "sht30"                      # 包名称，必填项，用于在 Conan 仓库中唯一标识该包
    package_type = "unknown"            # 包类型，驱动包通常使用"unknown"类型
                                        # 其他可选类型："library"（库）、"application"（应用）、"header-only"（纯头文件库）

    # ==================== 元数据信息（可选但建议填写） ====================
    user = "caisong123"                 # Conan 用户名，通常与 GitHub 用户名一致
    license = "Apache 2.0"              # 开源协议，常见的还有："MIT"、"GPL-3.0"、"BSD-3-Clause"等
    author = "caisong123"               # 包作者/维护者名称
    url = "<Package recipe repository url here, for issues about the package>"   
                                        # 包的仓库地址 (GitHub/GitLab 等），方便用户反馈问题
                                        # 远程打包时应填写实际的仓库 URL，如："https://github.com/user/repo"
    description = "SHT30 temperature and humidity sensor driver for SiFli-SDK"   
                                        # 包的详细描述，说明包的功能和用途
    topics = ("sensor", "sht30", "temperature", "humidity", "i2c")               
                                        # 包的标签/主题，便于搜索和分类，使用元组形式

    # ==================== 源文件导出 ====================
    # 指定需要从配方目录导出到打包目录的源文件
    # 支持通配符：*.h（所有头文件）、*.c（所有 C 源文件）等
    exports_sources = "*.h", "*.c", "SConscript", "Kconfig"
    
    # 其他常见文件类型示例：
    # exports_sources = "*.h", "*.c", "*.cpp", "CMakeLists.txt", "README.md", "LICENSE"

    # ==================== 依赖管理 ====================
    def requirements(self):
        """
        声明包的依赖项
        当前驱动需要依赖其他包时，在此方法中添加
        """
        # 添加依赖示例（取消注释并修改为实际依赖）:
        # self.requires("fmt/8.1.1")              # 依赖 fmt 库的 8.1.1 版本
        # self.requires("boost/1.80.0")           # 依赖 boost 库
        # self.requires("i2c-driver/1.0.0@user/stable")  # 依赖特定用户发布的稳定版 I2C 驱动
        
        # 添加工具依赖（仅在构建时需要，不会传递给依赖方）:
        # self.tool_requires("cmake/3.25.0")
        pass
    
    # ==================== 源码获取 ====================
    def source(self):
        """
        定义如何获取源代码
        - 本地打包：不需要实现此方法（默认使用 exports_sources 中的文件）
        - 远程打包：从 URL 下载源码并解压
        """
        # 远程打包示例（需配合 conandata.yml 使用）:
        # if hasattr(self, 'conan_data') and 'sources' in self.conan_data:
        #     # 从 conandata.yml 中读取 URL 和 SHA256，下载并解压源码
        #     get(self, **self.conan_data["sources"][self.version], strip_root=True)
        
        # 手动指定下载 URL 的示例：
        # get(self, 
        #     url="https://github.com/user/repo/archive/refs/tags/v1.0.0.tar.gz",
        #     sha256="abc123...",  # 文件的 SHA256 校验值
        #     strip_root=True)     # 去除压缩包的根目录
        pass

    # ==================== 生成构建配置 ====================
    def generate(self):
        """
        生成构建系统所需的配置文件
        对于简单的驱动包，通常不需要实现
        """
        # CMake 项目示例：
        # from conan.tools.cmake import CMakeToolchain
        # tc = CMakeToolchain(self)
        # tc.generate()
        
        # 环境变量设置示例：
        # from conan.tools.env import VirtualBuildEnv
        # env = VirtualBuildEnv(self)
        # env.generate()
        pass

    # ==================== 构建过程 ====================
    def build(self):
        """
        定义构建过程
        对于纯源码分发的驱动包，通常不需要编译，可以留空
        """
        # CMake 构建示例：
        # from conan.tools.cmake import CMake
        # cmake = CMake(self)
        # cmake.configure()
        # cmake.build()
        
        # 自定义构建命令示例：
        # self.run("make")
        # self.run("gcc -c *.c")
        pass

    # ==================== 打包过程 ====================
    def package(self):
        """
        定义如何将构建产物打包到最终的包中
        这是最重要的方法之一，决定了哪些文件会被包含在发布的包中
        """
        # 排除 Conan 生成的临时文件，这些文件不应该包含在最终的包中
        excludes_files = [
            "conanbuild.bat", "conanbuildenv.bat", "conanrun.sh", 
            "conanrunenv.sh", "deactivate_conanbuild.bat", "deactivate_conanbuild.sh",
            "deactivate_conanrun.sh", "conaninfo.txt", "conanmanifest.txt", 
            "Kconfig.conandeps", "SConscript_conandeps"
        ]

        # 复制头文件 (.h) 到包目录
        # src: 源目录 (self.source_folder 为源码目录）
        # dst: 目标目录 (self.package_folder 为包的根目录）
        # excludes: 排除的文件列表
        copy(self, "*.h", 
             src=self.source_folder, 
             dst=self.package_folder,
             excludes=excludes_files)
        
        # 复制 C 源文件 (.c) 到包目录
        copy(self, "*.c", 
             src=self.source_folder, 
             dst=self.package_folder,
             excludes=excludes_files)

        # 复制构建脚本文件
        copy(self, "SConscript", 
             src=self.source_folder, 
             dst=self.package_folder,
             excludes=excludes_files)

        # 复制配置文件
        copy(self, "Kconfig", 
             src=self.source_folder, 
             dst=self.package_folder,
             excludes=excludes_files)
        
        # 其他常见的打包操作示例：
        # 1. 复制到特定子目录
        # copy(self, "*.h", 
        #      src=os.path.join(self.source_folder, "include"),
        #      dst=os.path.join(self.package_folder, "include"))
        
        # 2. 复制库文件
        # copy(self, "*.a", 
        #      src=os.path.join(self.build_folder, "lib"),
        #      dst=os.path.join(self.package_folder, "lib"))
        
        # 3. 复制许可证和文档
        # copy(self, "LICENSE", src=self.source_folder, dst=self.package_folder)
        # copy(self, "README.md", src=self.source_folder, dst=self.package_folder)
        
        # 4. 复制整个目录
        # copy(self, "*", 
        #      src=os.path.join(self.source_folder, "docs"),
        #      dst=os.path.join(self.package_folder, "docs"))

    # ==================== 包信息配置 ====================
    def package_info(self):
        """
        定义包的使用信息
        告诉依赖此包的项目如何使用这个包（头文件路径、库文件路径等）
        """
        # 头文件搜索路径，设置为"."表示包的根目录
        # 使用此包的项目会自动添加这个路径到 include 搜索路径
        self.cpp_info.includedirs = ["."]
        
        # 源文件目录，用于某些构建系统直接引用源码
        self.cpp_info.srcdirs = ["."]
        
        # 库文件搜索路径，这里用于指示 SConscript 和 Kconfig 的根路径
        self.cpp_info.libdirs = ["."]
```

#### 配置 conandata.yml

由于使用本地打包方式，`conandata.yml` 文件在此处用处不大，保持默认即可：

```yml
sources:
  "0.0.1":
    url: ""
    sha256: ""
```

如果使用远程打包方式，需要在此文件中配置源码下载地址和 SHA256 校验值。

### 构建包（sf-pkg-build）

在驱动文件夹下，执行构建命令：

```bash
sdk.py sf-pkg-build --version 版本号
```

![构建包](assert\sf-pkg-build.png)

**版本号格式**：建议使用语义化版本号，如 `0.0.1`、`1.0.0` 等。

### 上传包（sf-pkg-upload）

#### 执行上传命令

构建成功后，执行上传命令：

```bash
sdk.py sf-pkg-upload --name 包名/版本号@用户名
```

![上传包](assert\sf-pkg-upload.png)

**命令格式说明**：
- `包名`：在 `conanfile.py` 中定义的包名
- `版本号`：构建时指定的版本号
- `用户名`：你的 GitHub 用户名

#### 上传失败的处理

如果出现构建成功但上传失败的情况，可以尝试以下步骤：

1. 清除本地缓存：
   ```bash
   sdk.py sf-pkg-remove --name 包名
   ```

2. 重新构建：
   ```bash
   sdk.py sf-pkg-build --version 版本号
   ```

3. 再次上传：
   ```bash
   sdk.py sf-pkg-upload --name 包名/版本号@用户名
   ```

#### 验证上传结果

上传成功后，可以在服务器网站上看到已上传的包：

![服务器上的包](assert\pkg_in_website.png)

## 常用命令速查表

| 命令                                                   | 说明             |
| ------------------------------------------------------ | ---------------- |
| `sdk.py sf-pkg-login -n <用户名> -t <令牌>`            | 登录 sf-pkg 系统 |
| `sdk.py sf-pkg-new --name <包名>`                      | 创建新的包配置   |
| `sdk.py sf-pkg-build --version <版本号>`               | 构建包           |
| `sdk.py sf-pkg-upload --name <包名>/<版本号>@<用户名>` | 上传包到服务器   |
| `sdk.py sf-pkg-remove --name <包名>`                   | 清除本地缓存     |
| `sdk.py sf-pkg-add-dep`                                | 添加依赖配置     |
| `sdk.py sf-pkg-install`                                | 安装依赖包       |
| `conan search "<包名>/*" -r=artifactory`               | 搜索可用的包     |

---

## 常见问题

### 上传失败怎么办？

**解决方案**：
1. 检查网络连接是否正常
2. 确认令牌是否有效
3. 清除本地缓存后重新构建和上传

### 依赖安装失败怎么办？

**解决方案**：
1. 检查 `conanfile.txt` 中的包名、版本号、用户名格式是否正确
2. 使用 `conan search` 命令确认包是否存在
3. 确认已执行 `.\export.ps1` 初始化环境

### 如何更新已安装的依赖？

**解决方案**：
1. 修改 `conanfile.txt` 中的版本号
2. 重新执行 `sdk.py sf-pkg-install` 命令