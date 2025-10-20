# DFU_PAN固件打包工具使用文档

## 1. 工具概述
固件升级 (DFU_PAN,Device Firmware Update PAN)：
此固件升级方式是通过蓝牙PAN连接进行OTA固件下载和安装功能，此处介绍如何使用此脚本工具打包固件。
脚本工具：`"$SIFLI_SDK/tool/dfu_pan_tool"`
使用的middleware：`"$SIFLI_SDK/middleware/dfu_pan"`

OTA固件打包工具是一个Python脚本，用于将多个bin文件打包成一个OTA固件包。该工具支持灵活的INI配置文件格式，统一使用小端序存储，魔数对应"_OTA"（0x5F4F5441）。

## 2. 使用方法

### 2.1 命令行参数

```bash
python dfu_pan_paket.py --ini <INI配置文件路径> [--output <输出OTA包路径>]
```

- `--ini`: 必填参数，指定INI配置文件路径
- `--output`: 可选参数，指定输出OTA包路径，默认为`ota_mix.bin`

### 2.2 示例

```bash
python dfu_pan_paket.py --ini ./dfu_pan.ini --output ./ota_package.bin
```

## 3. INI配置文件说明

INI配置文件分为两个部分：COMMON部分和各个bin文件部分。

### 3.1 COMMON部分

```ini
[COMMON]
IMG_FLAG=0x5F4F5441    # 魔数，固定为0x5F4F5441（_OTA）
IMG_VER=0x00000134     # 版本号，示例为1.3.4
FILE_PATH=./bin_files  # bin文件所在目录路径
```

### 3.2 bin文件部分

每个bin文件需要一个独立的部分，部分名称可以自定义。以下是各字段说明：

```ini
[APP]                  # 自定义部分名称
NAME=ER_IROM1.bin      # bin文件名
SEL=1                  # 是否选中（1=选中，0=不选中）
GZIP=0                 # 是否压缩（1=压缩，0=不压缩）
IDX=0                  # 文件ID
ADDR=0x12218000        # Flash地址
REGION_SIZE=0x00240000 # 擦除区域大小
```

可以配置多个bin文件部分，例如：

```ini
[COMMON]
IMG_FLAG=0x5F4F5441
IMG_VER=0x00000134
FILE_PATH=./bin_files

[APP]
NAME=ER_IROM1.bin
SEL=1
GZIP=0
IDX=0
ADDR=0x12218000
REGION_SIZE=0x00240000

[IMG]
NAME=ER_IROM3.bin
SEL=1
GZIP=0
IDX=1
ADDR=0x12460000
REGION_SIZE=0x00680000

[FONT]
NAME=ER_IROM2.bin
SEL=1
GZIP=0
IDX=2
ADDR=0x12AE0000
REGION_SIZE=0x00400000
```

## 4. 固件包结构

根据提供的INI配置文件示例，生成的OTA固件包具有以下结构：

```
+----------------------+ <- 0x00000000
| 总头部 (16字节)       |
| - 头部CRC32 (4字节)   | 0xF16C8B4A
| - 魔数 (4字节)        | 0x5F4F5441 (_OTA)
| - 版本号 (4字节)      | 0x00000134 (1.3.4)
| - bin文件数量 (4字节)  | 0x00000003 (3个文件)
+----------------------+ <- 0x00000010
| bin文件1头部 (76字节)  |
| - 文件名 (48字节)     | "ER_IROM1.bin\0\0\0..."
| - 文件ID (2字节)      | 0x0000
| - GZIP标志 (2字节)    | 0x0000
| - 数据长度 (4字节)    | [根据实际文件大小]
| - 原始长度 (4字节)    | [根据实际文件大小]
| - Flash地址 (4字节)   | 0x12218000
| - 数据CRC32 (4字节)   | [根据实际文件内容计算]
| - 原始数据CRC32 (4字节)| [根据实际文件内容计算]
| - 区域大小 (4字节)    | 0x00240000
+----------------------+ <- [偏移量]
| bin文件2头部 (76字节)  |
| - 文件名 (48字节)     | "ER_IROM3.bin\0\0\0..."
| - 文件ID (2字节)      | 0x0001
| - GZIP标志 (2字节)    | 0x0000
| - 数据长度 (4字节)    | [根据实际文件大小]
| - 原始长度 (4字节)    | [根据实际文件大小]
| - Flash地址 (4字节)   | 0x12460000
| - 数据CRC32 (4字节)   | [根据实际文件内容计算]
| - 原始数据CRC32 (4字节)| [根据实际文件内容计算]
| - 区域大小 (4字节)    | 0x00680000
+----------------------+ <- [偏移量]
| bin文件3头部 (76字节)  |
| - 文件名 (48字节)     | "ER_IROM2.bin\0\0\0..."
| - 文件ID (2字节)      | 0x0002
| - GZIP标志 (2字节)    | 0x0000
| - 数据长度 (4字节)    | [根据实际文件大小]
| - 原始长度 (4字节)    | [根据实际文件大小]
| - Flash地址 (4字节)   | 0x12AE0000
| - 数据CRC32 (4字节)   | [根据实际文件内容计算]
| - 原始数据CRC32 (4字节)| [根据实际文件内容计算]
| - 区域大小 (4字节)    | 0x00400000
+----------------------+ <- [偏移量]
| bin文件1数据          |
| ([实际文件大小]字节)  |
+----------------------+ <- [偏移量]
| bin文件2数据          |
| ([实际文件大小]字节)  |
+----------------------+ <- [偏移量]
| bin文件3数据          |
| ([实际文件大小]字节)  |
+----------------------+ <- [文件结束]
```

### 4.1 总头部结构 (16字节)

| 偏移量 | 字段名 | 大小 | 说明 |
|--------|--------|------|------|
| 0x00 | 头部CRC32 | 4字节 | 所有头部信息的CRC32校验值 |
| 0x04 | 魔数 | 4字节 | 固定值0x5F4F5441 ('_OTA') |
| 0x08 | 版本号 | 4字节 | 固件版本号 |
| 0x0C | bin文件数量 | 4字节 | 包含的bin文件数量 |

### 4.2 bin文件头部结构 (76字节)

每个bin文件都有一个76字节的头部，包含以下信息：

| 偏移量 | 字段名 | 大小 | 说明 |
|--------|--------|------|------|
| 0x00 | 文件名 | 48字节 | bin文件名，不足部分用'\0'填充 |
| 0x30 | 文件ID | 2字节 | 文件标识符 |
| 0x32 | GZIP标志 | 2字节 | 压缩标志，0表示未压缩，1表示已压缩 |
| 0x34 | 数据长度 | 4字节 | 实际数据长度（压缩后长度） |
| 0x38 | 原始长度 | 4字节 | 原始数据长度（对齐后） |
| 0x3C | Flash地址 | 4字节 | 在Flash中的目标地址 |
| 0x40 | 数据CRC32 | 4字节 | 数据的CRC32校验值 |
| 0x44 | 原始数据CRC32 | 4字节 | 原始数据的CRC32校验值 |
| 0x48 | 区域大小 | 4字节 | Flash擦除区域大小 |

### 4.3 头部CRC32计算过程

头部CRC32的计算基于以下数据：
1. 魔数: 0x5F4F5441 (小端序: 0x41, 0x54, 0x4F, 0x5F)
2. 版本号: 0x00000134 (小端序: 0x34, 0x01, 0x00, 0x00)
3. bin文件数量: 0x00000003 (小端序: 0x03, 0x00, 0x00, 0x00)
4. 所有bin文件头部信息

假设三个bin文件的头部信息如下（以小端序表示）：

bin文件1头部（ER_IROM1.bin）:
- 文件名: "ER_IROM1.bin\0\0\0..." (48字节)
- 文件ID: 0x0000 (小端序: 0x00, 0x00)
- GZIP标志: 0x0000 (小端序: 0x00, 0x00)
- 数据长度: [实际值] (小端序)
- 原始长度: [实际值] (小端序)
- Flash地址: 0x12218000 (小端序: 0x00, 0x80, 0x21, 0x12)
- 数据CRC32: [实际值] (小端序)
- 原始数据CRC32: [实际值] (小端序)
- 区域大小: 0x00240000 (小端序: 0x00, 0x00, 0x24, 0x00)

bin文件2头部（ER_IROM3.bin）:
- 文件名: "ER_IROM3.bin\0\0\0..." (48字节)
- 文件ID: 0x0001 (小端序: 0x01, 0x00)
- GZIP标志: 0x0000 (小端序: 0x00, 0x00)
- 数据长度: [实际值] (小端序)
- 原始长度: [实际值] (小端序)
- Flash地址: 0x12460000 (小端序: 0x00, 0x00, 0x46, 0x12)
- 数据CRC32: [实际值] (小端序)
- 原始数据CRC32: [实际值] (小端序)
- 区域大小: 0x00680000 (小端序: 0x00, 0x00, 0x68, 0x00)

bin文件3头部（ER_IROM2.bin）:
- 文件名: "ER_IROM2.bin\0\0\0..." (48字节)
- 文件ID: 0x0002 (小端序: 0x02, 0x00)
- GZIP标志: 0x0000 (小端序: 0x00, 0x00)
- 数据长度: [实际值] (小端序)
- 原始长度: [实际值] (小端序)
- Flash地址: 0x12AE0000 (小端序: 0x00, 0x00, 0xAE, 0x12)
- 数据CRC32: [实际值] (小端序)
- 原始数据CRC32: [实际值] (小端序)
- 区域大小: 0x00400000 (小端序: 0x00, 0x00, 0x40, 0x00)

将以上所有数据按顺序连接成一个字节流，然后使用CRC32算法（多项式0xEDB88320）计算CRC值。

具体计算步骤：
1. 初始化CRC为0xffffffff
2. 对每个字节进行处理：
   - crc = (crc >> 8) ^ crc32_table[(crc ^ byte) & 0xFF]
3. 最终结果为crc ^ 0xFFFFFFFF

通过计算得出头部CRC32值为: 0xF16C8B4A

## 5. 注意事项

1. 确保所有bin文件都存在于INI配置文件指定的目录中
2. Flash地址和区域大小需要根据实际硬件配置进行设置
3. 如果启用压缩（GZIP=1），工具会自动进行GZIP压缩处理，但C代码解析端并未适配，不要选择压缩！！！
4. 工具会自动计算并验证CRC32校验值，确保数据完整性
5. 所有数据均使用小端序存储格式
6. bin文件数据在写入前会进行16字节对齐处理，不足部分填充0xFF
7. 服务器端文件夹排列顺序需要高版本在前，以确保设备能正确获取最新版本
8. 上传的固件包文件名必须与所属文件夹名称一致，才能在JSON响应中正确生成对应的下载链接

## 6. OTA固件包解析匹配要求

### 6.1 CRC32算法一致性

固件包解析代码中使用的CRC32算法必须与Python打包脚本完全一致：

```c
// C代码中的CRC32实现
#define CRC32_POLY 0xEDB88320
static uint32_t crc32_table[256];

void init_crc32_table(void)
{
    for (int i = 0; i < 256; i++)
    {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ CRC32_POLY;
            else
                crc >>= 1;
        }
        crc32_table[i] = crc;
    }
}

uint32_t calculate_crc32(const uint8_t *data, size_t length, uint32_t crc)
{
    crc = crc ^ 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++)
    {
        crc = (crc >> 8) ^ crc32_table[(crc ^ data[i]) & 0xFF];
    }
    return crc ^ 0xFFFFFFFF;
}
```

```python
# Python脚本中的CRC32实现
CRC32_POLY = 0xEDB88320
crc32_table = []

def init_crc32_table():
    global crc32_table
    if crc32_table:
        return
    
    for i in range(256):
        crc = i
        for j in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ CRC32_POLY
            else:
                crc >>= 1
        crc32_table.append(crc)

def calculate_crc32(data: bytes, crc: int = 0xffffffff) -> int:
    init_crc32_table()
    crc = crc ^ 0xFFFFFFFF
    for byte in data:
        crc = (crc >> 8) ^ crc32_table[(crc ^ byte) & 0xFF]
    return crc ^ 0xFFFFFFFF
```

这两个实现必须完全一致，否则无法正确验证固件包的完整性。

### 6.2 固件包头部结构匹配

C代码中的`struct firmware_info`结构必须与Python脚本生成的头部结构完全匹配：

```c
// C代码中的固件信息结构
struct firmware_info {
    char name[48];              // 48字节
    uint16_t id;                // 2字节
    uint16_t gzip;              // 2字节
    uint32_t compressed_len;    // 4字节 - 压缩后长度
    uint32_t orig_length;       // 4字节 - 原始长度
    uint32_t addr;              // 4字节
    uint32_t data_crc;          // 4字节
    uint32_t data_oricrc;       // 4字节
    uint32_t region_size;       // 4字节
};
```

```python
# Python脚本中的打包格式
bin_header = struct.pack(
    '<48sHHIIIIII',
    name_bytes,
    bin_file.id,
    gzip_flag,
    bin_file.compressed_len,
    bin_file.orig_len,
    bin_file.addr,
    bin_file.data_crc,
    bin_file.orig_crc,
    bin_file.region_size
)
```

### 6.3 魔数匹配

```c
// C代码中的魔数验证
const uint32_t EXPECTED_MAGIC = 0x5F4F5441; // _OTA
if (magic != EXPECTED_MAGIC)
{
    // 魔数不匹配处理
}
```

```python
# Python脚本中的魔数设置
self.magic = magic  # 默认0x5F4F5441，对应_OTA
```

### 6.4 字节序一致性

C代码和Python脚本都使用小端序（`<`在Python中表示小端序）：

```c
// C代码中使用小端序读取数据
rt_memcpy(&headers_crc, buffer, 4);
rt_memcpy(&magic, buffer + 4, 4);
rt_memcpy(&version, buffer + 8, 4);
rt_memcpy(&bin_num, buffer + 12, 4);
```

```python
# Python脚本中使用小端序打包数据
total_header = struct.pack(
    '<IIII',
    header_crc,
    self.magic,
    self.version,
    len(self.bin_files)
)
```

### 6.5 头部大小匹配

每个bin文件头部大小必须是76字节：

```
48 (文件名) + 2 (ID) + 2 (gzip标志) + 4 (数据长度) + 4 (原始长度) 
+ 4 (地址) + 4 (数据CRC) + 4 (原始CRC) + 4 (区域大小) = 76字节
```

### 6.6 INI配置参数匹配

INI配置文件中的参数会直接影响生成的固件包内容，解析时需要匹配：

```ini
[COMMON]
IMG_FLAG=0x5F4F5441    # 必须与C代码中的EXPECTED_MAGIC匹配
IMG_VER=0x00000134     # 版本号

[APP]
NAME=ER_IROM1.bin      # 文件名，存储在firmware_info.name中
SEL=1                  # 选择标志，影响是否包含在固件包中
GZIP=0                 # 压缩标志，影响gzip字段
IDX=0                  # 文件ID，存储在firmware_info.id中
ADDR=0x12218000        # Flash地址，存储在firmware_info.addr中
REGION_SIZE=0x00240000 # 区域大小，存储在firmware_info.region_size中
```

### 6.7 总体结构匹配

固件包的整体结构必须在C代码解析和Python脚本生成之间保持一致：

1. 总头部(16字节)：头部CRC(4) + 魔数(4) + 版本(4) + 文件数(4)
2. 每个bin文件头部(76字节)：按firmware_info结构排列
3. 每个bin文件数据：按compressed_len字段指定的长度存储

## 7. OTA升级完整使用文档

### 7.1 生成固件包

#### 7.1.1 准备工作

首先，需要准备以下文件：
1. 多个bin文件（如：ER_IROM1.bin、ER_IROM3.bin、ER_IROM2.bin）(bin数量对应头文件中的MAX_FIRMWARE_FILES)
2. INI配置文件

#### 7.1.2 创建INI配置文件

创建一个INI配置文件，例如 [dfu_pan.ini](file:///Volumes/TB/solution2.0/solution2.0/solution/tools/dfu_pan/dfu_pan.ini)：

```ini
[COMMON]
IMG_FLAG=0x5F4F5441    # 魔数，固定为0x5F4F5441（_OTA）
IMG_VER=0x00000134     # 版本号，示例为1.3.4
FILE_PATH=./bin_files  # bin文件所在目录路径

[APP]
NAME=ER_IROM1.bin      # bin文件名
SEL=1                  # 是否选中（1=选中，0=不选中）
GZIP=0                 # 是否压缩（1=压缩，0=不压缩）
IDX=0                  # 文件ID
ADDR=0x12218000        # Flash地址
REGION_SIZE=0x00240000 # 擦除区域大小

[IMG]
NAME=ER_IROM3.bin
SEL=1
GZIP=0
IDX=1
ADDR=0x12460000
REGION_SIZE=0x00680000

[FONT]
NAME=ER_IROM2.bin
SEL=1
GZIP=0
IDX=2
ADDR=0x12AE0000
REGION_SIZE=0x00400000
```

#### 7.1.3 运行打包工具

使用Python脚本生成OTA固件包(按照数字版本命名固件包，方便部署服务器)：

```bash
python dfu_pan_paket.py --ini ./dfu_pan.ini --output ./vx.x.x.bin
```

生成的固件包将包含所有指定的bin文件及其元数据。

### 7.2 部署在服务器

#### 7.2.1 上传固件包

将生成的OTA固件包上传到OTA服务器。根据您的服务器配置，可以使用以下方法之一：

#### 7.2.2 配置服务器

确保服务器配置正确，以便设备可以访问固件包：

文件上传网址：https://ota.sifli.com/browser/
示例：https://ota.sifli.com/browser/xiaozhi/SF32LB52_ULP_NOR_TFT_CO5300/S2_watch_sf32lb52-ulp/v1.3.6?role=zhenpengfu
browser/ 后的路径则是每层文件夹名称 上传v1.3.6.bin(一定要与当前所属文件夹同名才能使得返回json网址产生对应json包)

返回json包网址：https://ota.sifli.com/v2/xiaozhi/SF32LB52_ULP_NOR_TFT_CO5300/S2_watch_sf32lb52-ulp （browser修改为v2）
返回的"zippath": 是一个模版，c代码中解析下载已对返回的模版网址进行拼接访问

### 7.3 OTA独立程序执行解析烧录

#### 7.3.1 设备端OTA流程

设备端OTA升级流程包括以下步骤：

##### 7.3.1.1 查询可用版本

设备首先向OTA服务器查询可用的固件版本：

```c
// 查询可用版本
dfu_pan_query_versions(); 
```

该函数会：
1. 向服务器发送GET请求
2. 解析服务器返回的JSON数据
3. 提取服务器至多5个可用版本信息（文件夹顺序需要高版本在前）
4. 将版本信息存储到Flash中

##### 7.3.1.2 进入bootloader
路径 sdk/example/boot_loader
执行检查版本函数之后，版本信息会存储到Flash中，其中bootloader会检查更新标志位选择是否进入OTA引导程序

**注意：** 
- DFU_PAN_LOADER_START_ADDR地址在 `dfu_pan_macro.h`中默认定义成0xFFFFFFFF，需要检查相应使用的开发版ptab.json中是否定义了DFU_PAN_LOADER段。
- 在boot_loader会判断 DFU_PAN_LOADER_START_ADDR != DFU_PAN_FLASH_UNINIT_32（0xFFFFFFFF）才会进行镜像跳转进入到ota引导程序

```c
run_img(DFU_PAN_LOADER_START_ADDR);
```

##### 7.3.1.3 OTA引导程序
路径：sdk/example/dfu_pan
OTA引导程序有一个简约UI界面，主要功能是下载固件包并解析，并引导用户进行固件升级
点击版本列表按钮即列出服务器返回的版本列表（查询可用版本）

点击对应列表版本号，进入下载固件，将执行：
```c
// 启动OTA升级线程
dfu_pan_create_worker_thread(version_index);
```

##### 7.3.1.4 下载并解析固件包

用户选择要升级的版本后，系统会启动OTA升级流程：
在OTA工作线程中：

```c
// 下载并解析固件包
dfu_pan_parse_package_from_url(version_list[version_index].zippath);
```

该函数会：
1. 从URL下载固件包
2. 解析固件包头部信息
3. 验证魔数和头部CRC32
4. 解析每个bin文件的头部信息
5. 将固件信息存储到Flash中
6. 逐个下载并烧录bin文件数据到Flash
7. 验证每个bin文件的CRC32

##### 7.3.1.5 烧录过程详解

固件包解析和烧录过程如下：

1. **解析总头部**：
   - 读取16字节的总头部
   - 验证魔数是否为0x5F4F5441 ('_OTA')
   - 读取版本号和bin文件数量

2. **解析每个bin文件头部**：
   - 依次读取每个bin文件的76字节头部
   - 提取文件名、ID、压缩标志、长度、地址、CRC等信息
   - 将这些信息存储到Flash中

3. **烧录bin文件数据**：
   - 按顺序读取每个bin文件的数据
   - 擦除目标Flash区域
   - 将数据写入Flash
   - 显示烧录进度

4. **验证CRC**：
   - 计算烧录数据的CRC32
   - 与头部中存储的CRC32进行比较
   - 如果不匹配，报告错误并停止升级

#### 7.3.2 错误处理

OTA过程中可能出现的错误及处理方式：

1. **网络错误**：
   - 重试下载
   - 如果多次重试失败，报告网络错误

2. **CRC校验失败**：
   - 停止升级过程
   - 报告CRC校验错误

3. **Flash操作失败**：
   - 报告Flash写入/擦除错误
   - 尝试回滚操作

4. **固件包格式错误**：
   - 验证魔数
   - 检查文件数量和大小
   - 报告格式错误

#### 7.3.3 升级完成处理

升级完成后：
1. 清除升级标志
2. 显示升级成功/失败消息
3. 根据需要重启设备

```c
// 升级成功后的处理
if (result == 0) {
    dfu_pan_clear_update_flags();  // 清除更新标志
    rt_thread_mdelay(50);
    dfu_pan_ui_update_message(UI_MSG_SHOW_SUCCESS_POPUP, NULL);  // 显示成功消息
} else {
    dfu_pan_ui_update_message(UI_MSG_SHOW_FAILURE_POPUP, NULL);  // 显示失败消息
}
```