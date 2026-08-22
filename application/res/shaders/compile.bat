echo Compiling Shaders...
%VULKAN_SDK%/Bin/slangc.exe basic.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o slang.spv

echo Shader Compilation Complete!