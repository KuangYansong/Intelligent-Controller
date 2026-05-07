# Smart Solid-State Circuit Breaker Firmware

First-stage firmware framework for TI TMS320F280049C/F280049CPZQ based on CCS 12.4 and C2000Ware.

## Layout

- `app`: startup flow, scheduler, system state machine
- `bsp`: board-level init and idle entry
- `driver`: ADC, ePWM, CMPSS, CAN, SPI wrappers
- `protection`: fault detection, RMS, I2T, recovery and lockout logic
- `protocol`: CAN frame encoding and command handling
- `storage`: FM25V20A FRAM access, parameters, fault log
- `selftest`: power-on and runtime self-checks
- `common`: shared types, config, CRC, time base
- `tests`: host-side logic tests

## Build Modes

Target firmware builds should define `SSCB_TARGET_C2000` and include C2000Ware driverlib headers/libraries through CCS project variables.

Host-side tests do not define `SSCB_TARGET_C2000`; hardware drivers compile as stubs so protocol, storage and protection logic can be validated without hardware.

## Hardware Map

- Voltage ADC: ADCB3, pin 8
- Current ADC: ADCB0, pin 41
- Temperature ADC: ADCB1, pin 40
- Short-circuit comparator: CMPSS3H, pin 7
- Gate PWM: ePWM3A, pin 75
- CAN: CANB, 500 kbps, standard 11-bit frames
- FRAM: FM25V20A on SPIB

智能固态断路器固件
基于 TI（德州仪器）TMS320F280049C/F280049CPZQ 芯片、依托 CCS 12.4（Code Composer Studio 12.4，TI 代码编写器工作室）和 C2000Ware（TI C2000 系列芯片配套软件包）开发的第一阶段固件框架。
目录结构
app：启动流程、调度器、系统状态机
bsp：板级初始化与空闲模式入口
driver：ADC（模数转换器）、ePWM（增强型脉冲宽度调制）、CMPSS（比较器子系统）、CAN（控制器局域网）、SPI（串行外设接口）驱动封装层
protection：故障检测、有效值（RMS）、I²T（电流平方时间）保护、故障恢复与锁定逻辑
protocol：CAN 帧编码与指令处理
storage：FM25V20A 铁电存储器（FRAM）访问、参数管理、故障日志
selftest：上电自检与运行时自检
common：共享数据类型、配置项、循环冗余校验（CRC）、时基模块
tests：主机端逻辑测试代码
构建模式
目标固件（硬件端）构建时需定义宏SSCB_TARGET_C2000，并通过 CCS 工程变量引入 C2000Ware 驱动库的头文件和库文件。
主机端测试无需定义SSCB_TARGET_C2000；此时硬件驱动会编译为桩代码（Stub），无需实际硬件即可验证协议、存储和保护逻辑的正确性。
硬件映射
电压采集 ADC：ADCB3，引脚 8
电流采集 ADC：ADCB0，引脚 41
温度采集 ADC：ADCB1，引脚 40
短路检测比较器：CMPSS3H，引脚 7
栅极驱动 PWM：ePWM3A，引脚 75
CAN 总线：CANB，波特率 500 kbps，标准 11 位帧格式
铁电存储器（FRAM）：SPIB 总线上的 FM25V20A