#include "fee.h"

FEE::FEE(){
   initTCMRegs();
   initPMregs();
   connect(&timer, &QTimer::timeout, this, &FEE::updateRegisters);
   timer.start(1000);
}

bool FEE::registerIsAvailable(quint32 address){ //case when the register is not initialised
    if(isPM(address)) return !nonavailablePMregs.contains(address % firstPMaddress);
    else return !(
                   nonavailableTCMregs.contains(address)          ||
                  (address >= 0x102 && address <= 0x1FF)          ||
                  (address > firstPMaddress && address <= 0x29FF && (address % firstPMaddress) >= 0x104) ||
                  (address >= 0x5000) || (address >= 0x2A00 && address <= 0x3FFF)
                );
}

quint32 FEE::Correct(quint32 address, quint32 value){
    quint16 addressInPM = address % 0x200;
    if(isPM(address) && addressInPM > 0x80 && addressInPM < 0xBB){
        if(addressInPM <0xB0){
            switch (addressInPM % 4) {
            case 0:
                return value > 300 ? (value < 30000 ? value : 30000) : 300;
            case 1:
            case 2:
                return static_cast<qint16>(value) > -500 ? (static_cast<qint16>(value) < 500 ? value : 500) : static_cast<quint16>(-500);
            case 3:
                return value < 20000 ? value : 20000;
            default:
                return value;
            }
        }else
             return value < 4000 ? value : 4000;
    }else
        return value;
}

//helping function while testing
bool FEE::setRegisterHard(quint32 address, quint32 value){
    if(!registerIsAvailable(address)) return false;
    else this->T0_Map[address] = value;
    return true;
}


quint8 FEE::writeWord(quint16 address, quint32 data, Log *log, TransactionType type){
    bool itIsPM        = isPM(address),
         itIsExists    = registerIsAvailable(address),
         SPIok         = isSPIEnabled(getPmNo(address)),
         itIsConnected = isConnectedPM(getPmNo(address)),
         itIs16bits    = is16bit(address),
         itIsFIFO      = isFIFO(address),
         itIsReadonly  = isReadOnly(address);
    QString logMessage;
    quint8 InfoCode;
    if(itIsPM){
       if(SPIok){
           if(itIsConnected){
               if(itIsReadonly || itIsFIFO){
                   logMessage = QString::asprintf("writing 0x%08X to read-only 0x%08X: forbidden", data, address);
                   InfoCode = 0x5;
               }else{
                   this->T0_Map[address] = itIsExists ? (itIs16bits ? Correct(address,data) % 0xFFFF : Correct(address,data)) : 0xFFFFF;
                   logMessage = QString::asprintf("writing 0x%08X to 0x%08X", this->T0_Map[address], address) + ((type == RMWbits || type == RMWsum) ? " (RMW operation)": "");
                   InfoCode = 0x0;
               }
           }else{
               logMessage = QString::asprintf("writing 0x%08X to 0x%08X", data, address) + ((type == RMWbits || type == RMWsum) ? " (RMW operation)": "");
               InfoCode = 0x0;
           }
       }else{
           logMessage = QString::asprintf("writing 0x%08X to SPI locked 0x%08X: forbidden", data, address);
           InfoCode = 0x5;
       }
    }else{
        if(itIsExists){
            if(itIsReadonly || itIsFIFO){
                logMessage = QString::asprintf("writing 0x%08X to read-only 0x%08X: forbidden", data, address);
                InfoCode = 0x5;
            }else{
                this->T0_Map[address] = itIs16bits ? data % 0xFFFF : data;
                logMessage = QString::asprintf("writing 0x%08X to 0x%08X", this->T0_Map[address], address) + ((type == RMWbits || type == RMWsum) ? "(RMW operation)": "");
                InfoCode = 0x0;
            }
        }else{
            logMessage = QString::asprintf("writing 0x%08X to non-existent 0x%08X: forbidden", data, address);
            InfoCode = 0x7;
        }

    }
    if(log) log->sendMessage(logMessage, file);
    return InfoCode;
}

quint8 FEE::readWord(quint16 address, quint32 &target, Log *log){
    if(isFIFO(address)){
        target = this->T0_Map.getFIFOpointer(address)->isEmpty() ? 0 : this->T0_Map.getFIFOpointer(address)->dequeue();
//        log->sendMessage(QString::asprintf("reading from 0x%08X (%s)", address, operationTypeString(type).toUtf8().data()));
        return 0x0;
    }
    //getting all information about register
    bool itIsPM        = isPM(address),
         itIsExists    = registerIsAvailable(address),
         SPIok         = isSPIEnabled(getPmNo(address)),
         itIsConnected = isConnectedPM(getPmNo(address)),
         itIs16bits    = is16bit(address);
    QString logMessage;
    quint8 InfoCode;
    if(itIsPM){
        if(SPIok){
            if(itIsConnected)
                target = itIsExists ? this->T0_Map[address] :  0xFFFF;
            else
                target = itIs16bits ? 0xFFFF : 0XFFFFFFFF;
//            logMessage = QString::asprintf("reading from 0x%08X (%s)", address, operationTypeString(type).toUtf8().data());
            InfoCode = 0x0;
        }else{
            logMessage = QString::asprintf("reading from 0x%08X: forbidden - SPI locked", address);
            InfoCode = 0x4;
        }
    }else{
        if(itIsExists){
            target = this->T0_Map[address];
//            logMessage = QString::asprintf("reading from 0x%08X (%s)", address, operationTypeString(type).toUtf8().data());
            InfoCode = 0x0;
        }else{
            logMessage = QString::asprintf("reading from non-existent 0x%08X: forbidden", address);
            InfoCode = 0x6;
        }
    }
    if(log && !logMessage.isEmpty()) log->sendMessage(logMessage, file);
    return InfoCode;
    }


void FEE::initTCMRegs(){
    //non-available regs
    for(quint8 i = 0; i < 17; ++i)
        nonavailableTCMregs.append(0x1F + i);
    for(quint8 i = 0; i < 21; ++i)
        nonavailableTCMregs.append(0x3B + i);
    for(quint8 i = 0; i < 15; ++i)
        nonavailableTCMregs.append(0x51 + i);
    for(quint8 i = 0; i < 5; ++i)
        nonavailableTCMregs.append(0x6B + i);
    for(quint8 i = 0; i < 89; ++i)
        nonavailableTCMregs.append(0x7F + i);

    //readonly regs
    readOnlyTCMList.append(0x5); //Temperature
    readOnlyTCMList.append(0x7); //Microcode & serial number
    for(quint8 i = 0; i < 10; ++i){ //PM Links
        readOnlyTCMList.append(0x10 + i);
        readOnlyTCMList.append(0x30 + i);
    }
    for(quint8 i = 0; i < 15; ++i)  //Counters
        readOnlyTCMList.append(0x70 + i);
    for(quint8 i = 0; i < 16; ++i)  //GBTstatus
        readOnlyTCMList.append(0xE8 + i);
    for(quint8 i = 0; i < 6 ; ++i) //FW and FIFO cnts
        readOnlyTCMList.append(0xFC + i);
    for(quint8 i = 0; i < 20; ++i){
        quint8 k = 0;
        while(k != 3)
            readOnlyTCMList.append(0x300 + 0x200 * i + (k++));
    }
}

void FEE::initPMregs(){
    //non-available regs
    nonavailablePMregs.append(0xBE);
    nonavailablePMregs.append(0xBF);
    for(quint8 i = 0; i < 253; ++i)
        nonavailablePMregs.append(0x103 + i);

    //readonly regs
    for(quint8 i = 0; i < 24; ++i) //ADC baselines
        readOnlyPMList.append(0xD + i);
    for(quint8 i = 0; i < 62; ++i)
        readOnlyPMList.append(0x3E + i);
    readOnlyPMList.append(0x7D); //Channel ADC baseline out of range
    readOnlyPMList.append(0xBC); //Board Temperature
    readOnlyPMList.append(0xBD); //Microcode & serial number
    for(quint8 i = 0; i < 24; ++i)
        readOnlyPMList.append(0xC0 + i);
    for(quint8 i = 0; i < 8; ++i)  //GBTstatus
        readOnlyPMList.append(0xE8 + i);
    for(quint8 i = 0; i < 4; ++i)
        readOnlyPMList.append(0xFC + i);
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

bool FEE::isPM(quint32 address){
    return (address % firstPMaddress) < 0x100 && address > 0x1FF && address < 0x2A00;
}

bool FEE::isConnectedPM(quint8 pmNo){
    return pmNo < 20 ? static_cast<bool>((1 << pmNo) & availPMs) : false;
}

bool FEE::isSPIEnabled(quint8 pmNo){
    return pmNo < 20 ? static_cast<bool>((1 << pmNo) & this->T0_Map.SPI_MASK) : false;
}

bool FEE::isFIFO(quint32 address){
    return FIFOregslist.contains(address);
}

bool FEE::isReadOnly(quint32 address){
    if(isPM(address)) return readOnlyPMList.contains(address % firstPMaddress) && this->readonlyIsAvailable;
    else return (readOnlyTCMList.contains(address) || (address >= 0x4000 && address <= 0x4FFF)) &&
                this->readonlyIsAvailable;
}

bool FEE::is16bit(quint32 address){
    if(isPM(address)) return (address % firstPMaddress) <= 0xBF;
    else return address <= 0xE;
}





