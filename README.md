# 灰烬 · ashflor

自托管的多人 PvP florr 风格游戏。基于 [gardn](https://github.com/trigonal-bacon/gardn) fork，C++ 编写。

玩法启发自 [florr.io](https://static.florr.io/old/)，与官方无关。

## 游玩

- [https://ashflor.com/](https://ashflor.com/)（推荐）
- [http://ashflor.com/](http://ashflor.com/) · [http://www.ashflor.com/](http://www.ashflor.com/)

自己开服看 [INSTALLATION.md](./INSTALLATION.md)。

## 已做

相对 gardn 上游：账号存档、组队、聊天、合成、中文界面与 i18n，以及地图 / 数值 / UI 调整。

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
- 大正方形地图、中心出生、稀疏中立缓冲
- 地形锯齿 / 弯曲河 / 螺旋方圆
- 断线或重部署时等待页，带接花瓣小游戏

## 技术

| 部分 | 说明 |
|------|------|
| ECS | 自制；见 `Shared/EntityDef.hh`、`Shared/Entity.hh` |
| Server | uWebSockets，或 Node.js + WASM；均匀网格碰撞 |
| Client | emscripten → WASM/JS；Canvas2D；`Client/Ui` 自研 UI |

## 许可

- 上游：[gardn](https://github.com/trigonal-bacon/gardn)
- 本仓库：[aqiangei/ashflor](https://github.com/aqiangei/ashflor)

见 [LICENSE](./LICENSE)。
