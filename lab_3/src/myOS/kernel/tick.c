#include "wallClock.h"
int system_ticks = 0;
int HH = 8, MM = 30, SS = 0;
//NOTE:你可以自行定义接口来辅助实现


void tick(void){
	//TODO 你需要填写它
	system_ticks++;
	if (system_ticks % 100 == 0) {  // 每秒更新
        SS++;
        if (SS >= 60) { SS=0; MM++;
        if (MM >= 60) { MM=0; HH++;
        if (HH >= 24) HH=0; }}}
    setWallClock(HH, MM, SS);  // 更新显示

	return;
}

