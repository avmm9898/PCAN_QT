// PCANBasicExampleDlg.h : header file
//

#pragma once

// Includes
//
#include "afxwin.h"
#include "afxcmn.h"
#include <Math.h>
#include "PCANBasicClass.h"


using namespace std;

// Constant values to make the code more comprehensible
// to add and read items from the List view
//
#define MSG_TYPE        0
#define MSG_ID          1
#define MSG_LENGTH      2
#define MSG_COUNT       3
#define MSG_TIME        4
#define MSG_DATA        5

// Critical Section class for thread-safe menbers access
//
#pragma region Critical Section Class
class clsCritical
{
private:
    CRITICAL_SECTION *m_objpCS;
    LONG volatile m_dwOwnerThread;
    LONG volatile m_dwLocked;
    bool volatile m_bDoRecursive;

public:
    explicit clsCritical(CRITICAL_SECTION *cs, bool createUnlocked = false, bool lockRecursively = false);
    ~clsCritical();
    int GetRecursionCount();
    bool IsLocked();
    int Enter();
    int Leave();
};
#pragma endregion

// Message Status structure used to show CAN Messages
// in a ListView
//
#pragma region Message Status class
class MessageStatus
{
private:
    TPCANMsgFD m_Msg;
    TPCANTimestampFD m_TimeStamp;
    TPCANTimestampFD m_oldTimeStamp;
    int m_iIndex;
    int m_Count;
    bool m_bShowPeriod;
    bool m_bWasChanged;

public:
    MessageStatus(TPCANMsgFD canMsg, TPCANTimestampFD canTimestamp, int listIndex);
    void Update(TPCANMsgFD canMsg, TPCANTimestampFD canTimestamp);

    TPCANMsgFD GetCANMsg();
    TPCANTimestampFD GetTimestamp();
    int GetPosition();
    CString GetTypeString();
    CString GetIdString();
    CString GetDataString();
    CString GetTimeString();
    int GetCount();
    bool GetShowingPeriod();
    bool GetMarkedAsUpdated();

    void SetShowingPeriod(bool value);
    void SetMarkedAsUpdated(bool value);

    __declspec(property (get = GetCANMsg)) TPCANMsgFD CANMsg;
    __declspec(property (get = GetTimestamp)) TPCANTimestampFD Timestamp;
    __declspec(property (get = GetPosition)) int Position;
    __declspec(property (get = GetTypeString)) CString TypeString;
    __declspec(property (get = GetIdString)) CString IdString;
    __declspec(property (get = GetDataString)) CString DataString;
    __declspec(property (get = GetTimeString)) CString TimeString;
    __declspec(property (get = GetCount)) int Count;
    __declspec(property (get = GetShowingPeriod, put = SetShowingPeriod)) bool ShowingPeriod;
    __declspec(property (get = GetMarkedAsUpdated, put = SetMarkedAsUpdated)) bool MarkedAsUpdated;
};
#pragma endregion

// PCANBasicExampleDlg dialog
class CPCANBasicExampleDlg : public CDialog
{
// Construction
public:
    CPCANBasicExampleDlg(CWnd* pParent = NULL); // standard constructor

// Dialog Data
    enum { IDD = IDD_PCANBASICEXAMPLE_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Connection
    //
    CComboBox cbbChannel;
    CButton btnRefresh;
    CComboBox cbbBaudrates;
    CComboBox cbbHwsType;
    CComboBox cbbIO;
    CComboBox cbbInterrupt;
    CButton btnInit;
    CButton btnRelease;
    CButton chbCanFD;
    CString txtBitrate;

    // Filtering
    CButton chbFilterExtended;
    CButton rdbFilterOpen;
    CButton rdbFilterClose;
    CButton rdbFilterCustom;
    CString txtFilterFrom;
    CString txtFilterTo;
    CSpinButtonCtrl nudFilterFrom;
    CSpinButtonCtrl nudFilterTo;
    CButton btnFilterApply;
    CButton btnFilterQuery;

    //Parameter
    CComboBox cbbParameter;
    CButton rdbParameterActive;
    CButton rdbParameterInactive;
    CEdit editParameterDevNumberOrDelay;
    CString txtParameterDevNumber;
    CSpinButtonCtrl nudParameterDevNumberOrDelay;
    CButton btnParameterGet;
    CButton btnParameterSet;

    //Reading
    CButton rdbReadingTimer;
    CButton rdbReadingEvent;
    CButton rdbReadingManual;
    CButton chbReadingTimeStamp;
    CButton btnRead;
    CButton btnReadingClear;
    CListCtrl lstMessages;

    //Writing
    CButton btnWrite;
    CSpinButtonCtrl nudLength;
    CString txtID;
    CString txtLength;
    CString txtData0;
    CString txtData1;
    CString txtData2;
    CString txtData3;
    CString txtData4;
    CString txtData5;
    CString txtData6;
    CString txtData7;
    CString txtData8;
    CString txtData9;
    CString txtData10;
    CString txtData11;
    CString txtData12;
    CString txtData13;
    CString txtData14;
    CString txtData15;
    CString txtData16;
    CString txtData17;
    CString txtData18;
    CString txtData19;
    CString txtData20;
    CString txtData21;
    CString txtData22;
    CString txtData23;
    CString txtData24;
    CString txtData25;
    CString txtData26;
    CString txtData27;
    CString txtData28;
    CString txtData29;
    CString txtData30;
    CString txtData31;
    CString txtData32;
    CString txtData33;
    CString txtData34;
    CString txtData35;
    CString txtData36;
    CString txtData37;
    CString txtData38;
    CString txtData39;
    CString txtData40;
    CString txtData41;
    CString txtData42;
    CString txtData43;
    CString txtData44;
    CString txtData45;
    CString txtData46;
    CString txtData47;
    CString txtData48;
    CString txtData49;
    CString txtData50;
    CString txtData51;
    CString txtData52;
    CString txtData53;
    CString txtData54;
    CString txtData55;
    CString txtData56;
    CString txtData57;
    CString txtData58;
    CString txtData59;
    CString txtData60;
    CString txtData61;
    CString txtData62;
    CString txtData63;
    BOOL chbExtended;
    BOOL chbRemote;
    BOOL chbFD;
    BOOL chbBRS;

    //Information
    CListBox listBoxInfo;
    CButton btnStatus;
    CButton btnReset;
    CButton btnVersions;

    // Event functions
    //
    void InitializeControls(void);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnCbnSelchangecbbChannel();
    afx_msg void OnCbnSelchangeCbbbaudrates();
    afx_msg void OnEnKillfocusTxtid();
    afx_msg void OnEnKillfocusTxtdata0();
    afx_msg void OnEnKillfocusTxtdata1();
    afx_msg void OnEnKillfocusTxtdata2();
    afx_msg void OnEnKillfocusTxtdata3();
    afx_msg void OnEnKillfocusTxtdata4();
    afx_msg void OnEnKillfocusTxtdata5();
    afx_msg void OnEnKillfocusTxtdata6();
    afx_msg void OnEnKillfocusTxtdata7();
    afx_msg void OnEnKillfocusTxtdata8();
    afx_msg void OnEnKillfocusTxtdata9();
    afx_msg void OnEnKillfocusTxtdata10();
    afx_msg void OnEnKillfocusTxtdata11();
    afx_msg void OnEnKillfocusTxtdata12();
    afx_msg void OnEnKillfocusTxtdata13();
    afx_msg void OnEnKillfocusTxtdata14();
    afx_msg void OnEnKillfocusTxtdata15();
    afx_msg void OnEnKillfocusTxtdata16();
    afx_msg void OnEnKillfocusTxtdata17();
    afx_msg void OnEnKillfocusTxtdata18();
    afx_msg void OnEnKillfocusTxtdata19();
    afx_msg void OnEnKillfocusTxtdata20();
    afx_msg void OnEnKillfocusTxtdata21();
    afx_msg void OnEnKillfocusTxtdata22();
    afx_msg void OnEnKillfocusTxtdata23();
    afx_msg void OnEnKillfocusTxtdata24();
    afx_msg void OnEnKillfocusTxtdata25();
    afx_msg void OnEnKillfocusTxtdata26();
    afx_msg void OnEnKillfocusTxtdata27();
    afx_msg void OnEnKillfocusTxtdata28();
    afx_msg void OnEnKillfocusTxtdata29();
    afx_msg void OnEnKillfocusTxtdata30();
    afx_msg void OnEnKillfocusTxtdata31();
    afx_msg void OnEnKillfocusTxtdata32();
    afx_msg void OnEnKillfocusTxtdata33();
    afx_msg void OnEnKillfocusTxtdata34();
    afx_msg void OnEnKillfocusTxtdata35();
    afx_msg void OnEnKillfocusTxtdata36();
    afx_msg void OnEnKillfocusTxtdata37();
    afx_msg void OnEnKillfocusTxtdata38();
    afx_msg void OnEnKillfocusTxtdata39();
    afx_msg void OnEnKillfocusTxtdata40();
    afx_msg void OnEnKillfocusTxtdata41();
    afx_msg void OnEnKillfocusTxtdata42();
    afx_msg void OnEnKillfocusTxtdata43();
    afx_msg void OnEnKillfocusTxtdata44();
    afx_msg void OnEnKillfocusTxtdata45();
    afx_msg void OnEnKillfocusTxtdata46();
    afx_msg void OnEnKillfocusTxtdata47();
    afx_msg void OnEnKillfocusTxtdata48();
    afx_msg void OnEnKillfocusTxtdata49();
    afx_msg void OnEnKillfocusTxtdata50();
    afx_msg void OnEnKillfocusTxtdata51();
    afx_msg void OnEnKillfocusTxtdata52();
    afx_msg void OnEnKillfocusTxtdata53();
    afx_msg void OnEnKillfocusTxtdata54();
    afx_msg void OnEnKillfocusTxtdata55();
    afx_msg void OnEnKillfocusTxtdata56();
    afx_msg void OnEnKillfocusTxtdata57();
    afx_msg void OnEnKillfocusTxtdata58();
    afx_msg void OnEnKillfocusTxtdata59();
    afx_msg void OnEnKillfocusTxtdata60();
    afx_msg void OnEnKillfocusTxtdata61();
    afx_msg void OnEnKillfocusTxtdata62();
    afx_msg void OnEnKillfocusTxtdata63();
    afx_msg void OnLbnDblclkListinfo();
    afx_msg void OnBnClickedChbfd();
    afx_msg void OnBnClickedChbbrs();
    afx_msg void OnDeltaposNudlength(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedChbextended();
    afx_msg void OnBnClickedChbremote();
    afx_msg void OnNMDblclkLstmessages(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedBtninit();
    afx_msg void OnBnClickedBtnrelease();
    afx_msg void OnBnClickedBtnwrite();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg void OnBnClickedRdbtimer();
    afx_msg void OnBnClickedRdbevent();
    afx_msg void OnBnClickedChbtimestamp();
    afx_msg void OnBnClickedButtonHwrefresh();
    afx_msg void OnCbnSelchangeCbbhwstype();
    afx_msg void OnBnClickedChbfilterextended();
    afx_msg void OnDeltaposNudfilterfrom(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnEnKillfocusTxtfilterfrom();
    afx_msg void OnDeltaposNudfilterto(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnEnKillfocusTxtfilterto();
    afx_msg void OnBnClickedButtonfilterapply();
    afx_msg void OnBnClickedButtonfilterquery();
    afx_msg void OnBnClickedRdbmanual();
    afx_msg void OnBnClickedButtonread();
    afx_msg void OnBnClickedButtonreadingclear();
    afx_msg void OnCbnSelchangeComboparameter();
    afx_msg void OnDeltaposNudparamdevnumber(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnEnKillfocusTxtparamdevnumber();
    afx_msg void OnBnClickedButtonparamset();
    afx_msg void OnBnClickedButtonparamget();
    afx_msg void OnClose();
    afx_msg void OnBnClickedButtonversion();
    afx_msg void OnBnClickedButtoninfoclear();
    afx_msg void OnBnClickedButtonstatus();
    afx_msg void OnBnClickedButtonreset();
    afx_msg void OnBnClickedChbfcanfd();

// Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()


private:
    // ------------------------------------------------------------------------------------------
    // Private Members
    // ------------------------------------------------------------------------------------------
    // Variables to store the current PCANBasic instance
    //
    bool m_Terminated;
    PCANBasicClass *m_objPCANBasic;

    // Saves the desired connection mode
    //
    bool m_IsFD;

    // Saves the handle of a PCAN hardware
    //
    TPCANHandle m_PcanHandle;

    // Saves the baudrate register for a conenction
    //
    TPCANBaudrate m_Baudrate;

    // Saves the type of a non-plug-and-play hardware
    //
    TPCANType m_HwType;

    // Variables to store the current reading mode
    // 0 : Timer Mode
    // 1 : Event Mode
    // 2 : Manual Mode
    //
    int m_ActiveReadingMode;

    // Read Timer identifier
    //
    UINT_PTR m_tmrRead;

    //Display Timer identifier
    //
    UINT_PTR m_tmrDisplay;

    // CAN messages array. Store the message status for its display
    //
    CPtrList *m_LastMsgsList;

    // Handle to set Received-Event
    //
    HANDLE m_hEvent;

    // Handle to the thread to read using Received-Event method
    //
    HANDLE m_hThread;

    // Handles of non plug and play PCAN-Hardware
    //
    TPCANHandle m_NonPnPHandles[9];
    // Handle for a Critical Section
    //
    CRITICAL_SECTION *m_objpCS;

    // ------------------------------------------------------------------------------------------
    // Help functions
    // ------------------------------------------------------------------------------------------
    // Convert a int value to a CString
    //
    CString IntToStr(int iValue);
    // Convert a int value to a CString formated in Hexadecimal
    //
    CString IntToHex(int iValue, short iDigits);
    // Convert hexadecimal Cstring into int value (Zero if error)
    //
    DWORD HexTextToInt(CString ToConvert);
    // Check txtData in an hexadecimal value
    //
    void CheckHexEditBox(CString* txtData);
    // Enables/Disables Data text boxes according with a given length
    //
    void EnableDisableDataFields(int length);
    // Create a LVItem renderer for MFC ListView
    //
    int AddLVItem(CString Caption);
    // Enable/Disable Read Timer
    //
    void SetTimerRead(bool bEnable);
    // Enable/Disable Display Timer
    //
    void SetTimerDisplay(bool bEnable);
    // Configures the data of all ComboBox components of the main-form
    //
    void FillComboBoxData();
    // Gets the formated text for a PCAN-Basic channel handle
    //
    CString FormatChannelName(TPCANHandle handle, bool isFD);
    // Gets the formated text for a PCAN-Basic channel handle
    //
    CString FormatChannelName(TPCANHandle handle);
    // Gets the name for a PCAN-Basic channel handle
    //
    CString GetTPCANHandleName(TPCANHandle handle);
    // Help Function used to get an error as text
    //
    CString GetFormatedError(TPCANStatus error);
    //Activates/deaactivates the different controls of the main-form according
    //with the current connection status
    //
    void SetConnectionStatus(bool bConnected);
    // Gets the current status of the PCAN-Basic message filter
    //
    bool GetFilterStatus(int* status);
    // Includes a new line of text into the information Listview
    //
    void IncludeTextMessage(CString strMsg);
    // Gets ComboBox selected label
    //
    CString GetComboBoxSelectedLabel(CComboBox* ccb);
    // Configures the Debug-Log file of PCAN-Basic
    //
    void ConfigureLogFile();
    // Configures the PCAN-Trace file for a PCAN-Basic Channel
    //
    void ConfigureTraceFile();

    // ------------------------------------------------------------------------------------------
    // Message-proccessing functions
    // ------------------------------------------------------------------------------------------
    // Display CAN messages in the Message-ListView
    //
    void DisplayMessages();
    // Create new MessageStatus using provided parameters
    //
    void InsertMsgEntry(TPCANMsgFD NewMsg, TPCANTimestampFD MyTimeStamp);
    // Processes a received message, in order to show it in the Message-ListView
    //
    void ProcessMessage(TPCANMsgFD theMsg, TPCANTimestampFD itsTimeStamp);
    void ProcessMessage(TPCANMsg MyMsg, TPCANTimestamp MyTimeStamp);
    // static Thread function to manage reading by event
    //
    static DWORD WINAPI CallCANReadThreadFunc(LPVOID lpParam);
    // member Thread function to manage reading by event
    //
    DWORD WINAPI CANReadThreadFunc(LPVOID lpParam);
    // Manage Reading method (Timer, Event or manual)
    //
    void ReadingModeChanged();
    // Functions for reading PCAN-Basic messages
    //
    TPCANStatus ReadMessageFD();
    TPCANStatus ReadMessage();
    void ReadMessages();
    // Functions for writing PCAN-Basic messages
    //
    TPCANStatus WriteFrame();
    TPCANStatus WriteFrameFD();

    // Critical section Ini/deinit functions
    //
    void InitializeProtection();
    void FinalizeProtection();

public:

    CStatic labelDeviceOrDelay;
};
