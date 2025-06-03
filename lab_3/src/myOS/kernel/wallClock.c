void setWallClock(int HH,int MM,int SS){
	//TODO 你需要填写它
	char timeStr[9];
	timeStr[0] = (HH / 10) + '0';
	timeStr[1] = (HH % 10) + '0';
	timeStr[2] = ':';
	timeStr[3] = (MM / 10) + '0';
	timeStr[4] = (MM % 10) + '0';
	timeStr[5] = ':';
	timeStr[6] = (SS / 10) + '0';
	timeStr[7] = (SS % 10) + '0';
	timeStr[8] = '\0';

	unsigned short *vga = (unsigned short*)0xB8000;
    int pos = 24 * 80 + 70;  // 右下角位置
    for (int i = 0; i < 8; i++) {
        vga[pos + i] = (0x07 << 8) | timeStr[i];  // 灰底白字
    }
}
void getWallClock(int *HH,int *MM,int *SS){
	//TODO 你需要填写它
	unsigned short *vga = (unsigned short*)0xB8000;
	int pos = 24 * 80 + 70;  // 右下角位置
	char timeStr[9];
	for (int i = 0; i < 8; i++) {
		timeStr[i] = vga[pos + i] & 0xFF;  // 获取字符
	}
	timeStr[8] = '\0';  // 添加字符串结束符

	*HH = (timeStr[0] - '0') * 10 + (timeStr[1] - '0');
	*MM = (timeStr[3] - '0') * 10 + (timeStr[4] - '0');
	*SS = (timeStr[6] - '0') * 10 + (timeStr[7] - '0');
}
