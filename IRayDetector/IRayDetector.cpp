#include "IRayDetector.h"

#include <qdebug.h>
#include <quuid.h>

#include "Common/Detector.h"
#include "Common/DisplayProgressbar.h"

#include "ApplicatioModeFileHelper.h"

#define TRACE qDebug

#pragma warning(disable:4996)

namespace {
	static CDetector* gs_pDetInstance = nullptr;
	static IRayTimer s_timer;
	static int s_nExpWindow = 0;
	static std::vector<ApplicatioMode> s_appmode;

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
		TRACE(pszMsg);
		qDebug() << "nEventID: " << nEventID
			<< " nEventLevel: " << nEventLevel
			<< " pszMsg: " << pszMsg
			<< " nParam1: " << nParam1
			<< " nParam2: " << nParam2
			<< " nPtrParamLen: " << nPtrParamLen
			<< " pParam: " << pParam;

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
				TRACE("nImageSize %d", nImageSize);
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


IRayDetector& IRayDetector::Instance()
{
	static IRayDetector iRayDetector;
	return iRayDetector;
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
	ret = gs_pDetInstance->Create("D:\\NDT1717MA", SDKCallbackHandler);
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

int IRayDetector::GetAttr(int nAttrID, int& nVal)
{
	return gs_pDetInstance->GetAttr(nAttrID, nVal);
}

int IRayDetector::GetAttr(int nAttrID, float& fVal)
{
	return gs_pDetInstance->GetAttr(nAttrID, fVal);
}

int IRayDetector::GetAttr(int nAttrID, std::string& strVal)
{
	return gs_pDetInstance->GetAttr(nAttrID, strVal);
}

int IRayDetector::UpdateMode(std::string mode)
{
	std::string current_mode;
	int ret = gs_pDetInstance->GetAttr(Attr_CurrentSubset, current_mode);
	if (current_mode == mode)
	{
		qDebug() << "目标模式与当前模式相同，当前模式" << current_mode.c_str();
		return Err_OK;
	}

	ret = gs_pDetInstance->SyncInvoke(Cmd_SetCaliSubset, mode, 50000);
	if (Err_OK != ret)
	{
		qDebug() << "修改探测器工作模式失败！";
		return ret;
	}

	int w{ -1 };
	gs_pDetInstance->GetAttr(Attr_Width, w);
	int h{ -1 };
	gs_pDetInstance->GetAttr(Attr_Height, h);
	int bin{ -1 };
	gs_pDetInstance->GetAttr(Attr_AcqParam_Binning_W, bin);
	int zoom{ -1 };
	gs_pDetInstance->GetAttr(Attr_AcqParam_Zoom_W, zoom);

	qDebug() << "修改探测器工作模式成功，"
		<< " 当前工作模式：" << mode.c_str()
		<< " Attr_Width: " << w
		<< " Attr_Height: " << h
		<< " Attr_AcqParam_Binning_W: " << bin
		<< " Attr_AcqParam_Zoom_W: " << zoom;

	return Err_OK;
}

int IRayDetector::GetCurrentCorrectOption(int& sw_offset, int& sw_gain, int& sw_defect)
{
	int nCurrentCorrectOption{ -1 };
	int ret = gs_pDetInstance->GetAttr(Attr_CurrentCorrectOption, nCurrentCorrectOption);
	if (Err_OK != ret)
	{
		return ret;
	}

	sw_offset = (nCurrentCorrectOption & Enm_CorrectOp_SW_PreOffset) ? 1 : 0;
	sw_gain = (nCurrentCorrectOption & Enm_CorrectOp_SW_Gain) ? 1 : 0;
	sw_defect = (nCurrentCorrectOption & Enm_CorrectOp_SW_Defect) ? 1 : 0;

	return Err_OK;
}

int IRayDetector::SetCorrectOption(int sw_offset, int sw_gain, int sw_defect)
{
	int nCorrectOption{ Enm_CorrectOp_Null };
	if (sw_offset)
	{
		nCorrectOption |= Enm_CorrectOp_SW_PreOffset;
	}

	if (sw_gain)
	{
		nCorrectOption |= Enm_CorrectOp_SW_Gain;
	}
	
	if (sw_defect)
	{
		nCorrectOption |= Enm_CorrectOp_SW_Defect;
	}

	int ret = gs_pDetInstance->SyncInvoke(Cmd_SetCorrectOption, nCorrectOption, 5000);
	if (Err_OK != ret)
	{
		qDebug() << "修改探测器校正模式失败！"
			<< gs_pDetInstance->GetErrorInfo(ret).c_str();
		return ret;
	}

	qDebug() << "修改探测器校正模式成功："
		<< " Enm_CorrectOp_SW_PreOffset: " << sw_offset
		<< " Enm_CorrectOp_SW_Gain: " << sw_gain
		<< " Enm_CorrectOp_SW_Defect: " << sw_defect;
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

