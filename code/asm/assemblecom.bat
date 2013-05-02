nasm -f bin -Ox -o sample.com samplecom.asm
@if %errorlevel% neq 0 goto err

ndisasm sample.com

sample.com
pause

@goto don
:err
@pause
:don