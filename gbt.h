#ifndef GBT_H
#define GBT_H

#include "IPbusHeaders.h"

struct GBTunit { // 32 registers * 4 bytes = 128 bytes
	union ControlData {
        quint32 registers[16] = {0};
        char pointer[16 * 4]; // 64 bytes
		struct {
			quint32
                DG_MODE              :  4, //┐
                TG_MODE              :  4, //│
                RESET				 :  8, //│
                SEND_READOUT_COMMAND :  4, //│
                HB_RESPONSE			 :  1, //│D8
                BYPASS_MODE			 :  1, //│
                READOUT_LOCK		 :  1, //│
                                     :  9, //┘
                DG_TRG_RESPOND_MASK,       //]D9
                DG_BUNCH_PATTERN,          //]DA
                TG_SINGLE_VALUE,           //]DB
                TG_PATTERN_1,              //]DC
                TG_PATTERN_0,              //]DD
                TG_CONT_VALUE,             //]DE
                DG_BUNCH_FREQ        : 16, //┐
                TG_BUNCH_FREQ        : 16, //┘DF
                DG_FREQ_OFFSET       : 12, //┐
                                     :  4, //│
                TG_FREQ_OFFSET       : 12, //│E0
                                     :  4, //┘
                RDH_PAR              : 16, //┐
                RDH_FEE_ID           : 16, //┘E1
                RDH_DET_FIELD        : 16, //┐
                RDH_MAX_PAYLOAD      : 16, //┘E2
                BCID_DELAY           : 12, //┐
                                     :  4, //│
                CRU_TRG_COMPARE_DELAY: 12, //│E3
                                     :  4, //┘
                DATA_SEL_TRG_MASK        , //]E4
                reserved[3]              ; //]E5-E7
		};
	} Control;
    union StatusData {
        quint32 registers[16] = {0};
        char pointer[16 * 4]; // 64 bytes
        struct {
            quint32
                BITS                        : 16, //┐
                READOUT_MODE                :  4, //│
                BCID_SYNC_MODE              :  4, //│E8
                RX_PHASE                    :  4, //│
                                            :  4, //┘
                CRU_ORBIT                   : 32, //┐
                CRU_BC                      : 12, //┘E9
                                            : 20, //┐
                RAW_FIFO                    : 16, //┘EA
                SEL_FIFO                    : 16, //]EB
                SEL_FIRST_HIT_DROPPED_ORBIT,      //]EC
                SEL_LAST_HIT_DROPPED_ORBIT,       //]ED
                SEL_HITS_DROPPED,                 //]EE
                READOUT_RATE                : 16, //┐
                                            : 16, //┘EF
                reserved[8]                     ; //]F0-F7
        };
    } Status;
    static const quint8
        controlSize   =   13,
        statusSize    =    8,
        controlAddress= 0xD8,
        statusAddress = 0xE8,
    //data generator states
        DG_noData	= 0,
        DG_main		= 1,
        DG_Tx		= 2,
    //trigger generator states
        TG_noTrigger  = 0,
        TG_continuous = 1,
    //readout commands
        RC_SOC = 1,
        RC_SOT = 2,
        RC_EOC = 3,
        RC_EOT = 4,
    //readout modes
        RO_idle       = 0,
        RO_continuous = 1,
        RO_trigger    = 2,
    //BCID sync modes
        BS_start = 0,
        BS_sync  = 1,
        BS_lost  = 2,
    //positions of reset bits
        RB_orbitSync			=  8,
        RB_droppingHitCounters	=  9,
        RB_generatorsBunchOffset= 10,
        RB_GBTerrors			= 11,
        RB_GBT					= 12,
        RB_RXphaseError			= 13;
};

#endif // GBT_H
