#include "IRayDetector.h"

#include <qdebug.h>
#include <quuid.h>

#include "Common/Detector.h"
#include "Common/DisplayProgressbar.h"

#define TRACE qDebug

namespace {
	static CDetector* gs_pDetInstance = nullptr;
	static IRayTimer s_timer;
	static int s_nExpWindow = 0;

	void TimeProc(int uTimerID)
	{
		s_nExpWindow -= 1;
		if (0 == s_nExpWindow)
		{
			s_timer.Close();
			return;
		}
		TRACE("Please expose in %ds\r", s_nExpWindow);
	}

	void SDKCallbackHandler(int nDetectorID, int nEventID, int nEventLevel,
		const char* pszMsg, int nParam1, int nParam2, int nPtrParamLen, void* pParam)
	{
		gs_pDetInstance->SDKCallback(nDetectorID, nEventID, nEventLevel, pszMsg, nParam1, nParam2, nPtrParamLen, pParam);
		switch (nEventID)
		{
		case Evt_ConnectProcess:
			TRACE(pszMsg);
			break;
		case Evt_Exp_Enable:
			TRACE("Prepare to expose\n");
			s_timer.Init(TimeProc, 1000);
			s_nExpWindow = nParam1 / 1000;
			TRACE("Please expose in %ds\r", s_nExpWindow);
			break;
		case Evt_Image:
			TRACE("\nGot image\n");
			{
				//must make deep copies of pParam
				IRayImage* pImg = (IRayImage*)pParam;
				unsigned short* pImageData = pImg->pData;
				int nImageSize = pImg->nWidth * pImg->nHeight * pImg->nBytesPerPixel;
				int nFrameNo = gs_pDetInstance->GetImagePropertyInt(&pImg->propList, Enm_ImageTag_FrameNo);
			}
			break;
		default:
			break;
		}
	}
}

IRayDetector::IRayDetector(QObject* parent)
	: QObject(parent)
{
	m_uuid = QUuid::createUuid().toString();
	TRACE() << "构造探测器实例：" << m_uuid;
}

IRayDetector::~IRayDetector()
{
	TRACE() << "析构探测器实例：" << m_uuid;
}


int IRayDetector::Initializte()
{
	gs_pDetInstance = new CDetector();
	TRACE("Load libray");
	int ret = gs_pDetInstance->LoadIRayLibrary();
	if (Err_OK != ret)
	{
		TRACE("\t\t\t[No ]\n");
		return ret;
	}
	else
		TRACE("\t\t\t[Yes]\n");

	TRACE("Create instance");
	ret = gs_pDetInstance->Create(GetWorkDirPath().c_str(), SDKCallbackHandler);
	if (Err_OK != ret)
	{
		TRACE("\t\t\t[No ] - error:%s\n", gs_pDetInstance->GetErrorInfo(ret).c_str());
		return ret;
	}
	else
		TRACE("\t\t\t[Yes]\n");

	TRACE("Connect device");
	ret = gs_pDetInstance->SyncInvoke(Cmd_Connect, 30000);
	if (Err_OK != ret)
	{
		TRACE("\t\t\t[No ] - error:%s\n", gs_pDetInstance->GetErrorInfo(ret).c_str());
		return ret;
	}
	else
		TRACE("\t\t\t[Yes]\n");

	return ret;
}

void IRayDetector::DeInitializte()
{
	if (gs_pDetInstance)
	{
		gs_pDetInstance->Destroy();
		gs_pDetInstance->FreeIRayLibrary();
		delete gs_pDetInstance;
		gs_pDetInstance = NULL;
	}
}

void IRayDetector::SingleAcq()
{
	TRACE("Set to SyncOut mode\n");
	gs_pDetInstance->SetAttr(Attr_UROM_FluroSync_W, Enm_FluroSync_SyncOut);
	gs_pDetInstance->SyncInvoke(Cmd_WriteUserRAM, 4000);
	int nExposeWindowTime = 5000;
	int nTimeOut = nExposeWindowTime + 2000;
	gs_pDetInstance->SetAttr(Cfg_ClearAcqParam_DelayTime, nExposeWindowTime);
	gs_pDetInstance->SyncInvoke(Cmd_ClearAcq, nTimeOut);
}

void IRayDetector::StartSeqAcq()
{
	TRACE("Sequence acquiring...\n");
	gs_pDetInstance->Invoke(Cmd_StartAcq);
}

void IRayDetector::StopSeqAcq()
{
	TRACE("Stop Sequence acquiring...\n");
	gs_pDetInstance->SyncInvoke(Cmd_StopAcq, 2000);
}

