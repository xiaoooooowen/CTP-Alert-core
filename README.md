# CTP-Alert-core

## 项目简介

`CTP-Alert-core` 是一个轻量级的 C++ 服务核心模块，用于：

- 订阅期货行情（通过 CTP 行情接口），
- 从 MySQL 数据库读取用户配置的预警单，
- 在满足价格或时间条件时触发通知（控制台输出 / 邮件）。

本仓库实现了行情接入、预警判断、数据库状态更新等核心逻辑，并支持灵活配置市场数据连接与数据库参数。

> ? **说明**：本模块是实习期间开发的“期货云端预警系统”的一部分。完整项目见 [FuturesCloudSentinel](https://github.com/RV64G/FuturesCloudSentinel)。

------

## 关键文件说明

| 文件                                 | 作用                                                |
| ------------------------------------ | --------------------------------------------------- |
| `Alert-core/Config.h` / `Config.cpp` | 配置加载逻辑：优先读取环境变量，回退至 `config.ini` |
| `Alert-core/MduserHandler.h`         | 核心逻辑：行情处理、数据库读取、预警判断与通知触发  |
| `Alert-core/config.ini`              | 示例配置文件（可被环境变量覆盖）                    |
| `Alert-core/main.cpp`                | 程序入口，调用 `StartMarketService()` 启动服务      |
| `Source/`                            | 实习公司提供的 CTP SDK 及相关资料                   |

------

## 快速启动（Windows + Visual Studio 2026）

### 前提条件

- Windows 10/11
- **Visual Studio 2026**（需安装 C++ 桌面开发工作负载）
- 可访问的 **MySQL Server** 实例
- 已正确配置 **MySQL Connector/C++**（包含头文件路径与链接库）

### 启动步骤

1. 克隆仓库：

   ```bash
   git clone https://github.com/xiaoooooowen/CTP-Alert-core.git
   ```

2. 在 Visual Studio 中打开项目，将 `Alert-core` 目录下的源文件加入工程。

3. 在项目属性中配置：

   - **C/C++ → 附加包含目录**：添加 MySQL Connector/C++ 的 `include` 路径
   - **链接器 → 附加库目录**：添加 `lib64/vs14`（或对应 VS 版本目录）
   - **链接器 → 输入 → 附加依赖项**：添加 `mysqlcppconn.lib` 和 `mysqlcppconn8.lib`

4. 配置连接参数（推荐使用环境变量，也可修改 `config.ini`）。

5. 构建并运行（按 F5，或在 **Debugging** 设置中配置后再启动）。

> ? **参考教程**：[VS2022 配置 MySQL C++ Connector（CSDN）](https://blog.csdn.net/weixin_74027669/article/details/137203874)

------

## 配置方式（优先级：环境变量 > config.ini > 默认值）

### 示例 `Alert-core/config.ini`

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

### 支持的环境变量（推荐用于生产环境）

- **行情连接**：`MD_ADDRESS`, `MD_BROKERID`, `MD_USERID`, `MD_PASSWORD`
- **数据库连接**：`DB_HOST`, `DB_PORT`, `DB_USER`, `DB_PASSWORD`, `DB_SCHEMA`

#### Windows CMD 示例

```cmd
set MD_ADDRESS=tcp://your-md-host:30011
set DB_HOST=192.168.x.x
start Alert-core\bin\Debug\Alert-core.exe
```

#### Visual Studio 调试时设置环境变量

- 项目属性 → **Debugging** → **Environment**

- 填入（分号分隔）：

  ```
  MD_ADDRESS=tcp://127.0.0.1:30011;DB_HOST=127.0.0.1;DB_USER=root;DB_PASSWORD=1234
  ```

> ? **安全建议**：**切勿将真实凭证提交到版本控制系统**。优先使用环境变量或机密管理服务（如 HashiCorp Vault、Azure Key Vault）。

------

## 运行行为摘要

1. 启动后加载配置，连接行情前置机；

2. 若配置了登录凭证，则自动发起登录请求；

3. 启动独立线程，**每 3 秒**从数据库加载 `state=0` 的未触发预警单到内存缓存；

4. 收到行情回调时，执行 

   ```
   CheckAlert()
   ```

    判断是否满足以下任一条件：

   - 最新价格 ≥ 上限 或 ≤ 下限
   - 当前时间 ≥ 预设触发时间（格式：`YYYY-MM-DD HH:MM:SS`）

5. 触发后：

   - 控制台打印日志
   - 调用邮件通知（若启用）
   - 更新数据库记录为 `state=1`
   - 从内存缓存中移除，避免重复触发

------

## 部署与调试要点

- 确保 **CTP 动态库**（如 `thostmduserapi.dll`）和 **MySQL Connector 运行时 DLL** 位于可执行文件目录或系统 `PATH` 中。
- 推荐在 CI/CD 或容器环境中通过环境变量注入敏感配置。
- 调试时可临时使用 `config.ini`，但生产环境务必禁用明文密码。

------

## 常见问题排查

| 问题               | 可能原因                                            | 解决方案                                                     |
| ------------------ | --------------------------------------------------- | ------------------------------------------------------------ |
| 无法连接行情前置机 | 地址错误、网络不通、防火墙拦截                      | 检查 `MD_ADDRESS`，测试 telnet 连通性                        |
| 登录失败           | BrokerID / UserID / Password 错误                   | 核对交易所分配的测试账号                                     |
| 无法连接数据库     | DB 配置错误、MySQL 服务未启动、Connector 未正确链接 | 按 CSDN 教程检查 VS 配置，确认 MySQL 可远程访问              |
| 预警不触发         | 时间格式不符、内存缓存未加载、条件未满足            | 检查 `alert_order.trigger_time` 是否为 `YYYY-MM-DD HH:MM:SS` |

------

## 技术亮点

- ? **灵活配置机制**：环境变量优先，兼容本地调试与容器化部署。

- ? **行业标准对接**：基于 **CTP SDK（CThostFtdcMdApi）**，确保与国内期货交易所兼容。

- ? **直接数据库操作**：使用 **MySQL Connector/C++**（JDBC 风格），便于集成现有运维体系。

- ? 

  线程安全设计

  ：

  - SDK 回调运行在独立线程，避免阻塞；
  - 行情缓存 `m_lastPrices` 与预警缓存 `m_alertMap` 使用互斥锁保护；
  - 预警加载由后台线程周期执行，避免在回调中进行重 I/O。

- ? **可扩展通知抽象**：通过 `INotifier` 接口支持邮件、短信、Webhook 等多种通知方式。

- ? **容错基础**：具备连接状态跟踪（`m_isConnected` / `m_isLoggedIn`），为后续实现重连策略打下基础。

------

## 关键技术选型理由

| 组件       | 选型                        | 理由                                         |
| ---------- | --------------------------- | -------------------------------------------- |
| 行情接入   | CTP SDK (`CThostFtdcMdApi`) | 行业事实标准，低延迟、事件驱动，支持深度行情 |
| 数据库驱动 | MySQL Connector/C++         | 官方原生 C++ 支持，类型安全，支持预编译语句  |
| 并发模型   | C++11 原生线程/互斥锁       | 轻量、无第三方依赖，满足低延迟要求           |
| 配置管理   | INI + 环境变量              | 简单可靠，易于 CI/CD 和容器注入              |
| 日志输出   | `printf`（当前）            | 快速调试；**建议生产替换为 `spdlog`**        |

------

## 业务流程（从启动到触发）

1. **启动**  
   - `main()` 调用 `StartMarketService()`  
   - `CMduserHandler` 构造时加载配置（环境变量 → `config.ini`）
2. **连接与登录**  
   - 创建 `CThostFtdcMdApi`，注册回调，调用 `Init()`  
   - `OnFrontConnected()` 触发后，若配置了凭证，则调用 `ReqUserLogin()`  
   - `OnRspUserLogin()` 成功后设置 `m_isLoggedIn = true`
3. **订阅行情**  
   - 调用 `subscribe(contracts)`，将合约列表转为 `char*[]` 并调用 `SubscribeMarketData()`  
   - 失败时自动重试
4. **预警加载（后台线程）**  
   - 启动 `StartAlertReloadThread()`，每 3 秒执行 `ReloadAlertsFromDB()`  
   - 加载 `state=0` 的预警单至 `m_alertMap`
5. **行情处理与判断**  
   - `OnRtnDepthMarketData()` 回调更新 `m_lastPrices`（加锁）  
   - 调用 `CheckAlert(symbol, price)` 判断是否触发
6. **触发与落库**  
   - 通过 `INotifier` 发送通知  
   - 调用 `MarkAlertTriggered(orderId)` 更新 DB 状态为 `state=1`  
   - 从内存缓存中移除该条目
7. **断连处理（基础）**  
   - `OnFrontDisconnected()` 清理状态  
   - **待增强**：实现指数退避重连与自动重登录

------

## 改进建议（短期 / 中期）

- **异步化 I/O**：将数据库操作与邮件发送放入工作队列 + 线程池，避免阻塞 SDK 回调线程。
- **数据库连接复用**：引入连接池，减少频繁创建/销毁连接的开销。
- **增强容错**：实现可配置的重连策略（最大重试次数、退避算法、熔断机制）。
- **日志升级**：替换 `printf` 为 `spdlog`，支持日志级别、文件滚动与结构化输出。
- **监控集成**：暴露 Prometheus 指标（如触发次数、连接状态、延迟）。
- **配置现代化**：支持 JSON/YAML（使用 `nlohmann/json` 或 `yaml-cpp`）。
- **安全加固**：敏感信息交由机密管理系统托管。
- **测试覆盖**：为核心逻辑（`Config`、`CheckAlert`）编写单元测试（GoogleTest / Catch2）。

------

## 开发与贡献

- 建议流程

  ：

  1. 创建 Issue 描述需求或问题
  2. 从 `main` 分支拉出新特性分支
  3. 开发完成后提交 Pull Request
  4. PR 描述中需包含改动说明与测试方法

- 请遵循项目编码规范（如有 `.editorconfig` 或 `CONTRIBUTING.md`，请参照执行）。

------

## 许可证与作者

- 请查看仓库根目录的 `LICENSE` 文件。
- 若尚未指定许可证，建议选择 **MIT** 或 **Apache-2.0** 开源协议。

> ? **作者备注**：这是本人首次独立完成的 C++ 项目（部分借助 Qwen / GitHub Copilot 辅助），欢迎批评指正！

------

**完整项目地址**：[RV64G/FuturesCloudSentinel](https://github.com/RV64G/FuturesCloudSentinel)