# ChatRoom
一款基于socket通信的Linux命令行局域网聊天系统
+ 多线程服务器
+ 关闭了time_wait选项，便于服务端快速重启
+ mysql数据存储

# 基本使用
+ `make`编译，生成客户端宇服务端程序
+ 服务端程序运行：`./server 端口号`
+ 客户端程序运行： `./client IP 端口号 房间号`
+ 账号密码是输入默认是注册模式
+ 如果账号已存在，则判断密码是否准确
+ 根据房间号的不同加载和存储聊天记录

# 数据库配置
参考`sqlhelper.cpp`注释

# 使用截图
服务端程序：会对客户端请求的连接和断开做一个输出

<img width="346" alt="image" src="https://github.com/Zane-Yu/ChatRoom/assets/57588387/ae8c658b-02ec-4d64-b885-30578223125d">

客户端

<img width="327" alt="image" src="https://github.com/Zane-Yu/ChatRoom/assets/57588387/b632aad5-d55b-4769-bde1-7a1426abbf44">

<img width="329" alt="image" src="https://github.com/Zane-Yu/ChatRoom/assets/57588387/fae70ea8-3f71-43d2-bcf1-3282cb89afd4">


