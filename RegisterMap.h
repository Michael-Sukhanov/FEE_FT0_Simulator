#ifndef REGISTERMAP_H
#define REGISTERMAP_H

#include "gbt.h"
#include <QQueue>

struct HDMIlinkStatus {
    quint32
        line0delay          : 5,
        line0signalLost     : 1,
        line0signalStable   : 1,
                            : 1,
        line1delay          : 5,
        line1signalLost     : 1,
        line1signalStable	: 1,
                            : 1,
        line2delay          : 5,
        line2signalLost     : 1,
        line2signalStable	: 1,
        bitPositionsOK		: 1,
        line3delay          : 5,
        line3signalLost     : 1,
        line3signalStable	: 1,
        linkOK              : 1;
    HDMIlinkStatus& operator=(const quint32 &val){
        *reinterpret_cast<quint32 *>(this) = val;
        return *this;
    }
    operator quint32(){return *reinterpret_cast<quint32 *>(this);
    }

};

//const quint32 Ok_link =0xD5D25150;
//const quint32 Bad_link=0x30B03030;

struct TypePM{
    quint32 OR_GATE               ,     //]00
            TIME_ALIGN  [12]      ,     //]01-0C
            ADC_BASELINE[12][2]   ,     //]0D-24 //[Ch][0] for ADC0, [Ch][1] for ADC1
            ADC_RANGE   [12][2]   ,     //]25-3C
            CFD_SATR              ;     //]3D
    qint32  TDC1tuning         : 8,     //┐
            TDC2tuning         : 8,     //│3E
                               :16,     //┘
            TDC3tuning         : 8,     //┐
                               :24;     //┘3F
    quint8  RAW_TDC_DATA[12][4]   ;     //]40-4B //[Ch][0] for val1, [Ch][1] for val2
    quint32 DISPERSION  [12][2]   ;     //]4C-63
    qint16  MEANAMPL    [12][2][2];     //]64-7B //[Ch][0][0] for ADC0, [Ch][1][0] for ADC1
    quint32 CH_MASK               ,     //]7C
            CH_DISPLACED          ,     //]7D
            reserved,                   //]7E
      mainPLLlocked      : 1,           //┐
         TDC1PLLlocked   : 1,           //│
         TDC2PLLlocked   : 1,           //│
         TDC3PLLlocked   : 1,           //│
         GBTlinkPresent  : 1,           //│
         GBTreceiverError: 1,           //│
         TDC1syncError   : 1,           //│
         TDC2syncError   : 1,           //│
         TDC3syncError   : 1,           //│7F
         RESET_COUNTERS  : 1,           //│
                         : 2,           //│
         GBTRxPhaseError : 1,           //│
         BCIDsyncLost    : 1,           //│
         droppingHits    : 1,           //│
                         :17;           //┘
    struct ChannelSettings{             //┐
        quint32 CFD_THRESHOLD :16,      //│
                              :16;      //│
        qint32  CFD_ZERO      :16,      //│
                              :16,      //│
                ADC_ZERO      :16,      //│80-AF
                              :16;      //│
        quint32 ADC_DELAY     :16,      //│
                              :16;      //│
    } Ch[12];                           //┘
    quint32 THRESHOLD_CALIBR[12],        //]B0-BB
            TEMPERATURE;                 //]BC
    quint32 MICROCODE          : 8,     //┐
            SERIAL_NUM         : 8,     //│BD
                               :16,     //┘
            reserved1[2]          ;     //]BE-BF
    struct{ quint32 CNT_CFD,            //┐
                    CNT_TRG;            //│C0-D7
    } Counters[12];                     //┘

    GBTunit GBT;                        //]D8-F7
    quint32 FW_command,                 //┐
            FW_data,                    //│
            FW_end_byte_address,        //│F8-FB
            FW_status;                  //┘
    quint32 FPGA_TEMP,                  //]FC
            POWER_1V,                   //]FD
            POWER_1_8V,                 //]FE
            FW_SECOND         : 6,      //┐
            FW_MINUTE         : 6,      //│
            FW_HOUR           : 5,      //│FF
            FW_YEAR           : 6,      //│
            FW_MONTH          : 4,      //│
            FW_DAY            : 5;      //┘
    quint32 COUNTER_VALUES_READOUT;                                 //┐
    quint32 COUNTERS_FIFO_LOAD: 9, : 23,                            //┘100-101
            reserved3[254];

};

struct RegisterMap{
        qint32  DELAY_A         :16, //┐
                                :16, //┘00
                DELAY_C         :16, //┐
                                :16, //┘01
                LASER_DELAY     :16, //┐
                                :16; //┘02
        quint32 ATTEN_VALUE     :13, //┐
                                : 1, //│
                attenBusy       : 1, //│03
                attenNotFound   : 1, //│
                                :16, //┘
                EXT_SW          : 4, //┐
                                :28; //┘04
        qint32  TEMPERATURE     :16, //┐
                                :16, //┘05
                                :32; //]06
        quint32 MICROCODE       :8,  //┐
                SERIAL_NUM      :8,  //│07
                                :16; //┘
        qint32  VTIME_LOW       : 9, //┐
                                :23, //┘08
                VTIME_HIGH      : 9, //┐
                                :23; //┘09
        quint32 SC_LEVEL_A      :16, //┐
                                :16, //┘0A
                SC_LEVEL_C      :16, //┐
                                :16, //┘0B
                C_LEVEL_A       :16, //┐
                                :16, //┘0C
                C_LEVEL_C       :16, //┐
                                :16, //┘0D
                ADD_C_DELAY     : 1, //┐
                C_SC_TRG_MODE   : 2, //│0E
                EXTENDED_READOUT: 1, //│
                                :28, //┘
                PLLlockC        : 1, //┐
                PLLlockA        : 1, //│
                systemRestarted : 1, //│
                externalClock   : 1, //│
                GBTRxReady      : 1, //│
                GBTRxError      : 1, //│
                GBTRxPhaseError : 1, //│0F
                BCIDsyncLost    : 1, //│
                droppingHits    : 1, //│
                resetCounters   : 1, //│
                forceLocalClock : 1, //│
                resetSystem     : 1, //│
                PMstatusChanged :20; //┘
        HDMIlinkStatus PM_LINK_A[10];//]10-19
        quint32 CH_MASK_A       :10, //┐
                                : 7, //│
                syncErrorInLinkA:10, //│
                masterLinkErrorA: 1, //│1A
                sideAenabled    : 1, //│
                delayRangeErrorA: 1, //│
                readinessChangeA: 1, //│
                sideAready      : 1, //┘
                LASER_DIV       :24, //┐
                                : 7, //│1B
                LASER_ON        : 1, //┘
                LASER_PATTERN_1,     //]1C
                LASER_PATTERN_0,     //]1D
                reserved1[18];       //]1E-2F
        HDMIlinkStatus PM_LINK_C[10];//]30-39
        quint32 CH_MASK_C       :10, //┐
                                : 7, //│
                syncErrorInLinkC:10, //│
                masterLinkErrorC: 1, //│
                sideCenabled    : 1, //│3A
                delayRangeErrorC: 1, //│
                readinessChangeC: 1, //│
                sideCready      : 1, //┘
                reserved2[21],       //]3B-4F
                COUNTERS_UPD_RATE,   //]50
                reserved3[15];       //]51-5F
        quint32 ORA_SIGN        :14, //┐
                                :18, //┘60
                ORA_RATE        :31, //┐
                                : 1, //┘61
                ORC_SIGN        :14, //┐
                                :18, //┘62
                ORC_RATE        :31, //┐
                                : 1, //┘63
                SC_SIGN         :14, //┐
                                :18, //┘64
                SC_RATE         :31, //┐
                                : 1, //┘65
                C_SIGN          :14, //┐
                                :18, //┘66
                C_RATE          :31, //┐
                                : 1, //┘67
                V_SIGN          :14, //┐
                                :18, //┘68
                V_RATE          :31, //┐
                                : 1, //┘69
                ORA_MODE        : 2, //┐
                ORA_ENABLED     : 1, //│
                ORC_MODE        : 2, //│
                ORC_ENABLED     : 1, //│
                SC_MODE         : 2, //│6A
                SC_ENABLED      : 1, //│
                C_MODE          : 2, //│
                C_ENABLED       : 1, //│
                V_MODE          : 2, //│
                V_ENABLED       : 1, //│
                                :17; //┘
        quint32 reserved4[5];        //]6B - 6F
        quint32 CNT_OR_A,                               //┐
                CNT_OR_C,                               //│
                CNT_SC,                                 //│
                CNT_C,                                  //│
                CNT_V,                                  //│
                CNT_bgA,                                //│
                CNT_bgC,                                //│
                CNT_bgA_and_bgC,                        //│70-7E
                CNT_bgA_or_bgC,                         //│
                CNT_orA_or_orC,                         //│
                CNT_orA_and_orC,                        //│
                CNT_bgA_and_not_orA,                    //│
                CNT_bgC_and_not_orC,                    //│
                CNT_bgA_and_not_orA_OR_bgC_and_not_orC, //│
                CNT_bgA_and_not_orA_AND_bgC_and_not_orC;//┘
        quint32 reserved5[89];
        GBTunit GBT;                 //]D8-F7
        quint32 reserved6[4];
        quint32 FPGA_TEMP,           //]FC
                POWER_1V,            //]FD
                POWER_1_8V,          //]FE
                FW_SECOND       : 6, //┐
                FW_MINUTE       : 6, //│
                FW_HOUR         : 5, //│FF
                FW_YEAR         : 6, //│
                FW_MONTH        : 4, //│
                FW_DAY          : 5; //┘
        quint32 COUNTER_VALUES_READOUT;
        quint32 COUNTERS_FIFO_LOAD : 10, : 22;
        quint32 reserved7[254];
        TypePM  PM[20];
        quint32 &operator[] (quint16 address){
            return *(reinterpret_cast<quint32 *>(this) + address);
            }
        QQueue<quint32>* getFIFOpointer(quint16 address){
            quint8 part = address / 0x200;
            return &FIFOS[part];
        }
        QQueue<quint32>* FIFOS = new QQueue<quint32>[21];
};


#endif // FEE_H
