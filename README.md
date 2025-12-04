# CTP-Alert-core 🚀

**项目简介**

CTP-Alert-core 是一个轻量级的 C++ 服务核心模块，专为云端期货预警而设计，具备如下核心能力：

- 📊 订阅期货行情（基于 CTP 行情接口）
- 🗂️ 从 MySQL 数据库读取用户配置的预警单
- ⏰ 实时监控，自动触发通知（控制台输出/邮件），并更新数据库状态

本模块作为[期货云端预警系统](https://github.com/RV64G/FuturesCloudSentinel)的核心组成部分，实现了行情接入、预警判断和数据管理等全流程，支持灵活的市场/数据库参数配置，适用于生产环境和本地开发。

---

## 📂 目录结构及关键文件

| 文件/目录                         | 说明                         |
| --------------------------------- | ---------------------------- |
| Alert-core/Config.h / Config.cpp  | 配置加载逻辑（环境变量优先，回退至 config.ini） |
| Alert-core/MduserHandler.h        | 核心：行情处理、预警判断、数据库操作、通知触发 |
| Alert-core/config.ini             | 示例配置（可用环境变量覆盖）  |
| Alert-core/main.cpp               | 程序入口，启动服务            |
| Source/                           | CTP SDK 及其他二方文件       |

---

## ⚡ 快速启动（Windows + Visual Studio 2026）

> **前置条件**
>
> - 🖥️ Windows 10/11
> - 🛠️ Visual Studio 2026（已安装 C++ 桌面开发环境）
> - 🗄️ 可访问的 MySQL Server
> - 🔗 正确配置 MySQL Connector/C++ 的 include 和链接路径

**步骤如下：**

1. 克隆项目

    ```bash
    git clone https://github.com/xiaoooooowen/CTP-Alert-core.git
    ```

2. 在 Visual Studio 打开项目，并将 `Alert-core` 下源码加入工程。

3. 配置依赖（见 C++ 工程属性页）：

    - `C/C++ -> 附加包含目录`：添加 MySQL Connector/C++ 的 `include` 路径
    - `链接器 -> 附加库目录`：添加 `lib64/vs14`（或你的 VS 版本目录）
    - `链接器 -> 输入 -> 附加依赖项`：添加 `mysqlcppconn.lib` 和 `mysqlcppconn8.lib`

4. 设置配置参数（推荐用环境变量，也可手动编辑 `config.ini`）

5. 编译运行（F5），或在“调试->环境变量”里填写参数，再启动调试

> 参考：[VS2022 配置 MySQL C++ Connector（CSDN）](https://blog.csdn.net/weixin_74027669/article/details/137203874)

---

## ⚙️ 配置说明（优先级：环境变量 > config.ini > 默认值）

**样例 `Alert-core/config.ini`:**

```ini
[Database]
Host=127.0.0.1
Port=3306
User=root
Password=1234
Schema=futurescloudsentinel

[MarketData]
Address=tcp://182.254.243.31:30011
BrokerID=
UserID=
Password=
```

**可用环境变量（推荐）：**

- 🛠️ 行情连接：`MD_ADDRESS`，`MD_BROKERID`，`MD_USERID`，`MD_PASSWORD`
- 🗄️ 数据库连接：`DB_HOST`，`DB_PORT`，`DB_USER`，`DB_PASSWORD`，`DB_SCHEMA`

**Windows CMD 示例：**

```cmd
set MD_ADDRESS=tcp://your-md-host:30011
set DB_HOST=192.168.x.x
start Alert-core\bin\Debug\Alert-core.exe
```

**VS 调试环境变量配置：**
项目属性 -> `Debugging -> Environment`，输入：

```
MD_ADDRESS=tcp://127.0.0.1:30011;DB_HOST=127.0.0.1;DB_USER=root;DB_PASSWORD=1234
```

> ⚠️ **安全提醒**：不要把生产密码写进代码仓库！强烈建议用环境变量或安全服务统一管理凭证。

---

## 🛠️ 项目运行流程简述

1. 🚀 加载配置 -> 连接行情前置机  
2. 🔗 自动登录（如果已配置凭证）  
3. ♻️ 启动后台线程，每 3 秒拉取一次数据库 `state=0` 的待触发预警单  
4. 📡 行情回调时执行预警判定  
    - 如：最新价格 ≥ 上限，≤ 下限，或达到定时触发点  
5. ✅ 符合条件即：
    - 控制台输出日志
    - 📧 邮件通知（如启用）
    - 📝 数据库标记已触发
    - 🧹 从内存缓存移除

> **Tips:** 支持预警“价格触发”和“定时触发”两种模式。

---

## 🤔 常见故障排查

| ❓ 问题          | 🕵️ 可能原因            | 🛠️ 解决建议                                    |
| ----------------- | --------------------- | --------------------------------------------- |
| 不能连行情前置机  | 地址/网络/防火墙问题  | 检查 MD_ADDRESS，尝试 `telnet` 目标主机         |
| 登录失败         | 账户/密码错误         | 对照交易所提供的测试账号                        |
| 无法连数据库      | 配置、服务、Connector异常 | 检查配置，保证 MySQL Connector 和远端访问正常   |
| 预警没触发        | 时间格式错误、条件不满足 | 检查数据库字段格式为 `YYYY-MM-DD HH:MM:SS`      |

---

## 🌐 技术架构与亮点

- **🔧 灵活配置**：环境变量优先，兼容本地调试及云端部署
- **📈 行业标准**：CTP SDK 直连期货市场
- **🗄️ 数据库驱动**：MySQL Connector/C++，高效安全
- **🔒 线程安全**：行情与预警缓存用 mutex 护航，后台循环定时加载
- **📡 多通知渠道**：通过 INotifier 接口支持后续扩展短信/Webhook
- **💪 容错准备**：连接、登录状态追踪，为后续重连打下基础

---

## ⚖️ 技术选型解读

| ⚙️ 组件          | 🤔 选型           | 🛠️ 理由                            |
| --------- | ----------------- | --------------------------------- |
| 行情接入   | CTP SDK           | 国内期货主流标准，低延迟、高兼容  |
| 数据库    | MySQL Connector/C++| 原生 C++ 支持，安全高效          |
| 并发模型   | C++11 线程+互斥锁 | 高性能、无三方依赖               |
| 配置加载   | INI + 环境变量    | 简单可靠，适合 CI/CD 和容器部署   |
| 日志输出   | printf（当前）    | 便于调试，建议生产用 spdlog       |

---

## 🕹️ 改进建议（欢迎参与）

- **异步 I/O**：数据库与通知走线程池，防止回调线程阻塞
- **数据库连接池**：减少频繁创建销毁
- **弹性容错**：可配置重试机制、熔断、退避算法
- **日志升级**：推荐集成 spdlog，支持滚动与分级
- **监控集成**：支持 Prometheus 指标导出
- **现代化配置**：支持 JSON/YAML 格式（如 nlohmann/json）
- **安全增强**：机密交由专业存储，不暴露明文
- **核心单元测试**：建议用 GoogleTest / Catch2

---

## 💡 开发指南

1. 先创建 Issue 描述需求或 Bug
2. 从 main 分支新建特性分支
3. 开发提交 Pull Request
4. PR 请简明写明改动内容和测试方式
5. 按照 `.editorconfig` / `CONTRIBUTING.md` 规范（如有）

---

## 🎓 许可证与作者

- 详见项目根目录 LICENSE 文件
- 建议采用 MIT 或 Apache-2.0 开源协议

> ✨ 本人首个 C++ 独立项目（得益于 Qwen / GitHub Copilot 辅助），欢迎拍砖、Star、Fork！

完整项目见：[RV64G/FuturesCloudSentinel](https://github.com/RV64G/FuturesCloudSentinel)

---

**本 README 使用了适量表情符号，提升趣味性与清晰度，适配各类平台下的渲染效果。** 😄