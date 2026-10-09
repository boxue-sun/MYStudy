#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
从record文件中提取H.264二进制数据
使用Apollo Cyber RT record API读取record文件
"""

import os
import sys
import argparse
from datetime import datetime

try:
    # 导入Apollo Cyber RT record API
    from cyber.python.cyber_py3 import record
    import ipcamera_pb2
except ImportError as e:
    print(f"Error: 无法导入必要的模块: {e}")
    print("请确保Apollo Cyber RT环境已正确配置")
    sys.exit(1)


def extract_h264_data(record_file, output_dir=None):
    """提取H.264二进制数据，自动匹配IP Camera话题
    
    Args:
        record_file: record文件路径
        output_dir: 输出目录路径（如果为None，则使用输入文件所在目录）
    """
    if not os.path.exists(record_file):
        print(f"Error: Record文件 {record_file} 不存在")
        return False
    
    # 如果未指定输出目录，使用输入文件所在目录
    if output_dir is None:
        output_dir = os.path.dirname(os.path.abspath(record_file))
        if not output_dir:  # 如果是当前目录
            output_dir = "."
    
    print(f"读取文件: {record_file}")
    print(f"输出目录: {output_dir}")
    
    # 确保输出目录存在
    os.makedirs(output_dir, exist_ok=True)
    
    # 统计信息
    total_extracted_frames = 0
    total_extracted_bytes = 0
    topic_stats = {}
    
    try:
        # 使用Apollo Cyber RT record reader
        reader = record.RecordReader(record_file)
        
        # 获取所有话题列表
        channels = reader.get_channellist()
        print(f"发现话题: {channels}")
        
        # 过滤IP Camera话题
        ipcamera_topics = []
        for channel in channels:
            if channel.startswith("/sensor/ipcamera/h264/"):
                ipcamera_topics.append(channel)
        
        if not ipcamera_topics:
            print("警告: 未发现IP Camera话题")
            return False
        
        print(f"发现IP Camera话题: {ipcamera_topics}")
        
        # 为每个话题创建文件
        topic_files = {}
        # 生成时间戳
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        
        for topic in ipcamera_topics:
            # 提取IP地址部分
            ip_part = topic.split("/")[-1]  # 获取最后一段作为IP地址
            filename = f"{ip_part}_{timestamp}_record.h264"
            filepath = os.path.join(output_dir, filename)
            
            topic_files[topic] = open(filepath, 'wb')
            topic_stats[topic] = {"frames": 0, "bytes": 0}
            print(f"创建文件: {filepath} 用于话题: {topic}")
        
        # 读取所有消息
        for channel_name, msg, data_type, timestamp in reader.read_messages():
            # 只处理IP Camera话题
            if channel_name not in topic_files:
                continue
            
            try:
                # 解析protobuf消息
                compressed_image = ipcamera_pb2.CompressedImage()
                compressed_image.ParseFromString(msg)
                
                # 检查是否有data字段
                if compressed_image.HasField('data'):
                    # 获取对应的文件
                    out_f = topic_files[channel_name]
                    h264_data = compressed_image.data
                    out_f.write(h264_data)
                    
                    # 更新统计信息
                    topic_stats[channel_name]["frames"] += 1
                    topic_stats[channel_name]["bytes"] += len(h264_data)
                    total_extracted_frames += 1
                    total_extracted_bytes += len(h264_data)
                    
                    # 显示进度
                    if total_extracted_frames % 100 == 0:
                        print(f"已提取 {total_extracted_frames} 帧，{total_extracted_bytes/1024/1024:.2f} MB")
            
            except Exception as e:
                print(f"解析消息失败: {e}")
                continue
        
        # 关闭所有文件
        for topic, file_obj in topic_files.items():
            file_obj.close()
            ip_part = topic.split("/")[-1]
            filename = f"{ip_part}_{timestamp}_record.h264"
            stats = topic_stats[topic]
            print(f"话题 {topic}: 提取 {stats['frames']} 帧，{stats['bytes']/1024/1024:.2f} MB -> {filename}")
    
    except Exception as e:
        print(f"Error: {e}")
        return False
    
    print(f"\n提取完成!")
    print(f"总帧数: {total_extracted_frames}")
    print(f"总字节: {total_extracted_bytes:,} ({total_extracted_bytes/1024/1024:.2f} MB)")
    print(f"输出目录: {output_dir}")
    
    return True


def main():
    parser = argparse.ArgumentParser(description='从record文件提取H.264二进制数据，自动匹配IP Camera话题')
    parser.add_argument('record_file', help='record文件路径')
    parser.add_argument('--output-dir', '-o', help='输出目录 (默认: 使用输入文件所在目录)')
    
    args = parser.parse_args()
    
    if not extract_h264_data(args.record_file, args.output_dir):
        sys.exit(1)


if __name__ == '__main__':
    main() 