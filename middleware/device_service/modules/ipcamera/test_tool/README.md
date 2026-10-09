# extract_h264.py

从record文件中提取H.264二进制数据的工具。

## 功能

- 自动识别IP Camera话题（格式：`/sensor/ipcamera/h264/{ip_address}`）
- 提取H.264二进制数据到独立的.h264文件
- 支持多路流数据分离保存
- 提供详细的提取统计信息

## 使用方法

```bash
# 基本用法
python3 extract_h264.py <record_file>

# 指定输出目录
python3 extract_h264.py <record_file> --output-dir /path/to/output
```

## 参数

- `record_file`: record文件路径
- `--output-dir`, `-o`: 输出目录（可选，默认使用输入文件所在目录）

## 输出文件

- 文件名格式：`{ip_address}_{timestamp}_record.h264`
- 示例：`172_20_65_193_20250708_145242_record.h264`

## 示例

```bash
# 提取record文件中的H.264数据
python3 extract_h264.py /home/airos/data/record.record

# 输出示例
读取文件: /home/airos/data/record.record
输出目录: /home/airos/data
发现话题: ['/sensor/ipcamera/h264/172_20_65_193', '/sensor/ipcamera/h264/172_20_65_194']
创建文件: /home/airos/data/172_20_65_193_20250708_145242_record.h264
创建文件: /home/airos/data/172_20_65_194_20250708_145242_record.h264
已提取 100 帧，2.5 MB
已提取 200 帧，5.1 MB
话题 /sensor/ipcamera/h264/172_20_65_193: 提取 1500 帧，2.5 MB -> 172_20_65_193_20250708_145242_record.h264
话题 /sensor/ipcamera/h264/172_20_65_194: 提取 1480 帧，2.3 MB -> 172_20_65_194_20250708_145242_record.h264

提取完成!
总帧数: 2980
总字节: 5,038,592 (4.81 MB)
输出目录: /home/airos/data
```

## 依赖

- Python 3.6+
- Apollo Cyber RT环境
- protobuf库

