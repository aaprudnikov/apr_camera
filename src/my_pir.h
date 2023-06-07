#ifndef MYPIR_H
#define MYPIR_H

enum PirMode {
    PIR_GUARD,
    PIR_OBSERVER
};

class myPIR{
    private:
        PirMode m_mode = PIR_GUARD;
    public:
//        void run(PirMode mode);
//        void stop();
        void handle();
        PirMode getState();
        void setState(PirMode mode);
};

#endif
