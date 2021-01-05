#include "fee.h"

FEE::FEE(){
   initTCMRegs();
   initPMregs();
   connect(&timer, &QTimer::timeout, this, &FEE::updateRegisters);
   timer.start(1000);
}

quint32 FEE::getValue(quint16 address){
    return isFIFO(address) ? (this->T0_Map.getFIFOpointer(address)->isEmpty() ? 0 : this->T0_Map.getFIFOpointer(address)->head()) : this->T0_Map[address];
}

void FEE::write_word(quint32 data, quint16 address){
    if(isFIFO(address)) this->T0_Map.getFIFOpointer(address)->enqueue(data);
    else  this->T0_Map[address] = data;
}

quint32 FEE::read_word(quint16 address){
    if(!registerIsAvailable(address)) this->T0_Map[address] = QRandomGenerator::global()->generate() % 2 ? 0x0 : 0xFFFFFFFF;
    return isFIFO(address) ? (this->T0_Map.getFIFOpointer(address)->isEmpty() ? 0 : this->T0_Map.getFIFOpointer(address)->dequeue()) : this->T0_Map[address];
}

bool FEE::registerIsAvailable(quint16 address){
    if(address < 0x200) return !nonavailableTCMregs.contains(address);
    else{
        if(availPMs & (1 << (address / 0x200 - 1))) return !nonavailablePMregs.contains(address - (address / 0x200) * 0x200);
        else return false;
    }
}


void FEE::initTCMRegs(){
    //non-available regs
        nonavailableTCMregs.append(0x6);
    for(quint8 i = 0; i < 18; ++i)
        nonavailableTCMregs.append(0x1E + i);
    for(quint8 i = 0; i < 21; ++i)
        nonavailableTCMregs.append(0x3B + i);
    for(quint8 i = 0; i < 15; ++i)
        nonavailableTCMregs.append(0x51 + i);
    for(quint8 i = 0; i < 5; ++i)
        nonavailableTCMregs.append(0x6B + i);
    for(quint8 i = 0; i < 89; ++i)
        nonavailableTCMregs.append(0x7F + i);
    for(quint8 i = 0; i < 254; ++i)
        nonavailableTCMregs.append(0x102 + i);
}

void FEE::initPMregs(){
    nonavailablePMregs.append(0x7E);
    nonavailablePMregs.append(0xBE);
    nonavailablePMregs.append(0xBF);
    for(quint8 i = 0; i < 253; ++i)
        nonavailablePMregs.append(0x103 + i);
}

void FEE::updateRegisters(){
//    TCM
    T0_Map.TEMPERATURE = static_cast<qint32>(*BoardTemperaturesList.at(0));
    T0_Map.FPGA_TEMP   = static_cast<quint32>(*FpgaTemperaturesList .at(0));
    T0_Map.POWER_1V    = static_cast<quint32>(*FpgaPower1Vlist      .at(0));
    T0_Map.POWER_1_8V  = static_cast<quint32>(*FpgaPower1_8Vlist    .at(0));
//    PMs
    for(quint8 pm_no = 0; pm_no < 20; ++pm_no){
        if(registerIsAvailable(0x200 + pm_no)){
            T0_Map.PM[pm_no].TEMPERATURE = static_cast<quint32>(*BoardTemperaturesList.at(pm_no + 1));
            T0_Map.PM[pm_no].FPGA_TEMP   = static_cast<quint32>(*FpgaTemperaturesList .at(pm_no + 1));
            T0_Map.PM[pm_no].POWER_1V    = static_cast<quint32>(*FpgaPower1Vlist      .at(pm_no + 1));
            T0_Map.PM[pm_no].POWER_1_8V  = static_cast<quint32>(*FpgaPower1_8Vlist    .at(pm_no + 1));
        }
    }

}

bool FEE::isFIFO(quint16 address){
    return FIFOregslist.contains(address);
}




