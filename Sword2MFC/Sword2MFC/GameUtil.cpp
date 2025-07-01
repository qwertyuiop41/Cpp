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
	0x75, 0x08,                        // jne 0x00730015 (�����ת 8�ֽ�)
	0xC7, 0x44, 0x24, 0x0C, 0x00, 0x00, 0x00, 0x00, // mov [esp+0C], 0x00000000
	0xE9, 0x2E, 0xFD, 0xCE, 0xFF       // jmp Sword2.exe+1FD48�������������ַҲҪ�������¼�������ƫ��
	};
	char hookCode[8] = {
	0xE9, 0xBB, 0x02, 0x31, 0x00, // jmp 0x00730000�������������ַ��Ҫ�滻��
	0x90,                         // nop
	0x90,                         // nop
	0x90                          // nop
	};
	DWORD oldProtect;
	// 1. ����Ѿ����ù��޵й��ܣ�����Ҫ�ظ������ڴ棬ֱ���ҵ����������������޸ģ��޸�ǰ8���ֽڣ�jmp���޵��߼���
	if (invincibleFun != NULL) {
		VirtualProtect((LPVOID)attackFun, 0x8, PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy((LPVOID)attackFun, hookCode, sizeof(hookCode));
		VirtualProtect((LPVOID)attackFun, 8, oldProtect, &oldProtect);
		return TRUE; // �Ѿ����ù��޵й��ܣ�ֱ�ӷ��سɹ�
	}


	// 2. �����һ�������޵й��ܣ�����Ҫ�����ڴ�
	else
	{	
		// 2.1. �����ڴ�
		char* pMemory = (char*)VirtualAlloc( NULL, 1024, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (pMemory==NULL)
		{
			return FALSE;
		}
		invincibleFun = (DWORD)pMemory;

		// 2.2. ��������ڴ�ռ���д���޵��߼�
		DWORD funReturn = attackFun + sizeof(hookCode);
		DWORD offset2 = funReturn - (DWORD)pMemory - sizeof(code); // ������תƫ����
		SIZE_T realWrite2 = 0;
		memcpy(code + 22, &offset2, 4); // ��ƫ�������µ�ַ��д��code
		memcpy( (char*)pMemory, code, sizeof(code)); // ��codeд��������ڴ�ռ�

		// 2.3. �ҵ�����������λ��д��hookcode������ԭʼ����
		DWORD offset = invincibleFun - attackFun - 5; // ������תƫ����
		memcpy(hookCode + 1, &offset, 4); // ��ƫ�������µ�ַ��д��hookCode
		VirtualProtect((LPVOID)attackFun, 0x8, PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy(originalCode, (LPVOID)attackFun, sizeof(originalCode)); // ����ԭʼ����
		memcpy((LPVOID)attackFun, hookCode, sizeof(hookCode));
		VirtualProtect((LPVOID)attackFun, 8, oldProtect, &oldProtect);

		return TRUE; // �ɹ������޵й���
	}
	return 0;
}

BOOL GameUtil::closeInvincibility()
{

	if (invincibleFun==NULL)
	{
		return TRUE;
	}
	// �ҵ�������������hookcode�滻Ϊԭʼ����
	DWORD oldProtect;
	VirtualProtect((LPVOID)attackFun, 8, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy((LPVOID)attackFun, originalCode, sizeof(originalCode));
	VirtualProtect( (LPVOID)attackFun, 8, oldProtect, &oldProtect);


	return True;
}


