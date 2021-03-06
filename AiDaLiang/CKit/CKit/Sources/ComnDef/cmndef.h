#ifndef __cmndef_H
#define __cmndef_H

//常用变量
#define STR_CLIENT_MASTER					TEXT("Client_Master.exe")
#define STR_CLIENT_BUBBLE					TEXT("Small_Ball.exe")
#define STR_CLIENT_PROTECT32				TEXT("MasterProtect.exe")
#define STR_CLIENT_PROTECT64				TEXT("MasterProtect64b.exe")
#define STR_CLIENT_TIP						TEXT("Tip_Window.exe")
#define STR_DRIVER_PD_DLL					TEXT("tmpsys.dll")
#define STR_DRIVER_FR_DLL					TEXT("tmpfilereg.dll")
#define STR_MSG_PROTECT						TEXT("tmpsysprotect.dll")

//网络定义
#define	HEAD_VER							1
#define	APP_VER								7
#define MAX_40K_BUFFER						40*1024
#define MSG_TAG_FLAG						0x20150801

#define STR_NET_KEY							"jdaf832rn,snde329r"
#define LEN_NET_KEY							strlen(STR_NET_KEY)

//互斥锁名称
#define MUTEX_TIPWINDOW						TEXT("MUTEX_TIP_WINDOW")
#define MUTEX_CLIENTMASTER					TEXT("MUTEX_CLIENT_MASTER")

//句柄有效
#define IsValidHandle(h) (h!=INVALID_HANDLE_VALUE && h!=NULL)

//关闭线程
#define SAFE_STOP_THREAD(h) if(IsValidHandle(h)){ ::TerminateThread(h,0);CloseHandle(h); h=NULL; }
//关闭事件
#define SAFE_CLOSE_EVENT(ev) if(IsValidHandle(ev)){::CloseHandle(ev);ev=NULL;}
#define SAFE_CLOSE_HANDLE(ev) if(IsValidHandle(ev)){::CloseHandle(ev);ev=NULL;}

//常用宏


//创建单例运行进程锁
#define CREATE_SINGLE_PROCESS_LOCK(globalName) \
	HANDLE hMutex = CreateMutex(NULL,FALSE,globalName);\
	if(hMutex && GetLastError() == ERROR_ALREADY_EXISTS) { CloseHandle(hMutex);return 0;}
//关闭单例运行进程锁
#define CLOSE_SINGLE_PROCESS_LOCK if(hMutex){ CloseHandle(hMutex); hMutex = NULL; }

//注入测试
#define INJECT_PROCESS_TEST(name) { DWORD dwInjectPId_ = GetProcessIdByName(name);\
	if (dwInjectPId_ != 0)\
{\
	HANDLE h = OpenProcess(PROCESS_ALL_ACCESS,FALSE,dwInjectPId_);\
	if (h)\
	{\
	CloseHandle(h);\
	}\
}}

//-------------------------------------------------------------------------------
// 插件导出函数(如果dll不处理任何宿主传来的消息，可以不用导出)
//-------------------------------------------------------------------------------
#define  PLUGIN_EXPORT_FUNCNAME "CreatePlugInterface"

#if _MSC_VER >= 1400
#	define  IMP_PLUGIN_EXPORT(classname,...) extern "C" __declspec(dllexport) void* CreatePlugInterface(){\
	static classname* pModuleInterface = NULL; if(pModuleInterface == NULL){\
	pModuleInterface = new classname( ##__VA_ARGS__ );\
		}\
		return pModuleInterface;\
	}
#endif

#define  IMP_PLUGIN_EXPORT_EX(clazz_pointer) extern "C" __declspec(dllexport) void* CreatePlugInterface(){\
	return clazz_pointer;\
}

typedef void* (WINAPI *PFN_CreatePlugInterface)();

//-------------------------------------------------------------------------------
// 网络管理接口
//-------------------------------------------------------------------------------
enum enNetMethod
{
	enTCP,
	enUDP,
};

#endif //__cmndef_H