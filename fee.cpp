#include "fee.h"

FEE::FEE(){
   initTCMRegs();
   initPMregs();
   connect(&timer, &QTimer::timeout, this, &FEE::updateRegisters);
   timer.start(1000);
}

bool FEE::registerExists(quint32 address){
    return address <= sizeof(T0_Map) / 4 - 3;
}

bool FEE::getValue(quint16 address, quint32 &target, Log *log, TransactionType type){
    if(registerExists(address)){
        target = isFIFO(address) ? (this->T0_Map.getFIFOpointer(address)->isEmpty() ? 0 : this->T0_Map.getFIFOpointer(address)->head()) : ( registerIsAvailable(address) ? this->T0_Map[address] : QRandomGenerator::global()->generate() % 2 ? 0x0 : 0xFFFFFFFF);
        return true;
    }else{
        if(log)log->sendMessage(QString::asprintf("getting value from non-existing 0x%08X (%s): forbidden", address, operationTypeString(type).toUtf8().data()), file);
        return false;
    }
}

bool FEE::writeWord(quint32 data, quint16 address, Log *log, TransactionType type){
   if(registerExists(address) && !isReadOnly(address)){
       if(type == nonIncrementingWrite){
           if(isFIFO(address)) this->T0_Map.getFIFOpointer(address)->enqueue(data);
           else{ T0_Map[address] = isHalfed(address) ? (0xFFFF & data) : data; data = T0_Map[address];} //getting value which we really wrote to register
           if(log)log->sendMessage(QString::asprintf("writing 0x%08X to 0x%08X", data, address) + ((type==RMWbits || type == RMWsum) ? " (RMW operation)": ""), file);
           return true;
       }else{
           if(isFIFO(address)){
               if(log)log->sendMessage(QString::asprintf("writing 0x%08X to %s 0x%08X: forbidden", data, "FIFO" , address), file);
               return false;
           }else{
               T0_Map[address] = isHalfed(address) ? (0xFFFF & data) : data;
               if(log)log->sendMessage(QString::asprintf("writing 0x%08X to 0x%08X", T0_Map[address], address) + ((type==RMWbits || type == RMWsum) ? " (RMW operation)": ""), file);
               return true;
           }
       }
   }else{
       if(log)log->sendMessage(QString::asprintf("writing 0x%08X to %s 0x%08X: forbidden", isHalfed(address) ? (0xFFFF & data) : data, isReadOnly(address) ? "read-only" : "non-existing" , address), file);
       return false;
   }
}

bool FEE::readWord(quint16 address, quint32 &target, Log *log){
    if(registerExists(address)){
        if(!registerIsAvailable(address)) this->T0_Map[address] = QRandomGenerator::global()->generate() % 2 ? 0x0 : 0xFFFFFFFF;
        target =  isFIFO(address) ? (this->T0_Map.getFIFOpointer(address)->isEmpty() ? 0 : this->T0_Map.getFIFOpointer(address)->dequeue()) : this->T0_Map[address];
        return true;
    }else{
        if(log)log->sendMessage(QString::asprintf("reading from %s 0x%08X: forbidden", "non-existing" , address), file);
        return false;
    }
}

//quint32 FEE::getValue(quint16 address){
//    return isFIFO(address) ? (this->T0_Map.getFIFOpointer(address)->isEmpty() ? 0 : this->T0_Map.getFIFOpointer(address)->head()) : this->T0_Map[address];
//}

//void FEE::writeWord(quint32 data, quint16 address){
//    if(isFIFO(address)) this->T0_Map.getFIFOpointer(address)->enqueue(data);
//    else  this->T0_Map[address] = data;
//}

//quint32 FEE::read_word(quint16 address){
//    if(!registerIsAvailable(address)) this->T0_Map[address] = QRandomGenerator::global()->generate() % 2 ? 0x0 : 0xFFFFFFFF;
//    return isFIFO(address) ? (this->T0_Map.getFIFOpointer(address)->isEmpty() ? 0 : this->T0_Map.getFIFOpointer(address)->dequeue()) : this->T0_Map[address];
//}



bool FEE::registerIsAvailable(quint32 address){ //case when the register is not initialised
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

    //readonly regs
    readOnlyTCMList.append(0x5); //Temperature
    readOnlyTCMList.append(0x7); //Microcode & serial number
    for(quint8 i = 0; i < 10; ++i){ //PM Links
        readOnlyTCMList.append(0x10 + i);
        readOnlyTCMList.append(0x30 + i);}
    for(quint8 i = 0; i < 16; ++i)  //GBTstatus
        readOnlyTCMList.append(0xE8 + i);
}

void FEE::initPMregs(){
    //non-available regs
    nonavailablePMregs.append(0x7E);
    nonavailablePMregs.append(0xBE);
    nonavailablePMregs.append(0xBF);
    for(quint8 i = 0; i < 253; ++i)
        nonavailablePMregs.append(0x103 + i);

    //readonly regs
    for(quint8 i = 0; i < 24; ++i) //ADC baselines
        readOnlyPMList.append(0xD + i);
    readOnlyPMList.append(0x3E);   //TDC1 TDC2 Phase tuning
    readOnlyPMList.append(0x3F);   //TDC3 Phase tuning
    for(quint8 i = 0; i < 12; ++i)
        readOnlyPMList.append(0x40 + i); //RAW TDC data
    for(quint8 i = 0; i < 24; ++i){
        readOnlyPMList.append(0x4C + i); //ADCs dispersion
        readOnlyPMList.append(0xC0 + i); // CFD & TRG counters
        readOnlyPMList.append(0x64 + i);} //Mean Amplitudes
    readOnlyPMList.append(0x7D); //Channel ADC baseline out of range
    readOnlyPMList.append(0xBC); //Board Temperature
    readOnlyPMList.append(0xBD); //Microcode & serial number
    for(quint8 i = 0; i < 16; ++i)  //GBTstatus
        readOnlyPMList.append(0xE8 + i);
}

void FEE::updateRegisters(){
//    TCM
    T0_Map.TEMPERATURE = static_cast<qint32> (*BoardTemperaturesList.at(0));
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

bool FEE::isFIFO(quint32 address){
    return FIFOregslist.contains(address);
}

bool FEE::isReadOnly(quint32 address){
    quint8 pmNo = address / 0x200;
    if(pmNo) return readOnlyPMList.contains(address % 0x200);
    else return readOnlyTCMList.contains(address);
}

bool FEE::isHalfed(quint32 address){
    bool PM = address / 0x200;
    return PM ? address % 0x200 < 0xC0 : address < 0xE;
}



