#pragma once
class GameUtil
{
public:
	DWORD attackFun;
	DWORD invincibleFun;
	char originalCode[8]{};
	GameUtil();
	BOOL setInvincibility();
	BOOL closeInvincibility();
};

