# Ra2Tool

# 开发文档

参考：[RA2YurisRevengeTrainer: 红色警戒2 尤里的复仇v1.001 内存修改器](https://github.com/AdjWang/RA2YurisRevengeTrainer)

原理参见：[RA2红色警戒红警2辅助](https://zhupite.com/sec/red-alert2.html)

SDK头文件源自：https://github.com/Phobos-developers/YRpp

```bash
git clone https://github.com/Phobos-developers/YRpp.git
```


# 使用说明
- 使用任何注入器将本DLL注入到游戏主进程即可，启动成功后默认全图；

- ALT + M：钱多多，仅单机模式有效，联网对战会平行世界；

- ALT + L：选中单位升级为三星，仅单机模式有效，联网对战会平行世界；

- ALT + P：开启心灵探测（默认已开启）；

- ALT + B：清除自己下过的所有信标；

- ALT + G：开启全科技，目前只开放了部分兵种；

- ALT + C：打开/关闭箱子提示；

- ALT + R：自动修理，默认修理全部建筑（数量可配置），不需要选中建筑；

- 配置文件为`Ra2Tool.ini`，须放在本模块的同级目录下，选项如下：
  
```ini
; debug 是否开启日志模式，用户排错，默认不开启；
; log 是否开启红警游戏自身的日志输出，用于分析游戏逻辑，默认不开启；
; AutoOpenMap	是否自动开启全图功能，默认开启；
; AutoOpenRadar 是否自动开启雷达功能，默认开启；
; DisableDisguise 是否开启反隐功能，默认1；
; AutoRepairTime 自动维修时间间隔，单位为秒，默认为0表示不进行自动维修；如果设置为整数，表示每隔多少秒自动维修一次建筑；
; RepairCount 自动维修的建筑数量，默认设置一个较大的数用于全部维修，如果每次只维修一个请设置1；
; crate 是否开启箱子提示，默认不开启；
; xinling 是否开启心灵探测，默认不开启；
; CustomTech 是否使用自定义科技，默认使用预定义的科技；如果开启，则会使用[tech]部分的配置来决定开启哪些科技。想要开启哪个科技，自行填写，找到对应的科技类型，后面要开启的单位用空格隔开；
; 7=72 73 表示开启7号科技（建筑物类型）
; 16=8 16 表示开启16号科技（兵种类型）
; 40=0 26 54 16 27 2 14 15 34 37 38 表示开启40号科技（坦克类型）
  
[main]
debug=0
log=0 
AutoOpenMap=1
AutoOpenRadar=1
DisableDisguise=1
AutoRepairTime=0
RepairCount=99
crate=0
xinling=0
CustomTech=0

[tech]
7=18 47 56 64 71 72 73 183
16=8 16
40=0 26 54 16 27 2 14 15 34 37 38
```

# 更新记录
## 20250818
- 科技开启可配置；

## 20250801
- 增加自动维修；
- 增加箱子提示；
- 增加心灵探测；
- 一键清除信标；

## 20250626
- 增加配置文件；
- 反隐身（仅限单机）；
- 无限金钱（仅限单机）；
- 选中单位升级三星（仅限单机）；

## 20241119
- 增加科技全开；

## 20240103
- 实现全图功能；