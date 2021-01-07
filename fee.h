#ifndef FEE_H
#define FEE_H

#include <QObject>
#include "RegisterMap.h"
#include "auxiliary/mrand.h"
#include "log.h"

const quint32 TCM_mean_T = 0xEB, PM_mean_T = 397, TCM_FPGA_mean_T = 40186, PM_FPGA_mean_T = 40382,
              FPGA1V_mean = 21739, FPGA1_8V_mean = 39130; 

class FEE:public QObject
{
    Q_OBJECT
public:
    FEE();
    bool isFIFO(quint32 address);
    bool isReadOnly(quint32 address);
    bool isHalfed(quint32 address);
    bool registerIsAvailable(quint32 address);
    bool registerExists(quint32 address);

    bool getValue(quint16 address, quint32& target, Log* log = nullptr, TransactionType type = read);
    bool writeWord(quint32 data, quint16 address, Log* log = nullptr, TransactionType type = read);
    bool readWord(quint16 address, quint32& target, Log* log = nullptr);

    quint32 getavailablePM(){return availPMs;}

public slots:
    void ChangeAvailablePMs(quint8 PMno){availPMs ^= (1 << PMno); /*qDebug() << QString::asprintf("0x%08X", availPMs);*/};

private:
    QTimer timer;
    RegisterMap T0_Map;
    const QList<quint16> FIFOregslist = {0x0100, 0x0300, 0x0500, 0x0700,
                                         0x0900, 0x0B00, 0x0D00, 0x0F00,
                                         0x1100, 0x1300, 0x1500, 0x1700,
                                         0x1900, 0x1B00, 0x1D00, 0x1F00,
                                         0x2100, 0x2300, 0x2500, 0x2700,
                                         0x2900};

    //List of TCM addresses which are read only
    QVector<quint16> readOnlyTCMList;
    //List of PM addresses which are read only
    QVector<quint16> readOnlyPMList;

    const QList<Mrand*> BoardTemperaturesList = {new Mrand(Mgaus,TCM_mean_T, 0.35, 0, 2),
                                                 new Mrand(Mgaus, PM_mean_T, 0.72, 0, 2), new Mrand(Mgaus, PM_mean_T, 0.72, 0, 2),
                                                 new Mrand(Mgaus, PM_mean_T, 0.72, 0, 2), new Mrand(Mgaus, PM_mean_T, 0.72, 0, 2),
                                                 new Mrand(Mgaus, PM_mean_T, 0.72, 0, 2), new Mrand(Mgaus, PM_mean_T, 0.72, 0, 2),
                                                 new Mrand(Mgaus, PM_mean_T, 0.72, 0, 2), new Mrand(Mgaus, PM_mean_T, 0.72, 0, 2),
                                                 new Mrand(Mgaus, PM_mean_T, 0.72, 0, 2), new Mrand(Mgaus, PM_mean_T, 0.72, 0, 2),
                                                 new Mrand(Mgaus, PM_mean_T, 0.72, 0, 2), new Mrand(Mgaus, PM_mean_T, 0.72, 0, 2),
                                                 new Mrand(Mgaus, PM_mean_T, 0.72, 0, 2), new Mrand(Mgaus, PM_mean_T, 0.72, 0, 2),
                                                 new Mrand(Mgaus, PM_mean_T, 0.72, 0, 2), new Mrand(Mgaus, PM_mean_T, 0.72, 0, 2),
                                                 new Mrand(Mgaus, PM_mean_T, 0.72, 0, 2), new Mrand(Mgaus, PM_mean_T, 0.72, 0, 2),
                                                 new Mrand(Mgaus, PM_mean_T, 0.72, 0, 2), new Mrand(Mgaus, PM_mean_T, 0.72, 0, 2)};

    const QList<Mrand*> FpgaTemperaturesList = { new Mrand(Mgaus,TCM_FPGA_mean_T, 35.45, 0, 269),
                                                 new Mrand(Mgaus,PM_FPGA_mean_T, 36.23, 0, 269), new Mrand(Mgaus,PM_FPGA_mean_T, 36.23, 0, 269),
                                                 new Mrand(Mgaus,PM_FPGA_mean_T, 36.23, 0, 269), new Mrand(Mgaus,PM_FPGA_mean_T, 36.23, 0, 269),
                                                 new Mrand(Mgaus,PM_FPGA_mean_T, 36.23, 0, 269), new Mrand(Mgaus,PM_FPGA_mean_T, 36.23, 0, 269),
                                                 new Mrand(Mgaus,PM_FPGA_mean_T, 36.23, 0, 269), new Mrand(Mgaus,PM_FPGA_mean_T, 36.23, 0, 269),
                                                 new Mrand(Mgaus,PM_FPGA_mean_T, 36.23, 0, 269), new Mrand(Mgaus,PM_FPGA_mean_T, 36.23, 0, 269),
                                                 new Mrand(Mgaus,PM_FPGA_mean_T, 36.23, 0, 269), new Mrand(Mgaus,PM_FPGA_mean_T, 36.23, 0, 269),
                                                 new Mrand(Mgaus,PM_FPGA_mean_T, 36.23, 0, 269), new Mrand(Mgaus,PM_FPGA_mean_T, 36.23, 0, 269),
                                                 new Mrand(Mgaus,PM_FPGA_mean_T, 36.23, 0, 269), new Mrand(Mgaus,PM_FPGA_mean_T, 36.23, 0, 269),
                                                 new Mrand(Mgaus,PM_FPGA_mean_T, 36.23, 0, 269), new Mrand(Mgaus,PM_FPGA_mean_T, 36.23, 0, 269),
                                                 new Mrand(Mgaus,PM_FPGA_mean_T, 36.23, 0, 269), new Mrand(Mgaus,PM_FPGA_mean_T, 36.23, 0, 269)
                                               };
    const QList<Mrand*> FpgaPower1Vlist          = { new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean),
                                                 new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean), new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean),
                                                 new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean), new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean),
                                                 new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean), new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean),
                                                 new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean), new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean),
                                                 new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean), new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean),
                                                 new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean), new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean),
                                                 new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean), new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean),
                                                 new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean), new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean),
                                                 new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean), new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean),
                                                 new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean), new Mrand(Muniform, FPGA1V_mean, 0.2*FPGA1V_mean)
    };
    const QList<Mrand*> FpgaPower1_8Vlist        = { new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean),
                                                 new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean), new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean),
                                                 new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean), new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean),
                                                 new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean), new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean),
                                                 new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean), new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean),
                                                 new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean), new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean),
                                                 new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean), new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean),
                                                 new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean), new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean),
                                                 new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean), new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean),
                                                 new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean), new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean),
                                                 new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean), new Mrand(Muniform, FPGA1_8V_mean, 0.2*FPGA1_8V_mean)
    };
    quint32 availPMs = 0;
    QVector<quint16> nonavailableTCMregs, nonavailablePMregs, TCMReadOnlyregs, PMReadOnlyregs;

    void initTCMRegs();
    void initPMregs();

private slots:
    void updateRegisters();

    QString operationTypeString(TransactionType type){
        switch (type){
        case read:
        case write:return "sequental";
        case nonIncrementingRead:
        case nonIncrementingWrite: return "non-incrementing";
        case RMWbits:
        case RMWsum: return "RMW operation";
        case configurationRead:
        case configurationWrite: return "";
        }
    }
};

#endif // FEE_H
