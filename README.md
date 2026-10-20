# 灰烬 · ashflor

自托管的多人 florr 风格游戏。基于 [gardn](https://github.com/trigonal-bacon/gardn) fork，C++ 编写。

玩法启发自 [florr.io](https://static.florr.io/old/)，与官方无关。

## 游玩

- [https://ashflor.com/](https://ashflor.com/)（推荐）
- [http://ashflor.com/](http://ashflor.com/) · [http://www.ashflor.com/](http://www.ashflor.com/)

自己开服看 [INSTALLATION.md](./INSTALLATION.md)。

## 已做

相对 gardn 上游：账号存档、组队、公会、聊天、合成、中文界面与 i18n，以及地图 / 数值 / UI 调整。

线上已落地的改动：

- 聊天气泡在头顶显示约 5 秒
- 玩家名字左下角、怪物稀有度右下角
- Nginx 反代免端口访问（域名入口）
- 重生卡死修复；客户端资源本地长缓存
- 左下角侧栏改为 IconPark 图标（背包 / 天赋 / 合成 / 特殊）
- 左上角更新日志按钮，局内可开
- 登录后固定显示账号游戏名（不再每次起名）
- 滚轮缩放视角；背包区域放大、滚轮滑动
- ESC 菜单扩展：移动方式、反转攻防
- 删除槽靠右；装备拖出回背包；同种花瓣堆叠 xN
- 丢弃槽扔地上可捡；T 键卸载；背包搜索 / 堆叠 / 整理
- **背包统一**：卸下与拾取满格都进背包堆叠（无单独仓库）；稀有度色块强化、滚动条靠右
- 靠近地图 Boss 时屏幕顶部大红血条（名称 + 百分比）
- 等级经验线性、死亡不回退；每升一级 +1 天赋点
- 三倍掉落加成可随时开关；点输入框外关闭键盘
- 标题屏提亮卡通风；去掉首页「重生携带」装备整理预览（死亡界面仍可整理）
- 跨群系 / 传说组装花瓣与配方扩充；V 键特殊组装车间
- 大正方形地图、中心出生、稀疏中立缓冲
- 地形锯齿 / 弯曲河 / 螺旋方圆；地形 CCD 防穿墙
- 七大地貌：花园 / 沙漠 / 海洋 / 雪原 / 丛林 / 蚁穴 / 火山
- 丛林可玩：密林迷宫、沼河、树根 / 树心生态；树心 Boss 阶段
- 王城可进：六区城墙迷宫、王城怪 / 瓣、堕落国王三阶段 Boss
- 场内传送：花园·沙漠 ↔ 王城，王城北门 → 火山
- 断线或重部署等待页，带接花瓣小游戏
- 公会：创建 / 邀请 / 公会聊天；社交面板
- 移动端：触控放大、浮动摇杆、圆形攻防键、摇杆避让侧栏；安全区 UI / 背包展开
- 空服 AI Bot；装备槽稀有度底色
- 死亡不掉瓣；退回主页软离场保留花瓣
- 手感与平衡：移速软曲线、花瓣轨道、Stinger 阶梯、野怪 leash、掉落多 roll、AOI / 插值微调
- `./deploy.sh` 零停机发版 + nginx `?v=` 路由；错版客户端自动硬刷

## 技术

| 部分 | 说明 |
|------|------|
| ECS | 自制；见 `Shared/EntityDef.hh`、`Shared/Entity.hh` |
| Server | uWebSockets，或 Node.js + WASM；默认 canonical 空间哈希 |
| Client | emscripten → WASM/JS；Canvas2D；`Client/Ui` 自研 UI |
| 发版 | `./deploy.sh` + nginx `?v=` 路由；错版客户端自动硬刷 |

默认编译开 `USE_CODEPOINT_LEN`（中文截断）与 `GENERAL_SPATIAL_HASH`（大体型碰撞）。

## 许可

- 上游：[gardn](https://github.com/trigonal-bacon/gardn)
- 本仓库：[aqiangei/ashflor](https://github.com/aqiangei/ashflor)

见 [LICENSE](./LICENSE)。
