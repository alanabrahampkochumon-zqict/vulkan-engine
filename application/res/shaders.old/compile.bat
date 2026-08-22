echo Compiling Shaders...

%VULKAN_SDK%/Bin/glslc.exe basic.vert -o basic.vert.spv
%VULKAN_SDK%/Bin/glslc.exe basic.frag -o basic.frag.spv

echo Shader Compilation Complete!