// ZipFindIpAndPort.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "ZipFindIpAndPort.h"
#include <Shellapi.h>
#include <Windowsx.h>
#include <atlconv.h>
#include "SearchIpAndPort.h"
#include <Shlwapi.h>
#include <process.h>
#include <CommCtrl.h>
#include <fstream>


#pragma comment(lib,"Shlwapi.lib")

//-------------------------------------------------------------------------------
std::string g_sLog;
static std::list<IPC_Data> listIpcDatas;
static bool bFinish = false;
static char szAppDir[MAX_PATH]={0};
static char szRetFile[MAX_PATH]={0};
static DWORD dwCastTime = 0;
static char szDownZipDir[MAX_PATH]={0};
static char szDownZipConfig[MAX_PATH]={0};
static SaveFileFromHttp gSafeFile;

extern HWND gMainHwnd=NULL;
extern HWND gParantHwnd=NULL;
extern int gFileIndex = 0;
extern char gszZipFile[4096] = {0};
extern char gszKeys[4096] = {0};
extern int gAnalysisMode = -1; //0����� 1���߳� 2�����̵��߳�
//-------------------------------------------------------------------------------
#define RESULT_FILE_NAME "RET.TXT"
#define DOWN_ZIP_DIR "DownZip\\"
#define DOWN_ZIP_CONFIG "DownUrl.txt"
//-------------------------------------------------------------------------------

void ProcessResultThread(void* p)
{
	while(1)
	{
		if (bFinish)
		{
			//MessageBoxA(NULL,"������ϣ���ʼִ���ļ�д��","��ʾ",MB_OK);
			int nPos = 0,nMax = listIpcDatas.size();
			SendMessage(GetDlgItem(gMainHwnd,IDC_PROGRESS_RESULT),PBM_SETPOS,nPos,0);
			std::map<std::string,int> mapUrls;
			for (auto it = listIpcDatas.begin();it != listIpcDatas.end();it++)
			{
				if (it->bProcessed)
				{
					{
						std::ifstream infile(it->szRetFile);
						char szLine[4096];
						while( infile.getline(szLine,sizeof(szLine)) )
						{
							mapUrls[szLine] = 0;
						}
					}
					
					//ɾ����ʱ�ļ�
					DeleteFileA(it->szRetFile);
				}

				SendMessage(GetDlgItem(gMainHwnd,IDC_PROGRESS_RESULT),PBM_SETPOS,(++nPos)*50/nMax,0);
			}

			nPos = 0;
			nMax = mapUrls.size();
	
			for (auto it = mapUrls.begin();it != mapUrls.end();it++)
			{
				std::ofstream outfile(szRetFile,std::ios::app);
				outfile << it->first<<std::endl;

				SendMessage(GetDlgItem(gMainHwnd,IDC_PROGRESS_RESULT),PBM_SETPOS,50+(++nPos)*50/nMax,0);
			}

			SendMessage(GetDlgItem(gMainHwnd,IDC_PROGRESS_RESULT),PBM_SETPOS,100,0);

			TCHAR szContent[4096];
			Edit_GetText(GetDlgItem(gMainHwnd,IDC_EDIT_RESULT),szContent,_countof(szContent));
			std::string sTmp = T2AString(szContent);
			sTmp += "\r\n��ʱ:";
			sTmp += std::to_string((LONGLONG)(GetTickCount()-dwCastTime));
			sTmp += "ms\r\n";
			sTmp += "ȫ�����,�õ��Ľ����:[";
			sTmp += std::to_string((LONGLONG)nMax);
			sTmp += "][���:RET.TXT]\r\n";
	
			Edit_SetText(GetDlgItem(gMainHwnd,IDC_EDIT_RESULT),A2TString(sTmp.c_str()).c_str());

			MessageBoxA(NULL,"�ļ�������д�����,��鿴RET.TXT��\r\n����ÿ���µĲ���ǰɾ������TXT�ļ�","��ʾ",MB_OK);

			EnableWindow(GetDlgItem(gMainHwnd,IDOK),TRUE);

			listIpcDatas.clear();

			bFinish = false;
		}

		Sleep(1000);
	}
}

BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	gMainHwnd = hwnd;
	Button_SetCheck(GetDlgItem(hwnd,IDC_RADIO_PROCESS),TRUE);
	SetWindowLong(hwnd,GWL_EXSTYLE,GetWindowLong(hwnd,GWL_EXSTYLE)|WS_EX_ACCEPTFILES);

	SendMessage(GetDlgItem(hwnd,IDC_PROGRESS_RESULT),PBM_SETRANGE32,0,100);
	SendMessage(GetDlgItem(hwnd,IDC_PROGRESS_RESULT),PBM_SETPOS,0,0);

	Button_SetCheck(GetDlgItem(hwnd,IDC_RADIO_LOCALFILE),TRUE);

	szAppDir[MAX_PATH-1] = 0;
	GetModuleFileNameA(NULL,szAppDir,MAX_PATH-1);
	strrchr(szAppDir,'\\')[1] = 0;
	sprintf(szRetFile,"%s%s",szAppDir,RESULT_FILE_NAME);
	sprintf(szDownZipDir,"%s%s",szAppDir,DOWN_ZIP_DIR);
	sprintf(szDownZipConfig,"%s%s",szAppDir,DOWN_ZIP_CONFIG);

	_beginthread(ProcessResultThread,0,0);
	return TRUE;
}

void Cls_OnDropFiles(HWND hwnd, HDROP hdrop)
{
	std::string sDragFilePath="";
	int nFiles = ::DragQueryFile(hdrop,0xFFFFFFFF,NULL,0);
	for (int i=0;i<nFiles;i++)
	{
		TCHAR szDropFile[MAX_PATH]={0};
		::DragQueryFile(hdrop,i,szDropFile,MAX_PATH);
		if (i>0) sDragFilePath += ";";
		sDragFilePath += T2AString(szDropFile);
	}
	

	Edit_SetText(GetDlgItem(hwnd,IDC_EDIT_PATH),A2TString(sDragFilePath.c_str()).c_str());
}

void AnalysisIpAndPortZipPravite(LPCSTR pszZipFile,LPCSTR pszKey)
{
	SearchIpAndPort s;

	std::string sKey = pszKey;
	size_t s_,snpos = std::string::npos;
	do 
	{
		std::string sFindKey;
		s_ = sKey.find(";");
		if (s_ != snpos)
		{
			sFindKey = sKey.substr(0,s_);
			sKey = sKey.substr(s_+1,snpos);
		}
		else
		{
			sFindKey = sKey;
		}

		s.AddKey(sFindKey.c_str());


	} while (s_!=snpos);

	int nZip = OpenZip(pszZipFile,NULL);
	if (nZip != 0)
	{
		char szFileList[4096]={0},*pFileName;
		GetZipFileList(nZip,szFileList);
		pFileName = szFileList;
		if (szFileList[0]!=0)
		{
			while(*pFileName)
			{
				int nSize = GetZipFileSize(nZip,pFileName);
				char* pData = new char[nSize+1];
				pData[nSize] = 0;
				ReadZipFile(nZip,pFileName,NULL,pData,nSize);

				//auto list = s.GetIpAndport(pszZipFile,pFileName,pData);
				auto list = s.GetIpAndportRegex(pszZipFile,pFileName,pData);
				//int nIndex = ListBox_GetCount(GetDlgItem(hwnd,IDC_LIST_IP_PORT));
				//g_sLog += "\t";
				//g_sLog += pFileName;
				//g_sLog += "\r\n";
				//for (auto it = list.begin();it != list.end();it++)
				//{
				//	//ListBox_InsertString(GetDlgItem(hwnd,IDC_LIST_IP_PORT),nIndex++,A2TString(it->c_str()).c_str());
				//	g_sLog	+= "\t\t";
				//	g_sLog	+= *it;
				//	g_sLog	+= "\r\n";
				//	Edit_SetText(GetDlgItem(hwnd,IDC_EDIT_RESULT),A2TString(g_sLog.c_str()).c_str());
				//}
				delete pData;

				pFileName += (strlen(pFileName)+1);
			}

			CloseZip(nZip,false);

			SendResultNotify(pszZipFile,s.GetRetFileName(),s.GetRetCnt());
			
			return;
		}

		CloseZip(nZip,false);
	}

	SendResultNotify(0,0,0);
}

void AnalysisIpAndPortZipMultiThread(void* p)
{
	char szThd[2][sizeof(gszKeys)];
	char** pszThd = (char**)p;
	strncpy(szThd[0],pszThd[0],sizeof(gszKeys));
	strncpy(szThd[1],pszThd[1],sizeof(gszKeys));
	delete[] pszThd[0];
	delete[] pszThd[1];
	delete pszThd;

	AnalysisIpAndPortZipPravite(szThd[0],szThd[1]);

	_endthread();
}

void AnalysisIpAndPortZipMultiProcess(LPCSTR pszZipFile,LPCSTR pszKey)
{
	//SendMessage(GetDlgItem(gMainHwnd,IDC_PROGRESS_RESULT),PBM_SETPOS,listIpcDatas.size()*100/gFileIndex,0);

	char szExe[MAX_PATH],szParam[MAX_PATH];
	GetModuleFileNameA(NULL,szExe,MAX_PATH);
	memset(szParam,0,sizeof(szParam));
	_snprintf(szParam,MAX_PATH-1,"%s %u,%d,%u,%s,%s",szExe,gMainHwnd,gAnalysisMode,gFileIndex,pszZipFile,pszKey);
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

	if( ::CreateProcessA(NULL,szParam,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi) )
	{
		// Wait until child process exits.
		//WaitForSingleObject( pi.hProcess, INFINITE );

		// Close process and thread handles. 
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );

	}

}

void AnalysisIpAndPortZip(LPCSTR pszZipFile,LPCSTR pszKey)
{
	//g_sLog += pszZipFile;
	//g_sLog += "\r\n";
	bool bProcessed = false;
	InterlockedExchangeAdd((long*)&gFileIndex,1);

	if (gMainHwnd)
	{
		TCHAR szContent[4096];
		Edit_GetText(GetDlgItem(gMainHwnd,IDC_EDIT_RESULT),szContent,_countof(szContent));
		std::string sTmp = T2AString(szContent);
		sTmp += pszZipFile;
		sTmp += " �������>>>>>>>>>>>\r\n";
		Edit_SetText(GetDlgItem(gMainHwnd,IDC_EDIT_RESULT),A2TString(sTmp.c_str()).c_str());

		if (gAnalysisMode == 0)
		{
			AnalysisIpAndPortZipMultiProcess(pszZipFile,pszKey);
			bProcessed = true;
		}
		else if (gAnalysisMode == 1)
		{
			char** szThd = new char*[2];
			
			szThd[0] = new char[sizeof(gszKeys)];
			szThd[1] = new char[sizeof(gszKeys)];
			strncpy(szThd[0],pszZipFile,sizeof(gszKeys));
			strncpy(szThd[1],pszKey,sizeof(gszKeys));
			_beginthread(AnalysisIpAndPortZipMultiThread,0,szThd);
			bProcessed = true;
		}
	}

	if (bProcessed==false) 
	{
		AnalysisIpAndPortZipPravite(pszZipFile,pszKey);
	}
}

void AnalysisIpAndPortDir(LPCSTR pszPath,LPCSTR pszKey)
{
	char szFind[MAX_PATH];
	char szFile[MAX_PATH];

	WIN32_FIND_DATAA FindFileData;

	strcpy(szFind,pszPath);
	strcat(szFind,"\\*.*" );

	HANDLE hFind=::FindFirstFileA(szFind,&FindFileData);
	if(INVALID_HANDLE_VALUE == hFind)    return;

	while(TRUE)
	{
		if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(FindFileData.cFileName[0]!='.')
			{
				strcpy(szFile,pszPath);
				strcat(szFile,"\\");
				strcat(szFile,FindFileData.cFileName);
				AnalysisIpAndPortDir(szFile,pszKey);
			}
		}
		else
		{
			strcpy(szFile,pszPath);
			strcat(szFile,"\\" );
			strcat(szFile,FindFileData.cFileName);

			AnalysisIpAndPortZip(szFile,pszKey);
		}
		if(!FindNextFileA(hFind,&FindFileData))    break;
	}
	FindClose(hFind);
}

void AnalysisIpAndPortThread(void* p)
{
	CreateAnalysisPath Path;
	Path.sFile = ((CreateAnalysisPath*)p)->sFile;
	Path.sKey = ((CreateAnalysisPath*)p)->sKey;
	delete p;

	std::string sFile = Path.sFile,sKey = Path.sKey;

	if(PathIsDirectoryA(sFile.c_str()))
	{
		AnalysisIpAndPortDir(sFile.c_str(),sKey.c_str());
	}
	else
	{
		AnalysisIpAndPortZip(sFile.c_str(),sKey.c_str());
	}

	if(gAnalysisMode == 2)bFinish = true;

	_endthread();
}

void AnalysisIpAndPort(LPCSTR szFile,LPCSTR szKey)
{
	std::string sPath = szFile,sKey = szKey;

	if(sPath.empty()) {
		MessageBoxA(NULL,"���ṩzip�ļ�","��ʾ",MB_OK);
		return;
	}

	if (sKey.empty())
	{
		if (MessageBoxA(NULL,"��ȷ��Ҫ�����޹ؼ���ȫ��������\r\nȫ���������ܽ����׼ȷ��","��ܰ��ʾ",MB_YESNO)==IDNO)
		{
			return;
		}
	}

	
	size_t s_,snpos = std::string::npos;
	do 
	{
		std::string sFile;
		s_ = sPath.find(";");
		if (s_ != snpos)
		{
			sFile = sPath.substr(0,s_);
			sPath = sPath.substr(s_+1,snpos);
		}
		else
		{
			sFile = sPath;
		}

		/*if(PathIsDirectoryA(sFile.c_str()))
		{
			AnalysisIpAndPortDir(sFile.c_str(),sKey.c_str());
		}
		else
		{
			AnalysisIpAndPortZip(sFile.c_str(),sKey.c_str());
		}*/
		CreateAnalysisPath* p = new CreateAnalysisPath;
		p->sFile = sFile;
		p->sKey = sKey;
		_beginthread(AnalysisIpAndPortThread,0,p);

	} while (s_!=snpos);
}

LRESULT Cls_OnCopyData(HWND hwnd,HWND hRemoteHwnd,PCOPYDATASTRUCT pCopyData)
{
	IPC_Data data;
	memcpy(&data,pCopyData->lpData,pCopyData->cbData);
	listIpcDatas.push_back(data);

	PostMessage(GetDlgItem(hwnd,IDC_PROGRESS_RESULT),PBM_SETPOS,listIpcDatas.size()*100/gFileIndex,0);

	TCHAR szContent[4096];
	Edit_GetText(GetDlgItem(hwnd,IDC_EDIT_RESULT),szContent,_countof(szContent));
	std::string sTmp = T2AString(szContent);
	sTmp += data.szFile;
	sTmp += " ���<<<<<<<<<<\r\n";
	Edit_SetText(GetDlgItem(hwnd,IDC_EDIT_RESULT),A2TString(sTmp.c_str()).c_str());
	if(gAnalysisMode!=2) bFinish = (listIpcDatas.size()==gFileIndex);

	return 0L;
}

void OnSelFileSource(HWND hwnd, int id, HWND hwndCtl)
{
	switch(id)
	{
	case IDC_RADIO_LOCALFILE:
		{
			Edit_Enable(GetDlgItem(hwnd,IDC_EDIT_PATH),TRUE);
			Edit_SetText(GetDlgItem(hwnd,IDC_EDIT_PATH),TEXT(""));
			break;
		}
	case IDC_RADIO_DOWNFILE:
		{
			Edit_Enable(GetDlgItem(hwnd,IDC_EDIT_PATH),FALSE);
			Edit_SetText(GetDlgItem(hwnd,IDC_EDIT_PATH),A2TString(szDownZipDir).c_str());
			break;
		}
	

	}
}

void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id)
	{
	case IDOK:
		{
			if (Button_GetCheck( GetDlgItem(hwnd,IDC_RADIO_DOWNFILE)) == TRUE )
			{
				_mkdir(szDownZipDir);
				
				gSafeFile.DownFromHttp(szDownZipConfig,szDownZipDir);
				::SetTimer(hwnd,1,1000,NULL);
				EnableWindow(hwnd,FALSE);

				TCHAR szContent[4096];
				Edit_GetText(GetDlgItem(hwnd,IDC_EDIT_RESULT),szContent,_countof(szContent));
				std::string sTmp = T2AString(szContent);
				sTmp = "��ʼ�����ļ�\r\n";
				Edit_SetText(GetDlgItem(hwnd,IDC_EDIT_RESULT),A2TString(sTmp.c_str()).c_str());
				return;
			}

			bFinish = false;
			gFileIndex = 0;
			dwCastTime = GetTickCount();
			SendMessage(GetDlgItem(hwnd,IDC_PROGRESS_RESULT),PBM_SETPOS,0,0);

			TCHAR szFile[sizeof(gszZipFile)]={0};
			Edit_GetText(GetDlgItem(hwnd,IDC_EDIT_PATH),szFile,sizeof(szFile));
			TCHAR szKey[sizeof(gszKeys)]={0};
			Edit_GetText(GetDlgItem(hwnd,IDC_EDIT_KEYWORD),szKey,sizeof(szKey));

			TCHAR szContent[4096];
			Edit_GetText(GetDlgItem(hwnd,IDC_EDIT_RESULT),szContent,_countof(szContent));
			std::string sTmp = T2AString(szContent);
			sTmp = "��ʼ�����ļ�[";
			sTmp += T2AString(szFile);
			sTmp += "]\r\n==============================================\r\n";
			Edit_SetText(GetDlgItem(hwnd,IDC_EDIT_RESULT),A2TString(sTmp.c_str()).c_str());

			if (Button_GetCheck(GetDlgItem(hwnd,IDC_RADIO_PROCESS)) == TRUE)
			{
				gAnalysisMode = 0;
			}
			else if (Button_GetCheck(GetDlgItem(hwnd,IDC_RADIO_THREAD)) == TRUE)
			{
				gAnalysisMode = 1;
			}
			else if (Button_GetCheck(GetDlgItem(hwnd,IDC_RADIO_SINGLE_PROCE_THREAD)) == TRUE)
			{
				gAnalysisMode = 2;
			}
			
			AnalysisIpAndPort(T2AString(szFile).c_str(),T2AString(szKey).c_str());

			EnableWindow(GetDlgItem(hwnd,IDOK),FALSE);

			break;
		}
	case IDCANCEL:
		{
			EndDialog(hwnd,id);
			break;
		}
	case IDC_BN_CLEAR:
		{
			Edit_SetText(GetDlgItem(hwnd,IDC_EDIT_RESULT),TEXT(""));
			break;
		}
	case IDC_RADIO_LOCALFILE:
	case IDC_RADIO_DOWNFILE:
		{
			OnSelFileSource(hwnd,id,hwndCtl);
			break;
		}
	}
}

INT_PTR CALLBACK DialogProc(
	__in  HWND hwndDlg,
	__in  UINT uMsg,
	__in  WPARAM wParam,
	__in  LPARAM lParam
	)
{
	switch(uMsg)
	{
		HANDLE_MSG(hwndDlg,WM_INITDIALOG,Cls_OnInitDialog); return TRUE;
		HANDLE_MSG(hwndDlg,WM_COMMAND,Cls_OnCommand);  return TRUE;
		HANDLE_MSG(hwndDlg,WM_DROPFILES,Cls_OnDropFiles); return TRUE;
		HANDLE_MSG(hwndDlg,WM_COPYDATA,Cls_OnCopyData);return TRUE;
	case MSG_DOWN_FINISH:
		{
			EnableWindow(hwndDlg,TRUE);
			Button_SetCheck( GetDlgItem(hwndDlg,IDC_RADIO_LOCALFILE),TRUE) ;
			Button_SetCheck( GetDlgItem(hwndDlg,IDC_RADIO_DOWNFILE),FALSE) ;
			PostMessage(hwndDlg,WM_COMMAND,IDOK,0);
			KillTimer(hwndDlg,1);

			TCHAR szContent[4096];
			Edit_GetText(GetDlgItem(hwndDlg,IDC_EDIT_RESULT),szContent,_countof(szContent));
			std::string sTmp = T2AString(szContent);
			sTmp = "��������ļ�,׼����ʼ����\r\n";
			Edit_SetText(GetDlgItem(hwndDlg,IDC_EDIT_RESULT),A2TString(sTmp.c_str()).c_str());

			return TRUE;
		}
	case WM_TIMER:
		{
			if(wParam == 1)
			{
				SendMessage(GetDlgItem(hwndDlg,IDC_PROGRESS_RESULT),PBM_SETPOS,gSafeFile.GetCurCnt()*100/gSafeFile.GetTotal(),0);
			}
			break;
		}
	}

	return 0L;
}


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	std::string sUrl = "�Ұ���.com";
	sUrl = urlencode(sUrl);
	sUrl = urldecode(sUrl);
	sUrl = URLEncode(sUrl);
	sUrl = URLDecode(sUrl);
	//MessageBox(NULL,lpCmdLine,NULL,MB_OK);
	sscanf(T2AString(lpCmdLine).c_str(),"%u,%d,%u,%[^,],%s",&gParantHwnd,&gAnalysisMode,&gFileIndex,gszZipFile,gszKeys);
	if (gParantHwnd)
	{
		//ִ���ӽ��̲���
		if (gAnalysisMode==0)
		{
			AnalysisIpAndPortZipPravite(gszZipFile,gszKeys);
		}
	}
	else
	{
		DialogBox(hInstance,MAKEINTRESOURCE(IDD_MAIN_DLG),NULL,DialogProc);
	}
	
	return 0;
}