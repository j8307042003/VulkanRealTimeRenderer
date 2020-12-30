

# Real-time Vulkan renderer (WIP).

![image](https://github.com/j8307042003/VulkanRealTimeRenderer/blob/main/screenshots/shot_1.png)
![image](https://github.com/j8307042003/VulkanRealTimeRenderer/blob/main/screenshots/shot_2.png)
![image](https://github.com/j8307042003/VulkanRealTimeRenderer/blob/main/screenshots/shot_3.png)


## Requirement
cmake

vulkan sdk(Molten on mac)

visual studio(windows)


## Build

### Mac
modify this line and put your Molten library path in CMakeLists.txt

```bash	
	#define Molten sdk path here!
	Set(MacVulkanSDK /Users/pine/lib/vulkansdk-macos-1.1.130.0)
```

### Mac and Windows
go to build folder
```bash
cmake ..
cmake --build .
```


### Usage

#### shader compiler tool
execute python script under build/working/shaderBuilder.py


