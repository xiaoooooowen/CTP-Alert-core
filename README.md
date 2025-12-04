# CTP-Alert-core

项目简介
- `CTP-Alert-core` 是一个轻量级 C++ 服务核心，用于订阅期货行情、从 MySQL 读取预警单并在满足条件时触发通知（控制台 / 邮件）。
- 本仓库包含行情接入、预警判断、DB 标记逻辑以及可配置的市场数据登录与数据库连接。

关键文件
- `Alert-core/Config.h`、`Alert-core/Config.cpp`：配置加载，环境变量优先，回退到 `config.ini`。
- `Alert-core/MduserHandler.h`：行情处理、数据库读取、预警判断及通知逻辑。
- `Alert-core/config.ini`：示例配置文件（可被环境变量覆盖）。
- `Alert-core/main.cpp`：程序入口（调用 `StartMarketService()`）。
- `Source` :实习公司提供的资料，根据这些资料开发此模块

快速启动（Windows + Visual Studio 2026）
前提

- Windows 10/11
- __Visual Studio__ 2026（C++ 桌面工作负载）
- MySQL Server 或可访问 MySQL 实例
- MySQL Connector/C++（确保包含目录与库链接正确配置）

步骤
1. 克隆仓库：
   - git clone https://github.com/xiaoooooowen/CTP-Alert-core.git
2. 在 __Visual Studio__ 中打开或创建项目并加入 `Alert-core` 源文件。
3. 在项目属性中配置 MySQL Connector/C++ 的包含目录和链接库。
4. 配置连接参数（环境变量 或 `Alert-core/config.ini`）。
5. 构建并运行（F5 或 使用 __Debugging__ 页面配置再启动）。

配置（优先级：环境变量 > config.ini > 默认）
示例 `Alert-core/config.ini`
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

支持的环境变量（推荐用于生产）
- MD_ADDRESS（例：tcp://host:port）
- MD_BROKERID、MD_USERID、MD_PASSWORD
- DB_HOST、DB_PORT、DB_USER、DB_PASSWORD、DB_SCHEMA

在 Windows CMD 示例
- set MD_ADDRESS=tcp://your-md-host:30011
- set DB_HOST=192.168.x.x
- start Alert-core\bin\Debug\Alert-core.exe

开发者运行的配置：
- MySQL：https://blog.csdn.net/weixin_74027669/article/details/137203874 按照此教程配置
- CTP-API，将需要的dll放在./x64/release文件夹下

在 __Visual Studio__ 调试设置环境变量
- 打开项目属性 -> __Debugging__，在 `Environment` 字段填入（用分号分隔）：
  MD_ADDRESS=tcp://127.0.0.1:30011;DB_HOST=127.0.0.1;DB_USER=root;DB_PASSWORD=1234

安全建议
- 不要将真实凭证提交到版本控制。优先使用环境变量或机密管理服务。

运行行为摘要
- 启动后加载配置、连接行情前置机并在连接成功后发起登录请求（若配置了凭证）。
- 周期性（默认每 3 秒）从数据库读取 `state=0` 的预警单并缓存。
- 接收到行情时运行 `CheckAlert` 判断条件：价格上下限或到达预定时间。
- 触发后：在控制台打印、调用邮件通知（若启用）、并将数据库记录标记为已触发（state=1），同时从内存缓存移除，避免重复触发。

排查与常见问题
- 无法连接行情前置机：确认 `MD_ADDRESS`、网络与防火墙。
- 登录失败：确认 `MD_BROKERID` / `MD_USERID` / `MD_PASSWORD`。
- 无法连接数据库：确认 `DB_*` 配置与 MySQL 服务可达，及 Connector/C++ 配置正确。
- 预警不触发：检查 `alert_order` 表字段及时间字符串格式是否符合代码解析逻辑（YYYY-MM-DD HH:MM:SS）。

建议改进（可选）
- 使用 JSON/YAML 配置并使用解析库（如 `nlohmann/json`、`yaml-cpp`）。
- 将 DB 操作抽象为接口并实现连接池 / 重试逻辑。
- 使用结构化日志（如 `spdlog`），支持级别和文件滚动。
- 把敏感信息托管于机密管理（Vault、Azure Key Vault 等）。
- 为核心逻辑（`Config`、`CheckAlert`）添加单元测试（GoogleTest / Catch2）。

开发与贡献
- 建议流程：先创建 Issue -> 新分支开发 -> 提交 PR，并在 PR 描述中写明改动与测试方法。
- 请遵循项目编码规范和 `.editorconfig` / `CONTRIBUTING.md`（若存在）。

许可证与作者
- 请参考仓库根目录的 `LICENSE` 文件。如缺失，请在合并前选定合适开源许可证（如 MIT 或 Apache-2.0）。

其他
这个项目是生产实习期货云端预警的一个小部分，作者第一次写项目所以迫不及待地把自己做（其实还有千问和Copilot）的部分上传到这里，完整项目在这里https://github.com/RV64G/FuturesCloudSentinel