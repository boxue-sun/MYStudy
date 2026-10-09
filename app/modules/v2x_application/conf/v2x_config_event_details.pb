# proto-file: v2xpb-config-event-details.proto
# proto-message: Configure

basic_event_config{
  # 逆行
  event_detail{
    event_type_mec: 1
    rte_detail{
      event_type_id: 904
      description: "There is a vehicle driving in the opposite direction."
      priority: 7
      impact_dist: 100
    }
  }

  # 占到施工
  event_detail{
    event_type_mec: 10
    rte_detail{
      event_type_id: 501
      description: " "
      priority: 7
      impact_dist: 100
    }
  }

  # 车辆故障
  event_detail{
    event_type_mec: 103
    rte_detail{
      event_type_id: 103
      description: " "
      priority: 7
      impact_dist: 100
    }
  }

  # 异常停车
  event_detail{
    event_type_mec: 412
    rte_detail{
      event_type_id: 412
      description: " "
      priority: 7
      impact_dist: 100
    }
  }

  # 超速
  event_detail{
    event_type_mec: 14
    rte_detail{
      event_type_id: 901
      description: " "
      priority: 7
      impact_dist: 100
    }
  }

  # 行人闯入机动车道
  event_detail{
    event_type_mec: 16
    rte_detail{
      event_type_id: 405
      description: " "
      priority: 7
      impact_dist: 100
    }
  }

  # 非机动车闯入机动车道
  event_detail{
    event_type_mec: 411
    rte_detail{
      event_type_id: 411
      description: " "
      priority: 7
      impact_dist: 100
    }
  }

  # 道路拥堵
  event_detail{
    event_type_mec: 2
    rte_detail{
      event_type_id: 707
      description: " "
      priority: 7
      impact_dist: 100
    }
  }

  # 信号灯故障
  event_detail{
    event_type_mec: 410
    rte_detail{
      event_type_id: 410
      description: " "
      priority: 7
      impact_dist: 100
    }
  }

  # 抛洒物
  event_detail{
    event_type_mec: 3
    rte_detail{
      event_type_id: 401
      description: " "
      priority: 7
      impact_dist: 100
    }
  }

  # 机动车闯红灯
  event_detail{
    event_type_mec: 417
    rte_detail{
      event_type_id: 417
      description: " "
      priority: 7
      impact_dist: 100
    }
  }

  # 非机动车闯红灯
  event_detail{
    event_type_mec: 416
    rte_detail{
      event_type_id: 416
      description: " "
      priority: 7
      impact_dist: 100
    }
  }

  # 行人闯红灯
  event_detail{
    event_type_mec: 415
    rte_detail{
      event_type_id: 415
      description: " "
      priority: 7
      impact_dist: 100
    }
  }

  # 紧急车辆
  event_detail{
    event_type_mec: 418
    rte_detail{
      event_type_id: 418
      description: " "
      priority: 7
      impact_dist: 100
    }
  }
}

special_event_config{
  special_event_detail{   # repeated
    # 事件类型的列表
    event_type_mec_list{
      event_type_mec: 0
    }
    # ROI区域点的集合
    region_of_interest{
      position{        # repeated 位置按照逆时针排序
          latitude: 39.7787473
          longitude: 116.5623675
          elevation: 0.0
      }
      position{
          latitude: 39.7786713
          longitude: 116.5618718
          elevation: 0.0
      }
      position{
          latitude: 39.7783403
          longitude: 116.5618211
          elevation: 0.0
      }
      position{
          latitude: 39.7784490
          longitude: 116.5623372
          elevation: 0.0
      }
    }
    # 影响区域点的集合
    affected_area{
      position_list{    # repeated
        position{       # repeated
          latitude: 39.7787473
          longitude: 116.5623675
          elevation: 0.0
        }
        position{
          latitude: 39.7787473
          longitude: 116.5623675
          elevation: 0.0
        }
        radius: 7.5
      }
    }
    # 影响路径集合, 通过link给出
    affected_path{
      link_path{      # repeated
          node_id{
            region: 1
            id: 2
          }
          upstream_node_id{
            region: 1
            id: 2
          }
          maneuver: 0   # 0:直行 1:左转 2:右转 3:掉头 4:全方向
      }
      link_path{
          node_id{
            region: 1
            id: 2
          }
          upstream_node_id{
            region: 1
            id: 2
          }
          maneuver:1 
      }
    }
  }
}
