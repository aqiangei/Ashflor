# 灰烬 · ashflor

自托管的多人 PvP florr 风格游戏，基于 [gardn](https://github.com/trigonal-bacon/gardn) fork，用 C++ 编写。

安装与运行见 [INSTALLATION.md](./INSTALLATION.md)。

---

## 回溯 · 从何而来

本仓库 fork 自 [trigonal-bacon/gardn](https://github.com/trigonal-bacon/gardn)：开源、可自托管的 florr.io 风格多人实现。上游用自制 ECS、uWebSockets / WASM 双路径服务端，以及 emscripten 客户端与自研 UI 引擎，搭好了接近原作的战斗与模拟骨架。

灰烬站在这条线上继续走。玩法启发自 [florr.io](https://static.florr.io/old/)，实现仍是独立代码；我们保留 gardn 的引擎与架构，在其上叠自己的账号、社交与中文体验，而不是另起炉灶。

上游仍值得阅读：`Shared/EntityDef.hh` 的实体宏、服务端均匀网格碰撞、`Client/Ui` 的非 HTML UI——这些是灰烬能快速迭代的地基。

---

## 当下 · 灰烬已做的

相对上游，当前主线大致包括：

- **账号存档** — 服务端账号与进度持久化（`Server/AccountStore`）
- **组队 / 派对** — 多人组队进出与同步（`Server/Party`）
- **聊天** — 局内交流（`Server/Chat`）
- **合成 / 工艺** — 花瓣相关合成逻辑（`Shared/Craft`）
- **中文与 i18n** — 静态文案与工具链（`Scripts/zh_*.py` 等）
- **地图与数值调整** — 按灰烬服需求改动的配置与静态数据

细节以代码与提交为准；上述是方向，不是功能清单的终点。

---

## 展望 · 往哪去

短期：把账号、组队、聊天跑稳，补齐中文与部署文档，让一台机器就能开服、进房、开打。

中期：打磨合成与进度循环，让「收集 → 合成 → 再上场」成为清晰主线；组队与社交再深一点，而不是只挂在协议上。

长期：灰烬成为可长期运营的自托管服——规则可配、内容可扩、中文优先，同时尽量保持对上游有意义改动的可回馈空间。玩法仍向 florr 致敬，节奏与内容则按灰烬自己的服走。

---

## 技术速览（继承自 gardn）

| 部分 | 说明 |
|------|------|
| **ECS** | 自制实体组件系统；宏与结构见 `Shared/EntityDef.hh`、`Shared/Entity.hh` |
| **Server** | 原生 uWebSockets，或 Node.js + WASM 备用路径；均匀网格宽相碰撞 |
| **Client** | C++ → WASM/JS（emscripten）；Canvas2D 渲染；`Client/Ui` 自研 UI，几乎不用 HTML/JS |

资源与部分动画来自原作相关脚本逆向（见 `Scripts/`），逻辑实现为自定义。

---

## 许可与致谢

- 上游项目：[gardn](https://github.com/trigonal-bacon/gardn) by trigonal-bacon
- 本仓库：[aqiangei/ashflor](https://github.com/aqiangei/ashflor) · 灰烬

灵感来自 florr.io；与官方无隶属关系。许可证见 [LICENSE](./LICENSE)。
