# Gimbal 云台工程

按 drone 示例的职责分层组织，当前仍为裸机工程，尚未接入 FreeRTOS。

```text
Gimbal/
├── Core/                              CubeMX 生成的初始化与中断入口
│   ├── Inc/
│   └── Src/                           main.c 调用 RcTaskInit/RcTaskProcess
├── Drivers/                           ST HAL 与 CMSIS 驱动
├── GeneralHeader/
│   └── rc_config.h                    协议参数、通道分配、角度范围、超时时间
├── PrivateDrivers/
│   └── DT7/
│       ├── rc_protocol.c/.h            DR16 18 字节解码与合法性检查
│       └── rc_receiver.c/.h            UART DMA 接收、帧交接与故障恢复
├── PrivateApplications/
│   └── RemoteControl/
│       └── rc_mapping.c/.h             去中值、死区和直接目标角度映射
├── Tasks/
│   ├── Inc/rc_task.h                   业务接口、状态快照与监视变量声明
│   └── Src/rc_task.c                   解析与映射调度、超时检查、状态发布
├── MDK-ARM/                           CMSIS 工程、链接脚本、调试配置及构建输出
│   ├── Gimbal.csolution.yml
│   └── Gimbal.cproject.yml
├── .vscode/                           编译、烧写、调试与 clangd 设置
└── Gimbal.ioc                         CubeMX 配置入口
```

数据流：USART1/DMA → rc_receiver → rc_protocol → rc_mapping → rc_task 发布状态。

main.c 只负责硬件初始化和业务调用；rc_task.c 中的 Task 表示任务职责，
当前 RcTaskProcess 由主循环调用。后续接入 RTOS 时再由独立线程调用，单一写入者原则不变。
目前未使用 FreeRTOS 或 USB Device，因此没有复制 drone 的 Middlewares、USB_DEVICE 及相关依赖。

## 编译与调试

用 VS Code 打开本工程根目录，活动方案为 MDK-ARM/Gimbal.csolution.yml，目标为 Gimbal。
在已激活工具环境的 MDK-ARM 终端运行：

```powershell
cbuild Gimbal.csolution.yml --active Gimbal
```

输出为 MDK-ARM/out/Gimbal/Gimbal/Gimbal.axf。原有监视变量名继续可用。
解析断点设置在 PrivateDrivers/DT7，映射断点设置在 PrivateApplications/RemoteControl，
超时和状态断点设置在 Tasks/Src/rc_task.c。

CubeMX 自定义调用保留在 USER CODE 区域；自定义模块放在 Core 外。
新加模块时需更新 Gimbal.cproject.yml 中的源码与头文件路径。
Gimbal.uvprojx 已同步分组与路径，本次验证使用 CMSIS/AC6，未验证 uVision 构建。
