nasm -f win32 -Ox -o win.obj win.asm
@if %errorlevel% neq 0 goto err

link /machine:x86 /align:16 /opt:ref /nologo /libpath:c:\bin\nasm /subsystem:windows /entry:main user32.lib kernel32.lib win.obj
@if %errorlevel% neq 0 goto err

win.exe

@goto don
:err
@pause
:don