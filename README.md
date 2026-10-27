# 灰烬 · ashflor

自托管的多人 [florr.io](https://florr.io) 风格游戏。基于 [gardn](https://github.com/trigonal-bacon/gardn)（AGPL-3.0）fork，C++ / WASM。

**游玩：** [https://ashflor.com/](https://ashflor.com/)  
自己开服见 [INSTALLATION.md](./INSTALLATION.md)。

## 网页入口

| | |
|--|--|
| 游戏 | [ashflor.com](https://ashflor.com/) |
| Live 控制台 | [/live](https://ashflor.com/live) — 在线 / Boss / 排行 / 小地图；可发公告、发瓣、刷怪（可选鉴权） |
| 图鉴 | [/codex](https://ashflor.com/codex) — 花瓣、怪物、机制与数值 |
| 留言板 | [/feedback.html](https://ashflor.com/feedback.html) — 公开留言；同页展示修复进度 |

## 相对上游做了什么

- **八域世界**：花园、沙漠、海洋、雪原、丛林、蚁穴、火山、王城；各图 Boss 与跨图传送
- **养成**：账号存档、九档稀有度（含究极 / 超凡 / 独特）、合成与材料配方、统一背包与云端配装、等级无硬顶与天赋树
- **社交**：组队、公会、聊天、多维排行榜、Boss / 高阶合成全服播报
- **探索**：指引精灵、程序化 ThemeArt、印记 / 宝箱等轻量世界事件
- **体验**：中文界面、PC / 移动端操作、私有掉落、死亡不掉瓣
- **运维**：原生 uWS 服、Live 看板与管理 API、公开留言与进度页、Nginx 边缘发版

细项变更以游戏内更新日志与留言板「修复进度」为准，不在此罗列。

## 技术栈

| | |
|--|--|
| 共享 | 自制 ECS（`Shared/Entity*.hh`） |
| 服务端 | `gardn-server-native`（uWebSockets）；亦可 WASM + Node |
| 客户端 | Emscripten → WASM；Canvas2D；`Client/Ui` |
| 编译默认 | `USE_CODEPOINT_LEN`、`GENERAL_SPATIAL_HASH` |

## 许可

Fork of [trigonal-bacon/gardn](https://github.com/trigonal-bacon/gardn)。请保留对上游作者的声明。本仓库：[aqiangei/ashflor](https://github.com/aqiangei/ashflor)。见 [LICENSE](./LICENSE)。
