#ifndef MYPIR_H
#define MYPIR_H

enum PirMode {
    PIR_GUARD,
    PIR_OBSERVER
};

class myPIR{
    private:
        PirMode m_mode;
    public:
        void run(PirMode mode);
        void stop();
        PirMode getState();
};

#endif
