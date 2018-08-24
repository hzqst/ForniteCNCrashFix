# ForniteCNCrashFix
the driver fix a bug that Fornite Chinese server version always crash under Windows 10.
本驱动解决堡垒之夜国服8月24日版本于win10崩溃闪退的问题，谨慎使用，怕封号的别来。

the driver do a memory patch at FortniteClient-Win64-Shipping+19236A6, replace the instruction "89 34 B8" which lead to crash to "nop nop nop"
本驱动对FortniteClient-Win64-Shipping+19236A6内存位置进行二进制补丁，将导致崩溃的指令"89 34 B8"替换为3个nop

How to use:
put FnCrashFix.sys at C:\
open command prompt as System Administrator, and use the commands below:

sc create FnCrashFix binPath= "\\??\\c:\FnCrashFix.sys type= kernel"
sc start FnCrashFix

Then you can start the game and enjoy.

If you want to unload the driver, use "sc stop FnCrashFix"

如何使用：
把FnCrashFix.sys放置于C盘根目录，以管理员权限打开CMD命令行，输入以下命令

sc create FnCrashFix binPath= "\\??\\c:\FnCrashFix.sys type= kernel"
sc start FnCrashFix

然后进入游戏即可。
