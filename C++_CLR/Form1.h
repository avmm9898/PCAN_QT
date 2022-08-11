#pragma once

#include ".\PCANBasicCLR.h"

namespace PCANBasicExample 
{
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Threading;

	/// <summary>
	/// Inclusion of PEAK PCAN-Basic namespace
	/// </summary>
	using namespace Peak::Can::Basic;

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

	/// <summary>
	/// Message Status structure used to show CAN Messages
	/// in a ListView
	///
	/// </summary>
	public value class MessageStatus 
	{
		private: 
			TPCANMsgFD m_Msg;
			TPCANTimestampFD m_TimeStamp;
			TPCANTimestampFD m_oldTimeStamp;
			int m_iIndex;
			int m_Count;
			bool m_bShowPeriod;
			bool m_bWasChanged;

			// Constructor
			//
		public: MessageStatus(TPCANMsgFD canMsg, TPCANTimestampFD canTimestamp, int listIndex)
			{
				m_Msg = canMsg;
				m_TimeStamp = canTimestamp;
				m_oldTimeStamp = canTimestamp;
				m_iIndex = listIndex;
				m_Count = 1;
				m_bShowPeriod = true;
				m_bWasChanged = false;
			}

		public: void Update(TPCANMsgFD ^canMsg, TPCANTimestampFD canTimestamp)
            {
                m_Msg = *canMsg;
                m_oldTimeStamp = m_TimeStamp;
                m_TimeStamp = canTimestamp;
                m_bWasChanged = true;
                m_Count += 1;
            }

		private: String^ GetTimeString()
            {
                double fTime;

                fTime = (m_TimeStamp / 1000.0);
                if (m_bShowPeriod)
                    fTime -= (m_oldTimeStamp / 1000.0);
                return fTime.ToString("F1");                
            }

		private: String^ GetDataString()
            {
                String^ strTemp;

                strTemp = "";

				if ((m_Msg.MSGTYPE & TPCANMessageType::PCAN_MESSAGE_RTR) == TPCANMessageType::PCAN_MESSAGE_RTR)
                    return "Remote Request";
                else
					for (int i = 0; i < GetLengthFromDLC(m_Msg.DLC, (m_Msg.MSGTYPE & TPCANMessageType::PCAN_MESSAGE_FD) == TPCANMessageType()); i++)
						strTemp += String::Format("{0:X2} ", m_Msg.DATA[i]);

                return strTemp;
            }

		private: String^ GetIdString()
            {
                // We format the ID of the message and show it
                //
				if ((m_Msg.MSGTYPE & TPCANMessageType::PCAN_MESSAGE_EXTENDED) == TPCANMessageType::PCAN_MESSAGE_EXTENDED)
					return String::Format("{0:X8}h", m_Msg.ID);
                else
					return String::Format("{0:X3}h", m_Msg.ID);
            }

		private: String^ GetMsgTypeString()
            {
                String^ strTemp;
                bool isEcho = (m_Msg.MSGTYPE & TPCANMessageType::PCAN_MESSAGE_ECHO) == TPCANMessageType::PCAN_MESSAGE_ECHO;

				if ((m_Msg.MSGTYPE & TPCANMessageType::PCAN_MESSAGE_STATUS) == TPCANMessageType::PCAN_MESSAGE_STATUS)
                    return "STATUS";

				if ((m_Msg.MSGTYPE & TPCANMessageType::PCAN_MESSAGE_ERRFRAME) == TPCANMessageType::PCAN_MESSAGE_ERRFRAME)
                    return "ERROR";

				if ((m_Msg.MSGTYPE & TPCANMessageType::PCAN_MESSAGE_EXTENDED) == TPCANMessageType::PCAN_MESSAGE_EXTENDED)
                    strTemp = "EXT";
                else
                    strTemp = "STD";

				if ((m_Msg.MSGTYPE & TPCANMessageType::PCAN_MESSAGE_RTR) == TPCANMessageType::PCAN_MESSAGE_RTR)
                    strTemp += isEcho ? "/RTR [ ECHO ]" : "/RTR";
				else
					if ((int)m_Msg.MSGTYPE > (int)TPCANMessageType::PCAN_MESSAGE_EXTENDED)
					{
                        if (isEcho)
                            strTemp += " [ ECHO";
                        else
                            strTemp += " [ ";
						if ((m_Msg.MSGTYPE & TPCANMessageType::PCAN_MESSAGE_FD) == TPCANMessageType::PCAN_MESSAGE_FD)
                            strTemp += " FD";
						if ((m_Msg.MSGTYPE & TPCANMessageType::PCAN_MESSAGE_BRS) == TPCANMessageType::PCAN_MESSAGE_BRS)
                            strTemp += " BRS";
						if ((m_Msg.MSGTYPE & TPCANMessageType::PCAN_MESSAGE_ESI) == TPCANMessageType::PCAN_MESSAGE_ESI)
                            strTemp += " ESI";
                        strTemp += " ]";
					}

                return strTemp;
            }

		public: property TPCANMsgFD CANMsg
            {
                TPCANMsgFD get() { return m_Msg; }
            }

		public: property TPCANTimestampFD Timestamp
            {
                TPCANTimestampFD get() { return m_TimeStamp; }
            }

		public: property int Position
            {
                int get() { return m_iIndex; }
            }

		public: property String^ TypeString
            {
                String^ get() { return GetMsgTypeString(); }
            }

		public: property String^ IdString
            {
                String^ get() { return GetIdString(); }
            }

		public: property String^ DataString
            {
                String^ get() { return GetDataString(); }
            }

		public: property int Count
            {
                int get() { return m_Count; }
            }

		public: property bool ShowingPeriod
            {
                bool get() { return m_bShowPeriod; }
                void set(bool value)
                {
                    if (m_bShowPeriod ^ value)
                    {
                        m_bShowPeriod = value;
                        m_bWasChanged = true;
                    }
                }
            }

		public: property bool MarkedAsUpdated
            {
                bool get() { return m_bWasChanged; }
                void set(bool value) { m_bWasChanged = value; }
            }

		public: property String^ TimeString
            {
                String^ get() { return GetTimeString(); }
            }
	};


	/// <summary>
	/// Summary for Form1
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
		Form1(void)
		{
            // Initializes Form's component
            //
			InitializeComponent();
            // Initializes specific components
            //
			InitializeBasicComponents();
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}
	
	private: System::Windows::Forms::CheckBox^  chbShowPeriod;
	private: System::Windows::Forms::RadioButton^  rdbManual;
	private: System::Windows::Forms::RadioButton^  rdbEvent;
	private: System::Windows::Forms::ListView^  lstMessages;
	private: System::Windows::Forms::ColumnHeader^  clhType;
	private: System::Windows::Forms::ColumnHeader^  clhID;
	private: System::Windows::Forms::ColumnHeader^  clhLength;
	private: System::Windows::Forms::ColumnHeader^  clhData;
	private: System::Windows::Forms::ColumnHeader^  clhCount;
	private: System::Windows::Forms::ColumnHeader^  clhRcvTime;
	private: System::Windows::Forms::Button^  btnMsgClear;
	private: System::Windows::Forms::ListBox^  lbxInfo;
	private: System::Windows::Forms::Button^  btnInfoClear;
	private: System::Windows::Forms::GroupBox^  groupBox5;
	private: System::Windows::Forms::RadioButton^  rdbTimer;
	private: System::Windows::Forms::Button^  btnRead;
	private: System::Windows::Forms::Button^  btnGetVersions;
	private: System::Windows::Forms::GroupBox^  groupBox6;
	private: System::Windows::Forms::GroupBox^  groupBox4;
	private: System::Windows::Forms::ComboBox^  cbbChannel;
	private: System::Windows::Forms::RadioButton^  rdbParamInactive;
	private: System::Windows::Forms::Button^  btnHwRefresh;
	private: System::Windows::Forms::ComboBox^  cbbHwType;
	private: System::Windows::Forms::ComboBox^  cbbInterrupt;
	private: System::Windows::Forms::Button^  btnFilterQuery;
	private: System::Windows::Forms::Label^  laInterrupt;
	private: System::Windows::Forms::GroupBox^  groupBox3;
	private: System::Windows::Forms::CheckBox^  chbFilterExt;
	private: System::Windows::Forms::NumericUpDown^  nudIdTo;
	private: System::Windows::Forms::NumericUpDown^  nudIdFrom;
	private: System::Windows::Forms::Label^  label8;
	private: System::Windows::Forms::Label^  label7;
	private: System::Windows::Forms::RadioButton^  rdbFilterOpen;
	private: System::Windows::Forms::RadioButton^  rdbFilterCustom;
	private: System::Windows::Forms::RadioButton^  rdbFilterClose;
	private: System::Windows::Forms::Button^  btnFilterApply;
	private: System::Windows::Forms::ComboBox^  cbbIO;
	private: System::Windows::Forms::Timer^  tmrRead;
	private: System::Windows::Forms::Label^  laIOPort;
	private: System::Windows::Forms::Label^  laHwType;
	private: System::Windows::Forms::ComboBox^  cbbBaudrates;
	private: System::Windows::Forms::Label^  laBaudrate;
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::Button^  btnInit;
	private: System::Windows::Forms::Button^  btnRelease;
	private: System::Windows::Forms::Button^  btnParameterGet;
	private: System::Windows::Forms::Label^  label10;
	private: System::Windows::Forms::NumericUpDown^  nudDeviceIdOrDelay;

	private: System::Windows::Forms::Label^  laDeviceOrDelay;

	private: System::Windows::Forms::ComboBox^  cbbParameter;
	private: System::Windows::Forms::GroupBox^  groupBox2;
	private: System::Windows::Forms::Label^  label6;
	private: System::Windows::Forms::RadioButton^  rdbParamActive;
	private: System::Windows::Forms::Button^  btnParameterSet;
	private: System::Windows::Forms::GroupBox^  groupBox1;
	private: System::Windows::Forms::Button^  btnReset;
	private: System::Windows::Forms::Button^  btnStatus;
	private: System::Windows::Forms::Timer^  tmrDisplay;
	private: System::Windows::Forms::CheckBox^  chbCanFD;
	private: System::Windows::Forms::TextBox^  txtBitrate;
	private: System::Windows::Forms::Label^  laBitrate;
	private: System::Windows::Forms::Label^  label5;
	private: System::Windows::Forms::Label^  laLength;
	private: System::Windows::Forms::Label^  label3;
	private: System::Windows::Forms::TextBox^  txtData60;
	private: System::Windows::Forms::TextBox^  txtData62;
	private: System::Windows::Forms::TextBox^  txtData47;
	private: System::Windows::Forms::TextBox^  txtData58;
	private: System::Windows::Forms::TextBox^  txtData57;
	private: System::Windows::Forms::TextBox^  txtData56;
	private: System::Windows::Forms::TextBox^  txtData55;
	private: System::Windows::Forms::TextBox^  txtData54;
	private: System::Windows::Forms::TextBox^  txtData61;
	private: System::Windows::Forms::TextBox^  txtData63;
	private: System::Windows::Forms::TextBox^  txtData48;
	private: System::Windows::Forms::TextBox^  txtData53;
	private: System::Windows::Forms::TextBox^  txtData52;
	private: System::Windows::Forms::TextBox^  txtData51;
	private: System::Windows::Forms::TextBox^  txtData50;
	private: System::Windows::Forms::TextBox^  txtData49;
	private: System::Windows::Forms::TextBox^  txtData59;
	private: System::Windows::Forms::TextBox^  txtData46;
	private: System::Windows::Forms::TextBox^  txtData45;
	private: System::Windows::Forms::TextBox^  txtData44;
	private: System::Windows::Forms::TextBox^  txtData43;
	private: System::Windows::Forms::TextBox^  txtData42;
	private: System::Windows::Forms::TextBox^  txtData41;
	private: System::Windows::Forms::TextBox^  txtData40;
	private: System::Windows::Forms::TextBox^  txtData39;
	private: System::Windows::Forms::TextBox^  txtData38;
	private: System::Windows::Forms::TextBox^  txtData37;
	private: System::Windows::Forms::TextBox^  txtData36;
	private: System::Windows::Forms::TextBox^  txtData35;
	private: System::Windows::Forms::TextBox^  txtData34;
	private: System::Windows::Forms::TextBox^  txtData33;
	private: System::Windows::Forms::TextBox^  txtData32;
	private: System::Windows::Forms::TextBox^  txtData31;
	private: System::Windows::Forms::TextBox^  txtData30;
	private: System::Windows::Forms::TextBox^  txtData29;
	private: System::Windows::Forms::TextBox^  txtData28;
	private: System::Windows::Forms::TextBox^  txtData27;
	private: System::Windows::Forms::TextBox^  txtData26;
	private: System::Windows::Forms::TextBox^  txtData25;
	private: System::Windows::Forms::TextBox^  txtData24;
	private: System::Windows::Forms::TextBox^  txtData23;
	private: System::Windows::Forms::TextBox^  txtData22;
	private: System::Windows::Forms::TextBox^  txtData21;
	private: System::Windows::Forms::TextBox^  txtData20;
	private: System::Windows::Forms::TextBox^  txtData19;
	private: System::Windows::Forms::TextBox^  txtData18;
	private: System::Windows::Forms::TextBox^  txtData17;
	private: System::Windows::Forms::TextBox^  txtData16;
	private: System::Windows::Forms::TextBox^  txtData15;
	private: System::Windows::Forms::TextBox^  txtData14;
	private: System::Windows::Forms::TextBox^  txtData13;
	private: System::Windows::Forms::TextBox^  txtData12;
	private: System::Windows::Forms::TextBox^  txtData11;
	private: System::Windows::Forms::TextBox^  txtData10;
	private: System::Windows::Forms::TextBox^  txtData9;
	private: System::Windows::Forms::TextBox^  txtData8;
	private: System::Windows::Forms::CheckBox^  chbBRS;
	private: System::Windows::Forms::CheckBox^  chbFD;
	private: System::Windows::Forms::CheckBox^  chbRemote;
	private: System::Windows::Forms::CheckBox^  chbExtended;
	private: System::Windows::Forms::Button^  btnWrite;
	private: System::Windows::Forms::Label^  label12;
	private: System::Windows::Forms::Label^  label13;
	private: System::Windows::Forms::TextBox^  txtID;
	private: System::Windows::Forms::TextBox^  txtData7;
	private: System::Windows::Forms::TextBox^  txtData6;
	private: System::Windows::Forms::TextBox^  txtData5;
	private: System::Windows::Forms::TextBox^  txtData4;
	private: System::Windows::Forms::TextBox^  txtData3;
	private: System::Windows::Forms::TextBox^  txtData2;
	private: System::Windows::Forms::TextBox^  txtData1;
	private: System::Windows::Forms::TextBox^  txtData0;
	private: System::Windows::Forms::NumericUpDown^  nudLength;
	private: System::ComponentModel::IContainer^  components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
            this->components = (gcnew System::ComponentModel::Container());
            System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(Form1::typeid));
            this->chbShowPeriod = (gcnew System::Windows::Forms::CheckBox());
            this->rdbManual = (gcnew System::Windows::Forms::RadioButton());
            this->rdbEvent = (gcnew System::Windows::Forms::RadioButton());
            this->lstMessages = (gcnew System::Windows::Forms::ListView());
            this->clhType = (gcnew System::Windows::Forms::ColumnHeader());
            this->clhID = (gcnew System::Windows::Forms::ColumnHeader());
            this->clhLength = (gcnew System::Windows::Forms::ColumnHeader());
            this->clhCount = (gcnew System::Windows::Forms::ColumnHeader());
            this->clhRcvTime = (gcnew System::Windows::Forms::ColumnHeader());
            this->clhData = (gcnew System::Windows::Forms::ColumnHeader());
            this->btnMsgClear = (gcnew System::Windows::Forms::Button());
            this->lbxInfo = (gcnew System::Windows::Forms::ListBox());
            this->btnInfoClear = (gcnew System::Windows::Forms::Button());
            this->groupBox5 = (gcnew System::Windows::Forms::GroupBox());
            this->rdbTimer = (gcnew System::Windows::Forms::RadioButton());
            this->btnRead = (gcnew System::Windows::Forms::Button());
            this->btnGetVersions = (gcnew System::Windows::Forms::Button());
            this->groupBox6 = (gcnew System::Windows::Forms::GroupBox());
            this->label5 = (gcnew System::Windows::Forms::Label());
            this->laLength = (gcnew System::Windows::Forms::Label());
            this->label3 = (gcnew System::Windows::Forms::Label());
            this->txtData60 = (gcnew System::Windows::Forms::TextBox());
            this->txtData62 = (gcnew System::Windows::Forms::TextBox());
            this->txtData47 = (gcnew System::Windows::Forms::TextBox());
            this->txtData58 = (gcnew System::Windows::Forms::TextBox());
            this->txtData57 = (gcnew System::Windows::Forms::TextBox());
            this->txtData56 = (gcnew System::Windows::Forms::TextBox());
            this->txtData55 = (gcnew System::Windows::Forms::TextBox());
            this->txtData54 = (gcnew System::Windows::Forms::TextBox());
            this->txtData61 = (gcnew System::Windows::Forms::TextBox());
            this->txtData63 = (gcnew System::Windows::Forms::TextBox());
            this->txtData48 = (gcnew System::Windows::Forms::TextBox());
            this->txtData53 = (gcnew System::Windows::Forms::TextBox());
            this->txtData52 = (gcnew System::Windows::Forms::TextBox());
            this->txtData51 = (gcnew System::Windows::Forms::TextBox());
            this->txtData50 = (gcnew System::Windows::Forms::TextBox());
            this->txtData49 = (gcnew System::Windows::Forms::TextBox());
            this->txtData59 = (gcnew System::Windows::Forms::TextBox());
            this->txtData46 = (gcnew System::Windows::Forms::TextBox());
            this->txtData45 = (gcnew System::Windows::Forms::TextBox());
            this->txtData44 = (gcnew System::Windows::Forms::TextBox());
            this->txtData43 = (gcnew System::Windows::Forms::TextBox());
            this->txtData42 = (gcnew System::Windows::Forms::TextBox());
            this->txtData41 = (gcnew System::Windows::Forms::TextBox());
            this->txtData40 = (gcnew System::Windows::Forms::TextBox());
            this->txtData39 = (gcnew System::Windows::Forms::TextBox());
            this->txtData38 = (gcnew System::Windows::Forms::TextBox());
            this->txtData37 = (gcnew System::Windows::Forms::TextBox());
            this->txtData36 = (gcnew System::Windows::Forms::TextBox());
            this->txtData35 = (gcnew System::Windows::Forms::TextBox());
            this->txtData34 = (gcnew System::Windows::Forms::TextBox());
            this->txtData33 = (gcnew System::Windows::Forms::TextBox());
            this->txtData32 = (gcnew System::Windows::Forms::TextBox());
            this->txtData31 = (gcnew System::Windows::Forms::TextBox());
            this->txtData30 = (gcnew System::Windows::Forms::TextBox());
            this->txtData29 = (gcnew System::Windows::Forms::TextBox());
            this->txtData28 = (gcnew System::Windows::Forms::TextBox());
            this->txtData27 = (gcnew System::Windows::Forms::TextBox());
            this->txtData26 = (gcnew System::Windows::Forms::TextBox());
            this->txtData25 = (gcnew System::Windows::Forms::TextBox());
            this->txtData24 = (gcnew System::Windows::Forms::TextBox());
            this->txtData23 = (gcnew System::Windows::Forms::TextBox());
            this->txtData22 = (gcnew System::Windows::Forms::TextBox());
            this->txtData21 = (gcnew System::Windows::Forms::TextBox());
            this->txtData20 = (gcnew System::Windows::Forms::TextBox());
            this->txtData19 = (gcnew System::Windows::Forms::TextBox());
            this->txtData18 = (gcnew System::Windows::Forms::TextBox());
            this->txtData17 = (gcnew System::Windows::Forms::TextBox());
            this->txtData16 = (gcnew System::Windows::Forms::TextBox());
            this->txtData15 = (gcnew System::Windows::Forms::TextBox());
            this->txtData14 = (gcnew System::Windows::Forms::TextBox());
            this->txtData13 = (gcnew System::Windows::Forms::TextBox());
            this->txtData12 = (gcnew System::Windows::Forms::TextBox());
            this->txtData11 = (gcnew System::Windows::Forms::TextBox());
            this->txtData10 = (gcnew System::Windows::Forms::TextBox());
            this->txtData9 = (gcnew System::Windows::Forms::TextBox());
            this->txtData8 = (gcnew System::Windows::Forms::TextBox());
            this->chbBRS = (gcnew System::Windows::Forms::CheckBox());
            this->chbFD = (gcnew System::Windows::Forms::CheckBox());
            this->chbRemote = (gcnew System::Windows::Forms::CheckBox());
            this->chbExtended = (gcnew System::Windows::Forms::CheckBox());
            this->btnWrite = (gcnew System::Windows::Forms::Button());
            this->label12 = (gcnew System::Windows::Forms::Label());
            this->label13 = (gcnew System::Windows::Forms::Label());
            this->txtID = (gcnew System::Windows::Forms::TextBox());
            this->txtData7 = (gcnew System::Windows::Forms::TextBox());
            this->txtData6 = (gcnew System::Windows::Forms::TextBox());
            this->txtData5 = (gcnew System::Windows::Forms::TextBox());
            this->txtData4 = (gcnew System::Windows::Forms::TextBox());
            this->txtData3 = (gcnew System::Windows::Forms::TextBox());
            this->txtData2 = (gcnew System::Windows::Forms::TextBox());
            this->txtData1 = (gcnew System::Windows::Forms::TextBox());
            this->txtData0 = (gcnew System::Windows::Forms::TextBox());
            this->nudLength = (gcnew System::Windows::Forms::NumericUpDown());
            this->groupBox4 = (gcnew System::Windows::Forms::GroupBox());
            this->btnReset = (gcnew System::Windows::Forms::Button());
            this->btnStatus = (gcnew System::Windows::Forms::Button());
            this->cbbChannel = (gcnew System::Windows::Forms::ComboBox());
            this->rdbParamInactive = (gcnew System::Windows::Forms::RadioButton());
            this->btnHwRefresh = (gcnew System::Windows::Forms::Button());
            this->cbbHwType = (gcnew System::Windows::Forms::ComboBox());
            this->cbbInterrupt = (gcnew System::Windows::Forms::ComboBox());
            this->btnFilterQuery = (gcnew System::Windows::Forms::Button());
            this->laInterrupt = (gcnew System::Windows::Forms::Label());
            this->groupBox3 = (gcnew System::Windows::Forms::GroupBox());
            this->chbFilterExt = (gcnew System::Windows::Forms::CheckBox());
            this->nudIdTo = (gcnew System::Windows::Forms::NumericUpDown());
            this->nudIdFrom = (gcnew System::Windows::Forms::NumericUpDown());
            this->label8 = (gcnew System::Windows::Forms::Label());
            this->label7 = (gcnew System::Windows::Forms::Label());
            this->rdbFilterOpen = (gcnew System::Windows::Forms::RadioButton());
            this->rdbFilterCustom = (gcnew System::Windows::Forms::RadioButton());
            this->rdbFilterClose = (gcnew System::Windows::Forms::RadioButton());
            this->btnFilterApply = (gcnew System::Windows::Forms::Button());
            this->cbbIO = (gcnew System::Windows::Forms::ComboBox());
            this->tmrRead = (gcnew System::Windows::Forms::Timer(this->components));
            this->laIOPort = (gcnew System::Windows::Forms::Label());
            this->laHwType = (gcnew System::Windows::Forms::Label());
            this->cbbBaudrates = (gcnew System::Windows::Forms::ComboBox());
            this->laBaudrate = (gcnew System::Windows::Forms::Label());
            this->label1 = (gcnew System::Windows::Forms::Label());
            this->btnInit = (gcnew System::Windows::Forms::Button());
            this->btnRelease = (gcnew System::Windows::Forms::Button());
            this->btnParameterGet = (gcnew System::Windows::Forms::Button());
            this->label10 = (gcnew System::Windows::Forms::Label());
            this->nudDeviceIdOrDelay = (gcnew System::Windows::Forms::NumericUpDown());
            this->laDeviceOrDelay = (gcnew System::Windows::Forms::Label());
            this->cbbParameter = (gcnew System::Windows::Forms::ComboBox());
            this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
            this->label6 = (gcnew System::Windows::Forms::Label());
            this->rdbParamActive = (gcnew System::Windows::Forms::RadioButton());
            this->btnParameterSet = (gcnew System::Windows::Forms::Button());
            this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
            this->chbCanFD = (gcnew System::Windows::Forms::CheckBox());
            this->txtBitrate = (gcnew System::Windows::Forms::TextBox());
            this->laBitrate = (gcnew System::Windows::Forms::Label());
            this->tmrDisplay = (gcnew System::Windows::Forms::Timer(this->components));
            this->groupBox5->SuspendLayout();
            this->groupBox6->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->nudLength))->BeginInit();
            this->groupBox4->SuspendLayout();
            this->groupBox3->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->nudIdTo))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->nudIdFrom))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->nudDeviceIdOrDelay))->BeginInit();
            this->groupBox2->SuspendLayout();
            this->groupBox1->SuspendLayout();
            this->SuspendLayout();
            // 
            // chbShowPeriod
            // 
            this->chbShowPeriod->AutoSize = true;
            this->chbShowPeriod->Checked = true;
            this->chbShowPeriod->CheckState = System::Windows::Forms::CheckState::Checked;
            this->chbShowPeriod->Location = System::Drawing::Point(374, 15);
            this->chbShowPeriod->Name = L"chbShowPeriod";
            this->chbShowPeriod->Size = System::Drawing::Size(123, 17);
            this->chbShowPeriod->TabIndex = 3;
            this->chbShowPeriod->Text = L"Timestamp as period";
            this->chbShowPeriod->UseVisualStyleBackColor = true;
            this->chbShowPeriod->CheckedChanged += gcnew System::EventHandler(this, &Form1::chbShowPeriod_CheckedChanged);
            // 
            // rdbManual
            // 
            this->rdbManual->AutoSize = true;
            this->rdbManual->Location = System::Drawing::Point(276, 14);
            this->rdbManual->Name = L"rdbManual";
            this->rdbManual->Size = System::Drawing::Size(89, 17);
            this->rdbManual->TabIndex = 2;
            this->rdbManual->Text = L"Manual Read";
            this->rdbManual->UseVisualStyleBackColor = true;
            this->rdbManual->CheckedChanged += gcnew System::EventHandler(this, &Form1::rdbTimer_CheckedChanged);
            // 
            // rdbEvent
            // 
            this->rdbEvent->AutoSize = true;
            this->rdbEvent->Location = System::Drawing::Point(131, 14);
            this->rdbEvent->Name = L"rdbEvent";
            this->rdbEvent->Size = System::Drawing::Size(139, 17);
            this->rdbEvent->TabIndex = 1;
            this->rdbEvent->Text = L"Reading using an Event";
            this->rdbEvent->UseVisualStyleBackColor = true;
            this->rdbEvent->CheckedChanged += gcnew System::EventHandler(this, &Form1::rdbTimer_CheckedChanged);
            // 
            // lstMessages
            // 
            this->lstMessages->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(6) {
                this->clhType, this->clhID,
                    this->clhLength, this->clhCount, this->clhRcvTime, this->clhData
            });
            this->lstMessages->FullRowSelect = true;
            this->lstMessages->HideSelection = false;
            this->lstMessages->Location = System::Drawing::Point(8, 37);
            this->lstMessages->MultiSelect = false;
            this->lstMessages->Name = L"lstMessages";
            this->lstMessages->Size = System::Drawing::Size(560, 96);
            this->lstMessages->TabIndex = 4;
            this->lstMessages->UseCompatibleStateImageBehavior = false;
            this->lstMessages->View = System::Windows::Forms::View::Details;
            this->lstMessages->DoubleClick += gcnew System::EventHandler(this, &Form1::lstMessages_DoubleClick);
            // 
            // clhType
            // 
            this->clhType->Text = L"Type";
            this->clhType->Width = 110;
            // 
            // clhID
            // 
            this->clhID->Text = L"ID";
            this->clhID->Width = 90;
            // 
            // clhLength
            // 
            this->clhLength->Text = L"Length";
            this->clhLength->Width = 50;
            // 
            // clhCount
            // 
            this->clhCount->Text = L"Count";
            this->clhCount->Width = 49;
            // 
            // clhRcvTime
            // 
            this->clhRcvTime->Text = L"Rcv Time";
            this->clhRcvTime->Width = 70;
            // 
            // clhData
            // 
            this->clhData->Text = L"Data";
            this->clhData->Width = 170;
            // 
            // btnMsgClear
            // 
            this->btnMsgClear->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->btnMsgClear->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->btnMsgClear->Location = System::Drawing::Point(639, 37);
            this->btnMsgClear->Name = L"btnMsgClear";
            this->btnMsgClear->Size = System::Drawing::Size(65, 23);
            this->btnMsgClear->TabIndex = 6;
            this->btnMsgClear->Text = L"Clear";
            this->btnMsgClear->UseVisualStyleBackColor = true;
            this->btnMsgClear->Click += gcnew System::EventHandler(this, &Form1::btnMsgClear_Click);
            // 
            // lbxInfo
            // 
            this->lbxInfo->FormattingEnabled = true;
            this->lbxInfo->Items->AddRange(gcnew cli::array< System::Object^  >(3) {
                L"Select a Hardware and a configuration for it. Then click \"Initialize\" button",
                    L"When activated, the Debug-Log file will be found in the same directory as this ap"
                    L"plication", L"When activated, the PCAN-Trace file will be found in the same directory as this a"
                    L"pplication"
            });
            this->lbxInfo->Location = System::Drawing::Point(10, 19);
            this->lbxInfo->Name = L"lbxInfo";
            this->lbxInfo->ScrollAlwaysVisible = true;
            this->lbxInfo->Size = System::Drawing::Size(558, 56);
            this->lbxInfo->TabIndex = 0;
            // 
            // btnInfoClear
            // 
            this->btnInfoClear->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->btnInfoClear->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->btnInfoClear->Location = System::Drawing::Point(639, 19);
            this->btnInfoClear->Name = L"btnInfoClear";
            this->btnInfoClear->Size = System::Drawing::Size(65, 23);
            this->btnInfoClear->TabIndex = 2;
            this->btnInfoClear->Text = L"Clear";
            this->btnInfoClear->UseVisualStyleBackColor = true;
            this->btnInfoClear->Click += gcnew System::EventHandler(this, &Form1::btnInfoClear_Click);
            // 
            // groupBox5
            // 
            this->groupBox5->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->groupBox5->Controls->Add(this->chbShowPeriod);
            this->groupBox5->Controls->Add(this->rdbManual);
            this->groupBox5->Controls->Add(this->rdbEvent);
            this->groupBox5->Controls->Add(this->lstMessages);
            this->groupBox5->Controls->Add(this->btnMsgClear);
            this->groupBox5->Controls->Add(this->rdbTimer);
            this->groupBox5->Controls->Add(this->btnRead);
            this->groupBox5->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->groupBox5->Location = System::Drawing::Point(12, 211);
            this->groupBox5->Name = L"groupBox5";
            this->groupBox5->Size = System::Drawing::Size(710, 140);
            this->groupBox5->TabIndex = 3;
            this->groupBox5->TabStop = false;
            this->groupBox5->Text = L" Messages Reading ";
            // 
            // rdbTimer
            // 
            this->rdbTimer->AutoSize = true;
            this->rdbTimer->Checked = true;
            this->rdbTimer->Location = System::Drawing::Point(8, 14);
            this->rdbTimer->Name = L"rdbTimer";
            this->rdbTimer->Size = System::Drawing::Size(117, 17);
            this->rdbTimer->TabIndex = 0;
            this->rdbTimer->TabStop = true;
            this->rdbTimer->Text = L"Read using a Timer";
            this->rdbTimer->UseVisualStyleBackColor = true;
            this->rdbTimer->CheckedChanged += gcnew System::EventHandler(this, &Form1::rdbTimer_CheckedChanged);
            // 
            // btnRead
            // 
            this->btnRead->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->btnRead->Enabled = false;
            this->btnRead->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->btnRead->Location = System::Drawing::Point(568, 37);
            this->btnRead->Name = L"btnRead";
            this->btnRead->Size = System::Drawing::Size(65, 23);
            this->btnRead->TabIndex = 5;
            this->btnRead->Text = L"Read";
            this->btnRead->UseVisualStyleBackColor = true;
            this->btnRead->Click += gcnew System::EventHandler(this, &Form1::btnRead_Click);
            // 
            // btnGetVersions
            // 
            this->btnGetVersions->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->btnGetVersions->Enabled = false;
            this->btnGetVersions->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->btnGetVersions->Location = System::Drawing::Point(568, 19);
            this->btnGetVersions->Name = L"btnGetVersions";
            this->btnGetVersions->Size = System::Drawing::Size(65, 23);
            this->btnGetVersions->TabIndex = 1;
            this->btnGetVersions->Text = L"Versions";
            this->btnGetVersions->UseVisualStyleBackColor = true;
            this->btnGetVersions->Click += gcnew System::EventHandler(this, &Form1::btnGetVersions_Click);
            // 
            // groupBox6
            // 
            this->groupBox6->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->groupBox6->Controls->Add(this->label5);
            this->groupBox6->Controls->Add(this->laLength);
            this->groupBox6->Controls->Add(this->label3);
            this->groupBox6->Controls->Add(this->txtData60);
            this->groupBox6->Controls->Add(this->txtData62);
            this->groupBox6->Controls->Add(this->txtData47);
            this->groupBox6->Controls->Add(this->txtData58);
            this->groupBox6->Controls->Add(this->txtData57);
            this->groupBox6->Controls->Add(this->txtData56);
            this->groupBox6->Controls->Add(this->txtData55);
            this->groupBox6->Controls->Add(this->txtData54);
            this->groupBox6->Controls->Add(this->txtData61);
            this->groupBox6->Controls->Add(this->txtData63);
            this->groupBox6->Controls->Add(this->txtData48);
            this->groupBox6->Controls->Add(this->txtData53);
            this->groupBox6->Controls->Add(this->txtData52);
            this->groupBox6->Controls->Add(this->txtData51);
            this->groupBox6->Controls->Add(this->txtData50);
            this->groupBox6->Controls->Add(this->txtData49);
            this->groupBox6->Controls->Add(this->txtData59);
            this->groupBox6->Controls->Add(this->txtData46);
            this->groupBox6->Controls->Add(this->txtData45);
            this->groupBox6->Controls->Add(this->txtData44);
            this->groupBox6->Controls->Add(this->txtData43);
            this->groupBox6->Controls->Add(this->txtData42);
            this->groupBox6->Controls->Add(this->txtData41);
            this->groupBox6->Controls->Add(this->txtData40);
            this->groupBox6->Controls->Add(this->txtData39);
            this->groupBox6->Controls->Add(this->txtData38);
            this->groupBox6->Controls->Add(this->txtData37);
            this->groupBox6->Controls->Add(this->txtData36);
            this->groupBox6->Controls->Add(this->txtData35);
            this->groupBox6->Controls->Add(this->txtData34);
            this->groupBox6->Controls->Add(this->txtData33);
            this->groupBox6->Controls->Add(this->txtData32);
            this->groupBox6->Controls->Add(this->txtData31);
            this->groupBox6->Controls->Add(this->txtData30);
            this->groupBox6->Controls->Add(this->txtData29);
            this->groupBox6->Controls->Add(this->txtData28);
            this->groupBox6->Controls->Add(this->txtData27);
            this->groupBox6->Controls->Add(this->txtData26);
            this->groupBox6->Controls->Add(this->txtData25);
            this->groupBox6->Controls->Add(this->txtData24);
            this->groupBox6->Controls->Add(this->txtData23);
            this->groupBox6->Controls->Add(this->txtData22);
            this->groupBox6->Controls->Add(this->txtData21);
            this->groupBox6->Controls->Add(this->txtData20);
            this->groupBox6->Controls->Add(this->txtData19);
            this->groupBox6->Controls->Add(this->txtData18);
            this->groupBox6->Controls->Add(this->txtData17);
            this->groupBox6->Controls->Add(this->txtData16);
            this->groupBox6->Controls->Add(this->txtData15);
            this->groupBox6->Controls->Add(this->txtData14);
            this->groupBox6->Controls->Add(this->txtData13);
            this->groupBox6->Controls->Add(this->txtData12);
            this->groupBox6->Controls->Add(this->txtData11);
            this->groupBox6->Controls->Add(this->txtData10);
            this->groupBox6->Controls->Add(this->txtData9);
            this->groupBox6->Controls->Add(this->txtData8);
            this->groupBox6->Controls->Add(this->chbBRS);
            this->groupBox6->Controls->Add(this->chbFD);
            this->groupBox6->Controls->Add(this->chbRemote);
            this->groupBox6->Controls->Add(this->chbExtended);
            this->groupBox6->Controls->Add(this->btnWrite);
            this->groupBox6->Controls->Add(this->label12);
            this->groupBox6->Controls->Add(this->label13);
            this->groupBox6->Controls->Add(this->txtID);
            this->groupBox6->Controls->Add(this->txtData7);
            this->groupBox6->Controls->Add(this->txtData6);
            this->groupBox6->Controls->Add(this->txtData5);
            this->groupBox6->Controls->Add(this->txtData4);
            this->groupBox6->Controls->Add(this->txtData3);
            this->groupBox6->Controls->Add(this->txtData2);
            this->groupBox6->Controls->Add(this->txtData1);
            this->groupBox6->Controls->Add(this->txtData0);
            this->groupBox6->Controls->Add(this->nudLength);
            this->groupBox6->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->groupBox6->Location = System::Drawing::Point(12, 357);
            this->groupBox6->Name = L"groupBox6";
            this->groupBox6->Size = System::Drawing::Size(710, 143);
            this->groupBox6->TabIndex = 4;
            this->groupBox6->TabStop = false;
            this->groupBox6->Text = L"Write Messages";
            // 
            // label5
            // 
            this->label5->AutoSize = true;
            this->label5->Location = System::Drawing::Point(664, 15);
            this->label5->Name = L"label5";
            this->label5->Size = System::Drawing::Size(43, 13);
            this->label5->TabIndex = 189;
            this->label5->Text = L"Length:";
            // 
            // laLength
            // 
            this->laLength->AutoSize = true;
            this->laLength->Location = System::Drawing::Point(671, 40);
            this->laLength->Name = L"laLength";
            this->laLength->Size = System::Drawing::Size(26, 13);
            this->laLength->TabIndex = 188;
            this->laLength->Text = L"8 B.";
            // 
            // label3
            // 
            this->label3->AutoSize = true;
            this->label3->Location = System::Drawing::Point(6, 15);
            this->label3->Name = L"label3";
            this->label3->Size = System::Drawing::Size(61, 13);
            this->label3->TabIndex = 187;
            this->label3->Text = L"Data (Hex):";
            // 
            // txtData60
            // 
            this->txtData60->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData60->Enabled = false;
            this->txtData60->Location = System::Drawing::Point(367, 115);
            this->txtData60->MaxLength = 2;
            this->txtData60->Name = L"txtData60";
            this->txtData60->Size = System::Drawing::Size(24, 20);
            this->txtData60->TabIndex = 183;
            this->txtData60->Text = L"00";
            this->txtData60->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData60->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData60->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData62
            // 
            this->txtData62->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData62->Enabled = false;
            this->txtData62->Location = System::Drawing::Point(427, 115);
            this->txtData62->MaxLength = 2;
            this->txtData62->Name = L"txtData62";
            this->txtData62->Size = System::Drawing::Size(24, 20);
            this->txtData62->TabIndex = 185;
            this->txtData62->Text = L"00";
            this->txtData62->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData62->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData62->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData47
            // 
            this->txtData47->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData47->Enabled = false;
            this->txtData47->Location = System::Drawing::Point(457, 89);
            this->txtData47->MaxLength = 2;
            this->txtData47->Name = L"txtData47";
            this->txtData47->Size = System::Drawing::Size(24, 20);
            this->txtData47->TabIndex = 170;
            this->txtData47->Text = L"00";
            this->txtData47->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData47->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData47->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData58
            // 
            this->txtData58->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData58->Enabled = false;
            this->txtData58->Location = System::Drawing::Point(307, 115);
            this->txtData58->MaxLength = 2;
            this->txtData58->Name = L"txtData58";
            this->txtData58->Size = System::Drawing::Size(24, 20);
            this->txtData58->TabIndex = 181;
            this->txtData58->Text = L"00";
            this->txtData58->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData58->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData58->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData57
            // 
            this->txtData57->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData57->Enabled = false;
            this->txtData57->Location = System::Drawing::Point(277, 115);
            this->txtData57->MaxLength = 2;
            this->txtData57->Name = L"txtData57";
            this->txtData57->Size = System::Drawing::Size(24, 20);
            this->txtData57->TabIndex = 180;
            this->txtData57->Text = L"00";
            this->txtData57->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData57->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData57->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData56
            // 
            this->txtData56->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData56->Enabled = false;
            this->txtData56->Location = System::Drawing::Point(247, 115);
            this->txtData56->MaxLength = 2;
            this->txtData56->Name = L"txtData56";
            this->txtData56->Size = System::Drawing::Size(24, 20);
            this->txtData56->TabIndex = 179;
            this->txtData56->Text = L"00";
            this->txtData56->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData56->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData56->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData55
            // 
            this->txtData55->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData55->Enabled = false;
            this->txtData55->Location = System::Drawing::Point(217, 115);
            this->txtData55->MaxLength = 2;
            this->txtData55->Name = L"txtData55";
            this->txtData55->Size = System::Drawing::Size(24, 20);
            this->txtData55->TabIndex = 178;
            this->txtData55->Text = L"00";
            this->txtData55->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData55->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData55->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData54
            // 
            this->txtData54->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData54->Enabled = false;
            this->txtData54->Location = System::Drawing::Point(187, 115);
            this->txtData54->MaxLength = 2;
            this->txtData54->Name = L"txtData54";
            this->txtData54->Size = System::Drawing::Size(24, 20);
            this->txtData54->TabIndex = 177;
            this->txtData54->Text = L"00";
            this->txtData54->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData54->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData54->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData61
            // 
            this->txtData61->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData61->Enabled = false;
            this->txtData61->Location = System::Drawing::Point(397, 115);
            this->txtData61->MaxLength = 2;
            this->txtData61->Name = L"txtData61";
            this->txtData61->Size = System::Drawing::Size(24, 20);
            this->txtData61->TabIndex = 184;
            this->txtData61->Text = L"00";
            this->txtData61->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData61->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData61->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData63
            // 
            this->txtData63->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData63->Enabled = false;
            this->txtData63->Location = System::Drawing::Point(457, 115);
            this->txtData63->MaxLength = 2;
            this->txtData63->Name = L"txtData63";
            this->txtData63->Size = System::Drawing::Size(24, 20);
            this->txtData63->TabIndex = 186;
            this->txtData63->Text = L"00";
            this->txtData63->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData63->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData63->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData48
            // 
            this->txtData48->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData48->Enabled = false;
            this->txtData48->Location = System::Drawing::Point(7, 115);
            this->txtData48->MaxLength = 2;
            this->txtData48->Name = L"txtData48";
            this->txtData48->Size = System::Drawing::Size(24, 20);
            this->txtData48->TabIndex = 171;
            this->txtData48->Text = L"00";
            this->txtData48->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData48->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData48->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData53
            // 
            this->txtData53->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData53->Enabled = false;
            this->txtData53->Location = System::Drawing::Point(157, 115);
            this->txtData53->MaxLength = 2;
            this->txtData53->Name = L"txtData53";
            this->txtData53->Size = System::Drawing::Size(24, 20);
            this->txtData53->TabIndex = 176;
            this->txtData53->Text = L"00";
            this->txtData53->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData53->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData53->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData52
            // 
            this->txtData52->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData52->Enabled = false;
            this->txtData52->Location = System::Drawing::Point(127, 115);
            this->txtData52->MaxLength = 2;
            this->txtData52->Name = L"txtData52";
            this->txtData52->Size = System::Drawing::Size(24, 20);
            this->txtData52->TabIndex = 175;
            this->txtData52->Text = L"00";
            this->txtData52->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData52->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData52->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData51
            // 
            this->txtData51->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData51->Enabled = false;
            this->txtData51->Location = System::Drawing::Point(97, 115);
            this->txtData51->MaxLength = 2;
            this->txtData51->Name = L"txtData51";
            this->txtData51->Size = System::Drawing::Size(24, 20);
            this->txtData51->TabIndex = 174;
            this->txtData51->Text = L"00";
            this->txtData51->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData51->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData51->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData50
            // 
            this->txtData50->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData50->Enabled = false;
            this->txtData50->Location = System::Drawing::Point(67, 115);
            this->txtData50->MaxLength = 2;
            this->txtData50->Name = L"txtData50";
            this->txtData50->Size = System::Drawing::Size(24, 20);
            this->txtData50->TabIndex = 173;
            this->txtData50->Text = L"00";
            this->txtData50->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData50->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData50->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData49
            // 
            this->txtData49->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData49->Enabled = false;
            this->txtData49->Location = System::Drawing::Point(37, 115);
            this->txtData49->MaxLength = 2;
            this->txtData49->Name = L"txtData49";
            this->txtData49->Size = System::Drawing::Size(24, 20);
            this->txtData49->TabIndex = 172;
            this->txtData49->Text = L"00";
            this->txtData49->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData49->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData49->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData59
            // 
            this->txtData59->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData59->Enabled = false;
            this->txtData59->Location = System::Drawing::Point(337, 115);
            this->txtData59->MaxLength = 2;
            this->txtData59->Name = L"txtData59";
            this->txtData59->Size = System::Drawing::Size(24, 20);
            this->txtData59->TabIndex = 182;
            this->txtData59->Text = L"00";
            this->txtData59->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData59->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData59->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData46
            // 
            this->txtData46->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData46->Enabled = false;
            this->txtData46->Location = System::Drawing::Point(427, 89);
            this->txtData46->MaxLength = 2;
            this->txtData46->Name = L"txtData46";
            this->txtData46->Size = System::Drawing::Size(24, 20);
            this->txtData46->TabIndex = 169;
            this->txtData46->Text = L"00";
            this->txtData46->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData46->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData46->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData45
            // 
            this->txtData45->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData45->Enabled = false;
            this->txtData45->Location = System::Drawing::Point(397, 89);
            this->txtData45->MaxLength = 2;
            this->txtData45->Name = L"txtData45";
            this->txtData45->Size = System::Drawing::Size(24, 20);
            this->txtData45->TabIndex = 168;
            this->txtData45->Text = L"00";
            this->txtData45->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData45->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData45->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData44
            // 
            this->txtData44->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData44->Enabled = false;
            this->txtData44->Location = System::Drawing::Point(367, 89);
            this->txtData44->MaxLength = 2;
            this->txtData44->Name = L"txtData44";
            this->txtData44->Size = System::Drawing::Size(24, 20);
            this->txtData44->TabIndex = 167;
            this->txtData44->Text = L"00";
            this->txtData44->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData44->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData44->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData43
            // 
            this->txtData43->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData43->Enabled = false;
            this->txtData43->Location = System::Drawing::Point(337, 89);
            this->txtData43->MaxLength = 2;
            this->txtData43->Name = L"txtData43";
            this->txtData43->Size = System::Drawing::Size(24, 20);
            this->txtData43->TabIndex = 166;
            this->txtData43->Text = L"00";
            this->txtData43->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData43->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData43->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData42
            // 
            this->txtData42->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData42->Enabled = false;
            this->txtData42->Location = System::Drawing::Point(307, 89);
            this->txtData42->MaxLength = 2;
            this->txtData42->Name = L"txtData42";
            this->txtData42->Size = System::Drawing::Size(24, 20);
            this->txtData42->TabIndex = 165;
            this->txtData42->Text = L"00";
            this->txtData42->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData42->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData42->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData41
            // 
            this->txtData41->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData41->Enabled = false;
            this->txtData41->Location = System::Drawing::Point(277, 89);
            this->txtData41->MaxLength = 2;
            this->txtData41->Name = L"txtData41";
            this->txtData41->Size = System::Drawing::Size(24, 20);
            this->txtData41->TabIndex = 164;
            this->txtData41->Text = L"00";
            this->txtData41->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData41->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData41->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData40
            // 
            this->txtData40->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData40->Enabled = false;
            this->txtData40->Location = System::Drawing::Point(247, 89);
            this->txtData40->MaxLength = 2;
            this->txtData40->Name = L"txtData40";
            this->txtData40->Size = System::Drawing::Size(24, 20);
            this->txtData40->TabIndex = 163;
            this->txtData40->Text = L"00";
            this->txtData40->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData40->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData40->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData39
            // 
            this->txtData39->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData39->Enabled = false;
            this->txtData39->Location = System::Drawing::Point(217, 89);
            this->txtData39->MaxLength = 2;
            this->txtData39->Name = L"txtData39";
            this->txtData39->Size = System::Drawing::Size(24, 20);
            this->txtData39->TabIndex = 162;
            this->txtData39->Text = L"00";
            this->txtData39->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData39->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData39->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData38
            // 
            this->txtData38->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData38->Enabled = false;
            this->txtData38->Location = System::Drawing::Point(187, 89);
            this->txtData38->MaxLength = 2;
            this->txtData38->Name = L"txtData38";
            this->txtData38->Size = System::Drawing::Size(24, 20);
            this->txtData38->TabIndex = 161;
            this->txtData38->Text = L"00";
            this->txtData38->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData38->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData38->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData37
            // 
            this->txtData37->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData37->Enabled = false;
            this->txtData37->Location = System::Drawing::Point(157, 89);
            this->txtData37->MaxLength = 2;
            this->txtData37->Name = L"txtData37";
            this->txtData37->Size = System::Drawing::Size(24, 20);
            this->txtData37->TabIndex = 160;
            this->txtData37->Text = L"00";
            this->txtData37->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData37->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData37->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData36
            // 
            this->txtData36->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData36->Enabled = false;
            this->txtData36->Location = System::Drawing::Point(127, 89);
            this->txtData36->MaxLength = 2;
            this->txtData36->Name = L"txtData36";
            this->txtData36->Size = System::Drawing::Size(24, 20);
            this->txtData36->TabIndex = 159;
            this->txtData36->Text = L"00";
            this->txtData36->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData36->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData36->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData35
            // 
            this->txtData35->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData35->Enabled = false;
            this->txtData35->Location = System::Drawing::Point(97, 89);
            this->txtData35->MaxLength = 2;
            this->txtData35->Name = L"txtData35";
            this->txtData35->Size = System::Drawing::Size(24, 20);
            this->txtData35->TabIndex = 158;
            this->txtData35->Text = L"00";
            this->txtData35->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData35->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData35->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData34
            // 
            this->txtData34->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData34->Enabled = false;
            this->txtData34->Location = System::Drawing::Point(67, 89);
            this->txtData34->MaxLength = 2;
            this->txtData34->Name = L"txtData34";
            this->txtData34->Size = System::Drawing::Size(24, 20);
            this->txtData34->TabIndex = 157;
            this->txtData34->Text = L"00";
            this->txtData34->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData34->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData34->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData33
            // 
            this->txtData33->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData33->Enabled = false;
            this->txtData33->Location = System::Drawing::Point(37, 89);
            this->txtData33->MaxLength = 2;
            this->txtData33->Name = L"txtData33";
            this->txtData33->Size = System::Drawing::Size(24, 20);
            this->txtData33->TabIndex = 156;
            this->txtData33->Text = L"00";
            this->txtData33->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData33->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData33->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData32
            // 
            this->txtData32->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData32->Enabled = false;
            this->txtData32->Location = System::Drawing::Point(7, 89);
            this->txtData32->MaxLength = 2;
            this->txtData32->Name = L"txtData32";
            this->txtData32->Size = System::Drawing::Size(24, 20);
            this->txtData32->TabIndex = 155;
            this->txtData32->Text = L"00";
            this->txtData32->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData32->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData32->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData31
            // 
            this->txtData31->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData31->Enabled = false;
            this->txtData31->Location = System::Drawing::Point(457, 63);
            this->txtData31->MaxLength = 2;
            this->txtData31->Name = L"txtData31";
            this->txtData31->Size = System::Drawing::Size(24, 20);
            this->txtData31->TabIndex = 154;
            this->txtData31->Text = L"00";
            this->txtData31->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData31->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData31->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData30
            // 
            this->txtData30->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData30->Enabled = false;
            this->txtData30->Location = System::Drawing::Point(427, 63);
            this->txtData30->MaxLength = 2;
            this->txtData30->Name = L"txtData30";
            this->txtData30->Size = System::Drawing::Size(24, 20);
            this->txtData30->TabIndex = 153;
            this->txtData30->Text = L"00";
            this->txtData30->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData30->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData30->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData29
            // 
            this->txtData29->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData29->Enabled = false;
            this->txtData29->Location = System::Drawing::Point(397, 63);
            this->txtData29->MaxLength = 2;
            this->txtData29->Name = L"txtData29";
            this->txtData29->Size = System::Drawing::Size(24, 20);
            this->txtData29->TabIndex = 152;
            this->txtData29->Text = L"00";
            this->txtData29->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData29->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData29->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData28
            // 
            this->txtData28->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData28->Enabled = false;
            this->txtData28->Location = System::Drawing::Point(367, 63);
            this->txtData28->MaxLength = 2;
            this->txtData28->Name = L"txtData28";
            this->txtData28->Size = System::Drawing::Size(24, 20);
            this->txtData28->TabIndex = 151;
            this->txtData28->Text = L"00";
            this->txtData28->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData28->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData28->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData27
            // 
            this->txtData27->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData27->Enabled = false;
            this->txtData27->Location = System::Drawing::Point(337, 63);
            this->txtData27->MaxLength = 2;
            this->txtData27->Name = L"txtData27";
            this->txtData27->Size = System::Drawing::Size(24, 20);
            this->txtData27->TabIndex = 150;
            this->txtData27->Text = L"00";
            this->txtData27->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData27->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData27->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData26
            // 
            this->txtData26->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData26->Enabled = false;
            this->txtData26->Location = System::Drawing::Point(307, 63);
            this->txtData26->MaxLength = 2;
            this->txtData26->Name = L"txtData26";
            this->txtData26->Size = System::Drawing::Size(24, 20);
            this->txtData26->TabIndex = 149;
            this->txtData26->Text = L"00";
            this->txtData26->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData26->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData26->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData25
            // 
            this->txtData25->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData25->Enabled = false;
            this->txtData25->Location = System::Drawing::Point(277, 63);
            this->txtData25->MaxLength = 2;
            this->txtData25->Name = L"txtData25";
            this->txtData25->Size = System::Drawing::Size(24, 20);
            this->txtData25->TabIndex = 147;
            this->txtData25->Text = L"00";
            this->txtData25->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData25->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData25->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData24
            // 
            this->txtData24->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData24->Enabled = false;
            this->txtData24->Location = System::Drawing::Point(247, 63);
            this->txtData24->MaxLength = 2;
            this->txtData24->Name = L"txtData24";
            this->txtData24->Size = System::Drawing::Size(24, 20);
            this->txtData24->TabIndex = 145;
            this->txtData24->Text = L"00";
            this->txtData24->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData24->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData24->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData23
            // 
            this->txtData23->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData23->Enabled = false;
            this->txtData23->Location = System::Drawing::Point(217, 63);
            this->txtData23->MaxLength = 2;
            this->txtData23->Name = L"txtData23";
            this->txtData23->Size = System::Drawing::Size(24, 20);
            this->txtData23->TabIndex = 144;
            this->txtData23->Text = L"00";
            this->txtData23->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData23->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData23->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData22
            // 
            this->txtData22->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData22->Enabled = false;
            this->txtData22->Location = System::Drawing::Point(187, 63);
            this->txtData22->MaxLength = 2;
            this->txtData22->Name = L"txtData22";
            this->txtData22->Size = System::Drawing::Size(24, 20);
            this->txtData22->TabIndex = 143;
            this->txtData22->Text = L"00";
            this->txtData22->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData22->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData22->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData21
            // 
            this->txtData21->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData21->Enabled = false;
            this->txtData21->Location = System::Drawing::Point(157, 63);
            this->txtData21->MaxLength = 2;
            this->txtData21->Name = L"txtData21";
            this->txtData21->Size = System::Drawing::Size(24, 20);
            this->txtData21->TabIndex = 142;
            this->txtData21->Text = L"00";
            this->txtData21->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData21->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData21->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData20
            // 
            this->txtData20->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData20->Enabled = false;
            this->txtData20->Location = System::Drawing::Point(127, 63);
            this->txtData20->MaxLength = 2;
            this->txtData20->Name = L"txtData20";
            this->txtData20->Size = System::Drawing::Size(24, 20);
            this->txtData20->TabIndex = 141;
            this->txtData20->Text = L"00";
            this->txtData20->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData20->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData20->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData19
            // 
            this->txtData19->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData19->Enabled = false;
            this->txtData19->Location = System::Drawing::Point(97, 63);
            this->txtData19->MaxLength = 2;
            this->txtData19->Name = L"txtData19";
            this->txtData19->Size = System::Drawing::Size(24, 20);
            this->txtData19->TabIndex = 140;
            this->txtData19->Text = L"00";
            this->txtData19->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData19->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData19->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData18
            // 
            this->txtData18->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData18->Enabled = false;
            this->txtData18->Location = System::Drawing::Point(68, 63);
            this->txtData18->MaxLength = 2;
            this->txtData18->Name = L"txtData18";
            this->txtData18->Size = System::Drawing::Size(24, 20);
            this->txtData18->TabIndex = 139;
            this->txtData18->Text = L"00";
            this->txtData18->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData18->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData18->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData17
            // 
            this->txtData17->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData17->Enabled = false;
            this->txtData17->Location = System::Drawing::Point(38, 63);
            this->txtData17->MaxLength = 2;
            this->txtData17->Name = L"txtData17";
            this->txtData17->Size = System::Drawing::Size(24, 20);
            this->txtData17->TabIndex = 138;
            this->txtData17->Text = L"00";
            this->txtData17->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData17->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData17->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData16
            // 
            this->txtData16->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData16->Enabled = false;
            this->txtData16->Location = System::Drawing::Point(7, 63);
            this->txtData16->MaxLength = 2;
            this->txtData16->Name = L"txtData16";
            this->txtData16->Size = System::Drawing::Size(24, 20);
            this->txtData16->TabIndex = 137;
            this->txtData16->Text = L"00";
            this->txtData16->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData16->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData16->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData15
            // 
            this->txtData15->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData15->Enabled = false;
            this->txtData15->Location = System::Drawing::Point(457, 37);
            this->txtData15->MaxLength = 2;
            this->txtData15->Name = L"txtData15";
            this->txtData15->Size = System::Drawing::Size(24, 20);
            this->txtData15->TabIndex = 136;
            this->txtData15->Text = L"00";
            this->txtData15->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData15->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData15->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData14
            // 
            this->txtData14->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData14->Enabled = false;
            this->txtData14->Location = System::Drawing::Point(427, 37);
            this->txtData14->MaxLength = 2;
            this->txtData14->Name = L"txtData14";
            this->txtData14->Size = System::Drawing::Size(24, 20);
            this->txtData14->TabIndex = 135;
            this->txtData14->Text = L"00";
            this->txtData14->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData14->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData14->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData13
            // 
            this->txtData13->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData13->Enabled = false;
            this->txtData13->Location = System::Drawing::Point(397, 37);
            this->txtData13->MaxLength = 2;
            this->txtData13->Name = L"txtData13";
            this->txtData13->Size = System::Drawing::Size(24, 20);
            this->txtData13->TabIndex = 134;
            this->txtData13->Text = L"00";
            this->txtData13->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData13->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData13->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData12
            // 
            this->txtData12->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData12->Enabled = false;
            this->txtData12->Location = System::Drawing::Point(367, 37);
            this->txtData12->MaxLength = 2;
            this->txtData12->Name = L"txtData12";
            this->txtData12->Size = System::Drawing::Size(24, 20);
            this->txtData12->TabIndex = 133;
            this->txtData12->Text = L"00";
            this->txtData12->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData12->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData12->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData11
            // 
            this->txtData11->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData11->Enabled = false;
            this->txtData11->Location = System::Drawing::Point(337, 37);
            this->txtData11->MaxLength = 2;
            this->txtData11->Name = L"txtData11";
            this->txtData11->Size = System::Drawing::Size(24, 20);
            this->txtData11->TabIndex = 132;
            this->txtData11->Text = L"00";
            this->txtData11->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData11->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData11->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData10
            // 
            this->txtData10->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData10->Enabled = false;
            this->txtData10->Location = System::Drawing::Point(307, 37);
            this->txtData10->MaxLength = 2;
            this->txtData10->Name = L"txtData10";
            this->txtData10->Size = System::Drawing::Size(24, 20);
            this->txtData10->TabIndex = 131;
            this->txtData10->Text = L"00";
            this->txtData10->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData10->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData10->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData9
            // 
            this->txtData9->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData9->Enabled = false;
            this->txtData9->Location = System::Drawing::Point(277, 37);
            this->txtData9->MaxLength = 2;
            this->txtData9->Name = L"txtData9";
            this->txtData9->Size = System::Drawing::Size(24, 20);
            this->txtData9->TabIndex = 130;
            this->txtData9->Text = L"00";
            this->txtData9->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData9->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData9->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData8
            // 
            this->txtData8->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData8->Enabled = false;
            this->txtData8->Location = System::Drawing::Point(247, 37);
            this->txtData8->MaxLength = 2;
            this->txtData8->Name = L"txtData8";
            this->txtData8->Size = System::Drawing::Size(24, 20);
            this->txtData8->TabIndex = 129;
            this->txtData8->Text = L"00";
            this->txtData8->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData8->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData8->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // chbBRS
            // 
            this->chbBRS->Cursor = System::Windows::Forms::Cursors::Default;
            this->chbBRS->Enabled = false;
            this->chbBRS->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->chbBRS->Location = System::Drawing::Point(567, 85);
            this->chbBRS->Name = L"chbBRS";
            this->chbBRS->Size = System::Drawing::Size(40, 24);
            this->chbBRS->TabIndex = 119;
            this->chbBRS->Text = L"BRS";
            this->chbBRS->Visible = false;
            // 
            // chbFD
            // 
            this->chbFD->Cursor = System::Windows::Forms::Cursors::Default;
            this->chbFD->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->chbFD->Location = System::Drawing::Point(499, 85);
            this->chbFD->Name = L"chbFD";
            this->chbFD->Size = System::Drawing::Size(37, 24);
            this->chbFD->TabIndex = 118;
            this->chbFD->Text = L"FD";
            this->chbFD->Visible = false;
            this->chbFD->CheckedChanged += gcnew System::EventHandler(this, &Form1::chbFD_CheckedChanged);
            // 
            // chbRemote
            // 
            this->chbRemote->Cursor = System::Windows::Forms::Cursors::Default;
            this->chbRemote->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->chbRemote->Location = System::Drawing::Point(567, 63);
            this->chbRemote->Name = L"chbRemote";
            this->chbRemote->Size = System::Drawing::Size(44, 24);
            this->chbRemote->TabIndex = 117;
            this->chbRemote->Text = L"RTR";
            this->chbRemote->CheckedChanged += gcnew System::EventHandler(this, &Form1::chbRemote_CheckedChanged);
            // 
            // chbExtended
            // 
            this->chbExtended->Cursor = System::Windows::Forms::Cursors::Default;
            this->chbExtended->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->chbExtended->Location = System::Drawing::Point(499, 63);
            this->chbExtended->Name = L"chbExtended";
            this->chbExtended->Size = System::Drawing::Size(62, 24);
            this->chbExtended->TabIndex = 116;
            this->chbExtended->Text = L"Extended";
            this->chbExtended->CheckedChanged += gcnew System::EventHandler(this, &Form1::chbExtended_CheckedChanged);
            // 
            // btnWrite
            // 
            this->btnWrite->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->btnWrite->Cursor = System::Windows::Forms::Cursors::Default;
            this->btnWrite->Enabled = false;
            this->btnWrite->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->btnWrite->Location = System::Drawing::Point(639, 85);
            this->btnWrite->Name = L"btnWrite";
            this->btnWrite->Size = System::Drawing::Size(65, 23);
            this->btnWrite->TabIndex = 121;
            this->btnWrite->Text = L"Write";
            this->btnWrite->Click += gcnew System::EventHandler(this, &Form1::btnWrite_Click);
            // 
            // label12
            // 
            this->label12->AutoSize = true;
            this->label12->Location = System::Drawing::Point(614, 15);
            this->label12->Name = L"label12";
            this->label12->Size = System::Drawing::Size(31, 13);
            this->label12->TabIndex = 148;
            this->label12->Text = L"DLC:";
            // 
            // label13
            // 
            this->label13->AutoSize = true;
            this->label13->Location = System::Drawing::Point(496, 15);
            this->label13->Name = L"label13";
            this->label13->Size = System::Drawing::Size(49, 13);
            this->label13->TabIndex = 146;
            this->label13->Text = L"ID (Hex):";
            // 
            // txtID
            // 
            this->txtID->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtID->Location = System::Drawing::Point(499, 37);
            this->txtID->MaxLength = 3;
            this->txtID->Name = L"txtID";
            this->txtID->Size = System::Drawing::Size(112, 20);
            this->txtID->TabIndex = 114;
            this->txtID->Text = L"0";
            this->txtID->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtID->Leave += gcnew System::EventHandler(this, &Form1::txtID_Leave);
            // 
            // txtData7
            // 
            this->txtData7->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData7->Location = System::Drawing::Point(217, 37);
            this->txtData7->MaxLength = 2;
            this->txtData7->Name = L"txtData7";
            this->txtData7->Size = System::Drawing::Size(24, 20);
            this->txtData7->TabIndex = 128;
            this->txtData7->Text = L"00";
            this->txtData7->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData7->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData7->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData6
            // 
            this->txtData6->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData6->Location = System::Drawing::Point(187, 37);
            this->txtData6->MaxLength = 2;
            this->txtData6->Name = L"txtData6";
            this->txtData6->Size = System::Drawing::Size(24, 20);
            this->txtData6->TabIndex = 127;
            this->txtData6->Text = L"00";
            this->txtData6->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData6->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData6->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData5
            // 
            this->txtData5->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData5->Location = System::Drawing::Point(157, 37);
            this->txtData5->MaxLength = 2;
            this->txtData5->Name = L"txtData5";
            this->txtData5->Size = System::Drawing::Size(24, 20);
            this->txtData5->TabIndex = 126;
            this->txtData5->Text = L"00";
            this->txtData5->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData5->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData5->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData4
            // 
            this->txtData4->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData4->Location = System::Drawing::Point(127, 37);
            this->txtData4->MaxLength = 2;
            this->txtData4->Name = L"txtData4";
            this->txtData4->Size = System::Drawing::Size(24, 20);
            this->txtData4->TabIndex = 125;
            this->txtData4->Text = L"00";
            this->txtData4->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData4->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData4->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData3
            // 
            this->txtData3->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData3->Location = System::Drawing::Point(97, 37);
            this->txtData3->MaxLength = 2;
            this->txtData3->Name = L"txtData3";
            this->txtData3->Size = System::Drawing::Size(24, 20);
            this->txtData3->TabIndex = 124;
            this->txtData3->Text = L"00";
            this->txtData3->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData3->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData3->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData2
            // 
            this->txtData2->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData2->Location = System::Drawing::Point(68, 37);
            this->txtData2->MaxLength = 2;
            this->txtData2->Name = L"txtData2";
            this->txtData2->Size = System::Drawing::Size(24, 20);
            this->txtData2->TabIndex = 123;
            this->txtData2->Text = L"00";
            this->txtData2->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData2->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData2->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData1
            // 
            this->txtData1->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData1->Location = System::Drawing::Point(38, 37);
            this->txtData1->MaxLength = 2;
            this->txtData1->Name = L"txtData1";
            this->txtData1->Size = System::Drawing::Size(24, 20);
            this->txtData1->TabIndex = 122;
            this->txtData1->Text = L"00";
            this->txtData1->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData1->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData1->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // txtData0
            // 
            this->txtData0->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->txtData0->Location = System::Drawing::Point(7, 37);
            this->txtData0->MaxLength = 2;
            this->txtData0->Name = L"txtData0";
            this->txtData0->Size = System::Drawing::Size(24, 20);
            this->txtData0->TabIndex = 120;
            this->txtData0->Text = L"00";
            this->txtData0->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->txtData0->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::txtID_KeyPress);
            this->txtData0->Leave += gcnew System::EventHandler(this, &Form1::txtData0_Leave);
            // 
            // nudLength
            // 
            this->nudLength->BackColor = System::Drawing::Color::White;
            this->nudLength->Location = System::Drawing::Point(617, 37);
            this->nudLength->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 8, 0, 0, 0 });
            this->nudLength->Name = L"nudLength";
            this->nudLength->ReadOnly = true;
            this->nudLength->Size = System::Drawing::Size(41, 20);
            this->nudLength->TabIndex = 115;
            this->nudLength->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 8, 0, 0, 0 });
            this->nudLength->ValueChanged += gcnew System::EventHandler(this, &Form1::nudLength_ValueChanged);
            // 
            // groupBox4
            // 
            this->groupBox4->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->groupBox4->Controls->Add(this->btnReset);
            this->groupBox4->Controls->Add(this->btnStatus);
            this->groupBox4->Controls->Add(this->btnGetVersions);
            this->groupBox4->Controls->Add(this->lbxInfo);
            this->groupBox4->Controls->Add(this->btnInfoClear);
            this->groupBox4->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->groupBox4->Location = System::Drawing::Point(12, 506);
            this->groupBox4->Name = L"groupBox4";
            this->groupBox4->Size = System::Drawing::Size(710, 87);
            this->groupBox4->TabIndex = 5;
            this->groupBox4->TabStop = false;
            this->groupBox4->Text = L"Information";
            // 
            // btnReset
            // 
            this->btnReset->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->btnReset->Enabled = false;
            this->btnReset->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->btnReset->Location = System::Drawing::Point(639, 48);
            this->btnReset->Name = L"btnReset";
            this->btnReset->Size = System::Drawing::Size(65, 23);
            this->btnReset->TabIndex = 4;
            this->btnReset->Text = L"Reset";
            this->btnReset->UseVisualStyleBackColor = true;
            this->btnReset->Click += gcnew System::EventHandler(this, &Form1::btnReset_Click);
            // 
            // btnStatus
            // 
            this->btnStatus->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->btnStatus->Enabled = false;
            this->btnStatus->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->btnStatus->Location = System::Drawing::Point(568, 48);
            this->btnStatus->Name = L"btnStatus";
            this->btnStatus->Size = System::Drawing::Size(65, 23);
            this->btnStatus->TabIndex = 3;
            this->btnStatus->Text = L"Status";
            this->btnStatus->UseVisualStyleBackColor = true;
            this->btnStatus->Click += gcnew System::EventHandler(this, &Form1::buttonStatus_Click);
            // 
            // cbbChannel
            // 
            this->cbbChannel->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->cbbChannel->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->cbbChannel->Items->AddRange(gcnew cli::array< System::Object^  >(28) {
                L"None", L"DNG-Channel 1", L"ISA-Channel 1", L"ISA-Channel 2",
                    L"ISA-Channel 3", L"ISA-Channel 4", L"ISA-Channel 5", L"ISA-Channel 6", L"ISA-Channel 7", L"ISA-Channel 8", L"PCC-Channel 1",
                    L"PCC-Channel 2", L"PCI-Channel 1", L"PCI-Channel 2", L"PCI-Channel 3", L"PCI-Channel 4", L"PCI-Channel 5", L"PCI-Channel 6",
                    L"PCI-Channel 7", L"PCI-Channel 8", L"USB-Channel 1", L"USB-Channel 2", L"USB-Channel 3", L"USB-Channel 4", L"USB-Channel 5",
                    L"USB-Channel 6", L"USB-Channel 7", L"USB-Channel 8"
            });
            this->cbbChannel->Location = System::Drawing::Point(8, 31);
            this->cbbChannel->Name = L"cbbChannel";
            this->cbbChannel->Size = System::Drawing::Size(127, 21);
            this->cbbChannel->TabIndex = 0;
            this->cbbChannel->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cbbChannel_SelectedIndexChanged);
            // 
            // rdbParamInactive
            // 
            this->rdbParamInactive->Location = System::Drawing::Point(300, 32);
            this->rdbParamInactive->Name = L"rdbParamInactive";
            this->rdbParamInactive->Size = System::Drawing::Size(67, 17);
            this->rdbParamInactive->TabIndex = 2;
            this->rdbParamInactive->Text = L"Inactive";
            this->rdbParamInactive->UseVisualStyleBackColor = true;
            // 
            // btnHwRefresh
            // 
            this->btnHwRefresh->Cursor = System::Windows::Forms::Cursors::Default;
            this->btnHwRefresh->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->btnHwRefresh->Location = System::Drawing::Point(141, 31);
            this->btnHwRefresh->Name = L"btnHwRefresh";
            this->btnHwRefresh->Size = System::Drawing::Size(57, 23);
            this->btnHwRefresh->TabIndex = 1;
            this->btnHwRefresh->Text = L"Refresh";
            this->btnHwRefresh->Click += gcnew System::EventHandler(this, &Form1::btnHwRefresh_Click);
            // 
            // cbbHwType
            // 
            this->cbbHwType->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->cbbHwType->Items->AddRange(gcnew cli::array< System::Object^  >(7) {
                L"ISA-82C200", L"ISA-SJA1000", L"ISA-PHYTEC", L"DNG-82C200",
                    L"DNG-82C200 EPP", L"DNG-SJA1000", L"DNG-SJA1000 EPP"
            });
            this->cbbHwType->Location = System::Drawing::Point(326, 31);
            this->cbbHwType->Name = L"cbbHwType";
            this->cbbHwType->Size = System::Drawing::Size(120, 21);
            this->cbbHwType->TabIndex = 3;
            this->cbbHwType->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cbbHwType_SelectedIndexChanged);
            // 
            // cbbInterrupt
            // 
            this->cbbInterrupt->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->cbbInterrupt->Items->AddRange(gcnew cli::array< System::Object^  >(9) {
                L"3", L"4", L"5", L"7", L"9", L"10", L"11",
                    L"12", L"15"
            });
            this->cbbInterrupt->Location = System::Drawing::Point(513, 31);
            this->cbbInterrupt->Name = L"cbbInterrupt";
            this->cbbInterrupt->Size = System::Drawing::Size(55, 21);
            this->cbbInterrupt->TabIndex = 5;
            // 
            // btnFilterQuery
            // 
            this->btnFilterQuery->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->btnFilterQuery->Enabled = false;
            this->btnFilterQuery->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->btnFilterQuery->Location = System::Drawing::Point(639, 26);
            this->btnFilterQuery->Name = L"btnFilterQuery";
            this->btnFilterQuery->Size = System::Drawing::Size(65, 23);
            this->btnFilterQuery->TabIndex = 7;
            this->btnFilterQuery->Text = L"Query";
            this->btnFilterQuery->UseVisualStyleBackColor = true;
            this->btnFilterQuery->Click += gcnew System::EventHandler(this, &Form1::btnFilterQuery_Click);
            // 
            // laInterrupt
            // 
            this->laInterrupt->Location = System::Drawing::Point(515, 15);
            this->laInterrupt->Name = L"laInterrupt";
            this->laInterrupt->Size = System::Drawing::Size(53, 23);
            this->laInterrupt->TabIndex = 44;
            this->laInterrupt->Text = L"Interrupt:";
            // 
            // groupBox3
            // 
            this->groupBox3->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->groupBox3->Controls->Add(this->btnFilterQuery);
            this->groupBox3->Controls->Add(this->chbFilterExt);
            this->groupBox3->Controls->Add(this->nudIdTo);
            this->groupBox3->Controls->Add(this->nudIdFrom);
            this->groupBox3->Controls->Add(this->label8);
            this->groupBox3->Controls->Add(this->label7);
            this->groupBox3->Controls->Add(this->rdbFilterOpen);
            this->groupBox3->Controls->Add(this->rdbFilterCustom);
            this->groupBox3->Controls->Add(this->rdbFilterClose);
            this->groupBox3->Controls->Add(this->btnFilterApply);
            this->groupBox3->Location = System::Drawing::Point(12, 83);
            this->groupBox3->Name = L"groupBox3";
            this->groupBox3->Size = System::Drawing::Size(710, 58);
            this->groupBox3->TabIndex = 1;
            this->groupBox3->TabStop = false;
            this->groupBox3->Text = L" Message Filtering ";
            // 
            // chbFilterExt
            // 
            this->chbFilterExt->AutoSize = true;
            this->chbFilterExt->Location = System::Drawing::Point(10, 33);
            this->chbFilterExt->Name = L"chbFilterExt";
            this->chbFilterExt->Size = System::Drawing::Size(103, 17);
            this->chbFilterExt->TabIndex = 0;
            this->chbFilterExt->Text = L"Extended Frame";
            this->chbFilterExt->UseVisualStyleBackColor = true;
            this->chbFilterExt->CheckedChanged += gcnew System::EventHandler(this, &Form1::chbFilterExt_CheckedChanged);
            // 
            // nudIdTo
            // 
            this->nudIdTo->Hexadecimal = true;
            this->nudIdTo->Location = System::Drawing::Point(438, 29);
            this->nudIdTo->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2047, 0, 0, 0 });
            this->nudIdTo->Name = L"nudIdTo";
            this->nudIdTo->Size = System::Drawing::Size(69, 20);
            this->nudIdTo->TabIndex = 5;
            this->nudIdTo->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2047, 0, 0, 0 });
            // 
            // nudIdFrom
            // 
            this->nudIdFrom->Hexadecimal = true;
            this->nudIdFrom->Location = System::Drawing::Point(363, 29);
            this->nudIdFrom->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 536870911, 0, 0, 0 });
            this->nudIdFrom->Name = L"nudIdFrom";
            this->nudIdFrom->Size = System::Drawing::Size(69, 20);
            this->nudIdFrom->TabIndex = 4;
            // 
            // label8
            // 
            this->label8->Location = System::Drawing::Point(436, 10);
            this->label8->Name = L"label8";
            this->label8->Size = System::Drawing::Size(28, 23);
            this->label8->TabIndex = 43;
            this->label8->Text = L"To:";
            // 
            // label7
            // 
            this->label7->Location = System::Drawing::Point(361, 12);
            this->label7->Name = L"label7";
            this->label7->Size = System::Drawing::Size(38, 23);
            this->label7->TabIndex = 42;
            this->label7->Text = L"From:";
            // 
            // rdbFilterOpen
            // 
            this->rdbFilterOpen->Checked = true;
            this->rdbFilterOpen->Location = System::Drawing::Point(120, 32);
            this->rdbFilterOpen->Name = L"rdbFilterOpen";
            this->rdbFilterOpen->Size = System::Drawing::Size(53, 17);
            this->rdbFilterOpen->TabIndex = 1;
            this->rdbFilterOpen->TabStop = true;
            this->rdbFilterOpen->Text = L"Open";
            this->rdbFilterOpen->UseVisualStyleBackColor = true;
            // 
            // rdbFilterCustom
            // 
            this->rdbFilterCustom->Location = System::Drawing::Point(238, 32);
            this->rdbFilterCustom->Name = L"rdbFilterCustom";
            this->rdbFilterCustom->Size = System::Drawing::Size(104, 17);
            this->rdbFilterCustom->TabIndex = 3;
            this->rdbFilterCustom->Text = L"Custom (expand)";
            this->rdbFilterCustom->UseVisualStyleBackColor = true;
            // 
            // rdbFilterClose
            // 
            this->rdbFilterClose->Location = System::Drawing::Point(177, 32);
            this->rdbFilterClose->Name = L"rdbFilterClose";
            this->rdbFilterClose->Size = System::Drawing::Size(58, 17);
            this->rdbFilterClose->TabIndex = 2;
            this->rdbFilterClose->Text = L"Close";
            this->rdbFilterClose->UseVisualStyleBackColor = true;
            // 
            // btnFilterApply
            // 
            this->btnFilterApply->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->btnFilterApply->Enabled = false;
            this->btnFilterApply->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->btnFilterApply->Location = System::Drawing::Point(568, 26);
            this->btnFilterApply->Name = L"btnFilterApply";
            this->btnFilterApply->Size = System::Drawing::Size(65, 23);
            this->btnFilterApply->TabIndex = 6;
            this->btnFilterApply->Text = L"Apply";
            this->btnFilterApply->UseVisualStyleBackColor = true;
            this->btnFilterApply->Click += gcnew System::EventHandler(this, &Form1::btnFilterApply_Click);
            // 
            // cbbIO
            // 
            this->cbbIO->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->cbbIO->Items->AddRange(gcnew cli::array< System::Object^  >(24) {
                L"0100", L"0120", L"0140", L"0200", L"0220", L"0240",
                    L"0260", L"0278", L"0280", L"02A0", L"02C0", L"02E0", L"02E8", L"02F8", L"0300", L"0320", L"0340", L"0360", L"0378", L"0380",
                    L"03BC", L"03E0", L"03E8", L"03F8"
            });
            this->cbbIO->Location = System::Drawing::Point(452, 31);
            this->cbbIO->Name = L"cbbIO";
            this->cbbIO->Size = System::Drawing::Size(55, 21);
            this->cbbIO->TabIndex = 4;
            // 
            // tmrRead
            // 
            this->tmrRead->Interval = 50;
            this->tmrRead->Tick += gcnew System::EventHandler(this, &Form1::tmrRead_Tick);
            // 
            // laIOPort
            // 
            this->laIOPort->Location = System::Drawing::Point(452, 15);
            this->laIOPort->Name = L"laIOPort";
            this->laIOPort->Size = System::Drawing::Size(55, 23);
            this->laIOPort->TabIndex = 43;
            this->laIOPort->Text = L"I/O Port:";
            // 
            // laHwType
            // 
            this->laHwType->Location = System::Drawing::Point(327, 15);
            this->laHwType->Name = L"laHwType";
            this->laHwType->Size = System::Drawing::Size(90, 23);
            this->laHwType->TabIndex = 42;
            this->laHwType->Text = L"Hardware Type:";
            // 
            // cbbBaudrates
            // 
            this->cbbBaudrates->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->cbbBaudrates->Items->AddRange(gcnew cli::array< System::Object^  >(14) {
                L"1 MBit/sec", L"800 kBit/sec", L"500 kBit/sec",
                    L"250 kBit/sec", L"125 kBit/sec", L"100 kBit/sec", L"95,238 kBit/sec", L"83,333 kBit/sec", L"50 kBit/sec", L"47,619 kBit/sec",
                    L"33,333 kBit/sec", L"20 kBit/sec", L"10 kBit/sec", L"5 kBit/sec"
            });
            this->cbbBaudrates->Location = System::Drawing::Point(204, 31);
            this->cbbBaudrates->Name = L"cbbBaudrates";
            this->cbbBaudrates->Size = System::Drawing::Size(116, 21);
            this->cbbBaudrates->TabIndex = 2;
            this->cbbBaudrates->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cbbBaudrates_SelectedIndexChanged);
            // 
            // laBaudrate
            // 
            this->laBaudrate->Location = System::Drawing::Point(204, 15);
            this->laBaudrate->Name = L"laBaudrate";
            this->laBaudrate->Size = System::Drawing::Size(56, 23);
            this->laBaudrate->TabIndex = 41;
            this->laBaudrate->Text = L"Baudrate:";
            // 
            // label1
            // 
            this->label1->Location = System::Drawing::Point(7, 16);
            this->label1->Name = L"label1";
            this->label1->Size = System::Drawing::Size(56, 23);
            this->label1->TabIndex = 40;
            this->label1->Text = L"Hardware:";
            // 
            // btnInit
            // 
            this->btnInit->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->btnInit->Cursor = System::Windows::Forms::Cursors::Default;
            this->btnInit->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->btnInit->Location = System::Drawing::Point(639, 11);
            this->btnInit->Name = L"btnInit";
            this->btnInit->Size = System::Drawing::Size(65, 23);
            this->btnInit->TabIndex = 6;
            this->btnInit->Text = L"Initialize";
            this->btnInit->Click += gcnew System::EventHandler(this, &Form1::btnInit_Click);
            // 
            // btnRelease
            // 
            this->btnRelease->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->btnRelease->Cursor = System::Windows::Forms::Cursors::Default;
            this->btnRelease->Enabled = false;
            this->btnRelease->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->btnRelease->Location = System::Drawing::Point(639, 36);
            this->btnRelease->Name = L"btnRelease";
            this->btnRelease->Size = System::Drawing::Size(65, 23);
            this->btnRelease->TabIndex = 7;
            this->btnRelease->Text = L"Release";
            this->btnRelease->Click += gcnew System::EventHandler(this, &Form1::btnRelease_Click);
            // 
            // btnParameterGet
            // 
            this->btnParameterGet->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->btnParameterGet->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->btnParameterGet->Location = System::Drawing::Point(639, 26);
            this->btnParameterGet->Name = L"btnParameterGet";
            this->btnParameterGet->Size = System::Drawing::Size(65, 23);
            this->btnParameterGet->TabIndex = 5;
            this->btnParameterGet->Text = L"Get";
            this->btnParameterGet->UseVisualStyleBackColor = true;
            this->btnParameterGet->Click += gcnew System::EventHandler(this, &Form1::btnParameterGet_Click);
            // 
            // label10
            // 
            this->label10->Location = System::Drawing::Point(241, 11);
            this->label10->Name = L"label10";
            this->label10->Size = System::Drawing::Size(59, 23);
            this->label10->TabIndex = 46;
            this->label10->Text = L"Activation:";
            // 
            // nudDeviceIdOrDelay
            // 
            this->nudDeviceIdOrDelay->Enabled = false;
            this->nudDeviceIdOrDelay->Location = System::Drawing::Point(408, 29);
            this->nudDeviceIdOrDelay->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { -1, 0, 0, 0 });
            this->nudDeviceIdOrDelay->Name = L"nudDeviceIdOrDelay";
            this->nudDeviceIdOrDelay->Size = System::Drawing::Size(99, 20);
            this->nudDeviceIdOrDelay->TabIndex = 3;
            // 
            // laDeviceOrDelay
            // 
            this->laDeviceOrDelay->Location = System::Drawing::Point(405, 12);
            this->laDeviceOrDelay->Name = L"laDeviceOrDelay";
            this->laDeviceOrDelay->Size = System::Drawing::Size(59, 23);
            this->laDeviceOrDelay->TabIndex = 45;
            this->laDeviceOrDelay->Text = L"Device ID:";
            // 
            // cbbParameter
            // 
            this->cbbParameter->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->cbbParameter->FormattingEnabled = true;
            this->cbbParameter->Items->AddRange(gcnew cli::array< System::Object^  >(22) {
                L"Device ID", L"5V Power", L"Auto-reset on BUS-OFF",
                    L"CAN Listen-Only", L"Debug\'s Log", L"Receive Status", L"CAN Controller Number", L"Trace File", L"Channel Identification (USB)",
                    L"Channel Capabilities", L"Bit rate Adaptation", L"Get Bit rate Information", L"Get Bit rate FD Information", L"Get CAN Nominal Speed Bit/s",
                    L"Get CAN Data Speed Bit/s", L"Get IP Address", L"Get LAN Service Status", L"Reception of Status Frames", L"Reception of RTR Frames",
                    L"Reception of Error Frames", L"Interframe Transmit Delay", L"Reception of Echo Frames"
            });
            this->cbbParameter->Location = System::Drawing::Point(10, 31);
            this->cbbParameter->Name = L"cbbParameter";
            this->cbbParameter->Size = System::Drawing::Size(217, 21);
            this->cbbParameter->TabIndex = 0;
            this->cbbParameter->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cbbParameter_SelectedIndexChanged);
            // 
            // groupBox2
            // 
            this->groupBox2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->groupBox2->Controls->Add(this->btnParameterGet);
            this->groupBox2->Controls->Add(this->label10);
            this->groupBox2->Controls->Add(this->nudDeviceIdOrDelay);
            this->groupBox2->Controls->Add(this->laDeviceOrDelay);
            this->groupBox2->Controls->Add(this->cbbParameter);
            this->groupBox2->Controls->Add(this->label6);
            this->groupBox2->Controls->Add(this->rdbParamActive);
            this->groupBox2->Controls->Add(this->rdbParamInactive);
            this->groupBox2->Controls->Add(this->btnParameterSet);
            this->groupBox2->Location = System::Drawing::Point(12, 147);
            this->groupBox2->Name = L"groupBox2";
            this->groupBox2->Size = System::Drawing::Size(710, 58);
            this->groupBox2->TabIndex = 2;
            this->groupBox2->TabStop = false;
            this->groupBox2->Text = L" Configuration Parameters ";
            // 
            // label6
            // 
            this->label6->Location = System::Drawing::Point(7, 14);
            this->label6->Name = L"label6";
            this->label6->Size = System::Drawing::Size(64, 23);
            this->label6->TabIndex = 43;
            this->label6->Text = L"Parameter:";
            // 
            // rdbParamActive
            // 
            this->rdbParamActive->Checked = true;
            this->rdbParamActive->Location = System::Drawing::Point(238, 32);
            this->rdbParamActive->Name = L"rdbParamActive";
            this->rdbParamActive->Size = System::Drawing::Size(56, 17);
            this->rdbParamActive->TabIndex = 1;
            this->rdbParamActive->TabStop = true;
            this->rdbParamActive->Text = L"Active";
            this->rdbParamActive->UseVisualStyleBackColor = true;
            // 
            // btnParameterSet
            // 
            this->btnParameterSet->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->btnParameterSet->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->btnParameterSet->Location = System::Drawing::Point(568, 26);
            this->btnParameterSet->Name = L"btnParameterSet";
            this->btnParameterSet->Size = System::Drawing::Size(65, 23);
            this->btnParameterSet->TabIndex = 4;
            this->btnParameterSet->Text = L"Set";
            this->btnParameterSet->UseVisualStyleBackColor = true;
            this->btnParameterSet->Click += gcnew System::EventHandler(this, &Form1::btnParameterSet_Click);
            // 
            // groupBox1
            // 
            this->groupBox1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->groupBox1->Controls->Add(this->btnHwRefresh);
            this->groupBox1->Controls->Add(this->cbbChannel);
            this->groupBox1->Controls->Add(this->cbbHwType);
            this->groupBox1->Controls->Add(this->cbbInterrupt);
            this->groupBox1->Controls->Add(this->laInterrupt);
            this->groupBox1->Controls->Add(this->cbbIO);
            this->groupBox1->Controls->Add(this->laIOPort);
            this->groupBox1->Controls->Add(this->laHwType);
            this->groupBox1->Controls->Add(this->cbbBaudrates);
            this->groupBox1->Controls->Add(this->laBaudrate);
            this->groupBox1->Controls->Add(this->label1);
            this->groupBox1->Controls->Add(this->btnInit);
            this->groupBox1->Controls->Add(this->btnRelease);
            this->groupBox1->Controls->Add(this->chbCanFD);
            this->groupBox1->Controls->Add(this->txtBitrate);
            this->groupBox1->Controls->Add(this->laBitrate);
            this->groupBox1->FlatStyle = System::Windows::Forms::FlatStyle::System;
            this->groupBox1->Location = System::Drawing::Point(12, 12);
            this->groupBox1->Name = L"groupBox1";
            this->groupBox1->Size = System::Drawing::Size(710, 65);
            this->groupBox1->TabIndex = 0;
            this->groupBox1->TabStop = false;
            this->groupBox1->Text = L" Connection ";
            // 
            // chbCanFD
            // 
            this->chbCanFD->AutoSize = true;
            this->chbCanFD->Location = System::Drawing::Point(574, 34);
            this->chbCanFD->Name = L"chbCanFD";
            this->chbCanFD->Size = System::Drawing::Size(65, 17);
            this->chbCanFD->TabIndex = 62;
            this->chbCanFD->Text = L"CAN-FD";
            this->chbCanFD->UseVisualStyleBackColor = true;
            this->chbCanFD->CheckedChanged += gcnew System::EventHandler(this, &Form1::chbCanFD_CheckedChanged);
            // 
            // txtBitrate
            // 
            this->txtBitrate->Location = System::Drawing::Point(204, 25);
            this->txtBitrate->Multiline = true;
            this->txtBitrate->Name = L"txtBitrate";
            this->txtBitrate->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
            this->txtBitrate->Size = System::Drawing::Size(364, 34);
            this->txtBitrate->TabIndex = 61;
            this->txtBitrate->Visible = false;
            // 
            // laBitrate
            // 
            this->laBitrate->AutoSize = true;
            this->laBitrate->Location = System::Drawing::Point(201, 8);
            this->laBitrate->Name = L"laBitrate";
            this->laBitrate->Size = System::Drawing::Size(43, 13);
            this->laBitrate->TabIndex = 60;
            this->laBitrate->Text = L"Bit rate:";
            this->laBitrate->Visible = false;
            // 
            // tmrDisplay
            // 
            this->tmrDisplay->Tick += gcnew System::EventHandler(this, &Form1::tmrDisplay_Tick);
            // 
            // Form1
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(734, 605);
            this->Controls->Add(this->groupBox5);
            this->Controls->Add(this->groupBox6);
            this->Controls->Add(this->groupBox4);
            this->Controls->Add(this->groupBox3);
            this->Controls->Add(this->groupBox2);
            this->Controls->Add(this->groupBox1);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
            this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
            this->MaximizeBox = false;
            this->Name = L"Form1";
            this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
            this->Text = L"PCAN-Basic Sample";
            this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &Form1::Form1_FormClosing);
            this->groupBox5->ResumeLayout(false);
            this->groupBox5->PerformLayout();
            this->groupBox6->ResumeLayout(false);
            this->groupBox6->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->nudLength))->EndInit();
            this->groupBox4->ResumeLayout(false);
            this->groupBox3->ResumeLayout(false);
            this->groupBox3->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->nudIdTo))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->nudIdFrom))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->nudDeviceIdOrDelay))->EndInit();
            this->groupBox2->ResumeLayout(false);
            this->groupBox1->ResumeLayout(false);
            this->groupBox1->PerformLayout();
            this->ResumeLayout(false);

        }
#pragma endregion

#pragma region Delegates
		/// <summary>
		/// Read-Delegate Handler
		/// </summary>
	private: 
		delegate void ReadDelegateHandler();
#pragma endregion

#pragma region Members
	private:
        /// <summary>
        /// Saves the desired connection mode
        /// </summary>
		bool m_IsFD;
		/// <summary>
		/// Saves the handle of a PCAN hardware
		/// </summary>
		TPCANHandle ^m_PcanHandle;
		/// <summary>
		/// Saves the baudrate register for a conenction
		/// </summary>
		TPCANBaudrate ^m_Baudrate;
		/// <summary>
		/// Saves the type of a non-plug-and-play hardware
		/// </summary>
		TPCANType ^m_HwType;
		/// <summary>
		/// Stores the status of received messages for its display
		/// </summary>
		System::Collections::ArrayList ^m_LastMsgsList;
		/// <summary>
		/// Read Delegate for calling the function "ReadMessages"
		/// </summary>
		ReadDelegateHandler ^m_ReadDelegate;
		/// <summary>
		/// Receive-Event
		/// </summary>
		System::Threading::AutoResetEvent ^m_ReceiveEvent;
		/// <summary>
		/// Thread for message reading (using events)
		/// </summary>
		System::Threading::Thread ^m_ReadThread;
		/// <summary>
		/// Handles of non plug and play PCAN-Hardware
		/// </summary>
		array<TPCANHandle^> ^m_NonPnPHandles;
#pragma endregion

#pragma region Methods

#pragma region Help functions
	/// <summary>
	/// Initialization of PCAN-Basic components
	/// </summary>
	private: void InitializeBasicComponents()
	{
			// Creates the list for received messages
			//
			m_LastMsgsList = gcnew System::Collections::ArrayList();
			// Creates the delegate used for message reading
			//
			m_ReadDelegate = gcnew ReadDelegateHandler(this, &Form1::ReadMessages);
			// Creates the event used for signalize incomming messages 
			//
			m_ReceiveEvent = gcnew System::Threading::AutoResetEvent(false);

			// Creates an array with all possible non plug-and-play PCAN-Channels
			//
			m_NonPnPHandles = gcnew array<TPCANHandle^> { 
				PCANBasic::PCAN_ISABUS1,
				PCANBasic::PCAN_ISABUS2,
				PCANBasic::PCAN_ISABUS3,
				PCANBasic::PCAN_ISABUS4,
				PCANBasic::PCAN_ISABUS5,
				PCANBasic::PCAN_ISABUS6,
				PCANBasic::PCAN_ISABUS7,
				PCANBasic::PCAN_ISABUS8,
				PCANBasic::PCAN_DNGBUS1
			};

			// Prepares the PCAN-Basic's debug-Log file
			//
			FillComboBoxData();

			// Prepares the PCAN-Basic's debug-Log file
			//
			ConfigureLogFile();
	}

    /// <summary>
    /// Gets the formated text for a CPAN-Basic channel handle
    /// </summary>
    /// <param name="handle">PCAN-Basic Handle to format</param>
    /// <param name="isFD">If the channel is FD capable</param>
    /// <returns>The formatted text for a channel</returns>
	 private: String ^FormatChannelName(TPCANHandle ^handle, bool isFD)
	 {
		TPCANDevice ^devDevice;
		Byte byChannel;
		
		// Gets the owner device and channel for a 
		// PCAN-Basic handle
		//
		if(safe_cast<UInt16>(handle) < 0x100)
		{
			devDevice = safe_cast<TPCANDevice>(safe_cast<UInt16>(handle) >> 4);
			byChannel = safe_cast<Byte>(safe_cast<UInt16>(handle) & 0xF);
		}
		else
		{		
			devDevice = safe_cast<TPCANDevice>(safe_cast<UInt16>(handle) >> 8);
			byChannel = safe_cast<Byte>(safe_cast<UInt16>(handle) & 0xFF);
		}

		// Constructs the PCAN-Basic Channel name and return it
		//
        if (isFD)
			return String::Format("{0}:FD {1} ({2:X2}h)", devDevice, byChannel, handle);
		else
			return String::Format("{0} {1} ({2:X2}h)", devDevice, byChannel, handle);
	 }

	/// <summary>
	/// Gets the formated text for a CPAN-Basic channel handle
	/// </summary>
	/// <param name="handle">PCAN-Basic Handle to format</param>
	/// <returns>The formatted text for a channel</returns>
	private: String ^FormatChannelName(TPCANHandle ^handle)
	{
		return FormatChannelName(handle, false);
	}
	
	/// <summary>
	/// Configures the data of all ComboBox components of the main-form
	/// </summary>
	private: void FillComboBoxData()
	{
		// Channels will be check
		//
		btnHwRefresh_Click(this, gcnew EventArgs());

        // FD Bitrate: 
        //      Arbitration: 1 Mbit/sec 
        //      Data: 2 Mbit/sec
        //
        txtBitrate->Text = "f_clock_mhz=20, nom_brp=5, nom_tseg1=2, nom_tseg2=1, nom_sjw=1, data_brp=2, data_tseg1=3, data_tseg2=1, data_sjw=1";

		// Baudrates 
		//
		cbbBaudrates->SelectedIndex = 2; // 500 K

		// Hardware Type for no plugAndplay hardware
		//
		cbbHwType->SelectedIndex = 0;

		// Interrupt for no plugAndplay hardware
		//
		cbbInterrupt->SelectedIndex = 0;

		// IO Port for no plugAndplay hardware
		//
		cbbIO->SelectedIndex = 0;

		// Parameters for GetValue and SetValue function calls
		//
		cbbParameter->SelectedIndex = 0;
	}

	/// <summary>
	/// Help Function used to get an error as text
	/// </summary>
	/// <param name="error">Error code to be translated</param>
	/// <returns>A text with the translated error</returns>
	private:String ^GetFormatedError(TPCANStatus ^error)
	{
		StringBuilder ^strTemp;

		// Creates a buffer big enough for a error-text
		//
		strTemp = gcnew StringBuilder(256);
		// Gets the text using the GetErrorText API function
		// If the function success, the translated error is returned. If it fails,
		// a text describing the current error is returned.
		//
		if (PCANBasic::GetErrorText(*error, 0, strTemp) != TPCANStatus::PCAN_ERROR_OK)
			return String::Format("An eror occurred. Error-code's text ({0:X}) couldn't be retrieved", error);
		else
			return strTemp->ToString();
	}

	/// <summary>
	/// Configure LogFile the Debug-Log file of PCAN-Basic
	/// </summary>
	/// <param name="bConnected">Current status. True if connected, false otherwise</param>
	private: void ConfigureLogFile()
	{
		UInt32 iBuffer;

		// Sets the mask to catch all events
		//
		iBuffer = PCANBasic::LOG_FUNCTION_ALL;

		// Configures the log file. 
		// NOTE: The Log capability is to be used with the NONEBUS Handle. Other handle than this will 
		// cause the function fail.
		//
		PCANBasic::SetValue(PCANBasic::PCAN_NONEBUS, TPCANParameter::PCAN_LOG_CONFIGURE, iBuffer, sizeof(UInt32));
	}

	private: void ConfigureTraceFile()
	{
		UInt32 iBuffer;
		TPCANStatus ^stsResult;

		// Configure the maximum size of a trace file to 5 megabytes
        //
		iBuffer = 5;
		stsResult = PCANBasic::SetValue(*m_PcanHandle, TPCANParameter::PCAN_TRACE_SIZE, iBuffer, sizeof(UInt32));
		if (*stsResult != TPCANStatus::PCAN_ERROR_OK)
			IncludeTextMessage(GetFormatedError(stsResult));

        // Configure the way how trace files are created: 
        // * Standard name is used
        // * Existing file is ovewritten, 
        // * Only one file is created.
        // * Recording stopts when the file size reaches 5 megabytes.
        //
		iBuffer = PCANBasic::TRACE_FILE_SINGLE | PCANBasic::TRACE_FILE_OVERWRITE;
		stsResult = PCANBasic::SetValue(*m_PcanHandle, TPCANParameter::PCAN_TRACE_CONFIGURE, iBuffer, sizeof(UInt32));
		if (*stsResult != TPCANStatus::PCAN_ERROR_OK)
			IncludeTextMessage(GetFormatedError(stsResult));
	}

	/// <summary>
	/// Activates/deaactivates the different controls of the main-form according
	/// with the current connection status
	/// </summary>
	/// <param name="bConnected">Current status. True if connected, false otherwise</param>
	private: void SetConnectionStatus(bool bConnected)
	{
		// Buttons
		//
		btnInit->Enabled = !bConnected;
		btnRead->Enabled = bConnected && rdbManual->Checked;
		btnWrite->Enabled = bConnected;
		btnRelease->Enabled = bConnected;
		btnFilterApply->Enabled = bConnected;
		btnFilterQuery->Enabled = bConnected;
		btnGetVersions->Enabled = bConnected;
		btnHwRefresh->Enabled = !bConnected;
		btnStatus->Enabled = bConnected;
		btnReset->Enabled = bConnected;

		// ComboBoxs
		//
		cbbBaudrates->Enabled = !bConnected;
		cbbChannel->Enabled = !bConnected;
		cbbHwType->Enabled = !bConnected;
		cbbIO->Enabled = !bConnected;
		cbbInterrupt->Enabled = !bConnected;

        // Check-Buttons
        //
        chbCanFD->Enabled = !bConnected;

		// Hardware configuration and read mode
		//
		if (!bConnected)
			cbbChannel_SelectedIndexChanged(this, gcnew EventArgs());
		else
			rdbTimer_CheckedChanged(this, gcnew EventArgs());

        // Display messages in grid
        //
        tmrDisplay->Enabled = bConnected;
	}

	/// <summary>
	/// Gets the current status of the PCAN-Basic message filter
	/// </summary>
	/// <param name="status">Buffer to retrieve the filter status</param>
	/// <returns>If calling the function was successfull or not</returns>
	private: bool GetFilterStatus([System::Runtime::InteropServices::Out] UInt32 %status)
	 {
		 TPCANStatus ^stsResult;

		 // Tries to get the stataus of the filter for the current connected hardware
		 //
		 stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_MESSAGE_FILTER, status, sizeof(UInt32));

		 // If it fails, a error message is shown
		 //
		 if (*stsResult != TPCANStatus::PCAN_ERROR_OK)
		 {
			 MessageBox::Show(GetFormatedError(stsResult));
			 return false;
		 }
		 return true;
	 }

	/// <summary>
	/// Includes a new line of text into the information Listview
	/// </summary>
	/// <param name="strMsg">Text to be included</param>
	private: void IncludeTextMessage(String ^strMsg)
	 {
		 lbxInfo->Items->Add(strMsg);
		 lbxInfo->SelectedIndex = lbxInfo->Items->Count - 1;
	 }
#pragma endregion

#pragma region Message-proccessing functions

	/// <summary>
	/// Thread-Function used for reading PCAN-Basic messages
	/// </summary>
	private:void CANReadThreadFunc()
	{
		UInt32 iBuffer;
		TPCANStatus ^stsResult;

		iBuffer = Convert::ToUInt32(m_ReceiveEvent->SafeWaitHandle->DangerousGetHandle().ToInt32());
		// Sets the handle of the Receive-Event.
		//
		stsResult = PCANBasic::SetValue(*m_PcanHandle, TPCANParameter::PCAN_RECEIVE_EVENT,iBuffer, sizeof(UInt32));
		
		if(*stsResult != TPCANStatus::PCAN_ERROR_OK)
		{
			MessageBox::Show(GetFormatedError(stsResult),"Error!",MessageBoxButtons::OK,MessageBoxIcon::Error);
			return;
		}

		// While this mode is selected
		while (rdbEvent->Checked)
		{
			// Waiting for Receive-Event
			// 
			if(m_ReceiveEvent->WaitOne(50))
				// Process Receive-Event using .NET Invoke function
				// in order to interact with Winforms UI (calling the 
				// function ReadMessages)
				// 
				this->Invoke(m_ReadDelegate);
		}

		// Resets the Event-handle configuration
		//
		iBuffer = 0;
		PCANBasic::SetValue(*m_PcanHandle, TPCANParameter::PCAN_RECEIVE_EVENT,iBuffer, sizeof(UInt32));
	}

    /// <summary>
    /// Function for reading messages on FD devices
    /// </summary>
    /// <returns>A TPCANStatus error code</returns>
	private: TPCANStatus ^ReadMessageFD()
	{
		TPCANMsgFD ^CANMsg;
        TPCANTimestampFD CANTimeStamp;
        TPCANStatus ^stsResult;

        // We execute the "Read" function of the PCANBasic                
        //
		CANMsg = gcnew TPCANMsgFD();
		stsResult = PCANBasic::ReadFD(*m_PcanHandle, *CANMsg, CANTimeStamp);
		if (*stsResult != TPCANStatus::PCAN_ERROR_QRCVEMPTY)
            // We process the received message
            //
            ProcessMessage(CANMsg, CANTimeStamp);

        return stsResult;
	}

	/// <summary>
	/// Function for reading CAN messages on normal CAN devices
	/// </summary>
	/// <returns>A TPCANStatus error code</returns>
	private: TPCANStatus ^ReadMessage()
	{
		TPCANMsg ^CANMsg;
		TPCANTimestamp ^CANTimeStamp;
		TPCANStatus ^stsResult;

        // We execute the "Read" function of the PCANBasic                
        //
		CANMsg = gcnew TPCANMsg();
		CANTimeStamp = gcnew TPCANTimestamp();
        stsResult = PCANBasic::Read(*m_PcanHandle, *CANMsg, *CANTimeStamp);
		if (*stsResult != TPCANStatus::PCAN_ERROR_QRCVEMPTY)
            // We process the received message
            //
            ProcessMessage(CANMsg, CANTimeStamp);

        return stsResult;
	}

	/// <summary>
	/// Function for reading PCAN-Basic messages
	/// </summary>
	private: void ReadMessages()
	{
		TPCANStatus ^stsResult;

		// We read at least one time the queue looking for messages.
		// If a message is found, we look again trying to find more.
		// If the queue is empty or an error occurr, we get out from
		// the dowhile statement.
		//			
		do
		{
			// We execute the "Read" function of the PCANBasic                
			//
			stsResult = m_IsFD ? ReadMessageFD() : ReadMessage();
			if (*stsResult == TPCANStatus::PCAN_ERROR_ILLOPERATION)
				break;
		} while (btnRelease->Enabled && (!Convert::ToBoolean(*stsResult & TPCANStatus::PCAN_ERROR_QRCVEMPTY)));
	}

    /// <summary>
    /// Processes a received message, in order to show it in the Message-ListView
    /// </summary>
    /// <param name="theMsg">The received PCAN-Basic message</param>
    /// <returns>True if the message must be created, false if it must be modified</returns>
	private: void ProcessMessage(TPCANMsgFD ^theMsg, TPCANTimestampFD itsTimeStamp)
    {
		try
		{
	        // We search if a message (Same ID and Type) is 
			// already received or if this is a new message
			//
			Monitor::Enter(m_LastMsgsList->SyncRoot);
            for each (MessageStatus^ msg in m_LastMsgsList)
            {
                if ((msg->CANMsg.ID == theMsg->ID) && (msg->CANMsg.MSGTYPE == theMsg->MSGTYPE))
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
		finally
		{
			Monitor::Exit(m_LastMsgsList->SyncRoot);
		}	
    }

	/// <summary>
	/// Processes a received message, in order to show it in the Message-ListView
	/// </summary>
	/// <param name="theMsg">The received PCAN-Basic message</param>
	/// <param name="itsTimeStamp">The Timestamp of the received message</param>
	private: void ProcessMessage(TPCANMsg ^theMsg, TPCANTimestamp ^itsTimeStamp)
	{
        TPCANMsgFD ^newMsg;
        TPCANTimestampFD newTimestamp;

        newMsg = gcnew TPCANMsgFD();
        newMsg->DATA = gcnew array<Byte>(64);
        newMsg->ID = theMsg->ID;
        newMsg->DLC = theMsg->LEN;
        for (int i = 0; i < ((theMsg->LEN > 8) ? 8 : theMsg->LEN); i++)
            newMsg->DATA[i] = theMsg->DATA[i];
        newMsg->MSGTYPE = theMsg->MSGTYPE;

		newTimestamp = Convert::ToUInt64(itsTimeStamp->micros + 1000 * itsTimeStamp->millis + 0x100000000 * 1000 * itsTimeStamp->millis_overflow);
        ProcessMessage(newMsg, newTimestamp);	
	}

	/// <summary>
	/// Inserts a new entry for a new message in the Message-ListView
	/// </summary>
	/// <param name="newMsg">The me3ssasge to be inserted</param>
	/// <param name="timeStamp">The Timesamp of the new message</param>
	private: void InsertMsgEntry(TPCANMsgFD ^newMsg, TPCANTimestampFD ^timeStamp)
	{
		ListViewItem ^lviCurrentItem;
		MessageStatus ^msgStsCurrentMsg;

		try
		{
			Monitor::Enter(m_LastMsgsList->SyncRoot);

            // We add this status in the last message list
            //
            msgStsCurrentMsg = gcnew MessageStatus(*newMsg, *timeStamp, lstMessages->Items->Count);
			msgStsCurrentMsg->ShowingPeriod = chbShowPeriod->Checked;
            m_LastMsgsList->Add(msgStsCurrentMsg);

            // Add the new ListView Item with the Type of the message
            //	
            lviCurrentItem = lstMessages->Items->Add(msgStsCurrentMsg->TypeString);
            // We set the ID of the message
            //
            lviCurrentItem->SubItems->Add(msgStsCurrentMsg->IdString);			
            // We set the length of the Message
            //
			lviCurrentItem->SubItems->Add(GetLengthFromDLC(newMsg->DLC, (newMsg->MSGTYPE & TPCANMessageType::PCAN_MESSAGE_FD) == TPCANMessageType()).ToString());
            // we set the message count message (this is the First, so count is 1)            
            //
            lviCurrentItem->SubItems->Add(msgStsCurrentMsg->Count.ToString());
            // Add time stamp information if needed
            //
            lviCurrentItem->SubItems->Add(msgStsCurrentMsg->TimeString);
            // We set the data of the message. 	
            //
            lviCurrentItem->SubItems->Add(msgStsCurrentMsg->DataString);
        }
		finally
		{
			Monitor::Exit(m_LastMsgsList->SyncRoot);
		}
	}


	/// <summary>
	/// Display CAN messages in the Message-ListView
	/// </summary>
	private: void DisplayMessages()
	{
		ListViewItem ^lviCurrentItem;

		try
		{
			Monitor::Enter(m_LastMsgsList->SyncRoot);
			
            for each (MessageStatus^ msgStatus in m_LastMsgsList)
            {
                // Get the data to actualize
                //
                if (msgStatus->MarkedAsUpdated)
                {
                    msgStatus->MarkedAsUpdated = false;
                    lviCurrentItem = lstMessages->Items[msgStatus->Position];

					lviCurrentItem->SubItems[2]->Text = GetLengthFromDLC(msgStatus->CANMsg.DLC, (msgStatus->CANMsg.MSGTYPE & TPCANMessageType::PCAN_MESSAGE_FD) == TPCANMessageType()).ToString();
                    lviCurrentItem->SubItems[3]->Text = msgStatus->Count.ToString();
                    lviCurrentItem->SubItems[4]->Text = msgStatus->TimeString;
                    lviCurrentItem->SubItems[5]->Text = msgStatus->DataString;
                }
            }	
		}
		finally
		{
			Monitor::Exit(m_LastMsgsList->SyncRoot);
		}
	}
#pragma endregion

#pragma region Event Handlers
	private: System::Void cbbChannel_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
	{
		bool bNonPnP;
		String ^strTemp;

		// Get the handle fromt he text being shown
		//
		strTemp = cbbChannel->Text;
		strTemp = strTemp->Substring(strTemp->IndexOf('(')+1, 3);

		strTemp = strTemp->Replace('h', ' ')->Trim(' ');

		// Determines if the handle belong to a No Plug&Play hardware 
		//
		m_PcanHandle = Convert::ToUInt16(strTemp,16);
		bNonPnP = static_cast<UInt16>(m_PcanHandle) <= PCANBasic::PCAN_DNGBUS1;

		// Activates/deactivates configuration controls according with the 
		// kind of hardware
		//
		cbbHwType->Enabled = bNonPnP;
		cbbIO->Enabled = bNonPnP;
		cbbInterrupt->Enabled = bNonPnP;
	}

	private: System::Void btnHwRefresh_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		UInt32 iChannelsCount;
		TPCANStatus ^stsResult;
		bool bIsFD;

		// Clears the Channel combioBox and fill it againa with 
		// the PCAN-Basic handles for no-Plug&Play hardware and
		// the detected Plug&Play hardware
		//
		cbbChannel->Items->Clear();
		
		try
		{
			// Includes all no-Plug&Play Handles
			for (int i = 0; i < m_NonPnPHandles->Length; i++)
				cbbChannel->Items->Add(FormatChannelName(m_NonPnPHandles[i]));

            // Checks for available Plug&Play channels
            //
            stsResult = PCANBasic::GetValue(PCANBasic::PCAN_NONEBUS, TPCANParameter::PCAN_ATTACHED_CHANNELS_COUNT, iChannelsCount, sizeof(UInt32));
			if ((*stsResult) == TPCANStatus::PCAN_ERROR_OK) 
			{
				array<TPCANChannelInformation> ^info = gcnew array<TPCANChannelInformation>(iChannelsCount);

				stsResult = PCANBasic::GetValue(PCANBasic::PCAN_NONEBUS, TPCANParameter::PCAN_ATTACHED_CHANNELS, info);
				if ((*stsResult) == TPCANStatus::PCAN_ERROR_OK) 
					for each (TPCANChannelInformation ^channel in info)
						if ((channel->channel_condition & PCANBasic::PCAN_CHANNEL_AVAILABLE) == PCANBasic::PCAN_CHANNEL_AVAILABLE)
						{
							bIsFD = (channel->device_features & PCANBasic::FEATURE_FD_CAPABLE) == PCANBasic::FEATURE_FD_CAPABLE;
                            cbbChannel->Items->Add(FormatChannelName(channel->channel_handle, bIsFD));
						}			
			}

			cbbChannel->SelectedIndex = cbbChannel->Items->Count - 1;
			btnInit->Enabled = cbbChannel->Items->Count > 0;

			if ((*stsResult) != TPCANStatus::PCAN_ERROR_OK) 
				MessageBox::Show(GetFormatedError(stsResult));
		}
		catch(DllNotFoundException^)
		{
			MessageBox::Show("Unable to find the library: PCANBasic.dll !", "Error!",MessageBoxButtons::OK,MessageBoxIcon::Error);
			Environment::Exit(-1);
		}
	}

	private:System::Void btnInit_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		TPCANStatus ^stsResult;

		// Connects a selected PCAN-Basic channel
		//
		if (m_IsFD)
			stsResult = PCANBasic::InitializeFD(*m_PcanHandle, txtBitrate->Text);
		else
			stsResult = PCANBasic::Initialize(*m_PcanHandle, *m_Baudrate, *m_HwType, Convert::ToUInt32(cbbIO->Text,16), Convert::ToUInt16(cbbInterrupt->Text));

		if (*stsResult != TPCANStatus::PCAN_ERROR_OK)
			if(*stsResult != TPCANStatus::PCAN_ERROR_CAUTION)
				MessageBox::Show(GetFormatedError(stsResult));
			else
			{
				IncludeTextMessage("******************************************************");
                IncludeTextMessage("The bitrate being used is different than the given one");
                IncludeTextMessage("******************************************************");
				stsResult = TPCANStatus::PCAN_ERROR_OK;
			}
		else
            // Prepares the PCAN-Basic's PCAN-Trace file
            //
			ConfigureTraceFile();

		// Sets the connection status of the main-form
		//
		SetConnectionStatus(*stsResult == TPCANStatus::PCAN_ERROR_OK);

	}

	private:System::Void rdbTimer_CheckedChanged(System::Object^  sender, System::EventArgs^  e) 
	{				
		// CheckedChanged event is fired for each Radiobutton of one group
		// So, we must only take in account the selected button
		RadioButton^ radionButton = dynamic_cast<RadioButton ^>(sender);
		if(radionButton && !radionButton->Checked)
			return;
				
		if (!btnRelease->Enabled)
			return;

		// According with the kind of reading, a timer, a thread or a button will be enabled
		//
		if (rdbTimer->Checked)
		{
			// Abort Read Thread if it exists
			//
			if (m_ReadThread != nullptr)
			{
				m_ReadThread->Abort();
				m_ReadThread->Join();
				m_ReadThread = nullptr;
			}

			// Enable Timer
			//
			tmrRead->Enabled = btnRelease->Enabled;
		}
		
		if (rdbEvent->Checked)
		{
			// Disable Timer
			//
			tmrRead->Enabled = false;
			// Create and start the tread to read CAN Message using SetRcvEvent()
			//
			System::Threading::ThreadStart ^threadDelegate = gcnew System::Threading::ThreadStart(this, &Form1::CANReadThreadFunc);
			m_ReadThread = gcnew System::Threading::Thread(threadDelegate);
			m_ReadThread->IsBackground = true;
			m_ReadThread->Start();
		}
		
		if (rdbManual->Checked)
		{
			// Abort Read Thread if it exists
			//
			if (m_ReadThread != nullptr)
			{
				m_ReadThread->Abort();
				m_ReadThread->Join();
				m_ReadThread = nullptr;
			}
			// Disable Timer
			//
			tmrRead->Enabled = false;
			btnRead->Enabled = btnRelease->Enabled && rdbManual->Checked;
		}
	}

	private:System::Void cbbBaudrates_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) 
	{
		// Saves the current selected baudrate register code
		//
		switch (cbbBaudrates->SelectedIndex)
		{
			case 0:
				m_Baudrate = TPCANBaudrate::PCAN_BAUD_1M;
				break;
			case 1:
				m_Baudrate = TPCANBaudrate::PCAN_BAUD_800K;
				break;
			case 2:
				m_Baudrate = TPCANBaudrate::PCAN_BAUD_500K;
				break;
			case 3:
				m_Baudrate = TPCANBaudrate::PCAN_BAUD_250K;
				break;
			case 4:
				m_Baudrate = TPCANBaudrate::PCAN_BAUD_125K;
				break;
			case 5:
				m_Baudrate = TPCANBaudrate::PCAN_BAUD_100K;
				break;
			case 6:
				m_Baudrate = TPCANBaudrate::PCAN_BAUD_95K;
				break;
			case 7:
				m_Baudrate = TPCANBaudrate::PCAN_BAUD_83K;
				break;
			case 8:
				m_Baudrate = TPCANBaudrate::PCAN_BAUD_50K;
				break;
			case 9:
				m_Baudrate = TPCANBaudrate::PCAN_BAUD_47K;
				break;
			case 10:
				m_Baudrate = TPCANBaudrate::PCAN_BAUD_33K;
				break;
			case 11:
				m_Baudrate = TPCANBaudrate::PCAN_BAUD_20K;
				break;
			case 12:
				m_Baudrate = TPCANBaudrate::PCAN_BAUD_10K;
				break;
			case 13:
				m_Baudrate = TPCANBaudrate::PCAN_BAUD_5K;
				break;
		}
	}

	private: System::Void cbbHwType_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) 
	{
		// Saves the current type for a no-Plug&Play hardware
		//
		switch (cbbHwType->SelectedIndex)
		{
			case 0:
				m_HwType = TPCANType::PCAN_TYPE_ISA;
				break;
			case 1:
				m_HwType = TPCANType::PCAN_TYPE_ISA_SJA;
				break;
			case 2:
				m_HwType = TPCANType::PCAN_TYPE_ISA_PHYTEC;
				break;
			case 3:
				m_HwType = TPCANType::PCAN_TYPE_DNG;
				break;
			case 4:
				m_HwType = TPCANType::PCAN_TYPE_DNG_EPP;
				break;
			case 5:
				m_HwType = TPCANType::PCAN_TYPE_DNG_SJA;
				break;
			case 6:
				m_HwType = TPCANType::PCAN_TYPE_DNG_SJA_EPP;
				break;
		}
	}

	private: System::Void btnRelease_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		//Stop Timer
		tmrRead->Enabled = false;

		//Stop Thread
		if (m_ReadThread != nullptr)
		{
			m_ReadThread->Abort();
			m_ReadThread->Join();
			m_ReadThread = nullptr;
		}

		// Releases a current connected PCAN-Basic channel
		//
		PCANBasic::Uninitialize(*m_PcanHandle);

		// Sets the connection status of the main-form
		//
		SetConnectionStatus(false);
	}

	private: System::Void chbFilterExt_CheckedChanged(System::Object^  sender, System::EventArgs^  e) 
	{
		int iMaxValue;

		iMaxValue = (chbFilterExt->Checked) ? 0x1FFFFFFF : 0x7FF;

		// We check that the maximum value for a selected filter 
		// mode is used
		//
		if (nudIdTo->Value > iMaxValue)
			nudIdTo->Value = iMaxValue;
		nudIdTo->Maximum = iMaxValue;

        if (nudIdFrom->Value > iMaxValue)
            nudIdFrom->Value = iMaxValue;
        nudIdFrom->Maximum = iMaxValue;
	}

	private: System::Void btnFilterApply_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		UInt32 iBuffer;
		TPCANStatus ^stsResult;

		// Gets the current status of the message filter
		//
		if (!GetFilterStatus(iBuffer))
			return;

		// Configures the message filter for a custom range of messages
		//
		if (rdbFilterCustom->Checked)
		{
			// Sets the custom filter
			//
			stsResult = PCANBasic::FilterMessages(*m_PcanHandle, Convert::ToUInt32(nudIdFrom->Value), Convert::ToUInt32(nudIdTo->Value), chbFilterExt->Checked ? TPCANMode::PCAN_MODE_EXTENDED : TPCANMode::PCAN_MODE_STANDARD);
			// If success, an information message is written, if it is not, an error message is shown
			//
			if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
				IncludeTextMessage(String::Format("The filter was customized. IDs from {0:X} to {1:X} will be received", nudIdFrom->Text, nudIdTo->Text));
			else
				MessageBox::Show(GetFormatedError(stsResult));

			return;
		}

		 // The filter will be full opened or complete closed
		 //
		 if (rdbFilterClose->Checked)
			 iBuffer = PCANBasic::PCAN_FILTER_CLOSE;
		 else
			 iBuffer = PCANBasic::PCAN_FILTER_OPEN;

		 // The filter is configured
		 //
		 stsResult = PCANBasic::SetValue(*m_PcanHandle, TPCANParameter::PCAN_MESSAGE_FILTER, iBuffer, sizeof(UInt32));

		 // If success, an information message is written, if it is not, an error message is shown
		 //
		 if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
			 IncludeTextMessage(String::Format("The filter was successfully {0}", rdbFilterClose->Checked ? "closed." : "opened."));
		 else
			 MessageBox::Show(GetFormatedError(stsResult));
	}

	private: System::Void btnRead_Click(System::Object^  sender, System::EventArgs^  e) 
	{
        TPCANStatus ^stsResult;

        // We execute the "Read" function of the PCANBasic                
        //
        stsResult = m_IsFD ? ReadMessageFD() : ReadMessage();
		if (*stsResult != TPCANStatus::PCAN_ERROR_OK)
            // If an error occurred, an information message is included
            //
            IncludeTextMessage(GetFormatedError(stsResult));
	}

	private: System::Void btnMsgClear_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		// The information contained in the messages List-View
		// is cleared
		//
		Monitor::Enter(m_LastMsgsList->SyncRoot);
		try
		{
			lstMessages->Items->Clear();
			m_LastMsgsList->Clear();
		}
		finally
		{
			Monitor::Exit(m_LastMsgsList->SyncRoot);
		}
	}

	private: System::Void txtID_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) 
	{
		Char chCheck;

		// We convert the Character to its Upper case equivalent
		//
		chCheck = Char::ToUpper(e->KeyChar);

		// The Key is the Delete (Backspace) Key
		//
		if (chCheck == 8)
			return;
		// The Key is a number between 0-9
		//
		if ((chCheck > 47) && (chCheck < 58))
			return;
		// The Key is a character between A-F
		//
		if ((chCheck > 64) && (chCheck < 71))
			return;

		// Is neither a number nor a character between A(a) and F(f)
		//
		e->Handled = true;
	}

	private: System::Void txtID_Leave(System::Object^  sender, System::EventArgs^  e) 
	{
		int iTextLength;
		UInt32 uiMaxValue;

		// Calculates the text length and Maximum ID value according
		// with the Message Type
		//
		iTextLength = (chbExtended->Checked) ? 8 : 3;
		uiMaxValue = (chbExtended->Checked) ? safe_cast<UInt32>(0x1FFFFFFF) : safe_cast<UInt32>(0x7FF);

		// The Textbox for the ID is represented with 3 characters for 
		// Standard and 8 characters for extended messages.
		// Therefore if the Length of the text is smaller than TextLength,  
		// we add "0"
		//
		while (txtID->Text->Length != iTextLength)
			txtID->Text = ("0" + txtID->Text);

		// We check that the ID is not bigger than current maximum value
		//
		if (Convert::ToUInt32(txtID->Text, 16) > uiMaxValue)
			txtID->Text = String::Format("{0:X" + iTextLength.ToString() + "}", uiMaxValue);
	}

	private: System::Void chbExtended_CheckedChanged(System::Object^  sender, System::EventArgs^  e) 
	{
		UInt32 uiTemp;

		txtID->MaxLength = (chbExtended->Checked) ? 8 : 3;
		
		// the only way that the text length can be bigger als MaxLength
		// is when the change is from Extended to Standard message Type.
		// We have to handle this and set an ID not bigger than the Maximum
		// ID value for a Standard Message (0x7FF)
		//
		if (txtID->Text->Length > txtID->MaxLength)
		{
			uiTemp = Convert::ToUInt32(txtID->Text, 16);
			txtID->Text = (uiTemp < 0x7FF) ? String::Format("{0:X3}", uiTemp) : "7FF";
		}

		txtID_Leave(this, gcnew EventArgs());
	}

	private: System::Void chbRemote_CheckedChanged(System::Object^  sender, System::EventArgs^  e) 
	{
		TextBox ^txtbCurrentTextBox;

		txtbCurrentTextBox = txtData0;

        chbFD->Enabled = !chbRemote->Checked;
        if (chbRemote->Checked)
			chbBRS->Checked = false;

		// If the message is a RTR, no data is sent. The textboxes for data 
		// will be disabled
		// 
		for (int i = 0; i < nudLength->Value; i++)
		{
			txtbCurrentTextBox->Visible = !chbRemote->Checked;
			if (i < nudLength->Value)
				txtbCurrentTextBox = safe_cast<TextBox^>(this->Controls->Find("txtData" + i.ToString(), true)[0]);
		}
	}

	private: System::Void txtData0_Leave(System::Object^  sender, System::EventArgs^  e) 
	{
		TextBox ^txtbCurrentTextbox;

		// all the Textbox Data fields are represented with 2 characters.
		// Therefore if the Length of the text is smaller than 2, we add
		// a "0"
		//
		if (sender->GetType()->Name == "TextBox")
		{
			txtbCurrentTextbox = safe_cast<TextBox^>(sender);
			while (txtbCurrentTextbox->Text->Length != 2)
				txtbCurrentTextbox->Text = ("0" + txtbCurrentTextbox->Text);
		}
	}
	private: TPCANStatus ^WriteFrame()
	{
        TPCANMsg ^CANMsg;
        TextBox ^txtbCurrentTextBox;

        // We create a TPCANMsg message structure 
        //
        CANMsg = gcnew TPCANMsg();
        CANMsg->DATA = gcnew array<Byte>(8);

        // We configurate the Message.  The ID,
        // Length of the Data, Message Type
        // and die data
        //
		CANMsg->ID = Convert::ToUInt32(txtID->Text, 16);
		CANMsg->LEN = Convert::ToByte(nudLength->Value);
		CANMsg->MSGTYPE = (chbExtended->Checked) ? TPCANMessageType::PCAN_MESSAGE_EXTENDED : TPCANMessageType::PCAN_MESSAGE_STANDARD;
        // If a remote frame will be sent, the data bytes are not important.
        //
        if (chbRemote->Checked)
			CANMsg->MSGTYPE = CANMsg->MSGTYPE | TPCANMessageType::PCAN_MESSAGE_RTR;
        else
        {
            // We get so much data as the Len of the message
            //
            for (int i = 0; i < GetLengthFromDLC(CANMsg->LEN, true); i++)
            {
				txtbCurrentTextBox = safe_cast<TextBox^>(this->Controls->Find("txtData" + i.ToString(), true)[0]);
                CANMsg->DATA[i] = Convert::ToByte(txtbCurrentTextBox->Text, 16);
            }
        }

        // The message is sent to the configured hardware
        //
		return PCANBasic::Write(*m_PcanHandle, *CANMsg);

	}

	private: TPCANStatus ^WriteFrameFD()
	{
		TPCANMsgFD ^CANMsg;
        TextBox ^txtbCurrentTextBox;
        int iLength;

         // We create a TPCANMsg message structure 
         //
         CANMsg = gcnew TPCANMsgFD();
         CANMsg->DATA = gcnew array<Byte>(64);

        // We configurate the Message.  The ID,
        // Length of the Data, Message Type 
        // and die data
        //
		CANMsg->ID = Convert::ToUInt32(txtID->Text, 16);
		CANMsg->DLC = Convert::ToByte(nudLength->Value);
		CANMsg->MSGTYPE = (chbExtended->Checked) ? TPCANMessageType::PCAN_MESSAGE_EXTENDED : TPCANMessageType::PCAN_MESSAGE_STANDARD;
		CANMsg->MSGTYPE = CANMsg->MSGTYPE  | ((chbFD->Checked) ? TPCANMessageType::PCAN_MESSAGE_FD : TPCANMessageType::PCAN_MESSAGE_STANDARD);
		CANMsg->MSGTYPE = CANMsg->MSGTYPE | ((chbBRS->Checked) ? TPCANMessageType::PCAN_MESSAGE_BRS : TPCANMessageType::PCAN_MESSAGE_STANDARD);

		// If a remote frame will be sent, the data bytes are not important.
        //
        if (chbRemote->Checked)
			CANMsg->MSGTYPE = CANMsg->MSGTYPE | TPCANMessageType::PCAN_MESSAGE_RTR;
        else
        {
			// We get so much data as the Len of the message
            //
			iLength = GetLengthFromDLC(CANMsg->DLC, (CANMsg->MSGTYPE & TPCANMessageType::PCAN_MESSAGE_FD) == TPCANMessageType());
            for (int i = 0; i < iLength; i++)
            {
				txtbCurrentTextBox = safe_cast<TextBox^>(this->Controls->Find("txtData" + i.ToString(), true)[0]);
                CANMsg->DATA[i] = Convert::ToByte(txtbCurrentTextBox->Text, 16);
            }
        }

        // The message is sent to the configured hardware
        //
		return PCANBasic::WriteFD(*m_PcanHandle, *CANMsg);
	}

	private: System::Void btnWrite_Click(System::Object^  sender, System::EventArgs^  e) 
	{    
        TPCANStatus ^stsResult;

        // Send the message
        //
        stsResult = m_IsFD ? WriteFrameFD() : WriteFrame();

        // The message was successfully sent
        //
		if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
            IncludeTextMessage("Message was successfully SENT");
        // An error occurred.  We show the error.
        //			
        else
			MessageBox::Show(GetFormatedError(stsResult));
	}

	private: System::Void btnGetVersions_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		TPCANStatus ^stsResult;
		StringBuilder ^strTemp;
		array<String^> ^strArrayVersion;

		strTemp = gcnew StringBuilder(256);

		// We get the vesion of the PCAN-Basic API
		//
		stsResult = PCANBasic::GetValue(PCANBasic::PCAN_NONEBUS, TPCANParameter::PCAN_API_VERSION, strTemp, 256);
		if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
		{
			IncludeTextMessage("API Version: " + strTemp->ToString());

			// We get the version of the firmware on the device
			//
			stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_FIRMWARE_VERSION, strTemp, 256);
			if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
				IncludeTextMessage("Firmare Version: " + strTemp->ToString());

			// We get the driver version of the channel being used
			//
			stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_CHANNEL_VERSION, strTemp, 256);
			if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
			{
				// Because this information contains line control characters (several lines)
				// we split this also in several entries in the Information List-Box
				//
				strArrayVersion = strTemp->ToString()->Split(gcnew array<Char> { '\n' });
				IncludeTextMessage("Channel/Driver Version: ");
				for(int i =0; i < strArrayVersion->Length; i++)
					IncludeTextMessage("     * " + strArrayVersion[i]);
			}
		}

		// If an error ccurred, a message is shown
		//
		if (*stsResult != TPCANStatus::PCAN_ERROR_OK)
			MessageBox::Show(GetFormatedError(stsResult));
	}

	private: System::Void btnInfoClear_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		// The information contained in the Information List-Box 
		// is cleared
		//
		lbxInfo->Items->Clear();
	}

	private: System::Void Form1_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) 
	{
		// Releases the used PCAN-Basic channel
		//
		if(btnRelease->Enabled)
			btnRelease_Click(nullptr , nullptr);
	}

	private: System::Void tmrRead_Tick(System::Object^  sender, System::EventArgs^  e) 
	{
		// Checks if in the receive-queue are currently messages for read
		// 
		ReadMessages();
	}

	private: System::Void tmrDisplay_Tick(System::Object^  sender, System::EventArgs^  e) 
	{
		DisplayMessages();
	}

	private: System::Void chbShowPeriod_CheckedChanged(System::Object^  sender, System::EventArgs^  e) 
	{
		// According with the check-value of this checkbox,
		// the recieved time of a messages will be interpreted as 
		// period (time between the two last messages) or as time-stamp
		// (the elapsed time since windows was started)
		//
		Monitor::Enter(m_LastMsgsList->SyncRoot);
		try
		{
			for each (MessageStatus^ msg in m_LastMsgsList)
				msg->ShowingPeriod = chbShowPeriod->Checked;
		}
		finally
		{
			Monitor::Exit(m_LastMsgsList->SyncRoot);
		}
	}

	private: System::Void btnFilterQuery_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		UInt32 iBuffer;

		// Queries the current status of the message filter
		//
		if (GetFilterStatus(iBuffer))
		{
			switch(iBuffer)
			{
				// The filter is closed
				//
				case PCANBasic::PCAN_FILTER_CLOSE:
					IncludeTextMessage("The Status of the filter is: closed.");
					break;
				// The filter is fully opened
				//
				case PCANBasic::PCAN_FILTER_OPEN:
					IncludeTextMessage("The Status of the filter is: full opened.");
					break;
				// The filter is customized
				//
				case PCANBasic::PCAN_FILTER_CUSTOM:
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

	private: System::Void cbbParameter_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) 
	{
		// Activates/deactivates controls according with the selected 
		// PCAN-Basic parameter 
		//
		rdbParamActive->Enabled = ((dynamic_cast<ComboBox^>(sender))->SelectedIndex != 0) && ((dynamic_cast<ComboBox^>(sender))->SelectedIndex != 20);
		rdbParamInactive->Enabled = rdbParamActive->Enabled;
		nudDeviceIdOrDelay->Enabled = !rdbParamActive->Enabled;
		laDeviceOrDelay->Text = (cbbParameter->SelectedIndex == 20) ? "Delay (ms):" : "Device ID:";
	}

	private: System::Void btnParameterSet_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		TPCANStatus ^stsResult;
		UInt32 iBuffer;
		bool bActivate;

		bActivate = rdbParamActive->Checked;

		// Sets a PCAN-Basic parameter value
		//
		switch (cbbParameter->SelectedIndex)
		{
			// The device identifier of a channel will be set
			//
			case 0:
				iBuffer = Convert::ToUInt32(nudDeviceIdOrDelay->Value);
				stsResult = PCANBasic::SetValue(*m_PcanHandle, TPCANParameter::PCAN_DEVICE_ID, iBuffer, sizeof(UInt32));
				if(*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage("The desired Device-ID was successfully configured");
				break;
			// The 5 Volt Power feature of a channel will be set
			//
			case 1:
				iBuffer = safe_cast<UInt32>(bActivate ? PCANBasic::PCAN_PARAMETER_ON : PCANBasic::PCAN_PARAMETER_OFF);
				stsResult = PCANBasic::SetValue(*m_PcanHandle, TPCANParameter::PCAN_5VOLTS_POWER, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The USB/PC-Card 5 power was successfully {0}", bActivate ? "activated" : "deactivated"));
				break;
			// The feature for automatic reset on BUS-OFF will be set
			//
			case 2:
				iBuffer = safe_cast<UInt32>(bActivate ? PCANBasic::PCAN_PARAMETER_ON : PCANBasic::PCAN_PARAMETER_OFF);
				stsResult = PCANBasic::SetValue(*m_PcanHandle, TPCANParameter::PCAN_BUSOFF_AUTORESET, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The automatic-reset on BUS-OFF was successfully {0}", bActivate ? "activated" : "deactivated"));
				break;
			// The CAN option "Listen Only" will be set
			//
			case 3:
				iBuffer = safe_cast<UInt32>(bActivate ? PCANBasic::PCAN_PARAMETER_ON : PCANBasic::PCAN_PARAMETER_OFF);
				stsResult = PCANBasic::SetValue(*m_PcanHandle, TPCANParameter::PCAN_LISTEN_ONLY, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The CAN option \"Listen Only\" was successfully {0}", bActivate ? "activated" : "deactivated"));
				break;
			// The feature for logging debug-information will be set
			//
			case 4:
				iBuffer = safe_cast<UInt32>(bActivate ? PCANBasic::PCAN_PARAMETER_ON : PCANBasic::PCAN_PARAMETER_OFF);
				stsResult = PCANBasic::SetValue(PCANBasic::PCAN_NONEBUS, TPCANParameter::PCAN_LOG_STATUS, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The feature for logging debug information was successfully {0}", bActivate ? "activated" : "deactivated"));
				break;
            // The channel option "Receive Status" will be set
            //			
			case 5:
				iBuffer = safe_cast<UInt32>(bActivate ? PCANBasic::PCAN_PARAMETER_ON : PCANBasic::PCAN_PARAMETER_OFF);
				stsResult = PCANBasic::SetValue(*m_PcanHandle, TPCANParameter::PCAN_RECEIVE_STATUS, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The channel option \"Receive Status\" was set to {0}", bActivate ? "ON" : "OFF"));
				break;
            // The feature for tracing will be set
            //			
			case 7:
				iBuffer = safe_cast<UInt32>(bActivate ? PCANBasic::PCAN_PARAMETER_ON : PCANBasic::PCAN_PARAMETER_OFF);
				stsResult = PCANBasic::SetValue(*m_PcanHandle, TPCANParameter::PCAN_TRACE_STATUS, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The feature for tracing data was successfully {0}", bActivate ? "activated" : "deactivated"));
				break;
            // The feature for identifying an USB Channel will be set
            //			
			case 8:
				iBuffer = safe_cast<UInt32>(bActivate ? PCANBasic::PCAN_PARAMETER_ON : PCANBasic::PCAN_PARAMETER_OFF);
				stsResult = PCANBasic::SetValue(*m_PcanHandle, TPCANParameter::PCAN_CHANNEL_IDENTIFYING, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The procedure for channel identification was successfully {0}", bActivate ? "activated" : "deactivated"));
				break;
            // The feature for using an already configured speed will be set
            //
            case 10:
				iBuffer = safe_cast<UInt32>(bActivate ? PCANBasic::PCAN_PARAMETER_ON : PCANBasic::PCAN_PARAMETER_OFF);
				stsResult = PCANBasic::SetValue(*m_PcanHandle, TPCANParameter::PCAN_BITRATE_ADAPTING, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The feature for bit rate adaptation was successfully {0}", bActivate ? "activated" : "deactivated"));
                break;
			// The option "Allow Status Frames" will be set
            //
			case 17:
				iBuffer = safe_cast<UInt32>(bActivate ? PCANBasic::PCAN_PARAMETER_ON : PCANBasic::PCAN_PARAMETER_OFF);
				stsResult = PCANBasic::SetValue(*m_PcanHandle, TPCANParameter::PCAN_ALLOW_STATUS_FRAMES, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The reception of Status frames was successfully {0}", bActivate ? "enabled" : "disabled"));
				break;
			// The option "Allow RTR Frames" will be set
            //
			case 18:
				iBuffer = safe_cast<UInt32>(bActivate ? PCANBasic::PCAN_PARAMETER_ON : PCANBasic::PCAN_PARAMETER_OFF);
				stsResult = PCANBasic::SetValue(*m_PcanHandle, TPCANParameter::PCAN_ALLOW_RTR_FRAMES, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The reception of RTR frames was successfully {0}", bActivate ? "enabled" : "disabled"));
				break;
			// The option "Allow Error Frames" will be set
            //
			case 19:
				iBuffer = safe_cast<UInt32>(bActivate ? PCANBasic::PCAN_PARAMETER_ON : PCANBasic::PCAN_PARAMETER_OFF);
				stsResult = PCANBasic::SetValue(*m_PcanHandle, TPCANParameter::PCAN_ALLOW_ERROR_FRAMES, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The reception of Error frames was successfully {0}", bActivate ? "enabled" : "disabled"));
				break;
			// The option "Interframes Delay" will be set
			//
			case 20:
				iBuffer = Convert::ToUInt32(nudDeviceIdOrDelay->Value);
				stsResult = PCANBasic::SetValue(*m_PcanHandle, TPCANParameter::PCAN_INTERFRAME_DELAY, iBuffer, sizeof(UInt32));
				if(*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage("The delay between transmitting frames was successfully set");
				break;
                // The option "Allow Echo Frames" will be set
                //
            case 21:
                iBuffer = safe_cast<UInt32>(bActivate ? PCANBasic::PCAN_PARAMETER_ON : PCANBasic::PCAN_PARAMETER_OFF);
                stsResult = PCANBasic::SetValue(*m_PcanHandle, TPCANParameter::PCAN_ALLOW_ECHO_FRAMES, iBuffer, sizeof(UInt32));
                if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
                    IncludeTextMessage(String::Format("The reception of Echo frames was successfully {0}", bActivate ? "enabled" : "disabled"));
                break;
			// The current parameter is invalid
			//
			default:
				*stsResult = TPCANStatus::PCAN_ERROR_UNKNOWN;
				MessageBox::Show("Wrong parameter code.");
				return;
		}

		// If the function fail, an error message is shown
		//
		if(*stsResult != TPCANStatus::PCAN_ERROR_OK)
			MessageBox::Show(GetFormatedError(stsResult));
	}

	private: System::Void btnParameterGet_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		TPCANStatus ^stsResult;
		UInt32 iBuffer;
		StringBuilder ^strBuffer;

		strBuffer = gcnew StringBuilder(255);

		// Gets a PCAN-Basic parameter value
		//
		switch (cbbParameter->SelectedIndex)
		{
			// The device identifier of a channel will be retrieved
			//
			case 0:
				stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_DEVICE_ID, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The configured Device-ID is {0:X}",iBuffer));
				break;
				// The activation status of the 5 Volt Power feature of a channel will be retrieved
				//
			case 1:
				stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_5VOLTS_POWER, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The 5-Volt Power of the USB/PC-Card is {0:X}", (iBuffer == PCANBasic::PCAN_PARAMETER_ON) ? "ON" : "OFF"));
				break;
			// The activation status of the feature for automatic reset on BUS-OFF will be retrieved
			//
			case 2:
				stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_BUSOFF_AUTORESET, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The automatic-reset on BUS-OFF is {0:X}", (iBuffer == PCANBasic::PCAN_PARAMETER_ON) ? "ON" : "OFF"));
					break;
			// The activation status of the CAN option "Listen Only" will be retrieved
			//
			case 3:
				stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_LISTEN_ONLY, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The CAN option \"Listen Only\" is {0:X}", (iBuffer == PCANBasic::PCAN_PARAMETER_ON) ? "ON" : "OFF"));
				break;
			// The activation status for the feature for logging debug-information will be retrieved
			//
			case 4:
				stsResult = PCANBasic::GetValue(PCANBasic::PCAN_NONEBUS, TPCANParameter::PCAN_LOG_STATUS, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The feature for logging debug information is {0:X}", (iBuffer == PCANBasic::PCAN_PARAMETER_ON) ? "ON" : "OFF"));
				break;
            // The activation status of the channel option "Receive Status"  will be retrieved
            //			
			case 5:
				stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_RECEIVE_STATUS, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The channel option \"Receive Status\" is {0}", (iBuffer == PCANBasic::PCAN_PARAMETER_ON) ? "ON" : "OFF"));
				break;
			// The Number of the CAN-Controller used by a PCAN-Channel
			//
			case 6:
				stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_CONTROLLER_NUMBER, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The CAN Controller number is {0}",iBuffer));
				break;
			// The activation status for the feature for tracing data will be retrieved
			//
			case 7:
				stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_TRACE_STATUS, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The feature for tracing data is {0}",(iBuffer == PCANBasic::PCAN_PARAMETER_ON) ? "ON" : "OFF"));
				break;
			// The activation status of the Channel Identifying procedure will be retrieved
			//
			case 8:
				stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_CHANNEL_IDENTIFYING, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The identification procedure of the selected channel is {0}",(iBuffer == PCANBasic::PCAN_PARAMETER_ON) ? "ON" : "OFF"));
				break;
			// The activation status of the Channel Identifying procedure will be retrieved
			//
			case 9:
				stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_CHANNEL_FEATURES, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
				{
					IncludeTextMessage(String::Format("The channel {0} Flexible Data-Rate (CAN-FD)",((iBuffer & PCANBasic::FEATURE_FD_CAPABLE) == PCANBasic::FEATURE_FD_CAPABLE) ? "does support " : "DOESN'T SUPPORT"));
					IncludeTextMessage(String::Format("The channel {0} an inter-frame delay for sending messages",((iBuffer & PCANBasic::FEATURE_DELAY_CAPABLE) == PCANBasic::FEATURE_DELAY_CAPABLE) ? "does support " : "DOESN'T SUPPORT"));
					IncludeTextMessage(String::Format("The channel {0} using I/O pins",((iBuffer & PCANBasic::FEATURE_IO_CAPABLE) == PCANBasic::FEATURE_IO_CAPABLE) ? "does allow " : "DOESN'T ALLOW"));
				}
				break;
            // The status of the speed adapting feature will be retrieved
            //
            case 10:
				stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_BITRATE_ADAPTING, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The feature for bit rate adaptation is {0}", (iBuffer == PCANBasic::PCAN_PARAMETER_ON) ? "ON" : "OFF"));
                break;
            // The bitrate of the connected channel will be retrieved (BTR0-BTR1 value)
            //
            case 11:
				stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_BITRATE_INFO, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The bit rate of the channel is {0:X4}h", iBuffer));
                break;
            // The bitrate of the connected FD channel will be retrieved (String value)
            //
            case 12:
				stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_BITRATE_INFO_FD, strBuffer, 255);
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
				{
					IncludeTextMessage("The bit rate FD of the channel is represented by the following values:");
					for each (String^ strPart in strBuffer->ToString()->Split(','))
						IncludeTextMessage("   * " + strPart);
				}
                break;
			// The nominal speed configured on the CAN bus
			//
			case 13:
				stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_BUSSPEED_NOMINAL, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The nominal speed of the channel is {0} bit/s", iBuffer));
				break;
			// The data speed configured on the CAN bus
			//
			case 14:
				stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_BUSSPEED_DATA, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The data speed of the channel is {0} bit/s", iBuffer));
				break;
			// The IP address of a LAN channel as string, in IPv4 format
			//
			case 15:
				stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_IP_ADDRESS, strBuffer, 255);
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The IP address of the channel is {0}", strBuffer->ToString()));
				break;
			// The running status of the LAN Service
			//
			case 16:
				stsResult = PCANBasic::GetValue(PCANBasic::PCAN_NONEBUS, TPCANParameter::PCAN_LAN_SERVICE_STATUS, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The LAN service is {0}", (iBuffer == PCANBasic::SERVICE_STATUS_RUNNING) ? "running" : "NOT running"));
				break;
			// The reception of Status frames
            //
			case 17:
				stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_ALLOW_STATUS_FRAMES, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The reception of Status frames is {0}", (iBuffer == PCANBasic::PCAN_PARAMETER_ON) ? "enabled" : "disabled"));
				break;
			// The reception of RTR frames
            //
			case 18:
				stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_ALLOW_RTR_FRAMES, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The reception of RTR frames is {0}", (iBuffer == PCANBasic::PCAN_PARAMETER_ON) ? "enabled" : "disabled"));
				break;
			// The reception of Error frames
            //
			case 19:
				stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_ALLOW_ERROR_FRAMES, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The reception of Error frames is {0}", (iBuffer == PCANBasic::PCAN_PARAMETER_ON) ? "enabled" : "disabled"));
				break;
			// The Interframe delay of an USB channel will be retrieved
			//
			case 20:
				stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_INTERFRAME_DELAY, iBuffer, sizeof(UInt32));
				if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
					IncludeTextMessage(String::Format("The configured interframe delay is {0} ms",iBuffer));
				break;
                // The reception of Echo frames
                //
            case 21:
                stsResult = PCANBasic::GetValue(*m_PcanHandle, TPCANParameter::PCAN_ALLOW_ECHO_FRAMES, iBuffer, sizeof(UInt32));
                if (*stsResult == TPCANStatus::PCAN_ERROR_OK)
                    IncludeTextMessage(String::Format("The reception of Echo frames is {0}", (iBuffer == PCANBasic::PCAN_PARAMETER_ON) ? "enabled" : "disabled"));
                break;
			// The current parameter is invalid
			//
			default:
				*stsResult = TPCANStatus::PCAN_ERROR_UNKNOWN;
				MessageBox::Show("Wrong parameter code.");
				return;
		}

		// If the function fail, an error message is shown
		//
		if (*stsResult != TPCANStatus::PCAN_ERROR_OK)
			MessageBox::Show(GetFormatedError(stsResult));
	}

	private: System::Void nudLength_ValueChanged(System::Object^  sender, System::EventArgs^  e) 
	{
		TextBox ^CurrentTextBox;
		int iLength;

		CurrentTextBox = txtData0;
		iLength = GetLengthFromDLC((int)nudLength->Value, !chbFD->Checked);
		laLength->Text = String::Format("{0} B.", iLength);

		// We enable so much TextBox Data fields as the length of the
		// message will be, that is the value of the UpDown control.
		// 
		for(int i=0; i<= 64; i++)
		{
			CurrentTextBox->Enabled = i <= iLength;
			if(i < 64)
			    CurrentTextBox = safe_cast<TextBox^>(this->Controls->Find("txtData" + i.ToString(), true)[0]);
		}
	}

	private: System::Void btnReset_Click(System::Object^  sender, System::EventArgs^  e)
	{
		TPCANStatus ^stsResult;

		// Resets the receive and transmit queues of a PCAN Channel.
		//
		stsResult = PCANBasic::Reset(*m_PcanHandle);

		// If it fails, a error message is shown
		//
		if (*stsResult != TPCANStatus::PCAN_ERROR_OK)
			MessageBox::Show(GetFormatedError(stsResult));
		else
			IncludeTextMessage("Receive and transmit queues successfully reset");
	}

	private: System::Void buttonStatus_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		TPCANStatus ^status;
		String ^errorName;

		// Gets the current BUS status of a PCAN Channel.
		//
		status = PCANBasic::GetStatus(*m_PcanHandle);

		// Switch On Error Name
		//
		switch(*status)
		{
			case TPCANStatus::PCAN_ERROR_INITIALIZE:
				errorName = "PCAN_ERROR_INITIALIZE";
				break;

			case TPCANStatus::PCAN_ERROR_BUSLIGHT:
				errorName = "PCAN_ERROR_BUSLIGHT";
				break;

			case TPCANStatus::PCAN_ERROR_BUSHEAVY: // TPCANStatus::PCAN_ERROR_BUSWARNING
                errorName = m_IsFD ? "PCAN_ERROR_BUSWARNING" : "PCAN_ERROR_BUSHEAVY";
                break;

			case TPCANStatus::PCAN_ERROR_BUSPASSIVE: 
                errorName = "PCAN_ERROR_BUSPASSIVE";
                break;

			case TPCANStatus::PCAN_ERROR_BUSOFF:
				errorName = "PCAN_ERROR_BUSOFF";
				break;

			case TPCANStatus::PCAN_ERROR_OK:
				errorName = "PCAN_ERROR_OK";
				break;

			default:
				errorName = "See Documentation";
				break;
		}

		// Display Message
		//
		IncludeTextMessage(String::Format("Status: {0} ({1:X}h)", errorName, *status));
	}

	private: System::Void chbCanFD_CheckedChanged(System::Object^  sender, System::EventArgs^  e) 
	{
        m_IsFD = chbCanFD->Checked;

        cbbBaudrates->Visible = !m_IsFD;
        cbbHwType->Visible = !m_IsFD;
        cbbInterrupt->Visible = !m_IsFD;
        cbbIO->Visible = !m_IsFD;
        laBaudrate->Visible = !m_IsFD;
        laHwType->Visible = !m_IsFD;
        laIOPort->Visible = !m_IsFD;
        laInterrupt->Visible = !m_IsFD;

        txtBitrate->Visible = m_IsFD;
        laBitrate->Visible = m_IsFD;
        chbFD->Visible = m_IsFD;
        chbBRS->Visible = m_IsFD;

        if ((nudLength->Maximum > 8) && !m_IsFD)
            chbFD->Checked = false;
	}

	private: System::Void chbFD_CheckedChanged(System::Object^  sender, System::EventArgs^  e) 
	{
        chbRemote->Enabled = !chbFD->Checked;
        chbBRS->Enabled = chbFD->Checked;
        if (!chbBRS->Enabled)
            chbBRS->Checked = false;
        nudLength->Maximum = chbFD->Checked ? 15 : 8;
	}

	private: System::Void lstMessages_DoubleClick(System::Object^  sender, System::EventArgs^  e) 
	{
	    // Clears the content of the Message List-View
        //
        btnMsgClear_Click(this, gcnew EventArgs());
	}
#pragma endregion

#pragma endregion
};
}

