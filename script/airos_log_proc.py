import os
import re
import shutil
from datetime import datetime

# 日志模块的名称
model_name = [
                "mainboard", "mainboard_default",
                "airos_app", "v2x_codec",  "rsu_service", "traffic_light_service", "airos_app_framework",
                "mec_service",
                "camera_cictci_service",
                "rsap",
                "om",  "bs_angle_offset", "bs_spat_src_data", "bs_v2x_data",
                "om_cloud", "om_inter", "om_mec", "om_camera", "om_radar", "om_mqtt", "om_device_status",  "alarm_om_camera", "alarm_om_radar",
                "ccindex_inter", "ccindex_cloud",  "radar_static", "radar_tc", "radar_trafficmetrics",  "radar_mqtt", 
                "ccindex","ccindex_mqtt", "count_radar_trafficmetrics",
                "radar_static_post", "radar_static_get",
                "mec_in", "om_monitor",
                "cyber_monitor", "v2x_map_service", "rtsp_tool", "cyber_py", "cyber_recorder", "ccindex", "om_check", "om_check_data", "om_check_link", "spill_reporter",
                "monitor_mec_radar_point_cloud"
              ]

# 文件夹路径
log_dir = '/home/airos/log'
max_log_num = 5
max_log_dir_size_gb = 20  # 最大日志目录大小（GB）

# 日志文件路径
log_file_path = os.path.join(log_dir, 'airos_log_proc.log')

# 获取所有日志文件
all_files = os.listdir(log_dir)

# 字典来存储每个前缀对应的文件列表
files_by_prefix = {prefix: [] for prefix in model_name}

# 文件名正则表达式模式,匹配如：v2x_codec.log.INFO.20240815-014901.386330 
# 或者 v2x_codec.log.INFO.20240815-054948.386330-20240815072001.gz 类型的文件
file_pattern = re.compile(r'(\w+)\.log\.INFO\.(\d{8}-\d{6})(?:\.\d+)?(?:-\d{8}\d{6})?(?:\.gz)?')

# 获取当前时间戳
def get_timestamp():
    return datetime.now().strftime('%Y-%m-%d %H:%M:%S')

# 获取目录大小（GB）
def get_directory_size_gb(directory):
    """获取目录大小，返回GB为单位"""
    total_size = 0
    try:
        for dirpath, dirnames, filenames in os.walk(directory):
            for filename in filenames:
                filepath = os.path.join(dirpath, filename)
                if os.path.exists(filepath):
                    total_size += os.path.getsize(filepath)
        return total_size / (1024**3)  # 转换为GB
    except Exception as e:
        print(f"Error calculating directory size: {e}")
        return 0

# 删除其他文件中最大的一个
def delete_largest_other_file(directory, file_pattern):
    """删除其他文件中最大的一个，返回删除的文件大小（GB）"""
    other_files = []
    
    try:
        for filename in os.listdir(directory):
            filepath = os.path.join(directory, filename)
            
            # 跳过子目录
            if os.path.isdir(filepath):
                continue
                
            # 跳过匹配正则表达式的文件
            if file_pattern.match(filename):
                continue
                
            # 跳过当前正在写入的日志文件
            if filename == 'airos_log_proc.log':
                continue
                
            # 获取文件大小
            if os.path.exists(filepath):
                file_size = os.path.getsize(filepath)
                other_files.append((filepath, filename, file_size))
        
        if not other_files:
            return 0
            
        # 按文件大小排序，删除最大的
        other_files.sort(key=lambda x: x[2], reverse=True)
        largest_file = other_files[0]
        
        file_size_gb = largest_file[2] / (1024**3)
        print(f"Deleting largest other file: {largest_file[1]} ({file_size_gb:.2f} GB)")
        
        os.remove(largest_file[0])
        return file_size_gb
        
    except Exception as e:
        print(f"Error deleting other file: {e}")
        return 0

# 打开日志文件以写入
with open(log_file_path, 'w') as log_file:
    log_file.write(f"{get_timestamp()} - Log rotation started.\n")
    
    # 检查目录大小
    current_size_gb = get_directory_size_gb(log_dir)
    log_file.write(f"{get_timestamp()} - Current log directory size: {current_size_gb:.2f} GB\n")
    
    # 如果目录大小超过限制，删除其他文件中最大的
    if current_size_gb > max_log_dir_size_gb:
        log_file.write(f"{get_timestamp()} - Directory size {current_size_gb:.2f} GB exceeds limit {max_log_dir_size_gb} GB\n")
        log_file.write(f"{get_timestamp()} - Starting cleanup of other files...\n")
        
        deleted_size_total = 0
        while current_size_gb > max_log_dir_size_gb:
            deleted_size = delete_largest_other_file(log_dir, file_pattern)
            if deleted_size == 0:
                log_file.write(f"{get_timestamp()} - No more other files to delete\n")
                break
                
            deleted_size_total += deleted_size
            current_size_gb = get_directory_size_gb(log_dir)
            log_file.write(f"{get_timestamp()} - Deleted file, current size: {current_size_gb:.2f} GB\n")
            
            # 防止无限循环
            if deleted_size_total > 100:  # 如果删除了超过100GB，停止
                log_file.write(f"{get_timestamp()} - Warning: Deleted over 100GB, stopping cleanup\n")
                break
        
        log_file.write(f"{get_timestamp()} - Cleanup completed, deleted {deleted_size_total:.2f} GB total\n")
    
    # 遍历所有文件，按前缀分组
    for file in all_files:
        match = file_pattern.match(file)
        if match:
            prefix = match.group(1)
            timestamp = match.group(2)
            if prefix in files_by_prefix:
                files_by_prefix[prefix].append((file, timestamp))
    
    # 处理每个前缀的日志文件
    for prefix, files in files_by_prefix.items():
        # 如果该前缀有日志文件
        if files:
            total_files = len(files)
            # 根据时间戳排序
            files.sort(key=lambda x: datetime.strptime(x[1], '%Y%m%d-%H%M%S'), reverse=True)
            # 保留最新的几个文件
            files_to_keep = files[:max_log_num]
            files_to_delete = set(file for file, _ in files) - set(file for file, _ in files_to_keep)
            
            log_file.write(f"{get_timestamp()} - Prefix {prefix}: {total_files} files found.\n")
            log_file.write(f"{get_timestamp()} - Prefix {prefix} - Files to keep ({len(files_to_keep)}):\n")
            for file, _ in files_to_keep:
                log_file.write(f"  {file}\n")
            log_file.write(f"{get_timestamp()} - Prefix {prefix} - Files to delete ({len(files_to_delete)}):\n")
            for file in files_to_delete:
                log_file.write(f" {get_timestamp()} - Remove {file}\n")
                os.remove(os.path.join(log_dir, file))
    
    # 最终检查目录大小
    final_size_gb = get_directory_size_gb(log_dir)
    log_file.write(f"{get_timestamp()} - Final log directory size: {final_size_gb:.2f} GB\n")
    log_file.write(f"{get_timestamp()} - Log rotation completed.\n")
