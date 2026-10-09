send_enable: false
send_period: 0.1
rsi_contents {
    rsi_content {               # repeated 
        alert_desc: "none, just for giving an example!"
        alert_type: 904
        position {              # 事件或者标牌的位置
            latitude: 39.7787473
            longitude: 116.5623675
            elevation: 0.0
        }
        enent_radius: 100
        pack_as_rte: false
        priority: 1
        rsi_msg_id: 1               # -1 无效
        start_time: "16:08:10"      # 每天开始的时间段
        end_time: "18:00:00"        # 每天结束的时间段
        is_time_set_valid: false    # 时间段设置是否生效， false时默认一直周期发送

        alert_paths {  
            position_list {    # repeated
                position {     # repeated
                    latitude: 39.7787473
                    longitude: 116.5623675
                    elevation: 0.0
                }
                position {
                    latitude: 39.7787473
                    longitude: 116.5623675
                    elevation: 0.0
                }
                radius: 7.5
            }
            position_list {    # repeated
                position {    # repeated
                    latitude: 39.7787480
                    longitude: 116.5623685
                    elevation: 0.0
                }
                position {
                    latitude: 39.7787443
                    longitude: 116.5623475
                    elevation: 0.0
                }
                radius: 7.5
            }
        }

        alert_links {
            alert_link {      # repeated
                node_id {
                    region: 101
                    id: 102
                }
                upstream_node_id {
                    region: 101
                    id: 103
                }
                reference_lanes {
                    lane_id: 1   # repeated
                    lane_id: 2
                }
            }
        }
    }
}
