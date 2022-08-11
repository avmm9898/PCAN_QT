// PCANBasicExampleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCANBasicExample.h"
#include "PCANBasicExampleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// CriticalSection class
//
#pragma region Critical Section Class
clsCritical::clsCritical(CRITICAL_SECTION *cs, bool createUnlocked, bool lockRecursively)
{
    ASSERT(cs != NULL);

    m_objpCS = cs;
    m_dwLocked = -1;
    m_bDoRecursive = lockRecursively;
    m_dwOwnerThread = GetCurrentThreadId();

    if(!createUnlocked)
        Enter();
}

clsCritical::~clsCritical()
{
    int iFail = (int)0x80000000;

    while(m_dwLocked >= 0)
        if(Leave() == iFail)
            break;
}

int clsCritical::Enter()
{
    if(m_dwOwnerThread != GetCurrentThreadId())
        throw "class clsCritical: Thread cross-over error. ";

    try
    {
        if(m_bDoRecursive || (m_dwLocked == -1))
        {
            EnterCriticalSection(m_objpCS);
            InterlockedIncrement(&m_dwLocked);
        }
        return m_dwLocked;
    }
    catch(...)
    {
        return 0x80000000;
    }
}

int clsCritical::Leave()
{
    if(m_dwOwnerThread != GetCurrentThreadId())
        throw "class clsCritical: Thread cross-over error. ";

    try
    {
        if(m_dwLocked >= 0)
        {
            LeaveCriticalSection(m_objpCS);
            InterlockedDecrement(&m_dwLocked);
            return m_dwLocked;
        }
        return -1;
    }
    catch(...)
    {
        return 0x80000000;
    }
}

bool clsCritical::IsLocked()
{
    return (m_dwLocked > -1);
}

int clsCritical::GetRecursionCount()
{
    return m_dwLocked;
}
#pragma endregion

/// <summary>
/// Convert a CAN DLC value into the actual data length of the CAN/CAN-FD frame.
/// </summary>
/// <param name="dlc">A value between 0 and 15 (CAN and FD DLC range)</param>
/// <param name="isSTD">A value indicating if the msg is a standard CAN (FD Flag not checked)</param>
/// <returns>The length represented by the DLC</returns>
static int GetLengthFromDLC(int dlc, bool isSTD)
{
    if (dlc <= 8)
        return dlc;

     if (isSTD)
        return 8;

     switch (dlc)
     {
        case 9: return 12;
        case 10: return 16;
        case 11: return 20;
        case 12: return 24;
        case 13: return 32;
        case 14: return 48;
        case 15: return 64;
        default: return dlc;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////
// MessageStatus class
//
#pragma region Message Status class
MessageStatus::MessageStatus(TPCANMsgFD canMsg, TPCANTimestampFD canTimestamp, int listIndex)
{
    m_Msg = canMsg;
    m_TimeStamp = canTimestamp;
    m_oldTimeStamp = canTimestamp;
    m_iIndex = listIndex;
    m_Count = 1;
    m_bShowPeriod = true;
    m_bWasChanged = false;
}

void MessageStatus::Update(TPCANMsgFD canMsg, TPCANTimestampFD canTimestamp)
{
    m_Msg = canMsg;
    m_oldTimeStamp = m_TimeStamp;
    m_TimeStamp = canTimestamp;
    m_bWasChanged = true;
    m_Count += 1;
}

TPCANMsgFD MessageStatus::GetCANMsg()
{
    return m_Msg;
}

TPCANTimestampFD MessageStatus::GetTimestamp()
{
    return m_TimeStamp;
}

int MessageStatus::GetPosition()
{
    return m_iIndex;
}

CString MessageStatus::GetTypeString()
{
    CString strTemp;
    bool isEcho = (m_Msg.MSGTYPE & PCAN_MESSAGE_ECHO) == PCAN_MESSAGE_ECHO;

    // Add the new ListView Item with the type of the message
    //
    if ((m_Msg.MSGTYPE & PCAN_MESSAGE_STATUS) != 0)
        return "STATUS";

    if ((m_Msg.MSGTYPE & PCAN_MESSAGE_ERRFRAME) != 0)
        return "ERROR";

    if((m_Msg.MSGTYPE & PCAN_MESSAGE_EXTENDED) != 0)
        strTemp = "EXT";
    else
        strTemp = "STD";

    if((m_Msg.MSGTYPE & PCAN_MESSAGE_RTR) == PCAN_MESSAGE_RTR)        
       strTemp += isEcho ? "/RTR [ ECHO ]" : "/RTR";
    else
        if(m_Msg.MSGTYPE > PCAN_MESSAGE_EXTENDED)
        {
            if (isEcho)
                strTemp.Append(" [ ECHO");
            else
                strTemp.Append(" [ ");
            if (m_Msg.MSGTYPE & PCAN_MESSAGE_FD)
                strTemp.Append(" FD");
            if (m_Msg.MSGTYPE & PCAN_MESSAGE_BRS)
                strTemp.Append(" BRS");
            if (m_Msg.MSGTYPE & PCAN_MESSAGE_ESI)
                strTemp.Append(" ESI");
            strTemp.Append(" ]");
        }

    return strTemp;
}

CString MessageStatus::GetIdString()
{
    CString strTemp;

    // We format the ID of the message and show it
    //
    if((m_Msg.MSGTYPE & PCAN_MESSAGE_EXTENDED) != 0)
        strTemp.Format("%08Xh",m_Msg.ID);
    else
        strTemp.Format("%03Xh",m_Msg.ID);

    return strTemp;
}

CString MessageStatus::GetDataString()
{
    CString strTemp, strTemp2;

    strTemp = "";
    strTemp2 = "";

    if((m_Msg.MSGTYPE & PCAN_MESSAGE_RTR) == PCAN_MESSAGE_RTR)
        return "Remote Request";
    else
        for(int i=0; i < GetLengthFromDLC(m_Msg.DLC, !(m_Msg.MSGTYPE & PCAN_MESSAGE_FD)); i++)
        {
            strTemp.Format("%s %02X", strTemp2, m_Msg.DATA[i]);
            strTemp2 = strTemp;
        }

    return strTemp2;
}

CString MessageStatus::GetTimeString()
{
    double fTime;
    CString str;

    fTime = (m_TimeStamp / 1000.0);
    if (m_bShowPeriod)
        fTime -= (m_oldTimeStamp / 1000.0);
    str.Format("%.1f", fTime);

    return str;
}

int MessageStatus::GetCount()
{
    return m_Count;
}

bool MessageStatus::GetShowingPeriod()
{
    return m_bShowPeriod;
}

bool MessageStatus::GetMarkedAsUpdated()
{
    return m_bWasChanged;
}

void MessageStatus::SetShowingPeriod(bool value)
{
    if (m_bShowPeriod ^ value)
    {
        m_bShowPeriod = value;
        m_bWasChanged = true;
    }
}

void MessageStatus::SetMarkedAsUpdated(bool value)
{
    m_bWasChanged = value;
}
#pragma endregion

//////////////////////////////////////////////////////////////////////////////////////////////
// PCANBasicExampleDlg dialog
//
CPCANBasicExampleDlg::CPCANBasicExampleDlg(CWnd* pParent)
    : CDialog(CPCANBasicExampleDlg::IDD, pParent)
    , txtID(_T(""))
, txtLength(_T(""))
, txtData0(_T(""))
, txtData1(_T(""))
, txtData2(_T(""))
, txtData3(_T(""))
, txtData4(_T(""))
, txtData5(_T(""))
, txtData6(_T(""))
, txtData7(_T(""))
, txtData8(_T(""))
, txtData9(_T(""))
, txtData10(_T(""))
, txtData11(_T(""))
, txtData12(_T(""))
, txtData13(_T(""))
, txtData14(_T(""))
, txtData15(_T(""))
, txtData16(_T(""))
, txtData17(_T(""))
, txtData18(_T(""))
, txtData19(_T(""))
, txtData20(_T(""))
, txtData21(_T(""))
, txtData22(_T(""))
, txtData23(_T(""))
, txtData24(_T(""))
, txtData25(_T(""))
, txtData26(_T(""))
, txtData27(_T(""))
, txtData28(_T(""))
, txtData29(_T(""))
, txtData30(_T(""))
, txtData31(_T(""))
, txtData32(_T(""))
, txtData33(_T(""))
, txtData34(_T(""))
, txtData35(_T(""))
, txtData36(_T(""))
, txtData37(_T(""))
, txtData38(_T(""))
, txtData39(_T(""))
, txtData40(_T(""))
, txtData41(_T(""))
, txtData42(_T(""))
, txtData43(_T(""))
, txtData44(_T(""))
, txtData45(_T(""))
, txtData46(_T(""))
, txtData47(_T(""))
, txtData48(_T(""))
, txtData49(_T(""))
, txtData50(_T(""))
, txtData51(_T(""))
, txtData52(_T(""))
, txtData53(_T(""))
, txtData54(_T(""))
, txtData55(_T(""))
, txtData56(_T(""))
, txtData57(_T(""))
, txtData58(_T(""))
, txtData59(_T(""))
, txtData60(_T(""))
, txtData61(_T(""))
, txtData62(_T(""))
, txtData63(_T(""))
, chbExtended(FALSE)
, chbRemote(FALSE)
, chbFD(FALSE)
, chbBRS(FALSE)
, txtBitrate(_T(""))
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPCANBasicExampleDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_CBBCHANNEL, cbbChannel);
    DDX_Control(pDX, IDC_BUTTON_HWREFRESH, btnRefresh);
    DDX_Control(pDX, IDC_CBBBAUDRATES, cbbBaudrates);
    DDX_Control(pDX, IDC_CBBHWSTYPE, cbbHwsType);
    DDX_Control(pDX, IDC_CBBIO, cbbIO);
    DDX_Control(pDX, IDC_CBBINTERRUPT, cbbInterrupt);
    DDX_Control(pDX, IDC_BTNINIT, btnInit);
    DDX_Control(pDX, IDC_BTNRELEASE, btnRelease);

    DDX_Control(pDX, IDC_CHBFILTEREXTENDED, chbFilterExtended);
    DDX_Control(pDX, IDC_RADIOFILTEROPEN, rdbFilterOpen);
    DDX_Control(pDX, IDC_RADIOFILTERCLOSE, rdbFilterClose);
    DDX_Control(pDX, IDC_RADIOFILTERCUSTOM, rdbFilterCustom);
    DDX_Control(pDX, IDC_NUDFILTERFROM, nudFilterFrom);
    DDX_Text(pDX, IDC_TXTFILTERFROM, txtFilterFrom);
    DDX_Control(pDX, IDC_NUDFILTERTO, nudFilterTo);
    DDX_Text(pDX, IDC_TXTFILTERTO, txtFilterTo);
    DDX_Control(pDX, IDC_BUTTONFILTERAPPLY, btnFilterApply);
    DDX_Control(pDX, IDC_BUTTONFILTERQUERY, btnFilterQuery);

    DDX_Control(pDX, IDC_COMBOPARAMETER, cbbParameter);
    DDX_Control(pDX, IDC_RADIOPARAMACTIVE, rdbParameterActive);
    DDX_Control(pDX, IDC_RADIOPARAMINACTIVE, rdbParameterInactive);
    DDX_Control(pDX, IDC_TXTPARAMDEVNUMBER, editParameterDevNumberOrDelay);
    DDX_Text(pDX, IDC_TXTPARAMDEVNUMBER, txtParameterDevNumber);
    DDX_Control(pDX, IDC_NUDPARAMDEVNUMBER, nudParameterDevNumberOrDelay);
    DDX_Control(pDX, IDC_BUTTONPARAMSET, btnParameterSet);
    DDX_Control(pDX, IDC_BUTTONPARAMGET, btnParameterGet);

    DDX_Control(pDX, IDC_RDBTIMER, rdbReadingTimer);
    DDX_Control(pDX, IDC_RADIO_BY_EVENT, rdbReadingEvent);
    DDX_Control(pDX, IDC_RDBMANUAL, rdbReadingManual);
    DDX_Control(pDX, IDC_CHBTIMESTAMP, chbReadingTimeStamp);
    DDX_Control(pDX, IDC_BUTTONREAD, btnRead);
    DDX_Control(pDX, IDC_BUTTONREADINGCLEAR, btnReadingClear);

    DDX_Control(pDX, IDC_LISTINFO, listBoxInfo);
    DDX_Control(pDX, IDC_BUTTONSTATUS, btnStatus);
    DDX_Control(pDX, IDC_BUTTONRESET, btnReset);

    DDX_Text(pDX, IDC_TXTLENGTH, txtLength);
    DDV_MaxChars(pDX, txtLength, 2);
    DDX_Control(pDX, IDC_NUDLENGTH, nudLength);
    DDX_Text(pDX, IDC_TXTID, txtID);
    DDV_MaxChars(pDX, txtID, 8);
    DDX_Text(pDX, IDC_TXTDATA0, txtData0);
    DDV_MaxChars(pDX, txtData0, 2);
    DDX_Text(pDX, IDC_TXTDATA1, txtData1);
    DDV_MaxChars(pDX, txtData1, 2);
    DDX_Text(pDX, IDC_TXTDATA2, txtData2);
    DDV_MaxChars(pDX, txtData2, 2);
    DDX_Text(pDX, IDC_TXTDATA3, txtData3);
    DDV_MaxChars(pDX, txtData3, 2);
    DDX_Text(pDX, IDC_TXTDATA4, txtData4);
    DDV_MaxChars(pDX, txtData4, 2);
    DDX_Text(pDX, IDC_TXTDATA5, txtData5);
    DDV_MaxChars(pDX, txtData5, 2);
    DDX_Text(pDX, IDC_TXTDATA6, txtData6);
    DDV_MaxChars(pDX, txtData6, 2);
    DDX_Text(pDX, IDC_TXTDATA7, txtData7);
    DDV_MaxChars(pDX, txtData7, 2);
    DDX_Text(pDX, IDC_TXTDATA8, txtData8);
    DDV_MaxChars(pDX, txtData8, 2);
    DDX_Text(pDX, IDC_TXTDATA9, txtData9);
    DDV_MaxChars(pDX, txtData9, 2);
    DDX_Text(pDX, IDC_TXTDATA10, txtData10);
    DDV_MaxChars(pDX, txtData10, 2);
    DDX_Text(pDX, IDC_TXTDATA11, txtData11);
    DDV_MaxChars(pDX, txtData11, 2);
    DDX_Text(pDX, IDC_TXTDATA12, txtData12);
    DDV_MaxChars(pDX, txtData12, 2);
    DDX_Text(pDX, IDC_TXTDATA13, txtData13);
    DDV_MaxChars(pDX, txtData13, 2);
    DDX_Text(pDX, IDC_TXTDATA14, txtData14);
    DDV_MaxChars(pDX, txtData14, 2);
    DDX_Text(pDX, IDC_TXTDATA15, txtData15);
    DDV_MaxChars(pDX, txtData15, 2);
    DDX_Text(pDX, IDC_TXTDATA16, txtData16);
    DDV_MaxChars(pDX, txtData16, 2);
    DDX_Text(pDX, IDC_TXTDATA17, txtData17);
    DDV_MaxChars(pDX, txtData17, 2);
    DDX_Text(pDX, IDC_TXTDATA18, txtData18);
    DDV_MaxChars(pDX, txtData18, 2);
    DDX_Text(pDX, IDC_TXTDATA19, txtData19);
    DDV_MaxChars(pDX, txtData19, 2);
    DDX_Text(pDX, IDC_TXTDATA20, txtData20);
    DDV_MaxChars(pDX, txtData20, 2);
    DDX_Text(pDX, IDC_TXTDATA21, txtData21);
    DDV_MaxChars(pDX, txtData21, 2);
    DDX_Text(pDX, IDC_TXTDATA22, txtData22);
    DDV_MaxChars(pDX, txtData22, 2);
    DDX_Text(pDX, IDC_TXTDATA23, txtData23);
    DDV_MaxChars(pDX, txtData23, 2);
    DDX_Text(pDX, IDC_TXTDATA24, txtData24);
    DDV_MaxChars(pDX, txtData24, 2);
    DDX_Text(pDX, IDC_TXTDATA25, txtData25);
    DDV_MaxChars(pDX, txtData25, 2);
    DDX_Text(pDX, IDC_TXTDATA26, txtData26);
    DDV_MaxChars(pDX, txtData26, 2);
    DDX_Text(pDX, IDC_TXTDATA27, txtData27);
    DDV_MaxChars(pDX, txtData27, 2);
    DDX_Text(pDX, IDC_TXTDATA28, txtData28);
    DDV_MaxChars(pDX, txtData28, 2);
    DDX_Text(pDX, IDC_TXTDATA29, txtData29);
    DDV_MaxChars(pDX, txtData29, 2);
    DDX_Text(pDX, IDC_TXTDATA30, txtData30);
    DDV_MaxChars(pDX, txtData30, 2);
    DDX_Text(pDX, IDC_TXTDATA31, txtData31);
    DDV_MaxChars(pDX, txtData31, 2);
    DDX_Text(pDX, IDC_TXTDATA32, txtData32);
    DDV_MaxChars(pDX, txtData32, 2);
    DDX_Text(pDX, IDC_TXTDATA33, txtData33);
    DDV_MaxChars(pDX, txtData33, 2);
    DDX_Text(pDX, IDC_TXTDATA34, txtData34);
    DDV_MaxChars(pDX, txtData34, 2);
    DDX_Text(pDX, IDC_TXTDATA35, txtData35);
    DDV_MaxChars(pDX, txtData35, 2);
    DDX_Text(pDX, IDC_TXTDATA36, txtData36);
    DDV_MaxChars(pDX, txtData36, 2);
    DDX_Text(pDX, IDC_TXTDATA37, txtData37);
    DDV_MaxChars(pDX, txtData37, 2);
    DDX_Text(pDX, IDC_TXTDATA38, txtData38);
    DDV_MaxChars(pDX, txtData38, 2);
    DDX_Text(pDX, IDC_TXTDATA39, txtData39);
    DDV_MaxChars(pDX, txtData39, 2);
    DDX_Text(pDX, IDC_TXTDATA40, txtData40);
    DDV_MaxChars(pDX, txtData40, 2);
    DDX_Text(pDX, IDC_TXTDATA41, txtData41);
    DDV_MaxChars(pDX, txtData41, 2);
    DDX_Text(pDX, IDC_TXTDATA42, txtData42);
    DDV_MaxChars(pDX, txtData42, 2);
    DDX_Text(pDX, IDC_TXTDATA43, txtData43);
    DDV_MaxChars(pDX, txtData43, 2);
    DDX_Text(pDX, IDC_TXTDATA44, txtData44);
    DDV_MaxChars(pDX, txtData44, 2);
    DDX_Text(pDX, IDC_TXTDATA45, txtData45);
    DDV_MaxChars(pDX, txtData45, 2);
    DDX_Text(pDX, IDC_TXTDATA46, txtData46);
    DDV_MaxChars(pDX, txtData46, 2);
    DDX_Text(pDX, IDC_TXTDATA47, txtData47);
    DDV_MaxChars(pDX, txtData47, 2);
    DDX_Text(pDX, IDC_TXTDATA48, txtData48);
    DDV_MaxChars(pDX, txtData48, 2);
    DDX_Text(pDX, IDC_TXTDATA49, txtData49);
    DDV_MaxChars(pDX, txtData49, 2);
    DDX_Text(pDX, IDC_TXTDATA50, txtData50);
    DDV_MaxChars(pDX, txtData50, 2);
    DDX_Text(pDX, IDC_TXTDATA51, txtData51);
    DDV_MaxChars(pDX, txtData51, 2);
    DDX_Text(pDX, IDC_TXTDATA52, txtData52);
    DDV_MaxChars(pDX, txtData52, 2);
    DDX_Text(pDX, IDC_TXTDATA53, txtData53);
    DDV_MaxChars(pDX, txtData53, 2);
    DDX_Text(pDX, IDC_TXTDATA54, txtData54);
    DDV_MaxChars(pDX, txtData54, 2);
    DDX_Text(pDX, IDC_TXTDATA55, txtData55);
    DDV_MaxChars(pDX, txtData55, 2);
    DDX_Text(pDX, IDC_TXTDATA56, txtData56);
    DDV_MaxChars(pDX, txtData56, 2);
    DDX_Text(pDX, IDC_TXTDATA57, txtData57);
    DDV_MaxChars(pDX, txtData57, 2);
    DDX_Text(pDX, IDC_TXTDATA58, txtData58);
    DDV_MaxChars(pDX, txtData58, 2);
    DDX_Text(pDX, IDC_TXTDATA59, txtData59);
    DDV_MaxChars(pDX, txtData59, 2);
    DDX_Text(pDX, IDC_TXTDATA60, txtData60);
    DDV_MaxChars(pDX, txtData60, 2);
    DDX_Text(pDX, IDC_TXTDATA61, txtData61);
    DDV_MaxChars(pDX, txtData61, 2);
    DDX_Text(pDX, IDC_TXTDATA62, txtData62);
    DDV_MaxChars(pDX, txtData62, 2);
    DDX_Text(pDX, IDC_TXTDATA63, txtData63);
    DDV_MaxChars(pDX, txtData63, 2);
    DDX_Check(pDX, IDC_CHBEXTENDED, chbExtended);
    DDX_Check(pDX, IDC_CHBREMOTE, chbRemote);
    DDX_Control(pDX, IDC_LSTMESSAGES, lstMessages);
    DDX_Control(pDX, IDC_BTNWRITE, btnWrite);
    DDX_Control(pDX, IDC_BUTTONVERSION, btnVersions);
    DDX_Control(pDX, IDC_CHBCANFD, chbCanFD);
    DDX_Check(pDX, IDC_CHBFD, chbFD);
    DDX_Check(pDX, IDC_CHBBRS, chbBRS);
    DDX_Text(pDX, IDC_TXTBITRATE, txtBitrate);
    DDX_Control(pDX, IDC_LA_DEVICEORDELAY, labelDeviceOrDelay);
}

BEGIN_MESSAGE_MAP(CPCANBasicExampleDlg, CDialog)
ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_TIMER()
    ON_CBN_SELCHANGE(IDC_CBBCHANNEL, OnCbnSelchangecbbChannel)
    ON_CBN_SELCHANGE(IDC_CBBBAUDRATES, OnCbnSelchangeCbbbaudrates)
    ON_EN_KILLFOCUS(IDC_TXTID, OnEnKillfocusTxtid)
    ON_EN_KILLFOCUS(IDC_TXTDATA0, OnEnKillfocusTxtdata0)
    ON_EN_KILLFOCUS(IDC_TXTDATA1, OnEnKillfocusTxtdata1)
    ON_EN_KILLFOCUS(IDC_TXTDATA2, OnEnKillfocusTxtdata2)
    ON_EN_KILLFOCUS(IDC_TXTDATA3, OnEnKillfocusTxtdata3)
    ON_EN_KILLFOCUS(IDC_TXTDATA4, OnEnKillfocusTxtdata4)
    ON_EN_KILLFOCUS(IDC_TXTDATA5, OnEnKillfocusTxtdata5)
    ON_EN_KILLFOCUS(IDC_TXTDATA6, OnEnKillfocusTxtdata6)
    ON_EN_KILLFOCUS(IDC_TXTDATA7, OnEnKillfocusTxtdata7)
    ON_NOTIFY(UDN_DELTAPOS, IDC_NUDLENGTH, OnDeltaposNudlength)
    ON_BN_CLICKED(IDC_CHBEXTENDED, OnBnClickedChbextended)
    ON_BN_CLICKED(IDC_CHBREMOTE, OnBnClickedChbremote)
    ON_NOTIFY(NM_DBLCLK, IDC_LSTMESSAGES, OnNMDblclkLstmessages)
    ON_BN_CLICKED(IDC_BTNINIT, OnBnClickedBtninit)
    ON_BN_CLICKED(IDC_BTNRELEASE, OnBnClickedBtnrelease)
    ON_BN_CLICKED(IDC_BTNWRITE, OnBnClickedBtnwrite)
    ON_WM_SHOWWINDOW()
    ON_BN_CLICKED(IDC_RDBTIMER, &CPCANBasicExampleDlg::OnBnClickedRdbtimer)
    ON_BN_CLICKED(IDC_RDBEVENT, &CPCANBasicExampleDlg::OnBnClickedRdbevent)
    ON_BN_CLICKED(IDC_CHBTIMESTAMP, &CPCANBasicExampleDlg::OnBnClickedChbtimestamp)
    ON_BN_CLICKED(IDC_BUTTON_HWREFRESH, &CPCANBasicExampleDlg::OnBnClickedButtonHwrefresh)
    ON_CBN_SELCHANGE(IDC_CBBHWSTYPE, &CPCANBasicExampleDlg::OnCbnSelchangeCbbhwstype)
    ON_BN_CLICKED(IDC_CHBFILTEREXTENDED, &CPCANBasicExampleDlg::OnBnClickedChbfilterextended)
    ON_NOTIFY(UDN_DELTAPOS, IDC_NUDFILTERFROM, &CPCANBasicExampleDlg::OnDeltaposNudfilterfrom)
    ON_EN_KILLFOCUS(IDC_TXTFILTERFROM, &CPCANBasicExampleDlg::OnEnKillfocusTxtfilterfrom)
    ON_NOTIFY(UDN_DELTAPOS, IDC_NUDFILTERTO, &CPCANBasicExampleDlg::OnDeltaposNudfilterto)
    ON_EN_KILLFOCUS(IDC_TXTFILTERTO, &CPCANBasicExampleDlg::OnEnKillfocusTxtfilterto)
    ON_BN_CLICKED(IDC_BUTTONFILTERAPPLY, &CPCANBasicExampleDlg::OnBnClickedButtonfilterapply)
    ON_BN_CLICKED(IDC_BUTTONFILTERQUERY, &CPCANBasicExampleDlg::OnBnClickedButtonfilterquery)
    ON_BN_CLICKED(IDC_RDBMANUAL, &CPCANBasicExampleDlg::OnBnClickedRdbmanual)
    ON_BN_CLICKED(IDC_BUTTONREAD, &CPCANBasicExampleDlg::OnBnClickedButtonread)
    ON_BN_CLICKED(IDC_BUTTONREADINGCLEAR, &CPCANBasicExampleDlg::OnBnClickedButtonreadingclear)
    ON_CBN_SELCHANGE(IDC_COMBOPARAMETER, &CPCANBasicExampleDlg::OnCbnSelchangeComboparameter)
    ON_NOTIFY(UDN_DELTAPOS, IDC_NUDPARAMDEVNUMBER, &CPCANBasicExampleDlg::OnDeltaposNudparamdevnumber)
    ON_EN_KILLFOCUS(IDC_TXTPARAMDEVNUMBER, &CPCANBasicExampleDlg::OnEnKillfocusTxtparamdevnumber)
    ON_BN_CLICKED(IDC_BUTTONPARAMSET, &CPCANBasicExampleDlg::OnBnClickedButtonparamset)
    ON_BN_CLICKED(IDC_BUTTONPARAMGET, &CPCANBasicExampleDlg::OnBnClickedButtonparamget)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BUTTONVERSION, &CPCANBasicExampleDlg::OnBnClickedButtonversion)
    ON_BN_CLICKED(IDC_BUTTONINFOCLEAR, &CPCANBasicExampleDlg::OnBnClickedButtoninfoclear)
    ON_BN_CLICKED(IDC_BUTTONSTATUS, &CPCANBasicExampleDlg::OnBnClickedButtonstatus)
    ON_BN_CLICKED(IDC_BUTTONRESET, &CPCANBasicExampleDlg::OnBnClickedButtonreset)
    ON_BN_CLICKED(IDC_CHBCANFD, &CPCANBasicExampleDlg::OnBnClickedChbfcanfd)
    ON_EN_KILLFOCUS(IDC_TXTDATA8, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata8)
    ON_EN_KILLFOCUS(IDC_TXTDATA9, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata9)
    ON_EN_KILLFOCUS(IDC_TXTDATA10, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata10)
    ON_EN_KILLFOCUS(IDC_TXTDATA11, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata11)
    ON_EN_KILLFOCUS(IDC_TXTDATA12, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata12)
    ON_EN_KILLFOCUS(IDC_TXTDATA13, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata13)
    ON_EN_KILLFOCUS(IDC_TXTDATA14, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata14)
    ON_EN_KILLFOCUS(IDC_TXTDATA15, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata15)
    ON_EN_KILLFOCUS(IDC_TXTDATA16, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata16)
    ON_EN_KILLFOCUS(IDC_TXTDATA17, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata17)
    ON_EN_KILLFOCUS(IDC_TXTDATA18, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata18)
    ON_EN_KILLFOCUS(IDC_TXTDATA19, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata19)
    ON_EN_KILLFOCUS(IDC_TXTDATA20, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata20)
    ON_EN_KILLFOCUS(IDC_TXTDATA21, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata21)
    ON_EN_KILLFOCUS(IDC_TXTDATA22, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata22)
    ON_EN_KILLFOCUS(IDC_TXTDATA23, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata23)
    ON_EN_KILLFOCUS(IDC_TXTDATA24, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata24)
    ON_EN_KILLFOCUS(IDC_TXTDATA25, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata25)
    ON_EN_KILLFOCUS(IDC_TXTDATA26, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata26)
    ON_EN_KILLFOCUS(IDC_TXTDATA27, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata27)
    ON_EN_KILLFOCUS(IDC_TXTDATA28, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata28)
    ON_EN_KILLFOCUS(IDC_TXTDATA29, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata29)
    ON_EN_KILLFOCUS(IDC_TXTDATA30, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata30)
    ON_EN_KILLFOCUS(IDC_TXTDATA31, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata31)
    ON_EN_KILLFOCUS(IDC_TXTDATA32, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata32)
    ON_EN_KILLFOCUS(IDC_TXTDATA33, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata33)
    ON_EN_KILLFOCUS(IDC_TXTDATA34, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata34)
    ON_EN_KILLFOCUS(IDC_TXTDATA35, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata35)
    ON_EN_KILLFOCUS(IDC_TXTDATA36, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata36)
    ON_EN_KILLFOCUS(IDC_TXTDATA37, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata37)
    ON_EN_KILLFOCUS(IDC_TXTDATA38, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata38)
    ON_EN_KILLFOCUS(IDC_TXTDATA39, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata39)
    ON_EN_KILLFOCUS(IDC_TXTDATA40, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata40)
    ON_EN_KILLFOCUS(IDC_TXTDATA41, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata41)
    ON_EN_KILLFOCUS(IDC_TXTDATA42, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata42)
    ON_EN_KILLFOCUS(IDC_TXTDATA43, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata43)
    ON_EN_KILLFOCUS(IDC_TXTDATA44, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata44)
    ON_EN_KILLFOCUS(IDC_TXTDATA45, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata45)
    ON_EN_KILLFOCUS(IDC_TXTDATA46, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata46)
    ON_EN_KILLFOCUS(IDC_TXTDATA47, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata47)
    ON_EN_KILLFOCUS(IDC_TXTDATA48, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata48)
    ON_EN_KILLFOCUS(IDC_TXTDATA49, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata49)
    ON_EN_KILLFOCUS(IDC_TXTDATA50, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata50)
    ON_EN_KILLFOCUS(IDC_TXTDATA51, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata51)
    ON_EN_KILLFOCUS(IDC_TXTDATA52, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata52)
    ON_EN_KILLFOCUS(IDC_TXTDATA53, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata53)
    ON_EN_KILLFOCUS(IDC_TXTDATA54, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata54)
    ON_EN_KILLFOCUS(IDC_TXTDATA55, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata55)
    ON_EN_KILLFOCUS(IDC_TXTDATA56, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata56)
    ON_EN_KILLFOCUS(IDC_TXTDATA57, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata57)
    ON_EN_KILLFOCUS(IDC_TXTDATA58, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata58)
    ON_EN_KILLFOCUS(IDC_TXTDATA59, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata59)
    ON_EN_KILLFOCUS(IDC_TXTDATA60, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata60)
    ON_EN_KILLFOCUS(IDC_TXTDATA61, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata61)
    ON_EN_KILLFOCUS(IDC_TXTDATA62, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata62)
    ON_EN_KILLFOCUS(IDC_TXTDATA63, &CPCANBasicExampleDlg::OnEnKillfocusTxtdata63)
    ON_LBN_DBLCLK(IDC_LISTINFO, &CPCANBasicExampleDlg::OnLbnDblclkListinfo)
    ON_BN_CLICKED(IDC_CHBFD, &CPCANBasicExampleDlg::OnBnClickedChbfd)
    ON_BN_CLICKED(IDC_CHBBRS, &CPCANBasicExampleDlg::OnBnClickedChbbrs)
    END_MESSAGE_MAP()


// PCANBasicExampleDlg message handlers
//
BOOL CPCANBasicExampleDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);         // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    // Extra initialization here
    InitializeControls();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPCANBasicExampleDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPCANBasicExampleDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CPCANBasicExampleDlg::InitializeControls(void)
{
    // Initialize the Critical Section
    //
    InitializeProtection();

    // Creates an array with all possible non plug-and-play PCAN-Channels
    //
    m_NonPnPHandles[0] = PCAN_ISABUS1;
    m_NonPnPHandles[1] = PCAN_ISABUS2;
    m_NonPnPHandles[2] = PCAN_ISABUS3;
    m_NonPnPHandles[3] = PCAN_ISABUS4;
    m_NonPnPHandles[4] = PCAN_ISABUS5;
    m_NonPnPHandles[5] = PCAN_ISABUS6;
    m_NonPnPHandles[6] = PCAN_ISABUS7;
    m_NonPnPHandles[7] = PCAN_ISABUS8;
    m_NonPnPHandles[8] = PCAN_DNGBUS1;

    // List Control
    //
    lstMessages.InsertColumn(MSG_TYPE,"Type",LVCFMT_LEFT,110,1);
    lstMessages.InsertColumn(MSG_ID,"ID",LVCFMT_LEFT,90,2);
    lstMessages.InsertColumn(MSG_LENGTH,"Length",LVCFMT_LEFT,50,3);
    lstMessages.InsertColumn(MSG_COUNT,"Count",LVCFMT_LEFT,49,5);
    lstMessages.InsertColumn(MSG_TIME,"Rcv Time",LVCFMT_LEFT,70,5);
    lstMessages.InsertColumn(MSG_DATA,"Data",LVCFMT_LEFT,170,4);
    lstMessages.SetExtendedStyle(lstMessages.GetExtendedStyle()|LVS_EX_FULLROWSELECT);

    // Initializes Edit Controls
    //
    txtID = "0";
    txtLength = "8";
    txtData0 = "00";
    txtData1 = "00";
    txtData2 = "00";
    txtData3 = "00";
    txtData4 = "00";
    txtData5 = "00";
    txtData6 = "00";
    txtData7 = "00";
    txtData8 = "00";
    txtData9 = "00";
    txtData10 = "00";
    txtData11 = "00";
    txtData12 = "00";
    txtData13 = "00";
    txtData14 = "00";
    txtData15 = "00";
    txtData16 = "00";
    txtData17 = "00";
    txtData18 = "00";
    txtData19 = "00";
    txtData20 = "00";
    txtData21 = "00";
    txtData22 = "00";
    txtData23 = "00";
    txtData24 = "00";
    txtData25 = "00";
    txtData26 = "00";
    txtData27 = "00";
    txtData28 = "00";
    txtData29 = "00";
    txtData30 = "00";
    txtData31 = "00";
    txtData32 = "00";
    txtData33 = "00";
    txtData34 = "00";
    txtData35 = "00";
    txtData36 = "00";
    txtData37 = "00";
    txtData38 = "00";
    txtData39 = "00";
    txtData40 = "00";
    txtData41 = "00";
    txtData42 = "00";
    txtData43 = "00";
    txtData44 = "00";
    txtData45 = "00";
    txtData46 = "00";
    txtData47 = "00";
    txtData48 = "00";
    txtData49 = "00";
    txtData50 = "00";
    txtData51 = "00";
    txtData52 = "00";
    txtData53 = "00";
    txtData54 = "00";
    txtData55 = "00";
    txtData56 = "00";
    txtData57 = "00";
    txtData58 = "00";
    txtData59 = "00";
    txtData60 = "00";
    txtData61 = "00";
    txtData62 = "00";
    txtData63 = "00";

    // As defautl, normal CAN hardware is used
    //
    m_IsFD = false;

    // We set the variable for the current
    // PCAN Basic Class instance to use it
    //
    m_objPCANBasic = new PCANBasicClass();

    // We set the variable to know which reading mode is
    // currently selected (Timer by default)
    //
    m_ActiveReadingMode = 0;

    // Create a list to store the displayed messages
    //
    m_LastMsgsList = new CPtrList();

    // Create Event to use Received-event
    //
    m_hEvent = CreateEvent(NULL, FALSE, FALSE, "");

    // Prepares the PCAN-Basic's debug-Log file
    //
    FillComboBoxData();

    // UpDown Length
    //
    nudLength.SetRange(0,8);
    nudLength.SetPos(8);

    // UpDown Filter From
    //
    nudFilterFrom.SetRange32(0,0x1FFFFFFF);
    nudFilterFrom.SetPos32(0);
    txtFilterFrom = "0";

    // UpDown Filter To
    //
    nudFilterTo.SetRange32(0,0x7FF);
    nudFilterTo.SetPos32(0x7FF);
    txtFilterTo = "7FF";

    // UpDown Device Number
    //
    nudParameterDevNumberOrDelay.SetRange32(0,254);
    nudParameterDevNumberOrDelay.SetPos32(0);
    txtParameterDevNumber = "0";

    // Init CheckBox
    rdbFilterOpen.SetCheck(1);
    rdbReadingTimer.SetCheck(1);
    rdbParameterActive.SetCheck(1);
    chbReadingTimeStamp.SetCheck(1);

    // Set default connection status
    SetConnectionStatus(false);

    // Init log parameters
    ConfigureLogFile();

    // Update UI
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::InitializeProtection()
{
    m_objpCS = new CRITICAL_SECTION();
    InitializeCriticalSection(m_objpCS);
}

void CPCANBasicExampleDlg::FinalizeProtection()
{
    try
    {
        DeleteCriticalSection(m_objpCS);
        delete m_objpCS;
        m_objpCS = NULL;
    }
    catch(...)
    {
        throw;
    }
}

void CPCANBasicExampleDlg::OnTimer(UINT_PTR nIDEvent)
{
    if(nIDEvent == 1)
        // Read Pending Messages
        //
        ReadMessages();
    if(nIDEvent == 2)
        // Display messages
        //
        DisplayMessages();

    CDialog::OnTimer(nIDEvent);
}

void CPCANBasicExampleDlg::OnCbnSelchangecbbChannel()
{
    bool bNonPnP;
    CString strTemp;
    int pcanHandleTemp;

    // Get the handle from the text being shown
    //
    strTemp = GetComboBoxSelectedLabel(&cbbChannel);
    strTemp = strTemp.Mid(strTemp.Find('(') + 1, 3);

    strTemp.Replace('h', ' ');
    strTemp.Trim(' ');

    // Determines if the handle belong to a No Plug&Play hardware
    //
    pcanHandleTemp = HexTextToInt(strTemp);
    m_PcanHandle = pcanHandleTemp;
    bNonPnP = m_PcanHandle <= PCAN_DNGBUS1;

    // Activates/deactivates configuration controls according with the
    // kind of hardware
    //
    cbbIO.EnableWindow(bNonPnP);
    cbbInterrupt.EnableWindow(bNonPnP);
    cbbHwsType.EnableWindow(bNonPnP);
}

void CPCANBasicExampleDlg::OnCbnSelchangeCbbbaudrates()
{
    // We save the corresponding Baudrate enumeration
    // type value for every selected Baudrate from the
    // list.
    //
    switch(cbbBaudrates.GetCurSel())
    {
    case 0:
        m_Baudrate = PCAN_BAUD_1M;
        break;
    case 1:
        m_Baudrate = PCAN_BAUD_800K;
        break;
    case 2:
        m_Baudrate = PCAN_BAUD_500K;
        break;
    case 3:
        m_Baudrate = PCAN_BAUD_250K;
        break;
    case 4:
        m_Baudrate = PCAN_BAUD_125K;
        break;
    case 5:
        m_Baudrate = PCAN_BAUD_100K;
        break;
    case 6:
        m_Baudrate = PCAN_BAUD_95K;
        break;
    case 7:
        m_Baudrate = PCAN_BAUD_83K;
        break;
    case 8:
        m_Baudrate = PCAN_BAUD_50K;
        break;
    case 9:
        m_Baudrate = PCAN_BAUD_47K;
        break;
    case 10:
        m_Baudrate = PCAN_BAUD_33K;
        break;
    case 11:
        m_Baudrate = PCAN_BAUD_20K;
        break;
    case 12:
        m_Baudrate = PCAN_BAUD_10K;
        break;
    case 13:
        m_Baudrate = PCAN_BAUD_5K;
        break;
    default:
        m_Baudrate = (TPCANBaudrate)0;
        break;
    }
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtid()
{
    int iTest;

    // Aplly Pending Updates
    //
    UpdateData(TRUE);

    // Format to Upper
    //
    txtID.MakeUpper();

    // Convert string value
    //
    iTest = HexTextToInt(txtID);

    // The Textbox for the ID is represented with 3 characters for
    // Standard and 8 characters for extended messages.
    // Therefore if the Length of the text is smaller than TextLength,
    // we add "0"
    //
    if(chbExtended)
    {
        if(iTest > 0)
            txtID = IntToHex(iTest,8);
        else
            txtID = "00000000";
    }
    else
    {
        if(iTest > 0x7FF)
            txtID = "7FF";
        else
        {
            // We test if the given ID is a valid hexadecimal number.
            //
            iTest = HexTextToInt(txtID);
            if(iTest > 0)
                txtID = IntToHex(iTest,3);
            else
                txtID = "000";
        }
    }

    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata0()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData0);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata1()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData1);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata2()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData2);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata3()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData3);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata4()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData4);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata5()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData5);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata6()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData6);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata7()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData7);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata8()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData8);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata9()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData9);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata10()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData10);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata11()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData11);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata12()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData12);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata13()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData13);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata14()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData14);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata15()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData15);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata16()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData16);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata17()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData17);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata18()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData18);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata19()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData19);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata20()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData20);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata21()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData21);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata22()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData22);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata23()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData23);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata24()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData24);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata25()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData25);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata26()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData26);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata27()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData27);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata28()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData28);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata29()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData29);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata30()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData30);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata31()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData31);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata32()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData32);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata33()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData33);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata34()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData34);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata35()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData35);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata36()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData36);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata37()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData37);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata38()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData38);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata39()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData39);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata40()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData40);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata41()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData41);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata42()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData42);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata43()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData43);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata44()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData44);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata45()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData45);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata46()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData46);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata47()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData47);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata48()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData48);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata49()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData49);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata50()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData50);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata51()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData51);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata52()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData52);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata53()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData53);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata54()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData54);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata55()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData55);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata56()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData56);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata57()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData57);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata58()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData58);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata59()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData59);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata60()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData60);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata61()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData61);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata62()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData62);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtdata63()
{
    UpdateData(TRUE);
    // Check textBox content
    //
    CheckHexEditBox(&txtData63);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnDeltaposNudlength(NMHDR *pNMHDR, LRESULT *pResult)
{
    int iNewVal;
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    // Compute new selected value
    //
    iNewVal =  pNMUpDown->iPos + ((pNMUpDown->iDelta > 0) ? 1 : -1);
    if(iNewVal > (chbFD ? 15 : 8)){
        iNewVal = (chbFD ? 15 : 8);
        pNMUpDown->iPos -= 1;
    }
    if(iNewVal < 0)
        iNewVal = 0;

    iNewVal = GetLengthFromDLC(iNewVal,!chbFD);
    GetDlgItem(IDC_LALENGTH)->SetWindowText(IntToStr(iNewVal) + __T(" B."));

    *pResult = 0;
    EnableDisableDataFields(chbRemote ? 0 : iNewVal);
}

void CPCANBasicExampleDlg::EnableDisableDataFields(int length)
{
    // Enable or disable control according selected message length
    //
    ((CEdit*)GetDlgItem(IDC_TXTDATA0))->EnableWindow(0 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA1))->EnableWindow(1 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA2))->EnableWindow(2 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA3))->EnableWindow(3 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA4))->EnableWindow(4 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA5))->EnableWindow(5 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA6))->EnableWindow(6 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA7))->EnableWindow(7 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA8))->EnableWindow(8 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA9))->EnableWindow(9 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA10))->EnableWindow(10 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA11))->EnableWindow(11 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA12))->EnableWindow(12 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA13))->EnableWindow(13 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA14))->EnableWindow(14 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA15))->EnableWindow(15 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA16))->EnableWindow(16 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA17))->EnableWindow(17 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA18))->EnableWindow(18 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA19))->EnableWindow(19 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA20))->EnableWindow(20 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA21))->EnableWindow(21 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA22))->EnableWindow(22 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA23))->EnableWindow(23 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA24))->EnableWindow(24 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA25))->EnableWindow(25 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA26))->EnableWindow(26 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA27))->EnableWindow(27 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA28))->EnableWindow(28 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA29))->EnableWindow(29 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA30))->EnableWindow(30 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA31))->EnableWindow(31 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA32))->EnableWindow(32 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA33))->EnableWindow(33 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA34))->EnableWindow(34 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA35))->EnableWindow(35 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA36))->EnableWindow(36 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA37))->EnableWindow(37 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA38))->EnableWindow(38 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA39))->EnableWindow(39 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA40))->EnableWindow(40 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA41))->EnableWindow(41 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA42))->EnableWindow(42 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA43))->EnableWindow(43 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA44))->EnableWindow(44 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA45))->EnableWindow(45 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA46))->EnableWindow(46 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA47))->EnableWindow(47 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA48))->EnableWindow(48 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA49))->EnableWindow(49 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA50))->EnableWindow(50 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA51))->EnableWindow(51 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA52))->EnableWindow(52 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA53))->EnableWindow(53 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA54))->EnableWindow(54 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA55))->EnableWindow(55 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA56))->EnableWindow(56 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA57))->EnableWindow(57 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA58))->EnableWindow(58 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA59))->EnableWindow(59 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA60))->EnableWindow(60 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA61))->EnableWindow(61 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA62))->EnableWindow(62 < length);
    ((CEdit*)GetDlgItem(IDC_TXTDATA63))->EnableWindow(63 < length);
}

void CPCANBasicExampleDlg::OnBnClickedChbextended()
{
    // Check Message ID
    //
    OnEnKillfocusTxtid();
}

void CPCANBasicExampleDlg::OnBnClickedChbremote()
{
    UpdateData(TRUE);

    // Show or Hide control according message is a RTR
    //
    EnableDisableDataFields(chbRemote ? 0 : nudLength.GetPos());

    GetDlgItem(IDC_CHBFD)->EnableWindow(!chbRemote);
}

void CPCANBasicExampleDlg::OnNMDblclkLstmessages(NMHDR *pNMHDR, LRESULT *pResult)
{
    *pResult = 0;
    OnBnClickedButtonreadingclear();
}

typedef struct
{
    DWORD a;
    BYTE b;
    BYTE c;
    BYTE d;
    WORD e;
    BYTE f[64];
}Temp1;

void CPCANBasicExampleDlg::OnBnClickedBtninit()
{
    TPCANStatus stsResult;
    int selectedIO;
    int selectedInterrupt;

    UpdateData(TRUE);

    // Parse IO and Interrupt
    //
    selectedIO = HexTextToInt(GetComboBoxSelectedLabel(&cbbIO));
    selectedInterrupt = atoi(GetComboBoxSelectedLabel(&cbbInterrupt));

    // Connects a selected PCAN-Basic channel
    //
    if (m_IsFD)
        stsResult = m_objPCANBasic->InitializeFD(m_PcanHandle, txtBitrate.GetBuffer());
    else
        stsResult = m_objPCANBasic->Initialize(m_PcanHandle, m_Baudrate, m_HwType, selectedIO, selectedInterrupt);

    if (stsResult != PCAN_ERROR_OK)
        if (stsResult != PCAN_ERROR_CAUTION)
            ::MessageBox(NULL, GetFormatedError(stsResult), "Error!",MB_ICONERROR);
        else
        {
            IncludeTextMessage("******************************************************");
            IncludeTextMessage("The bitrate being used is different than the given one");
            IncludeTextMessage("******************************************************");
            stsResult = PCAN_ERROR_OK;
        }
    else
        // Prepares the PCAN-Basic's PCAN-Trace file
        //
        ConfigureTraceFile();

    // Sets the connection status of the main-form
    //
    SetConnectionStatus(stsResult == PCAN_ERROR_OK);
}

void CPCANBasicExampleDlg::OnBnClickedBtnrelease()
{
    // Terminate Read Thread if it exists
    //
    if(m_hThread != NULL)
    {
        m_Terminated = true;
        WaitForSingleObject(m_hThread,-1);
        m_hThread = NULL;
    }

    // We stop to read from the CAN queue
    //
    SetTimerRead(false);

    // Releases a current connected PCAN-Basic channel
    //
    m_objPCANBasic->Uninitialize(m_PcanHandle);

    // Sets the connection status of the main-form
    //
    SetConnectionStatus(false);
}

TPCANStatus CPCANBasicExampleDlg::WriteFrame()
{
    TPCANMsg CANMsg;

    // We configurate the Message.  The ID (max 0x1FF),
    // Length of the Data, Message Type (Standard in
    // this example) and die data
    //
    CANMsg.ID = HexTextToInt(txtID);
    CANMsg.LEN = (BYTE)nudLength.GetPos32();
    CANMsg.MSGTYPE = (chbExtended) ? PCAN_MESSAGE_EXTENDED : PCAN_MESSAGE_STANDARD;
    // If a remote frame will be sent, the data bytes are not important.
    //
    if (chbRemote)
        CANMsg.MSGTYPE = CANMsg.MSGTYPE | PCAN_MESSAGE_RTR;
    else
    {
        // We get so much data as the Len of the message
        //
        CANMsg.DATA[0] = (BYTE)HexTextToInt(txtData0);
        CANMsg.DATA[1] = (BYTE)(HexTextToInt(txtData1));
        CANMsg.DATA[2] = (BYTE)(HexTextToInt(txtData2));
        CANMsg.DATA[3] = (BYTE)(HexTextToInt(txtData3));
        CANMsg.DATA[4] = (BYTE)(HexTextToInt(txtData4));
        CANMsg.DATA[5] = (BYTE)(HexTextToInt(txtData5));
        CANMsg.DATA[6] = (BYTE)(HexTextToInt(txtData6));
        CANMsg.DATA[7] = (BYTE)(HexTextToInt(txtData7));
    }

    // The message is sent to the configured hardware
    //
    return m_objPCANBasic->Write(m_PcanHandle, &CANMsg);
}

TPCANStatus CPCANBasicExampleDlg::WriteFrameFD()
{
    TPCANMsgFD CANMsg;

    // We configurate the Message.  The ID (max 0x1FF),
    // Length of the Data, Message Type (Standard in
    // this example) and the data
    //
    CANMsg.ID = HexTextToInt(txtID);
    CANMsg.DLC = (BYTE)nudLength.GetPos32();
    CANMsg.MSGTYPE = (chbExtended) ? PCAN_MESSAGE_EXTENDED : PCAN_MESSAGE_STANDARD;
    CANMsg.MSGTYPE |= (chbFD) ? PCAN_MESSAGE_FD : PCAN_MESSAGE_STANDARD;
    CANMsg.MSGTYPE |= (chbBRS) ? PCAN_MESSAGE_BRS : PCAN_MESSAGE_STANDARD;

    // If a remote frame will be sent, the data bytes are not important.
    //
    if (chbRemote)
        CANMsg.MSGTYPE = CANMsg.MSGTYPE | PCAN_MESSAGE_RTR;
    else
    {
        // We get so much data as the Len of the message
        //
        CANMsg.DATA[0] = (BYTE)HexTextToInt(txtData0);
        CANMsg.DATA[1] = (BYTE)(HexTextToInt(txtData1));
        CANMsg.DATA[2] = (BYTE)(HexTextToInt(txtData2));
        CANMsg.DATA[3] = (BYTE)(HexTextToInt(txtData3));
        CANMsg.DATA[4] = (BYTE)(HexTextToInt(txtData4));
        CANMsg.DATA[5] = (BYTE)(HexTextToInt(txtData5));
        CANMsg.DATA[6] = (BYTE)(HexTextToInt(txtData6));
        CANMsg.DATA[7] = (BYTE)(HexTextToInt(txtData7));
        CANMsg.DATA[8] = (BYTE)(HexTextToInt(txtData8));
        CANMsg.DATA[9] = (BYTE)(HexTextToInt(txtData9));
        CANMsg.DATA[10] = (BYTE)(HexTextToInt(txtData10));
        CANMsg.DATA[11] = (BYTE)(HexTextToInt(txtData11));
        CANMsg.DATA[12] = (BYTE)(HexTextToInt(txtData12));
        CANMsg.DATA[13] = (BYTE)(HexTextToInt(txtData13));
        CANMsg.DATA[14] = (BYTE)(HexTextToInt(txtData14));
        CANMsg.DATA[15] = (BYTE)(HexTextToInt(txtData15));
        CANMsg.DATA[16] = (BYTE)(HexTextToInt(txtData16));
        CANMsg.DATA[17] = (BYTE)(HexTextToInt(txtData17));
        CANMsg.DATA[18] = (BYTE)(HexTextToInt(txtData18));
        CANMsg.DATA[19] = (BYTE)(HexTextToInt(txtData19));
        CANMsg.DATA[20] = (BYTE)(HexTextToInt(txtData20));
        CANMsg.DATA[21] = (BYTE)(HexTextToInt(txtData21));
        CANMsg.DATA[22] = (BYTE)(HexTextToInt(txtData22));
        CANMsg.DATA[23] = (BYTE)(HexTextToInt(txtData23));
        CANMsg.DATA[24] = (BYTE)(HexTextToInt(txtData24));
        CANMsg.DATA[25] = (BYTE)(HexTextToInt(txtData25));
        CANMsg.DATA[26] = (BYTE)(HexTextToInt(txtData26));
        CANMsg.DATA[27] = (BYTE)(HexTextToInt(txtData27));
        CANMsg.DATA[28] = (BYTE)(HexTextToInt(txtData28));
        CANMsg.DATA[29] = (BYTE)(HexTextToInt(txtData29));
        CANMsg.DATA[30] = (BYTE)(HexTextToInt(txtData30));
        CANMsg.DATA[31] = (BYTE)(HexTextToInt(txtData31));
        CANMsg.DATA[32] = (BYTE)(HexTextToInt(txtData32));
        CANMsg.DATA[33] = (BYTE)(HexTextToInt(txtData33));
        CANMsg.DATA[34] = (BYTE)(HexTextToInt(txtData34));
        CANMsg.DATA[35] = (BYTE)(HexTextToInt(txtData35));
        CANMsg.DATA[36] = (BYTE)(HexTextToInt(txtData36));
        CANMsg.DATA[37] = (BYTE)(HexTextToInt(txtData37));
        CANMsg.DATA[38] = (BYTE)(HexTextToInt(txtData38));
        CANMsg.DATA[39] = (BYTE)(HexTextToInt(txtData39));
        CANMsg.DATA[40] = (BYTE)(HexTextToInt(txtData40));
        CANMsg.DATA[41] = (BYTE)(HexTextToInt(txtData41));
        CANMsg.DATA[42] = (BYTE)(HexTextToInt(txtData42));
        CANMsg.DATA[43] = (BYTE)(HexTextToInt(txtData43));
        CANMsg.DATA[44] = (BYTE)(HexTextToInt(txtData44));
        CANMsg.DATA[45] = (BYTE)(HexTextToInt(txtData45));
        CANMsg.DATA[46] = (BYTE)(HexTextToInt(txtData46));
        CANMsg.DATA[47] = (BYTE)(HexTextToInt(txtData47));
        CANMsg.DATA[48] = (BYTE)(HexTextToInt(txtData48));
        CANMsg.DATA[49] = (BYTE)(HexTextToInt(txtData49));
        CANMsg.DATA[50] = (BYTE)(HexTextToInt(txtData50));
        CANMsg.DATA[51] = (BYTE)(HexTextToInt(txtData51));
        CANMsg.DATA[52] = (BYTE)(HexTextToInt(txtData52));
        CANMsg.DATA[53] = (BYTE)(HexTextToInt(txtData53));
        CANMsg.DATA[54] = (BYTE)(HexTextToInt(txtData54));
        CANMsg.DATA[55] = (BYTE)(HexTextToInt(txtData55));
        CANMsg.DATA[56] = (BYTE)(HexTextToInt(txtData56));
        CANMsg.DATA[57] = (BYTE)(HexTextToInt(txtData57));
        CANMsg.DATA[58] = (BYTE)(HexTextToInt(txtData58));
        CANMsg.DATA[59] = (BYTE)(HexTextToInt(txtData59));
        CANMsg.DATA[60] = (BYTE)(HexTextToInt(txtData60));
        CANMsg.DATA[61] = (BYTE)(HexTextToInt(txtData61));
        CANMsg.DATA[62] = (BYTE)(HexTextToInt(txtData62));
        CANMsg.DATA[63] = (BYTE)(HexTextToInt(txtData63));
    }

    // The message is sent to the configured hardware
    //
    return m_objPCANBasic->WriteFD(m_PcanHandle, &CANMsg);
}

void CPCANBasicExampleDlg::OnBnClickedBtnwrite()
{
    TPCANStatus stsResult;

    // The message is sent
    //
    stsResult = m_IsFD ? WriteFrameFD() : WriteFrame();

    // The Hardware was successfully sent
    //
    if (stsResult == PCAN_ERROR_OK)
        IncludeTextMessage("Message was successfully SENT");
    else
        // An error occurred.  We show the error.
        //
        ::MessageBox(NULL, GetFormatedError(stsResult), "Error!",MB_ICONERROR);
}

void CPCANBasicExampleDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialog::OnShowWindow(bShow, nStatus);
}


void CPCANBasicExampleDlg::OnBnClickedRdbtimer()
{
    // Check reading mode selection change
    //
    if(rdbReadingTimer.GetCheck() && (m_ActiveReadingMode != 0))
    {
        // Process change
        //
        m_ActiveReadingMode = 0;
        ReadingModeChanged();
    }
}
void CPCANBasicExampleDlg::OnBnClickedRdbevent()
{
    // Check reading mode selection change
    //
    if(rdbReadingEvent.GetCheck() && (m_ActiveReadingMode != 1))
    {
        // Process change
        //
        m_ActiveReadingMode = 1;
        ReadingModeChanged();
    }
}
void CPCANBasicExampleDlg::OnBnClickedChbtimestamp()
{
    MessageStatus* msgStsCurrentMessage;
    CString str;
    POSITION pos;
    BOOL bChecked;

    // According with the check-value of this checkbox,
    // the recieved time of a messages will be interpreted as
    // period (time between the two last messages) or as time-stamp
    // (the elapsed time since windows was started).
    // - (Protected environment)
    //
    {
        clsCritical locker(m_objpCS);

        pos = m_LastMsgsList->GetHeadPosition();
        bChecked = chbReadingTimeStamp.GetCheck();
        while(pos)
        {
            msgStsCurrentMessage = (MessageStatus*)m_LastMsgsList->GetNext(pos);
            msgStsCurrentMessage->ShowingPeriod = bChecked > 0;
        }
    }
}

void CPCANBasicExampleDlg::OnBnClickedButtonHwrefresh()
{
    TPCANChannelInformation *info;
    TPCANStatus stsResult;
    DWORD iChannelsCount;
    bool bIsFD;

    // Clears the Channel combioBox and fill it againa with
    // the PCAN-Basic handles for no-Plug&Play hardware and
    // the detected Plug&Play hardware
    //
    cbbChannel.ResetContent();

    // Includes all no-Plug&Play Handles
    for (int i = 0; i < (sizeof(m_NonPnPHandles) /sizeof(TPCANHandle)) ; i++)
        cbbChannel.AddString(FormatChannelName(m_NonPnPHandles[i]));

    stsResult = m_objPCANBasic->GetValue(PCAN_NONEBUS, PCAN_ATTACHED_CHANNELS_COUNT, (void*)&iChannelsCount, sizeof(iChannelsCount));
    if (stsResult == PCAN_ERROR_OK)
    {
        info = new TPCANChannelInformation[iChannelsCount];
        stsResult = m_objPCANBasic->GetValue(PCAN_NONEBUS, PCAN_ATTACHED_CHANNELS, (void*)info, iChannelsCount * sizeof(TPCANChannelInformation));
        if (stsResult == PCAN_ERROR_OK)
            // Include only connectable channels
            //
            for (int i=0; i < (int)iChannelsCount; i++)
                if (info[i].channel_condition & PCAN_CHANNEL_AVAILABLE)
                {
                    bIsFD = info[i].device_features & FEATURE_FD_CAPABLE;
                    cbbChannel.AddString(FormatChannelName(info[i].channel_handle, bIsFD));
                }

        delete [] info;
    }

    // Select Last One
    //
    cbbChannel.SetCurSel(cbbChannel.GetCount() - 1);
    OnCbnSelchangecbbChannel();
}

void CPCANBasicExampleDlg::OnCbnSelchangeCbbhwstype()
{
    // Saves the current type for a no-Plug&Play hardware
    //
    switch (cbbHwsType.GetCurSel())
    {
    case 0:
        m_HwType = PCAN_TYPE_ISA;
        break;
    case 1:
        m_HwType = PCAN_TYPE_ISA_SJA;
        break;
    case 2:
        m_HwType = PCAN_TYPE_ISA_PHYTEC;
        break;
    case 3:
        m_HwType = PCAN_TYPE_DNG;
        break;
    case 4:
        m_HwType = PCAN_TYPE_DNG_EPP;
        break;
    case 5:
        m_HwType = PCAN_TYPE_DNG_SJA;
        break;
    case 6:
        m_HwType = PCAN_TYPE_DNG_SJA_EPP;
        break;
    }
}

void CPCANBasicExampleDlg::OnBnClickedChbfilterextended()
{
    int iMaxValue;

    iMaxValue = (chbFilterExtended.GetCheck()) ? 0x1FFFFFFF : 0x7FF;

    // We check that the maximum value for a selected filter
    // mode is used
    //
    nudFilterTo.SetRange32(0,iMaxValue);
    if (nudFilterTo.GetPos32() > iMaxValue)
    {
        nudFilterTo.SetPos32(iMaxValue);
        txtFilterTo.Format("%X", iMaxValue);
    }

    nudFilterFrom.SetRange32(0,iMaxValue);
    if (nudFilterFrom.GetPos32() > iMaxValue)
    {
        nudFilterFrom.SetPos32(iMaxValue);
        txtFilterFrom.Format("%X", iMaxValue);
    }

    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnDeltaposNudfilterfrom(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    int iNewVal;

    //Compute new selected From value
    iNewVal =  pNMUpDown->iPos + ((pNMUpDown->iDelta > 0) ? 1 : -1);
    if(iNewVal < 0)
        iNewVal = 0;

    //Update textBox
    txtFilterFrom.Format("%X", iNewVal);
    UpdateData(FALSE);

    *pResult = 0;
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtfilterfrom()
{
    int iMaxValue;
    iMaxValue = (chbFilterExtended.GetCheck()) ? 0x1FFFFFFF : 0x7FF;
    int newValue;
    UpdateData(TRUE);

    // Compute new edited value
    //
    newValue = HexTextToInt(txtFilterFrom);
    if(newValue > iMaxValue)
        newValue = iMaxValue;
    else if(newValue < 0)
        newValue = 0;
    // Update Nud control
    //
    nudFilterFrom.SetPos32(newValue);
    txtFilterFrom.Format("%X", newValue);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnDeltaposNudfilterto(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    int iNewVal;
    int iMaxValue;

    iMaxValue = (chbFilterExtended.GetCheck()) ? 0x1FFFFFFF : 0x7FF;

    // Compute new selected From value
    //
    iNewVal =  pNMUpDown->iPos + ((pNMUpDown->iDelta > 0) ? 1 : -1);
    if (iNewVal < 0)
        iNewVal = 0;

    if (iNewVal > iMaxValue)
        iNewVal = iMaxValue;

    // Update textBox
    //
    txtFilterTo.Format("%X", iNewVal);
    UpdateData(FALSE);

    *pResult = 0;
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtfilterto()
{
    int iMaxValue;
    iMaxValue = (chbFilterExtended.GetCheck()) ? 0x1FFFFFFF : 0x7FF;
    int newValue;
    UpdateData(TRUE);

    // Compute new edited value
    //
    newValue = HexTextToInt(txtFilterTo);
    if(newValue > iMaxValue)
        newValue = iMaxValue;
    else if(newValue < 0)
        newValue = 0;
    // Update Nud control
    //
    nudFilterTo.SetPos32(newValue);
    txtFilterTo.Format("%X", newValue);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnBnClickedButtonfilterapply()
{
    int iBuffer;
    CString info;
    TPCANStatus stsResult;

    // Gets the current status of the message filter
    //
    if (!GetFilterStatus(&iBuffer))
        return;

    // Configures the message filter for a custom range of messages
    //
    if (rdbFilterCustom.GetCheck())
    {
        // Sets the custom filter
        //
        stsResult = m_objPCANBasic->FilterMessages(m_PcanHandle, nudFilterFrom.GetPos32(), nudFilterTo.GetPos32(), chbFilterExtended.GetCheck() ? PCAN_MODE_EXTENDED : PCAN_MODE_STANDARD);
        // If success, an information message is written, if it is not, an error message is shown
        //
        if (stsResult == PCAN_ERROR_OK)
        {
            info.Format("The filter was customized. IDs from {%X} to {%X} will be received", nudFilterFrom.GetPos32(), nudFilterTo.GetPos32());
            IncludeTextMessage(info);
        }
        else
            ::MessageBox(NULL, GetFormatedError(stsResult), "Error!",MB_ICONERROR);

        return;
    }

    // The filter will be full opened or complete closed
    //
    if (rdbFilterClose.GetCheck())
        iBuffer = PCAN_FILTER_CLOSE;
    else
        iBuffer = PCAN_FILTER_OPEN;

    // The filter is configured
    //
    stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_MESSAGE_FILTER, (void*)&iBuffer, sizeof(int));

    // If success, an information message is written, if it is not, an error message is shown
    //
    if (stsResult == PCAN_ERROR_OK)
    {
        info.Format("The filter was successfully %s", rdbFilterClose.GetCheck() ? "closed." : "opened.");
        IncludeTextMessage(info);
    }
    else
        ::MessageBox(NULL, GetFormatedError(stsResult), "Error!",MB_ICONERROR);
}


void CPCANBasicExampleDlg::OnBnClickedButtonfilterquery()
{
    int iBuffer;

    // Queries the current status of the message filter
    //
    if (GetFilterStatus(&iBuffer))
    {
        switch(iBuffer)
        {
            // The filter is closed
            //
        case PCAN_FILTER_CLOSE:
            IncludeTextMessage("The Status of the filter is: closed.");
            break;
            // The filter is fully opened
            //
        case PCAN_FILTER_OPEN:
            IncludeTextMessage("The Status of the filter is: full opened.");
            break;
            // The filter is customized
            //
        case PCAN_FILTER_CUSTOM:
            IncludeTextMessage("The Status of the filter is: customized.");
            break;
            // The status of the filter is undefined. (Should never happen)
            //
        default:
            IncludeTextMessage("The Status of the filter is: Invalid.");
            break;
        }
    }
}

void CPCANBasicExampleDlg::OnBnClickedRdbmanual()
{
    // Check reading mode selection change
    //
    if(rdbReadingManual.GetCheck() && (m_ActiveReadingMode != 2))
    {
        // Process change
        //
        m_ActiveReadingMode = 2;
        ReadingModeChanged();
    }
}

void CPCANBasicExampleDlg::OnBnClickedButtonread()
{
    TPCANStatus stsResult;

    // We execute the "Read" function of the PCANBasic
    //
    stsResult = m_IsFD ? ReadMessageFD() : ReadMessage();
    if (stsResult != PCAN_ERROR_OK)
        // If an error occurred, an information message is included
        //
        IncludeTextMessage(GetFormatedError(stsResult));
}

void CPCANBasicExampleDlg::OnBnClickedButtonreadingclear()
{
    // (Protected environment)
    //
    {
        clsCritical locker(m_objpCS);

        // Remove all messages
        //
        lstMessages.DeleteAllItems();
        while(m_LastMsgsList->GetCount())
            delete m_LastMsgsList->RemoveHead();
    }
}

void CPCANBasicExampleDlg::OnCbnSelchangeComboparameter()
{
    // Activates/deactivates controls according with the selected
    // PCAN-Basic parameter
    //
    rdbParameterActive.EnableWindow((cbbParameter.GetCurSel() != 0) && (cbbParameter.GetCurSel() != 20));
    rdbParameterInactive.EnableWindow(rdbParameterActive.IsWindowEnabled());
    nudParameterDevNumberOrDelay.EnableWindow(!rdbParameterActive.IsWindowEnabled());
    editParameterDevNumberOrDelay.EnableWindow(!rdbParameterActive.IsWindowEnabled());
    labelDeviceOrDelay.SetWindowText((cbbParameter.GetCurSel() == 20) ? "Delay (ms):" : "Device ID:");
}

void CPCANBasicExampleDlg::OnDeltaposNudparamdevnumber(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    int iNewVal;

    // Compute new selected From value
    //
    iNewVal =  pNMUpDown->iPos + ((pNMUpDown->iDelta > 0) ? 1 : -1);
    if(iNewVal < 0)
        iNewVal = 0;
    // Update textBox value
    //
    txtParameterDevNumber.Format("%d", iNewVal);
    UpdateData(FALSE);

    *pResult = 0;
}

void CPCANBasicExampleDlg::OnEnKillfocusTxtparamdevnumber()
{
    int newValue;
    UpdateData(TRUE);
    // Compute new edited value
    //
    newValue = atoi(txtParameterDevNumber);
    if(newValue > 255)
        newValue = 255;
    else if(newValue < 0)
        newValue = 0;

    // Update Nud control
    //
    nudParameterDevNumberOrDelay.SetPos32(newValue);
    txtParameterDevNumber.Format("%d", newValue);
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnBnClickedButtonparamset()
{
    TPCANStatus stsResult;
    int iBuffer;
    CString info;
    TCHAR szDirectory[MAX_PATH] = "";
    bool bActivate;

    bActivate = rdbParameterActive.GetCheck() > 0;

    // Sets a PCAN-Basic parameter value
    //
    switch (cbbParameter.GetCurSel())
    {
        // The device identifier of a channel will be set
        //
    case 0:
        iBuffer = nudParameterDevNumberOrDelay.GetPos32();
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_DEVICE_ID, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
            IncludeTextMessage("The desired Device-ID was successfully configured");
        break;

        // The 5 Volt Power feature of a channel will be set
        //
    case 1:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_5VOLTS_POWER, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The USB/PC-Card 5 power was successfully %s", bActivate ? "activated" : "deactivated");
            IncludeTextMessage(info);
        }
        break;

        // The feature for automatic reset on BUS-OFF will be set
        //
    case 2:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_BUSOFF_AUTORESET, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The automatic-reset on BUS-OFF was successfully %s", bActivate ? "activated" : "deactivated");
            IncludeTextMessage(info);
        }
        break;

        // The CAN option "Listen Only" will be set
        //
    case 3:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_LISTEN_ONLY, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The CAN option \"Listen Only\" was successfully %s", bActivate ? "activated" : "deactivated");
            IncludeTextMessage(info);
        }
        break;

        // The feature for logging debug-information will be set
        //
    case 4:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(PCAN_NONEBUS, PCAN_LOG_STATUS, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The feature for logging debug information was successfully %s", bActivate ? "activated" : "deactivated");
            IncludeTextMessage(info);
            ::GetCurrentDirectory(sizeof(szDirectory) - 1, szDirectory);
            info.Format("Log file folder: %s" , szDirectory);
            IncludeTextMessage(info);
        }
        break;

        // The channel option "Receive Status" will be set
        //
    case 5:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_RECEIVE_STATUS, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The channel option \"Receive Status\" was set to %s", bActivate ? "ON" : "OFF");
            IncludeTextMessage(info);
        }
        break;

        // The feature for tracing will be set
        //
    case 7:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_TRACE_STATUS, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The feature for tracing data was successfully %s", bActivate ? "activated" : "deactivated");
            IncludeTextMessage(info);
        }
        break;

        // The feature for identifying an USB Channel will be set
        //
    case 8:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_CHANNEL_IDENTIFYING, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The procedure for channel identification was successfully %s", bActivate ? "activated" : "deactivated");
            IncludeTextMessage(info);
        }
        break;

        // The feature for using an already configured speed will be set
        //
    case 10:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_BITRATE_ADAPTING, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The feature for bit rate adaptation was successfully %s", bActivate ? "activated" : "deactivated");
            IncludeTextMessage(info);
        }
        break;

        // The option "Allow Status Frames" will be set
        //
    case 17:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_ALLOW_STATUS_FRAMES, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The reception of Status frames was successfully %s", bActivate ? "enabled" : "disabled");
            IncludeTextMessage(info);
        }
        break;

        // The option "Allow RTR Frames" will be set
        //
    case 18:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_ALLOW_RTR_FRAMES, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The reception of RTR frames was successfully %s", bActivate ? "enabled" : "disabled");
            IncludeTextMessage(info);
        }
        break;

        // The option "Allow Error Frames" will be set
        //
    case 19:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_ALLOW_ERROR_FRAMES, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The reception of Error frames was successfully %s", bActivate ? "enabled" : "disabled");
            IncludeTextMessage(info);
        }
        break;

        // The option "Interframes Delay" will be set
        //
    case 20:
        iBuffer = nudParameterDevNumberOrDelay.GetPos32();
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_INTERFRAME_DELAY, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
            IncludeTextMessage("The delay between transmitting frames was successfully set");
        break;

        // The option "Allow Echo Frames" will be set
        //
    case 21:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_ALLOW_ECHO_FRAMES, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info.Format("The reception of Echo frames was successfully %s", bActivate ? "enabled" : "disabled");
            IncludeTextMessage(info);
        }
        break;

        // The current parameter is invalid
        //
    default:
        stsResult = PCAN_ERROR_UNKNOWN;
        ::MessageBox(NULL, "Wrong parameter code.", "Error!",MB_ICONERROR);
        return;
    }

    // If the function fail, an error message is shown
    //
    if(stsResult != PCAN_ERROR_OK)
        ::MessageBox(NULL, GetFormatedError(stsResult), "Error!",MB_ICONERROR);
}

void CPCANBasicExampleDlg::OnBnClickedButtonparamget()
{
    TPCANStatus stsResult;
    int iBuffer;
    char strBuffer[256];
    CString info;

    // Sets a PCAN-Basic parameter value
    //
    switch (cbbParameter.GetCurSel())
    {
        // The device identifier of a channel will be get
        //
    case 0:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_DEVICE_ID, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The configured Device-ID is %d", iBuffer);
            IncludeTextMessage(info);
        }
        break;

        // The 5 Volt Power feature of a channel will be get
        //
    case 1:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_5VOLTS_POWER, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The 5-Volt Power of the USB/PC-Card is %s", (iBuffer == PCAN_PARAMETER_ON) ? "activated" : "deactivated");
            IncludeTextMessage(info);
        }
        break;

        // The feature for automatic reset on BUS-OFF will be get
        //
    case 2:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_BUSOFF_AUTORESET, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The automatic-reset on BUS-OFF is %s", (iBuffer == PCAN_PARAMETER_ON) ? "activated" : "deactivated");
            IncludeTextMessage(info);
        }
        break;

        // The CAN option "Listen Only" will be get
        //
    case 3:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_LISTEN_ONLY, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The CAN option \"Listen Only\" is %s", (iBuffer == PCAN_PARAMETER_ON) ? "activated" : "deactivated");
            IncludeTextMessage(info);
        }
        break;

        // The feature for logging debug-information will be get
        //
    case 4:
        stsResult = m_objPCANBasic->GetValue(PCAN_NONEBUS, PCAN_LOG_STATUS, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The feature for logging debug information is %s", (iBuffer == PCAN_PARAMETER_ON) ? "activated" : "deactivated");
            IncludeTextMessage(info);
        }
        break;

        // The activation status of the channel option "Receive Status"  will be retrieved
        //
    case 5:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_RECEIVE_STATUS, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The channel option \"Receive Status\" is %s", (iBuffer == PCAN_PARAMETER_ON) ? "ON" : "OFF");
            IncludeTextMessage(info);
        }
        break;

        // The Number of the CAN-Controller used by a PCAN-Channel
        //
    case 6:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_CONTROLLER_NUMBER, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The CAN Controller number is %d", iBuffer);
            IncludeTextMessage(info);
        }
        break;

        // The activation status for the feature for tracing data will be retrieved
        //
    case 7:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_TRACE_STATUS, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The feature for tracing data is %s", (iBuffer == PCAN_PARAMETER_ON) ? "ON" : "OFF");
            IncludeTextMessage(info);
        }
        break;

        // The activation status of the Channel Identifying procedure will be retrieved
        //
    case 8:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_CHANNEL_IDENTIFYING, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The identification procedure of the selected channel is %s", (iBuffer == PCAN_PARAMETER_ON) ? "ON" : "OFF");
            IncludeTextMessage(info);
        }
        break;

        // The activation status of the Channel Identifying procedure will be retrieved
        //
    case 9:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_CHANNEL_FEATURES, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The channel %s Flexible Data-Rate (CAN-FD)", (iBuffer & FEATURE_FD_CAPABLE) ? "does support" : "DOESN'T SUPPORT");
            IncludeTextMessage(info);
            info.Format("The channel %s an inter-frame delay for sending messages", (iBuffer & FEATURE_DELAY_CAPABLE) ? "does support" : "DOESN'T SUPPORT");
            IncludeTextMessage(info);
            info.Format("The channel %s using I/O pins", (iBuffer & FEATURE_IO_CAPABLE) ? "does allow" : "DOESN'T ALLOW");
            IncludeTextMessage(info);
        }
        break;

        // The status of the speed adapting feature will be retrieved
        //
    case 10:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_BITRATE_ADAPTING, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info.Format("The feature for bit rate adaptation is %s", (iBuffer == PCAN_PARAMETER_ON) ? "ON" : "OFF");
            IncludeTextMessage(info);
        }
        break;

        // The bitrate of the connected channel will be retrieved (BTR0-BTR1 value)
        //
    case 11:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_BITRATE_INFO, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info.Format("The bit rate of the channel is %.4Xh", iBuffer);
            IncludeTextMessage(info);
        }
        break;

        // The bitrate of the connected FD channel will be retrieved (String value)
        //
    case 12:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_BITRATE_INFO_FD, strBuffer, 255);
        if (stsResult == PCAN_ERROR_OK)
        {
            int partPos = 0;
            CString strPart = ((CString)strBuffer).Tokenize(",", partPos);

            IncludeTextMessage("The bit rate FD of the channel is represented by the following values:");
            while(!strPart.IsEmpty())
            {
                IncludeTextMessage("   * " + strPart);
                strPart = ((CString)strBuffer).Tokenize(",", partPos);
            }
        }
        break;

        // The nominal speed configured on the CAN bus
        //
    case 13:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_BUSSPEED_NOMINAL, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info.Format("The nominal speed of the channel is %d bit/s", iBuffer);
            IncludeTextMessage(info);
        }
        break;
        // The data speed configured on the CAN bus
        //
    case 14:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_BUSSPEED_DATA, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info.Format("The data speed of the channel is %d bit/s", iBuffer);
            IncludeTextMessage(info);
        }
        break;
        // The IP address of a LAN channel as string, in IPv4 format
        //
    case 15:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_IP_ADDRESS, strBuffer, 255);
        if (stsResult == PCAN_ERROR_OK)
        {
            info.Format("The IP address of the channel is %s", strBuffer);
            IncludeTextMessage(info);
        }
        break;
        // The running status of the LAN Service
        //
    case 16:
        stsResult = m_objPCANBasic->GetValue(PCAN_NONEBUS, PCAN_LAN_SERVICE_STATUS, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info.Format("The LAN service is %s", (iBuffer == SERVICE_STATUS_RUNNING) ? "running" : "NOT running");
            IncludeTextMessage(info);
        }
        break;
        // The reception of Status frames
        //
    case 17:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_ALLOW_STATUS_FRAMES, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info.Format("The reception of Status frames is %s", (iBuffer == PCAN_PARAMETER_ON) ? "enabled" : "disabled");
            IncludeTextMessage(info);
        }
        break;
        // The reception of RTR frames
        //
    case 18:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_ALLOW_RTR_FRAMES, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info.Format("The reception of RTR frames is %s", (iBuffer == PCAN_PARAMETER_ON) ? "enabled" : "disabled");
            IncludeTextMessage(info);
        }
        break;
        // The reception of Error frames
        //
    case 19:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_ALLOW_ERROR_FRAMES, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info.Format("The reception of Error frames is %s", (iBuffer == PCAN_PARAMETER_ON) ? "enabled" : "disabled");
            IncludeTextMessage(info);
        }
        break;

        // The Interframe delay of an USB channel will be retrieved
        //
    case 20:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_INTERFRAME_DELAY, (void*)&iBuffer, sizeof(iBuffer));
        if(stsResult == PCAN_ERROR_OK)
        {
            info.Format("The configured interframe delay is %d", iBuffer);
            IncludeTextMessage(info);
        }
        break;

        // The reception of Echo frames
        //
    case 21:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_ALLOW_ECHO_FRAMES, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info.Format("The reception of Echo frames is %s", (iBuffer == PCAN_PARAMETER_ON) ? "enabled" : "disabled");
            IncludeTextMessage(info);
        }
        break;

        // The current parameter is invalid
        //
    default:
        stsResult = PCAN_ERROR_UNKNOWN;
        ::MessageBox(NULL, "Wrong parameter code.", "Error!",MB_ICONERROR);
        return;
    }

    // If the function fail, an error message is shown
    //
    if(stsResult != PCAN_ERROR_OK)
        ::MessageBox(NULL, GetFormatedError(stsResult), "Error!",MB_ICONERROR);
}


void CPCANBasicExampleDlg::OnClose()
{
    // Release Hardware if need be
    //
    if(btnRelease.IsWindowEnabled())
        OnBnClickedBtnrelease();

    // Close the Read-Event
    //
    CloseHandle(m_hEvent);

    // (Protected environment)
    //
    {
        clsCritical locker(m_objpCS);
        //Free Ressources
        //
        delete m_objPCANBasic;

        while(m_LastMsgsList->GetCount())
            delete m_LastMsgsList->RemoveHead();
        delete m_LastMsgsList;
    }

    // Uninitialize the Critical Section
    //
    FinalizeProtection();

    CDialog::OnClose();
}

void CPCANBasicExampleDlg::OnBnClickedButtonstatus()
{
    TPCANStatus status;
    CString errorName;
    CString info;

    // Gets the current BUS status of a PCAN Channel.
    //
    status = m_objPCANBasic->GetStatus(m_PcanHandle);

    // Switch On Error Name
    //
    switch(status)
    {
        case PCAN_ERROR_INITIALIZE:
            errorName = "PCAN_ERROR_INITIALIZE";
            break;

        case PCAN_ERROR_BUSLIGHT:
            errorName = "PCAN_ERROR_BUSLIGHT";
            break;

        case PCAN_ERROR_BUSHEAVY: // PCAN_ERROR_BUSWARNING
            errorName = m_IsFD ? "PCAN_ERROR_BUSWARNING" : "PCAN_ERROR_BUSHEAVY";
            break;

        case PCAN_ERROR_BUSPASSIVE:
            errorName = "PCAN_ERROR_BUSPASSIVE";
            break;

        case PCAN_ERROR_BUSOFF:
            errorName = "PCAN_ERROR_BUSOFF";
            break;

        case PCAN_ERROR_OK:
            errorName = "PCAN_ERROR_OK";
            break;

        default:
            errorName = "See Documentation";
            break;
    }

    // Display Message
    //
    info.Format("Status: %s (%Xh)", errorName, status);
    IncludeTextMessage(info);
}

void CPCANBasicExampleDlg::OnBnClickedButtonreset()
{
    TPCANStatus stsResult;

    // Resets the receive and transmit queues of a PCAN Channel.
    //
    stsResult = m_objPCANBasic->Reset(m_PcanHandle);

    // If it fails, a error message is shown
    //
    if (stsResult != PCAN_ERROR_OK)
        ::MessageBox(NULL, GetFormatedError(stsResult), "Error!",MB_ICONERROR);
    else
        IncludeTextMessage("Receive and transmit queues successfully reset");
}


void CPCANBasicExampleDlg::OnBnClickedButtonversion()
{
    TPCANStatus stsResult;
    char buffer[256];
    CString info, strToken;
    int iPos = 0;

    memset(buffer,'\0',255);

    // We get the vesion of the PCAN-Basic API
    //
    stsResult = m_objPCANBasic->GetValue(PCAN_NONEBUS, PCAN_API_VERSION, buffer, 256);
    if (stsResult == PCAN_ERROR_OK)
    {
        info.Format("API Version: %s", buffer);
        IncludeTextMessage(info);

        // We get the version of the firmware on the device
        //
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_FIRMWARE_VERSION, buffer, 256);
        if (stsResult == PCAN_ERROR_OK)
        {
            info.Format("Firmare Version: %s", buffer);
            IncludeTextMessage(info);
        }

        // We get the driver version of the channel being used
        //
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_CHANNEL_VERSION, buffer, 256);
        if (stsResult == PCAN_ERROR_OK)
        {
            info = buffer;
            IncludeTextMessage("Channel/Driver Version: ");

            // Because this information contains line control characters (several lines)
            // we split this also in several entries in the Information List-Box
            //
            strToken = info.Tokenize("\n",iPos);
            while(strToken != "")
            {
                strToken.Insert(0,"     * ");
                IncludeTextMessage(strToken);
                strToken = info.Tokenize("\n",iPos);
            }
        }
    }

    // If the function fail, an error message is shown
    //
    if(stsResult != PCAN_ERROR_OK)
        ::MessageBox(NULL, GetFormatedError(stsResult), "Error!",MB_ICONERROR);
}


void CPCANBasicExampleDlg::OnBnClickedButtoninfoclear()
{
    //Reset listBox Content
    listBoxInfo.ResetContent();
    UpdateData(TRUE);
}


CString CPCANBasicExampleDlg::IntToStr(int iValue)
{
    char chToReceive[20];

    _itoa_s(iValue,chToReceive,10);
    return chToReceive;
}

CString CPCANBasicExampleDlg::IntToHex(int iValue, short iDigits)
{
    CString strTemp, strtest;

    strTemp.Format("%0" + IntToStr(iDigits) + "X",iValue);

    return strTemp;
}
DWORD CPCANBasicExampleDlg::HexTextToInt(CString ToConvert)
{
    DWORD iToReturn = 0;
    int iExp = 0;
    char chByte;

    // The string to convert is empty
    //
    if(ToConvert == "")
        return 0;
    // The string have more than 8 character (the equivalent value
    // exeeds the DWORD capacyty
    //
    if(ToConvert.GetLength() > 8)
        return 0;
    // We convert any character to its Upper case
    //
    ToConvert = ToConvert.MakeUpper();

    try
    {
        // We calculate the number using the Hex To Decimal formula
        //
        for(int i= ToConvert.GetLength()-1; i >= 0; i--){
            chByte = ToConvert[i];
            switch(int(chByte)){
                case 65:
                    iToReturn += (DWORD)(10*pow(16.0f,iExp));
                    break;
                case 66:
                    iToReturn += (DWORD)(11*pow(16.0f,iExp));
                    break;
                case 67:
                    iToReturn += (DWORD)(12*pow(16.0f,iExp));
                    break;
                case 68:
                    iToReturn += (DWORD)(13*pow(16.0f,iExp));
                    break;
                case 69:
                    iToReturn += (DWORD)(14*pow(16.0f,iExp));
                    break;
                case 70:
                    iToReturn += (DWORD)(15*pow(16.0f,iExp));
                    break;
                default:
                    if((int(chByte) <48)||(int(chByte)>57))
                        return -1;
                    iToReturn += (DWORD)(atoi(&chByte)*pow(16.0f,iExp));
                    break;

            }
            iExp++;
        }
    }
    catch(...)
    {
        // Error, return 0
        //
        return 0;
    }

    return iToReturn;
}
void CPCANBasicExampleDlg::CheckHexEditBox(CString* txtData)
{
    int iTest;

    txtData->MakeUpper();

    // We test if the given ID is a valid hexadecimal number.
    //
    iTest = HexTextToInt(*txtData);
    if(iTest > 0)
        *txtData = IntToHex(iTest,2);
    else
        *txtData = "00";
}

int CPCANBasicExampleDlg::AddLVItem(CString Caption)
{
    LVITEM NewItem;

    // Initialize LVITEM
    //
    NewItem.mask = LVIF_TEXT;
    NewItem.iSubItem = 0;
    NewItem.pszText = Caption.GetBuffer();
    NewItem.iItem = lstMessages.GetItemCount();

    // Insert it in the message list
    //
    lstMessages.InsertItem(&NewItem);

    return NewItem.iItem;
}

void CPCANBasicExampleDlg::SetTimerRead(bool bEnable)
{
    // Init Timer
    //
    if(bEnable)
        m_tmrRead = SetTimer(1, 50, 0);
    else
    {
        //Delete Timer
        KillTimer(m_tmrRead);
        m_tmrRead = 0;
    }
}

void CPCANBasicExampleDlg::SetTimerDisplay(bool bEnable)
{
    if(bEnable)
        m_tmrDisplay = SetTimer(2, 100, 0);
    else
    {
        KillTimer(m_tmrDisplay);
        m_tmrDisplay = 0;
    }
}

void CPCANBasicExampleDlg::DisplayMessages()
{
    POSITION pos;
    int iCurrentItem;
    MessageStatus *msgStatus;
    CString strTempCount;

    // We search if a message (Same ID and Type) is
    // already received or if this is a new message
    // (in a protected environment)
    //
    {
        clsCritical locker(m_objpCS);

        pos = m_LastMsgsList->GetHeadPosition();
        for(int i=0; i < m_LastMsgsList->GetCount(); i++)
        {
            msgStatus = (MessageStatus*)m_LastMsgsList->GetNext(pos);
            if(msgStatus->MarkedAsUpdated)
            {
                msgStatus->MarkedAsUpdated = false;

                iCurrentItem = msgStatus->Position;

                strTempCount = lstMessages.GetItemText(iCurrentItem,MSG_COUNT);

                lstMessages.SetItemText(iCurrentItem,MSG_LENGTH,IntToStr(GetLengthFromDLC(msgStatus->CANMsg.DLC, !(msgStatus->CANMsg.MSGTYPE & PCAN_MESSAGE_FD))));
                lstMessages.SetItemText(iCurrentItem,MSG_COUNT,IntToStr(msgStatus->Count));
                lstMessages.SetItemText(iCurrentItem, MSG_TIME, msgStatus->TimeString);
                lstMessages.SetItemText(iCurrentItem,MSG_DATA,msgStatus->DataString);
            }
        }
    }
}

void CPCANBasicExampleDlg::InsertMsgEntry(TPCANMsgFD NewMsg, TPCANTimestampFD timeStamp)
{
    MessageStatus *msgStsCurrentMsg;
    int iCurrentItem;

    // (Protected environment)
    //
    {
        clsCritical locker(m_objpCS);

        // We add this status in the last message list
        //
        msgStsCurrentMsg = new MessageStatus(NewMsg, timeStamp, lstMessages.GetItemCount());
        msgStsCurrentMsg->ShowingPeriod = chbReadingTimeStamp.GetCheck() > 0;
        m_LastMsgsList->AddTail(msgStsCurrentMsg);

        // Add the new ListView Item with the Type of the message
        //
        iCurrentItem = AddLVItem(msgStsCurrentMsg->TypeString);
        // We set the ID of the message
        //
        lstMessages.SetItemText(iCurrentItem,MSG_ID,msgStsCurrentMsg->IdString);
        // We set the length of the Message
        //
        lstMessages.SetItemText(iCurrentItem,MSG_LENGTH,IntToStr(GetLengthFromDLC(NewMsg.DLC, !(NewMsg.MSGTYPE & PCAN_MESSAGE_FD))));
        // we set the message count message (this is the First, so count is 1)
        //
        lstMessages.SetItemText(iCurrentItem,MSG_COUNT,IntToStr(msgStsCurrentMsg->Count));
        // Add timestamp information
        //
        lstMessages.SetItemText(iCurrentItem, MSG_TIME, msgStsCurrentMsg->TimeString);
        // We set the data of the message.
        //
        lstMessages.SetItemText(iCurrentItem,MSG_DATA,msgStsCurrentMsg->DataString);
    }
}

void CPCANBasicExampleDlg::ProcessMessage(TPCANMsgFD theMsg, TPCANTimestampFD itsTimeStamp)
{
    POSITION pos;
    MessageStatus *msg;

    // We search if a message (Same ID and Type) is
    // already received or if this is a new message
    // (in a protected environment)
    //
    {
        clsCritical locker(m_objpCS);

        pos = m_LastMsgsList->GetHeadPosition();
        for(int i=0; i < m_LastMsgsList->GetCount(); i++)
        {
            msg = (MessageStatus*)m_LastMsgsList->GetNext(pos);
            if((msg->CANMsg.ID == theMsg.ID) && (msg->CANMsg.MSGTYPE == theMsg.MSGTYPE))
            {
                // Modify the message and exit
                //
                msg->Update(theMsg, itsTimeStamp);
                return;
            }
        }
        // Message not found. It will created
        //
        InsertMsgEntry(theMsg, itsTimeStamp);
    }
}

void CPCANBasicExampleDlg::ProcessMessage(TPCANMsg theMsg, TPCANTimestamp itsTimeStamp)
{
    TPCANMsgFD newMsg;
    TPCANTimestampFD newTimestamp;

    newMsg = TPCANMsgFD();
    newMsg.ID = theMsg.ID;
    newMsg.DLC = theMsg.LEN;
    for (int i = 0; i < ((theMsg.LEN > 8) ? 8 : theMsg.LEN); i++)
        newMsg.DATA[i] = theMsg.DATA[i];
    newMsg.MSGTYPE = theMsg.MSGTYPE;

    newTimestamp = (itsTimeStamp.micros + 1000 * itsTimeStamp.millis + 0x100000000 * 1000 * itsTimeStamp.millis_overflow);
    ProcessMessage(newMsg, newTimestamp);
}

TPCANStatus CPCANBasicExampleDlg::ReadMessageFD()
{
    TPCANMsgFD CANMsg;
    TPCANTimestampFD CANTimeStamp;
    TPCANStatus stsResult;

    // We execute the "Read" function of the PCANBasic
    //
    stsResult = m_objPCANBasic->ReadFD(m_PcanHandle, &CANMsg, &CANTimeStamp);
    if (stsResult != PCAN_ERROR_QRCVEMPTY)
        // We process the received message
        //
        ProcessMessage(CANMsg, CANTimeStamp);

    return stsResult;
}

TPCANStatus CPCANBasicExampleDlg::ReadMessage()
{
    TPCANMsg CANMsg;
    TPCANTimestamp CANTimeStamp;
    TPCANStatus stsResult;

    // We execute the "Read" function of the PCANBasic
    //
    stsResult = m_objPCANBasic->Read(m_PcanHandle, &CANMsg, &CANTimeStamp);
    if (stsResult != PCAN_ERROR_QRCVEMPTY)
        // We process the received message
        //
        ProcessMessage(CANMsg, CANTimeStamp);

    return stsResult;
}

void CPCANBasicExampleDlg::ReadMessages()
{
    TPCANStatus stsResult;

    // We read at least one time the queue looking for messages.
    // If a message is found, we look again trying to find more.
    // If the queue is empty or an error occurr, we get out from
    // the dowhile statement.
    //
    do
    {
        stsResult =  m_IsFD ? ReadMessageFD() : ReadMessage();
        if (stsResult == PCAN_ERROR_ILLOPERATION)
            break;
    } while (btnRelease.IsWindowEnabled() && (!(stsResult & PCAN_ERROR_QRCVEMPTY)));
}

DWORD WINAPI CPCANBasicExampleDlg::CallCANReadThreadFunc(LPVOID lpParam)
{
    // Cast lpParam argument to PCANBasicExampleDlg*
    //
    CPCANBasicExampleDlg* dialog = (CPCANBasicExampleDlg*)lpParam;

    // Call PCANBasicExampleDlg Thread member function
    //
    return dialog->CANReadThreadFunc(NULL);
}

DWORD WINAPI CPCANBasicExampleDlg::CANReadThreadFunc(LPVOID lpParam)
{
    TPCANStatus stsResult;
    DWORD result, dwTemp;

    m_Terminated = false;

    // Sets the handle of the Receive-Event.
    //
    stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_RECEIVE_EVENT ,&m_hEvent, sizeof(m_hEvent));

    // If it fails, a error message is shown
    //
    if (stsResult != PCAN_ERROR_OK)
    {
        ::MessageBox(NULL, GetFormatedError(stsResult), "Error!",MB_ICONERROR);
        m_Terminated = true;
        return 1;
    }

    // While this mode is selected
    //
    while(!m_Terminated)
    {
        //Wait for CAN Data...
        result = WaitForSingleObject(m_hEvent, 1);

        if (result == WAIT_OBJECT_0)
            ReadMessages();
    }

    // Resets the Event-handle configuration
    //
    dwTemp = 0;
    m_objPCANBasic->SetValue(m_PcanHandle, PCAN_RECEIVE_EVENT ,&dwTemp, sizeof(dwTemp));

    return 0;
}

void CPCANBasicExampleDlg::ReadingModeChanged()
{
    if (!btnRelease.IsWindowEnabled())
        return;

    // If active reading mode is By Timer
    //
    if(m_ActiveReadingMode == 0)
    {
        // Terminate Read Thread if it exists
        //
        if(m_hThread != NULL)
        {
            m_Terminated = true;
            WaitForSingleObject(m_hThread,-1);
            m_hThread = NULL;
        }
        // We start to read
        //
        SetTimerRead(true);
    }
    // If active reading mode is By Event
    //
    else if(m_ActiveReadingMode == 1)
    {
        // We stop to read from the CAN queue
        //
        SetTimerRead(false);

        // Create Reading Thread ....
        //
        m_hThread = CreateThread(NULL, NULL, CPCANBasicExampleDlg::CallCANReadThreadFunc, (LPVOID)this, NULL, NULL);

        if(m_hThread == NULL)
            ::MessageBox(NULL,"Create CANRead-Thread failed","Error!",MB_ICONERROR);
    }
    else
    {
        // Terminate Read Thread if it exists
        //
        if(m_hThread != NULL)
        {
            m_Terminated = true;
            WaitForSingleObject(m_hThread,-1);
            m_hThread = NULL;
        }
        // We start to read
        //
        SetTimerRead(false);
        btnRead.EnableWindow(btnRelease.IsWindowEnabled() && rdbReadingManual.GetCheck());
    }
}
void CPCANBasicExampleDlg::FillComboBoxData()
{
    // Channels will be check
    //
    OnBnClickedButtonHwrefresh();

    // FD Bitrate:
    //      Arbitration: 1 Mbit/sec
    //      Data: 2 Mbit/sec
    //
    txtBitrate = "f_clock_mhz=20, nom_brp=5, nom_tseg1=2, nom_tseg2=1, nom_sjw=1, data_brp=2, data_tseg1=3, data_tseg2=1, data_sjw=1";

    // TPCANBaudrate
    //
    cbbBaudrates.SetCurSel(2); // 500 K
    OnCbnSelchangeCbbbaudrates();

    // Hardware Type for no plugAndplay hardware
    //
    cbbHwsType.SetCurSel(0);
    OnCbnSelchangeCbbhwstype();

    // Interrupt for no plugAndplay hardware
    //
    cbbInterrupt.SetCurSel(0);

    // IO Port for no plugAndplay hardware
    //
    cbbIO.SetCurSel(0);

    // Parameters for GetValue and SetValue function calls
    //
    cbbParameter.SetCurSel(0);
    OnCbnSelchangeComboparameter();
}

CString CPCANBasicExampleDlg::FormatChannelName(TPCANHandle handle, bool isFD)
{
    CString result;
    BYTE byChannel;

    // Gets the owner device and channel for a
    // PCAN-Basic handle
    //
    if(handle < 0x100)
        byChannel = (BYTE)(handle) & 0xF;
    else
        byChannel = (BYTE)(handle) & 0xFF;

    // Constructs the PCAN-Basic Channel name and return it
    //
    result.Format(isFD ? "%s:FD %d (%Xh)" : "%s %d (%Xh)", GetTPCANHandleName(handle), byChannel, handle);
    return result;
}

CString CPCANBasicExampleDlg::FormatChannelName(TPCANHandle handle)
{
    return FormatChannelName(handle, false);
}

CString CPCANBasicExampleDlg::GetTPCANHandleName(TPCANHandle handle)
{
    CString result = "PCAN_NONE";
    switch(handle)
    {
    case PCAN_ISABUS1:
    case PCAN_ISABUS2:
    case PCAN_ISABUS3:
    case PCAN_ISABUS4:
    case PCAN_ISABUS5:
    case PCAN_ISABUS6:
    case PCAN_ISABUS7:
    case PCAN_ISABUS8:
        result = "PCAN_ISA";
        break;

    case PCAN_DNGBUS1:
        result = "PCAN_DNG";
        break;

    case PCAN_PCIBUS1:
    case PCAN_PCIBUS2:
    case PCAN_PCIBUS3:
    case PCAN_PCIBUS4:
    case PCAN_PCIBUS5:
    case PCAN_PCIBUS6:
    case PCAN_PCIBUS7:
    case PCAN_PCIBUS8:
    case PCAN_PCIBUS9:
    case PCAN_PCIBUS10:
    case PCAN_PCIBUS11:
    case PCAN_PCIBUS12:
    case PCAN_PCIBUS13:
    case PCAN_PCIBUS14:
    case PCAN_PCIBUS15:
    case PCAN_PCIBUS16:
        result = "PCAN_PCI";
        break;

    case PCAN_USBBUS1:
    case PCAN_USBBUS2:
    case PCAN_USBBUS3:
    case PCAN_USBBUS4:
    case PCAN_USBBUS5:
    case PCAN_USBBUS6:
    case PCAN_USBBUS7:
    case PCAN_USBBUS8:
    case PCAN_USBBUS9:
    case PCAN_USBBUS10:
    case PCAN_USBBUS11:
    case PCAN_USBBUS12:
    case PCAN_USBBUS13:
    case PCAN_USBBUS14:
    case PCAN_USBBUS15:
    case PCAN_USBBUS16:
        result = "PCAN_USB";
        break;

    case PCAN_PCCBUS1:
    case PCAN_PCCBUS2:
        result = "PCAN_PCC";
        break;

    case PCAN_LANBUS1:
    case PCAN_LANBUS2:
    case PCAN_LANBUS3:
    case PCAN_LANBUS4:
    case PCAN_LANBUS5:
    case PCAN_LANBUS6:
    case PCAN_LANBUS7:
    case PCAN_LANBUS8:
    case PCAN_LANBUS9:
    case PCAN_LANBUS10:
    case PCAN_LANBUS11:
    case PCAN_LANBUS12:
    case PCAN_LANBUS13:
    case PCAN_LANBUS14:
    case PCAN_LANBUS15:
    case PCAN_LANBUS16:
        result = "PCAN_LAN";
        break;
    }
    return result;
}


CString CPCANBasicExampleDlg::GetComboBoxSelectedLabel(CComboBox* ccb)
{
    CString strTemp;
    int item = ccb->GetCurSel();
    if(item != CB_ERR)
        ccb->GetLBText(item, strTemp);

    return strTemp;
}


CString CPCANBasicExampleDlg::GetFormatedError(TPCANStatus error)
{
    TPCANStatus status;
    char buffer[256];
    CString result;

    memset(buffer,'\0',255);
    // Gets the text using the GetErrorText API function
    // If the function success, the translated error is returned. If it fails,
    // a text describing the current error is returned.
    //
    status = m_objPCANBasic->GetErrorText(error, 0, buffer);
    if(status != PCAN_ERROR_OK)
        result.Format("An error ocurred. Error-code's text (%Xh) couldn't be retrieved", error);
    else
        result = buffer;
    return result;
}

void CPCANBasicExampleDlg::SetConnectionStatus(bool bConnected)
{
    // Buttons
    //
    btnInit.EnableWindow(!bConnected);
    btnRead.EnableWindow(bConnected && rdbReadingManual.GetCheck());
    btnWrite.EnableWindow(bConnected);
    btnRelease.EnableWindow(bConnected);
    btnFilterApply.EnableWindow(bConnected);
    btnFilterQuery.EnableWindow(bConnected);
    btnVersions.EnableWindow(bConnected);
    btnRefresh.EnableWindow(!bConnected);
    btnStatus.EnableWindow(bConnected);
    btnReset.EnableWindow(bConnected);

    // ComboBoxs
    //
    cbbChannel.EnableWindow(!bConnected);
    cbbBaudrates.EnableWindow(!bConnected);
    cbbHwsType.EnableWindow(!bConnected);
    cbbIO.EnableWindow(!bConnected);
    cbbInterrupt.EnableWindow(!bConnected);

    // Check-Buttons
    //
    chbCanFD.EnableWindow(!bConnected);

    // Hardware configuration and read mode
    //
    if (!bConnected)
        OnCbnSelchangecbbChannel();
    else
        ReadingModeChanged();

    // Display messages in grid
    //
    SetTimerDisplay(bConnected);
}

bool CPCANBasicExampleDlg::GetFilterStatus(int* status)
{
    TPCANStatus stsResult;

    // Tries to get the stataus of the filter for the current connected hardware
    //
    stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_MESSAGE_FILTER, (void*)status, sizeof(int));

    // If it fails, a error message is shown
    //
    if (stsResult != PCAN_ERROR_OK)
    {
        ::MessageBox(NULL, GetFormatedError(stsResult), "Error!",MB_ICONERROR);
        return false;
    }
    return true;
}

void CPCANBasicExampleDlg::IncludeTextMessage(CString strMsg)
{
    listBoxInfo.AddString(strMsg);
    listBoxInfo.SetCurSel(listBoxInfo.GetCount() - 1);
}

void CPCANBasicExampleDlg::ConfigureLogFile()
{
    int iBuffer;

    // Sets the mask to catch all events
    //
    iBuffer = LOG_FUNCTION_ALL;

    // Configures the log file.
    // NOTE: The Log capability is to be used with the NONEBUS Handle. Other handle than this will
    // cause the function fail.
    //
    m_objPCANBasic->SetValue(PCAN_NONEBUS, PCAN_LOG_CONFIGURE, (void*)&iBuffer, sizeof(iBuffer));
}

void CPCANBasicExampleDlg::ConfigureTraceFile()
{
    int iBuffer;
    TPCANStatus stsResult;

    // Configure the maximum size of a trace file to 5 megabytes
    //
    iBuffer = 5;
    stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_TRACE_SIZE, (void*)&iBuffer, sizeof(iBuffer));
    if (stsResult != PCAN_ERROR_OK)
        IncludeTextMessage(GetFormatedError(stsResult));

    // Configure the way how trace files are created:
    // * Standard name is used
    // * Existing file is ovewritten,
    // * Only one file is created.
    // * Recording stopts when the file size reaches 5 megabytes.
    //
    iBuffer = TRACE_FILE_SINGLE | TRACE_FILE_OVERWRITE;
    stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_TRACE_CONFIGURE, (void*)&iBuffer, sizeof(iBuffer));
    if (stsResult != PCAN_ERROR_OK)
        IncludeTextMessage(GetFormatedError(stsResult));
}

void CPCANBasicExampleDlg::OnBnClickedChbfcanfd()
{
    m_IsFD = chbCanFD.GetCheck() > 0;

    cbbBaudrates.ShowWindow(!m_IsFD ? SW_SHOW : SW_HIDE);
    cbbHwsType.ShowWindow(!m_IsFD ? SW_SHOW : SW_HIDE);
    cbbIO.ShowWindow(!m_IsFD ? SW_SHOW : SW_HIDE);
    cbbInterrupt.ShowWindow(!m_IsFD ? SW_SHOW : SW_HIDE);
    GetDlgItem(IDC_LABAUDRATE)->ShowWindow(!m_IsFD ? SW_SHOW : SW_HIDE);
    GetDlgItem(IDC_LAHWTYPE)->ShowWindow(!m_IsFD ? SW_SHOW : SW_HIDE);
    GetDlgItem(IDC_LAIOPORT)->ShowWindow(!m_IsFD ? SW_SHOW : SW_HIDE);
    GetDlgItem(IDC_LAINTERRUPT)->ShowWindow(!m_IsFD ? SW_SHOW : SW_HIDE);

    GetDlgItem(IDC_TXTBITRATE)->ShowWindow(m_IsFD ? SW_SHOW : SW_HIDE);
    GetDlgItem(IDC_LABITRATE)->ShowWindow(m_IsFD ? SW_SHOW : SW_HIDE);
    GetDlgItem(IDC_CHBFD)->ShowWindow(m_IsFD ? SW_SHOW : SW_HIDE);
    GetDlgItem(IDC_CHBBRS)->ShowWindow(m_IsFD ? SW_SHOW : SW_HIDE);
}



void CPCANBasicExampleDlg::OnLbnDblclkListinfo()
{
    OnBnClickedButtoninfoclear();
}

void CPCANBasicExampleDlg::OnBnClickedChbfd()
{
    CButton *ctrlRemote, *ctrlBRS, *ctrlFD;
    CString strTemp;
    int iMax;
    UpdateData(TRUE);

    ctrlRemote = (CButton*)GetDlgItem(IDC_CHBREMOTE);
    ctrlBRS = (CButton*)GetDlgItem(IDC_CHBBRS);
    ctrlFD = (CButton*)GetDlgItem(IDC_CHBFD);

    ctrlRemote->EnableWindow(!chbFD);
    ctrlBRS->EnableWindow(chbFD);

    if (!ctrlBRS->IsWindowEnabled())
        chbBRS = false;

    iMax = chbFD ? 15 : 8;
    nudLength.SetRange(0, iMax);
    if(nudLength.GetPos() > iMax)
    {
        txtLength = IntToStr(iMax);
        EnableDisableDataFields(iMax);
    }
    UpdateData(FALSE);
}

void CPCANBasicExampleDlg::OnBnClickedChbbrs()
{
    UpdateData(TRUE);
}
