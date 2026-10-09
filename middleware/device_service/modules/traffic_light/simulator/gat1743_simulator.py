
# -*- coding: utf-8 -*-
# import sys
import socket
import time
# import datetime
import logging
import sched
import threading
import json
import math

logging.basicConfig(
    level=logging.INFO,
    filename='./simulator.log',
    filemode='w',
    format='%(asctime)s - %(filename)s[line:%(lineno)d] - %(levelname)s: %(message)s'
)

logger = logging.getLogger(__name__)


class Telegrapher(object):
    def __init__(self):
        self.socket_fd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.addr_local = ('', 10023)
        self.socket_fd.bind(self.addr_local)
        self.addr_remote = ('127.0.0.1', 10050)
        self.input_fds = [self.socket_fd]
        self.output_fds = []

    def send(self, data):
        logger.info("send : %s", data.hex())
        self.socket_fd.sendto(data, self.addr_remote)

    def recv(self):
        data, addr = self.socket_fd.recvfrom(2048)
        logger.info("recv <%s> : %s", str(addr), data.hex())
        return data


class GBT1743Protocol(object):
    def __init__(self):
        self.telegrapher = Telegrapher()
        self._schedule = sched.scheduler(time.time, time.sleep)
        self._control_mode = 5
        # 下一个步色
        self._next_color_step = dict()  # channelId: ((color, time))
        # 一个周期内的融合步色（闪灯转为亮灯，相邻灯色结合）
        self._channels_countdown_duty = dict()  # channelId: ((color1, time1), (color2, time2), (color3, time3))
        # 一个周期内的步色
        self._channels_color_duty = dict()  # channelId: ((color1, time1), (color2, time2), (color3, time3))
        self._progress_seconds = 1  # 周期进行的秒数
        self._progress_total = 0  # 周期总秒数
        self._frame_period_broadcast = bytearray()  # 周期广播帧（由于周期固定，因此预先构造号周期内容）
        self._frame_period_response = bytearray()  # 周期响应帧
        self._light_group_type = {
            0: 0,
            1: 0x25,
            2: 0x19,
            3: 0x16,
            11: 0x35,
            12: 0x1D,
            13: 0x17,
        }

        self._altitude = 0  # 海拔
        self._administrative_region = 110115  # 行政区域代码（参考身份证前6位）
        self._longitude = 1165013145
        self._latitude = 397948989
        self.scheme = dict()  # 信号机步色周期方案

        self._schedule.enter(0, 1, self.schedule_update_signal_state, {})

    def _check_scheme(self, scheme):
        for angle, angle_value in scheme.items():
            for channel, channel_list in angle_value.items():
                periods_tuple = channel_list[1]
                total_time = int(0)
                for one_period in periods_tuple:
                    total_time += one_period[1]

                if self._progress_total == 0:
                    self._progress_total = total_time
                else:
                    if self._progress_total != total_time:
                        logger.error("channel [%d] color steps total count is invalid", channel)
                        return False
        return True

    def _load_scheme(self, scheme_file_path):
        with open(scheme_file_path) as fp:
            json_root = json.load(fp)
        tmp_scheme = dict()

        for angle_key, angle_val in json_root.items():
            tmp_scheme[int(angle_key)] = dict()
            for id_key, id_val in angle_val.items():
                light_type = id_val["type"]
                light_step_list = id_val["step"]
                light_step_list_new = list()
                for one_step in light_step_list:
                    light_color = int(one_step[0])
                    light_duration = int(one_step[1])
                    light_step_list_new.append((light_color, light_duration))
                tmp_scheme[int(angle_key)][int(id_key)] = (light_type, tuple(light_step_list_new))
        if self._check_scheme(tmp_scheme) is False:
            return None
        else:
            return tmp_scheme

    def channels_duty_init(self):
        tmp_scheme = dict()
        tmp_scheme.update(self.scheme)
        # 获取所有通道的原始步色信息（初始化灯色任务周期）
        for angle, angle_dict in tmp_scheme.items():
            for channel, channel_tuple in angle_dict.items():
                steps_tuple = channel_tuple[1]
                self._channels_color_duty[channel] = steps_tuple
        # 同色合并（初始化倒计时周期）
        tmp_color_duty = dict()
        tmp_color_duty.update(self._channels_color_duty)
        for channel, steps_tuple in tmp_color_duty.items():
            # 元祖转为列表
            tmp_step_list = list()
            for one_step in steps_tuple:
                tmp_step_list.append(list(one_step))
            # 闪灯转为亮灯
            for one_step in tmp_step_list:
                if one_step[0] > 10:
                    one_step[0] -= 10
            # 同色合并
            new_tmp_step_list = list()
            index = int(0)
            tmp_one_step = list()
            while index < len(tmp_step_list):
                if len(tmp_one_step) == 0:
                    tmp_one_step = tmp_step_list[index]
                else:
                    if tmp_one_step[0] != tmp_step_list[index][0]:
                        new_tmp_step_list.append(tuple(tmp_one_step))
                        tmp_one_step = tmp_step_list[index]
                    else:
                        tmp_one_step[1] += tmp_step_list[index][1]
                index += 1
            new_tmp_step_list.append(tuple(tmp_one_step))

            self._channels_countdown_duty[channel] = tuple(new_tmp_step_list)

        print("<color duty> : \n", self._channels_color_duty)
        print("<countdown duty> : \n", self._channels_countdown_duty)

    def period_frame_init(self):
        self._frame_period_broadcast = self._build_frame_period(True)
        self._frame_period_response = self._build_frame_period(False)
        # print("period frame : \n", self._frame_period.hex())

    def task_process_requests(self):
        while True:
            request = self.telegrapher.recv()
            package = self._parse_frame(request)
            if package is None:
                continue

            if package["opt_type"] != 0x80:
                logger.warning("received frame invalid, opt_type = %s", package["opt_type"])
                continue

            if package["obj_flag"] == 0x0103:  # 查询红绿灯
                frame = self._build_frame_signal_light_state(False)
                self.telegrapher.send(frame)
            elif package["obj_flag"] == 0x0301:  # 查询当前周期方案
                self.telegrapher.send(self._frame_period_response)
            elif package["obj_flag"] == 0x0302:  # 查询下一个周期方案
                logger.warning("received frame invalid, obj_flag = %s", package["obj_flag"])
            else:
                logger.warning("received frame invalid, obj_flag = %s", package["obj_flag"])

    def schedule_update_signal_state(self):
        self._send_light_state_change_broadcast()  # 每过一秒发送一次灯色状态信息
        if self._progress_seconds == 1:
            self.telegrapher.send(self._frame_period_broadcast)  # 周期起始时发出当前周期广播

        if self._progress_seconds == self._progress_total:
            self._progress_seconds = 1
        else:
            self._progress_seconds += 1
        self._schedule.enter(1, 1, self.schedule_update_signal_state, {})

    def start_run(self, scheme_file, mode=5):
        '''
        :param scheme_file: 方案文件路径
        :param mode: 控制模式，取值如下
            # 1：黄闪控制
            # 2：多时段控制
            # 3：手动控制
            # 4：感应控制
            # 5：无电缆协调控制
            # 6：单点优化控制
            # 7：公交信号优先
            # 8：紧急事件优先
            # 9：其他
        :return:
        '''
        self._control_mode = mode
        # 加载方案，方案字段说明参考【说明A】
        self.scheme = self._load_scheme(scheme_file)
        if self.scheme is None:
            raise "scheme is invalid"
        print("scheme : \n%s" % str(self.scheme))

        self.period_frame_init()  # 初始化周期报文帧
        self.channels_duty_init()  # 初始化色步倒计时方案
        # 接收和处理请求的任务
        thread_task = threading.Thread(target=self.task_process_requests)
        thread_task.setDaemon(True)
        # thread_task.daemon = True;
        thread_task.start()

        logger.info("signal device start run")
        self._schedule.run()  # 信号机开始进行倒计时

    def _escape_char_frame(self, frame_ba):
        frame_ba_new = bytearray()
        for item in frame_ba:
            if item == 0xC0:
                frame_ba_new.append(0xDB)
                frame_ba_new.append(0xDC)
            elif item == 0xDB:
                frame_ba_new.append(0xDB)
                frame_ba_new.append(0xDD)
            else:
                frame_ba_new.append(item)
        return frame_ba_new

    def _reverse_escape_char_frame(self, frame_ba):
        frame_ba_new = bytearray()
        ba_c0 = bytearray([0xDB, 0xDC])
        ba_db = bytearray([0xDB, 0xDD])
        i = 0
        while i < len(frame_ba):
            if frame_ba[i: i + 2] == ba_c0:
                frame_ba_new.append(0xC0)
                i += 2
            elif frame_ba[i: i + 2] == ba_db:
                frame_ba_new.append(0xDB)
                i += 2
            else:
                frame_ba_new.append(frame_ba[i])
                i += 1
        return frame_ba_new

    def _crc16(self, data_ba):
        crc_in = 0xFFFF
        crc_poly = 0x8005

        for item in data_ba:
            crc_in ^= item << 8
            crc_in &= 0xFFFF
            for i in range(8):
                if (crc_in & 0x8000) == 0x8000:
                    crc_in = (crc_in << 1) ^ crc_poly
                else:
                    crc_in = crc_in << 1
                crc_in &= 0xFFFF
        return crc_in

    def _build_frame(self, opt_type, obj_flag, data):
        # 链路码(2B)：数据接收链路，用于不同链路数据转发，按位取值，
        #     Bit0：路侧车联网通信设备
        #     Bit1：交通管控与信息服务平台
        #     Bit2：除信号机以外的其他路侧交通管控设备
        #     Bit3：信号机
        body = bytearray([4, 0])
        # 发送方标识/接收方标识: 行政区划代码(3B) + 类型(2B) + 编号(2B)
        #    行政区划代码：包含省、市、县级，6位数字，取值应符合GB/T 2260的规定
        # 发送/接收方类别，按位取值：
        #    Bit15：路侧车联网通信设备
        #    Bit14：交通管控与信息服务平台
        #    Bit13：除信号机以外的其他路侧交通管控设备
        #    Bit12：信号机
        #    Bit11~Bit0：保留
        # 设备或平台的唯一编号，其中，广播方式接收方标识取值65535（0xFFFF）
        region_ba = self._administrative_region.to_bytes(3, byteorder='little')
        body += region_ba
        body += bytearray([0, 0x10, 0xFF, 0xFF])
        body += region_ba
        body += bytearray([0, 0x20, 0xFF, 0xFF])
        # 时间戳： 前4个字节为UTC时间，后2个字节保留，用于时间扩展；
        # current_time_ms = int(time.time())
        # time_stamp_ba = current_time_ms.to_bytes(4, byteorder='little')
        # body += time_stamp_ba
        # body += bytearray([0, 0])
        cur_times = math.modf(time.time())
        current_time_s = int(cur_times[1])
        current_time_ms = int(cur_times[0] * 1000)
        time_stamp_s_ba = current_time_s.to_bytes(4, byteorder='little')
        body += time_stamp_s_ba
        time_stamp_ms_ba = current_time_ms.to_bytes(2, byteorder='little')
        body += time_stamp_ms_ba
        # 生存时间(1B)（TTL）：时间戳之后数据表有效的时间，单位为秒（s）
        body.append(0x3C)
        # 协议版本(1B)：协议的具体版本号，取值0x10
        body.append(0x10)
        # 操作类型(1B)：数据表的查询、设置、应答等操作类型
        #    0x80	查询请求	发送查询消息
        #    0x81	设置请求	保留，用于发送设置消息
        #    0x82	主动上报	主动上报数据
        #    0x83	查询应答	对查询请求的应答
        #    0x84	设置应答	保留，用于对设置请求的应答
        #    0x85	主动上报应答	保留，对主动上报的应答
        #    0x86	出错应答	保留，用于接收到的数据包存在错误应答
        #    0x87	信息广播	广播数据
        body.append(opt_type)
        # 对象标识(2B)：数据对象唯一编码，对象分类编码（1B）+对象名称编码（1B）
        #    信号机运行状态/01	0x0101	描述信号机当前运行状态。如：正常工作状态，未工作状态，故障状态等
        #    信号控制方式/02	0x0102	描述信号机当前控制方式，如：黄闪控制、多时段控制、手动控制、感应控制、无电缆协调控制、单点优化控制、公交信号优先、紧急事件优先等
        #    信号灯灯色状态/03	0x0103	描述当前信号灯组的灯色和剩余时间
        #    车道功能状态/01	0x0201	描述可变车道当前车道功能、是否过渡状态等
        #    车道/匝道控制信息/02	0x0202	描述当前车道/匝道关闭或开启信息
        #    当前信号方案色步信息/01	0x0301	描述信号机当前运行方案的灯色及时长
        #    下一个周期信号方案色步信息/02	0x0302	描述信号机下一个周期将要运行新方案的灯色及时长
        #    交通流信息/01	0x0401	描述各车道交通流量、平均车速、排队长度等
        #    交通运行状态信息/02	0x0402	描述各车道的交通运行状态
        #    车辆运行状态信息/01	0x0501	描述行驶车辆当前的位置坐标、速度、车头方向角等
        #    交通事件信息/02	0x0502	描述车辆上报的交通事故、路面障碍等
        body += obj_flag.to_bytes(2, byteorder='little')
        # 签名标记：标记数据表内容是否具有“签名证书”字
        #    Bit0取值：0-无签名证书字段，1-有签名证书字段
        #    Bit1~Bit7保留
        body.append(0)
        # 保留(3B)：用于数据表扩展的数据标识或内容定义
        body += bytearray([0, 0, 0])
        # 消息内容
        body += data
        # CRC16校验码(2B)
        crc16 = self._crc16(body)
        body += crc16.to_bytes(2, byteorder='little')

        body = self._escape_char_frame(body)  # 转义字符

        frame = bytearray()
        frame.append(0xC0)  # 帧头
        frame += body
        frame.append(0xC0)  # 帧尾
        return frame

    def _parse_frame(self, frame):
        # 0. 解析帧头帧尾并校验
        head = int()  # 帧头1
        head = head.from_bytes(frame[0:1], byteorder='little')
        tail = int()  # 帧尾1
        tail = tail.from_bytes(frame[-1:], byteorder='little')
        if head != 0xC0 or tail != 0xC0:
            logger.warning("head or tail invalid")
            return None

        # 1. 去转义字符
        frame = self._reverse_escape_char_frame(frame)
        # 2. 解析其它字段
        link_code = int()  # 链路码2
        link_code = link_code.from_bytes(frame[1:3], byteorder='little')
        sender_code = frame[3:10]  # 发送方标识7
        sender_region_code = int()  ## 行政区代码3
        sender_region_code = sender_region_code.from_bytes(sender_code[0:3], byteorder='little')
        sender_type = int()  ## 类型2
        sender_type = sender_type.from_bytes(sender_code[3:5], byteorder='little')
        sender_id = int()  ## 编码2
        sender_id = sender_id.from_bytes(sender_code[5:7], byteorder='little')
        receiver_code = frame[10:17]  # 接收方标识7
        receiver_region_code = int()  ## 行政区代码3
        receiver_region_code = receiver_region_code.from_bytes(receiver_code[0:3], byteorder='little')
        receiver_type = int()  ## 类型2
        receiver_type = receiver_type.from_bytes(receiver_code[3:5], byteorder='little')
        receiver_id = int()  ## 编码2
        receiver_id = receiver_id.from_bytes(receiver_code[5:7], byteorder='little')
        time_stamp = int()  # 时间戳6
        time_stamp = time_stamp.from_bytes(frame[17:23], byteorder='little')
        ttl = int()  # 生存时间1
        ttl = ttl.from_bytes(frame[23:24], byteorder='little')
        version = int()  # 协议版本1
        version = version.from_bytes(frame[24:25], byteorder='little')
        opt_type = int()  # 操作类型1
        opt_type = opt_type.from_bytes(frame[25:26], byteorder='little')
        obj_flag = int()  # 对象标识2
        obj_flag = obj_flag.from_bytes(frame[26:28], byteorder='little')
        sign_flag = int()  # 签名标记1
        sign_flag = sign_flag.from_bytes(frame[28:29], byteorder='little')
        rfu = frame[29:32]  # 保留3
        data = frame[32:-3]  # 数据域
        crc16 = int()  # CRC16
        crc16 = crc16.from_bytes(frame[-3:-1], byteorder='little')
        package = {
            "head": head,
            "link_code": link_code,
            "sender_code": {
                "sender_region_code": sender_region_code,
                "sender_type": sender_type,
                "sender_id": sender_id,
            },
            "receiver_code": {
                "receiver_region_code": receiver_region_code,
                "receiver_type": receiver_type,
                "receiver_id": receiver_id
            },
            "time_stamp": time_stamp,
            "ttl": ttl,
            "version": version,
            "opt_type": opt_type,
            "obj_flag": obj_flag,
            "sign_flag": sign_flag,
            "rfu": rfu,
            "data": data,
            "crc16": crc16,
            "tail": tail
        }
        # 3. 检查校验码
        my_crc16 = self._crc16(frame[1:-3])
        if my_crc16 != crc16:
            logger.warning("crc16 invalid")
            return None

        return package

    def _build_frame_signal_light_state(self, is_broadcast):
        """
        构造灯色状态信息帧
        """
        lights_info = bytearray()
        for angle, angle_dict in self.scheme.items():
            one_grou_angle = angle.to_bytes(2, byteorder='little')
            one_group_cnt = len(angle_dict.keys())
            one_group_info = bytearray()
            for channel, channel_list in angle_dict.items():
                light_type = channel_list[0]
                # 根据进行秒数确定当前灯色
                tmp_total_time = 0
                tmp_light_color = 0
                for one_step in self._channels_color_duty[channel]:
                    tmp_total_time += one_step[1]
                    if tmp_total_time >= self._progress_seconds:  # 定位到灯色段
                        tmp_light_color = one_step[0]
                        break
                light_color = self._light_group_type[tmp_light_color]  # 当前灯色
                if self._control_mode == 3:
                    light_countdown = 0  # 手控模式下倒计时为0
                else:
                    # 根据进行秒数确定当前倒计时
                    tmp_total_time = 0
                    tmp_light_color = -1
                    tmp_channel_step_tuple = self._channels_countdown_duty[channel]
                    index = 0
                    max_index = len(tmp_channel_step_tuple)
                    while index < max_index:
                        tmp_one_step = tmp_channel_step_tuple[index]
                        tmp_one_color = tmp_one_step[0]
                        tmp_one_time = tmp_one_step[1]
                        index += 1
                        if tmp_light_color == -1:
                            tmp_total_time += tmp_one_time
                        if tmp_total_time >= self._progress_seconds:  # 定位到周期段
                            if tmp_light_color == -1:
                                tmp_light_color = tmp_one_color
                            else:
                                if tmp_light_color != tmp_one_color:  # 相邻不同色，结束
                                    break
                                else:  # 相邻同色，时长加入
                                    tmp_total_time += tmp_one_time
                        if (index == max_index) and (self._channels_countdown_duty[channel][0][0] == tmp_light_color):
                            tmp_total_time += self._channels_countdown_duty[channel][0][1]  # 首尾同色，时长相加

                    light_countdown = tmp_total_time - self._progress_seconds + 1
                    # print("light_countdown %d %d %d %d" % (channel, light_countdown, tmp_total_time, self._progress_seconds))
                one_light_info = bytearray()
                one_light_info.append(channel)
                one_light_info.append(light_type)
                one_light_info.append(light_color)
                one_light_info.append(light_countdown)
                one_group_info += one_light_info
            lights_info += one_grou_angle
            lights_info.append(one_group_cnt)
            lights_info += one_group_info

        entrance_cnt = len(self.scheme.keys())
        # 消息长度(2B) + 经度(4B) + 纬度(4B) + 海拔(2B) + 路口进口数量(1B) + 灯色状态信息(nB)
        data = bytearray()
        data += self._longitude.to_bytes(4, byteorder='little')
        data += self._latitude.to_bytes(4, byteorder='little')
        data += self._altitude.to_bytes(2, byteorder='little')
        data.append(entrance_cnt)
        data += lights_info
        data_len_ba = len(data).to_bytes(2, byteorder='little')
        data = data_len_ba + data
        if is_broadcast is True:
            opt_type = 0x87
        else:
            opt_type = 0x83
        frame = self._build_frame(opt_type, 0x0103, data)
        return frame

    def _build_frame_period(self, is_broadcast):
        '''
        构造当前周期方案帧
        :param is_broadcast: 是否是广播报文
        :return: 构造完成的报文
        '''
        channel_total_cnt = 0
        periods_info = bytearray()
        for angle, angle_dict in self.scheme.items():
            channel_total_cnt += len(angle_dict)
            periods_info += angle.to_bytes(2, byteorder='little')
            periods_info.append(len(angle_dict))
            for channel, channel_list in angle_dict.items():
                light_type = channel_list[0]
                light_periods_list = channel_list[1]
                periods_info.append(channel)
                periods_info.append(light_type)
                periods_info.append(len(light_periods_list))
                for one_period in light_periods_list:
                    color = one_period[0]
                    period_time = one_period[1]
                    periods_info.append(self._light_group_type[color])
                    periods_info += period_time.to_bytes(2, byteorder='little')

        # 消息长度(2B) + 经度(4B) + 纬度(4B) + 海拔(2B) + 灯组总数量(1B) + 路口进口数量(1B) + 灯组步色信息(nB)
        data = bytearray()
        data += self._longitude.to_bytes(4, byteorder='little')
        data += self._latitude.to_bytes(4, byteorder='little')
        data += self._altitude.to_bytes(2, byteorder='little')
        data.append(channel_total_cnt)
        data.append(len(self.scheme.keys()))
        data += periods_info
        data_len_ba = len(data).to_bytes(2, byteorder='little')
        data = data_len_ba + data
        if is_broadcast is True:
            opt_type = 0x87
        else:
            opt_type = 0x83
        frame = self._build_frame(opt_type, 0x0301, data)
        return frame

    def _send_light_state_change_broadcast(self):
        frame = self._build_frame_signal_light_state(True)
        self.telegrapher.send(frame)


if __name__ == "__main__":
    head_title = "################ GAT1743 simulator start ################"
    print(head_title)
    logger.info(head_title)
    gat_1743_player = GBT1743Protocol()
    gat_1743_player.start_run("./signal_scheme.json", 5)
    tail_title = "################ GAT1743 simulator end ################"
    print(tail_title)
    logger.info(tail_title)

