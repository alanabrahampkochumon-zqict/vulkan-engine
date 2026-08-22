echo "Update the script with your VulkanSDK Path/Version!"
echo "Compiling Shaders..."
/home/user/VulkanSDK/1.4.350.0/x86_64/bin/slangc shader.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o slang.spv
echo "Shaders compiled successfully!"
