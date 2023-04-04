# JoyEngine
Toy engine written in C++ and Directx 12. 

Trello board with progress: 
https://trello.com/b/7in0jgTy/joyengine

![Main screenshot](https://github.com/drzhn/DemoPicturesAndGifs/blob/main/Screenshot_7.png?raw=true)

## **FEATURES:**

### **DDGI**

Dynamic Diffuse Global Illumination with software raytracing fully on GPU side based on LBVH and Radix Sort. 
![DDGI demo](https://github.com/drzhn/DemoPicturesAndGifs/blob/main/ddgi_demo.gif?raw=true)
Credits:
- N. Satish, M. Harris and M. Garland, "Designing efficient sorting algorithms for manycore GPUs," 2009 IEEE International Symposium on Parallel & Distributed Processing, Rome, Italy, 2009, pp. 1-10
- https://developer.nvidia.com/blog/thinking-parallel-part-iii-tree-construction-gpu/
- Zander Majercik, Jean-Philippe Guertin, Derek Nowrouzezahrai, and Morgan McGuire, Dynamic Diffuse Global Illumination with Ray-Traced Irradiance Fields, _Journal of Computer Graphics Techniques (JCGT)_, vol. 8, no. 2, 1-30, 2019

### **Clustered deferred shading**

![Clusterd shading demo](https://github.com/drzhn/DemoPicturesAndGifs/blob/main/ice_video_20230316-230741.gif?raw=true)

### **Custom deserialization**

Simple rapidjson-based classes deserialization. Usage: 
 
    // in SomeClass.h
    DECLARE_CLASS(SomeClass)
    class SomeClass : public Serializable {
            DECLARE_CLASS_NAME(SomeClass)
            REFLECT_FIELD(float, someFloat);
            REFLECT_FIELD(int, someInt);
    };

### **Separate project for asset building**

All assets are loaded into the engine using custom binary format. I have JoyAssetBuilder project written in C#/WinForms for cooking all that data. In future engine and builder will be compined together in one engine project with custom editor.
![enter image description here](https://github.com/drzhn/DemoPicturesAndGifs/blob/main/Screenshot_8.png?raw=true)

### **Other features**
 - Bindless textures
 - Tonemapping
 - Gamma correction
 - Block compressed textures
 - HDR skybox

### **Building**
Clone this project with dependencies and build JoyEngineDX.sln

### **Dependencies** 

 - [glm](https://github.com/g-truc/glm)
 - [ImGui](https://github.com/ocornut/imgui)
 - [rapidjson](https://github.com/Tencent/rapidjson/releases)
 - [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) (for Asset Builder)

**WARNING**: for GPU sorting part in software raytracing I used new HLSL wave intrinsics for scan stage. So it's obligation to run this project on Nvidia GPUs because of lane size equal to 32. 
DO NOT RUN IT ON AMD
