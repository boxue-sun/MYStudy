
# Air-runtime
简要说明
Air-runtime是airos的通信中间件接口，封装不同消息中间件，提供统一抽象接口，用以对消息的发布、订阅，以及实时程序调度。目前只封装了Apollo Cyber RT，后续将支持更多消息中间件。
Apollo Cyber RT是专门为自动驾驶场景设计的开源、高性能运行时框架，关于Apollo Cyber RT请参考文档：https://github.com/ApolloAuto/apollo/blob/master/cyber/README.md

## 概念
Channel:数据通信的通道，用户可以通过订阅、发布相同的Channel来实现数据的通信。
Node:通信中间件节点，一个节点通过创建reader和writer来订阅和发布消息。
Reader:消息的订阅者，reader需要绑定一个回调函数，当channel中的消息到达后，回调会被调用。
Writer:消息的发布者，每个writer对应一个特定消息类型的channel。
Component:组件，其中包含了一个Node和Reader，用户可以通过编写dag文件灵活的控制Component订阅的Channel、配置参数等信息。Component可以通过dag文件和launch文件灵活的控制互相之间的数据流向和组件的增删，而无需修改代码。dag文件和launch文件编写请参考文档https://github.com/ApolloAuto/apollo/blob/master/docs/cyber/CyberRT_API_for_Developers.md 中的Launch和Dag file format两个部分
## 示例
用户可以通过两种方式进行通信：
1：通过手动创建Node、Reader、Writer进行通信。
程序的示例请参考demo文件夹下的node_demo.cpp、node_listener_demo.cpp、node_talker_demo.cpp三个文件。其中node_demo.cpp演示了如何创建node以及通过创建reader和writer在同一个进程里进行消息通信。node_listener_demo.cpp、node_talker_demo.cpp两个文件需要配合运行，演示了如何创建node以及通过创建reader和writer进行跨进程通信。需要注意，如果是跨进程通信，消息类型需要为protobuff格式。
2：通过编写Component和dag配置文件进行通信。
程序的示例请参考demo文件夹下的component_demo.cpp、demo_cyberrt.dag两个文件。其中component_demo.cpp演示如何实现一个组件，接收消息并处理消息发送消息。demo_cyberrt.dag演示了如何配置一个Component订阅的channel和参数文件。
