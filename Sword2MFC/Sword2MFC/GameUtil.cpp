#include "pch.h"
#include "GameUtil.h"

GameUtil::GameUtil()
{
	attackFun = 0x0041FD40;
	invincibleFun = NULL;
}

BOOL GameUtil::setInvincibility()
{

	char code[26] = {
	0x56,                               // push esi
	0x8B, 0xF1,                         // mov esi, ecx
	0x57,                               // push edi
	0x81, 0x7E, 0x04, 0x02, 0x00, 0x00, 0x00,  // cmp [esi+04], 0x00000002
	0x75, 0x08,                        // jne 0x00730015 (相对跳转 8字节)
	0xC7, 0x44, 0x24, 0x0C, 0x00, 0x00, 0x00, 0x00, // mov [esp+0C], 0x00000000
	0xE9, 0x2E, 0xFD, 0xCE, 0xFF       // jmp Sword2.exe+1FD48！！！！这个地址也要换成重新计算的相对偏移
	};
	char hookCode[8] = {
	0xE9, 0xBB, 0x02, 0x31, 0x00, // jmp 0x00730000！！！！这个地址是要替换的
	0x90,                         // nop
	0x90,                         // nop
	0x90                          // nop
	};
	DWORD oldProtect;
	// 1. 如果已经设置过无敌功能，则不需要重复申请内存，直接找到攻击函数并进行修改（修改前8个字节，jmp到无敌逻辑）
	if (invincibleFun != NULL) {
		VirtualProtect((LPVOID)attackFun, 0x8, PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy((LPVOID)attackFun, hookCode, sizeof(hookCode));
		VirtualProtect((LPVOID)attackFun, 8, oldProtect, &oldProtect);
		return TRUE; // 已经设置过无敌功能，直接返回成功
	}


	// 2. 如果第一次设置无敌功能：则需要申请内存
	else
	{	
		// 2.1. 申请内存
		char* pMemory = (char*)VirtualAlloc( NULL, 1024, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (pMemory==NULL)
		{
			return FALSE;
		}
		invincibleFun = (DWORD)pMemory;

		// 2.2. 在申请的内存空间中写入无敌逻辑
		DWORD funReturn = attackFun + sizeof(hookCode);
		DWORD offset2 = funReturn - (DWORD)pMemory - sizeof(code); // 计算跳转偏移量
		SIZE_T realWrite2 = 0;
		memcpy(code + 22, &offset2, 4); // 将偏移量（新地址）写入code
		memcpy( (char*)pMemory, code, sizeof(code)); // 将code写入申请的内存空间

		// 2.3. 找到攻击函数的位置写入hookcode并保存原始代码
		DWORD offset = invincibleFun - attackFun - 5; // 计算跳转偏移量
		memcpy(hookCode + 1, &offset, 4); // 将偏移量（新地址）写入hookCode
		VirtualProtect((LPVOID)attackFun, 0x8, PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy(originalCode, (LPVOID)attackFun, sizeof(originalCode)); // 保存原始代码
		memcpy((LPVOID)attackFun, hookCode, sizeof(hookCode));
		VirtualProtect((LPVOID)attackFun, 8, oldProtect, &oldProtect);

		return TRUE; // 成功设置无敌功能
	}
	return 0;
}

BOOL GameUtil::closeInvincibility()
{

	if (invincibleFun==NULL)
	{
		return TRUE;
	}
	// 找到攻击函数并将hookcode替换为原始代码
	DWORD oldProtect;
	VirtualProtect((LPVOID)attackFun, 8, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy((LPVOID)attackFun, originalCode, sizeof(originalCode));
	VirtualProtect( (LPVOID)attackFun, 8, oldProtect, &oldProtect);


	return True;
}


