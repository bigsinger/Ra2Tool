// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 RA2DLL_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// RA2DLL_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef RA2DLL_EXPORTS
#define RA2DLL_API __declspec(dllexport)
#else
#define RA2DLL_API __declspec(dllimport)
#endif

// 此类是从 dll 导出的
class RA2DLL_API CRa2Dll {
public:
	CRa2Dll(void);
	// TODO: 在此处添加方法。
};

extern RA2DLL_API int nRa2Dll;

RA2DLL_API int fnRa2Dll(void);

//地图全开
void openAllMap();