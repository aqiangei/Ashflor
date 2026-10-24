# 灰烬 · Ashflor

自托管的多人 [florr.io](https://florr.io) 风格花园射击游戏。C++ / WASM 客户端 + **原生 uWebSockets** 服务端，中文界面优先，支持 PC 与移动端。

**正式服：** [https://ashflor.com/](https://ashflor.com/)  
**备用入口：** [http://ashflor.com/](http://ashflor.com/) · [http://www.ashflor.com/](http://www.ashflor.com/)

---

## Fork 声明（请保留）

本项目是 **[trigonal-bacon/gardn](https://github.com/trigonal-bacon/gardn)** 的 **fork**，遵循 **AGPL-3.0**。

| | 仓库 | 作者 |
|---|------|------|
| **上游** | [github.com/trigonal-bacon/gardn](https://github.com/trigonal-bacon/gardn) | **trigonal-bacon** |
| **本仓库** | [github.com/aqiangei/ashflor](https://github.com/aqiangei/ashflor) | **aqiangei**（二次创作 / 运营） |

游戏内设置页与标题屏右上角亦提供上游链接；二次分发请保留对原作者与本声明的致谢。素材致谢见游戏内「素材：M28 等」。完整条款见 [LICENSE](./LICENSE)。

---

## 交流

| | |
|---|---|
| **QQ 交流群** | **227990150** |
| 一键加群 | [https://qm.qq.com/q/E0OG9CLIg](https://qm.qq.com/q/E0OG9CLIg) |
| 游戏内 | 设置 / 暂停菜单 →「加入 QQ 群」（可复制群号） |
| 给作者留言 | 首页 / 设置 →「给作者留言」（需登录；公开留言板） |

---

## 游戏控制台（Live）

运维与观察面板，与正式服同域：

| 入口 | 用途 |
|------|------|
| [`/live`](https://ashflor.com/live) | 在线名册、分图人数、Boss 状态、排行摘要 |
| `/api/live` | JSON 快照（在线、Boss、版本等） |
| `/api/catalog` | 花瓣 / 怪物等目录 |
| `/api/admin` | 公告、发瓣、刷怪等管理接口（生产可开 `ASHFLOR_LIVE_AUTH`） |

静态看板说明与发版戳记见 [`Client/public/deploy-info.json`](./Client/public/deploy-info.json)。云服部署步骤见仓库上级目录 [`DEPLOY.md`](../DEPLOY.md) / [`部署.md`](../部署.md)（一键：`.\deploy-update.ps1`）。

---

## 相对上游做了什么

在 gardn 核心（ECS、花瓣环绕、空间哈希、视口兴趣同步）之上，Ashflor 主要扩展：

### 世界与战斗
- **八域地图**：花园 / 沙漠 / 海洋 / 雪原 / 丛林 / 蚁穴 / 火山 / **王城**（难度环、地形、刷怪与传送）
- 每图 **Boss 锚点**、顶部血条、击杀全服播报；高等级与 Unique 向数值加强
- **九档稀有度**（普通→…→究极→超凡→独特）；怪物体型随稀有度温和放大
- **私有掉落**（组队每人一份，仅本人可见可拾）；拾取进背包，不自动装槽
- 玩家 **soft-block 互推**；怪物倒下有短时尸体姿态；视野同步无硬截断闪怪

### 花瓣 · 合成 · 成长
- 材料合成、跨图组装（V）、金币商店；**Unique 账号等级**（主手第一枚加成）
- 配装预设（主副槽一起保存；K/L / 移动端切装）
- **等级无硬顶**、天赋树、死亡保本级经验；花园 **九档训练假人**（个人 DPS，k/m/b 读数）

### 社交 · 进度 · UI
- 账号存档、组队 / 公会、聊天与气泡、多维排行榜
- 世界事件 / 探索 / 指引精灵；怪物与花瓣图鉴、Codex
- 中文 i18n、移动端摇杆与触控、滚轮缩放、公开留言板与 QQ 群入口

### 引擎 · 运维
- 生产默认 **原生 uWS**（非 Node 跑 WASM）；视口 AOI、碰撞格子、刷怪拒绝、区块缓存等
- 零停机发版：`cacheV` + 香港静态边缘 + 北京源站；Live 看板与健康探测

更细的逐日说明见游戏内 **更新日志**，或 `Client/Ui/TitleScreen/Changelog.cc`。

---

## 技术栈

| 部分 | 说明 |
|------|------|
| ECS | 自制；`Shared/EntityDef.hh`、`Shared/Entity.hh` |
| 服务端 | 原生 **uWebSockets**（`Server/Native.cc`）；可选 WASM/Node 路径 |
| 客户端 | Emscripten → WASM/JS；Canvas2D；`Client/Ui` |
| 空间查询 | Canonical 空间哈希（默认 `GENERAL_SPATIAL_HASH`），格约 512 |
| 中文 | `USE_CODEPOINT_LEN` 截断；`Client/I18n*` |

本地编译与自建服见 [INSTALLATION.md](./INSTALLATION.md)。正式云服发版用上级目录的 `deploy-update.ps1`（WSL 编客户端 + 北京编 native）。

---

## 仓库结构（简）

```
ashflor/
  Client/     浏览器 WASM 客户端与 public 静态资源
  Server/     游戏逻辑、原生 / WASM 服务端
  Shared/     协议、实体、地图与数值
  Helpers/    数学、二进制等
  tools/      压测与回归脚本
```

上级目录 `d:\ashflor` 还含部署脚本、Nginx 配置、`reference/` 二创对照与 `todo/`，不属于本 git 子仓必选内容。

---

## 许可

AGPL-3.0 — 见 [LICENSE](./LICENSE)。

再次致谢：**trigonal-bacon** 的 [gardn](https://github.com/trigonal-bacon/gardn)，以及素材作者 **M28** 等。
