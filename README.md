Sword2/p161 实现无敌功能：

开辟新的内存空间判断是否是主角，是主角就不扣血；修改原函数开头部分，使其跳转到新开辟的内存空间。直接修改硬编码实现。

Sword2/p163 实现自动加血

通过给已有的游戏进程创建一个remote thread，自动调用我们写的函数实现。while(ture)循环内检测hp<500就调用加血方法

Sword2MFC 外挂窗口编写：

编写MFC.exe程序,所有的外挂功能都通过VirutalAllocEx WriteProcessMemry写入到目标进程；编写MFCdll注入到游戏进程。

PE/PE 解析PE文件headers

PE/PE 解析PE文件导出表（导入表/重定位表的解析还未实现）

Vulnerability 通过修改password.txt+栈溢出使进程返回“密码正确”
