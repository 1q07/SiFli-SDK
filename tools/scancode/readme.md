# 许可证和版权检查器

该脚本根据提供的配置文件和 `scancode-toolkit` 的JSON输出，分析源代码文件以验证许可证和版权信息。

## 概述

`license_check.py` 脚本处理 `scancode-toolkit` 工具生成的 JSON 输出，以确保所有被扫描的文件都符合在 YAML 配置文件中定义的许可和版权规则。它主要检查以下内容：

*   缺失许可证
*   无效或未知的许可证
*   缺失版权声明

脚本会生成一份包含所有违规项的报告。

## 依赖项

*   Python 3
*   [scancode-toolkit](https://github.com/nexB/scancode-toolkit)
*   PyYAML (`pip install pyyaml`)

## 配置文件

脚本的行为通过一个 YAML 文件 (例如, `license_config.yml`) 进行配置。

```yaml
license:
  main: apache-2.0
  additional: 
    - MIT
  report_invalid: true
  report_unknown: true
  report_missing: true
  category: Permissive
copyright:
  check: true
exclude:
  extensions:
    - yml
    - yaml
    - html
    - rst
    - conf
    - cfg
    - config
    - bat
  special_file:
    - SConscript
    - SConstruct
    - Kconfig
    - rtconfig.h
  langs:
    - HTML
```

### 配置选项

*   **`license`**:
    *   `main`: 主要可接受的许可证 (例如, `apache-2.0`)。
    *   `additional`: 其他可接受的许可证列表。
    *   `report_invalid`: (布尔值) 是否报告存在无效许可证（未在 `main` 或 `additional` 列表中）的文件。
    *   `report_unknown`: (布尔值) 是否报告包含未知 SPDX 许可证标识符的文件。
    *   `report_missing`: (布尔值) 是否报告未检测到许可证的文件。
    *   `category`: 可接受的许可证类别。
*   **`copyright`**:
    *   `check`: (布尔值) 启用或禁用对缺失版权声明的检查。
*   **`exclude`**:
    *   `extensions`: 要忽略的文件扩展名列表。
    *   `special_file`: 要忽略的特定文件名列表。
    *   `langs`: 要忽略的编程语言列表。

## 使用方法

1.  **使用 `scancode-toolkit` 扫描您的项目**:
    ```bash
    scancode -clipeu --license --license-text --license-text-diagnostics --classify --summary --verbose /path/to/your/project --processes 4 --json "scancode.json" --html "scancode.html"
    ```
2.  **运行许可证检查脚本**:
    ```bash
    python license_check.py \
        -c license_config.yml \
        -s scancode_output.json \
        -f /path/to/your/project \
        -o license_report.txt
    ```

### 命令行参数

*   `-c, --config_file`: YAML 配置文件的路径。
*   `-s, --scancode-output`: `scancode-toolkit` 生成的 JSON 输出文件的路径。
*   `-f, --scanned_files`: 被扫描的项目目录的路径。
*   `-o, --output_file`: 用于写入违规报告的输出文件路径。

## 输出

*   如果发现违规项，脚本将打印 "Copyright check FAILED."，将违规详情写入指定的输出文件，并以退出码 `1` 结束。
*   如果没有发现违规项，脚本将打印 "Copyright check PASSED." 并以退出码 `0` 结束。
