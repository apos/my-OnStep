// -------------------------------------------------------------------------------------------------------
// Handle encoders, both CW/CCW and Quadrature A/B types are supported

#include "MountStatus.h"

#if ENCODERS == ON

#if defined(ESP8266) || defined(ESP32)
  #include <Esp.h>
#endif

#if ENC_AUTO_SYNC_DEFAULT == ON
  bool encAutoSync=true;
#else
  bool encAutoSync=false;
#endif
double Axis1EncTicksPerDeg=AXIS1_ENC_TICKS_DEG;
int  Axis1EncRev     =AXIS1_ENC_REVERSE;
long Axis1EncDiffTo  =AXIS1_ENC_DIFF_LIMIT_TO;
long Axis1EncDiffFrom=AXIS1_ENC_DIFF_LIMIT_FROM;
long Axis1EncDiffAbs =0;
double Axis2EncTicksPerDeg=AXIS2_ENC_TICKS_DEG;
int  Axis2EncRev     =AXIS2_ENC_REVERSE;
long Axis2EncDiffTo  =AXIS2_ENC_DIFF_LIMIT_TO;
long Axis2EncDiffFrom=AXIS2_ENC_DIFF_LIMIT_FROM;
long Axis2EncDiffAbs =0;

// encoder position
volatile int32_t __p1,__p2;

// bring in support for the various encoder types
#include "Enc_AB.h"
#include "Enc_CwCcw.h"
#include "Enc_BiSS_C_BC.h"

// encoder polling rate in seconds, default=2.0
#define POLLING_RATE 2.0

// encoder rate control
#if AXIS1_ENC_RATE_CONTROL == ON

  // user interface and settings
  bool encSweep=true;
  bool encRateControl=false;
  long Axis1EncProp=10;
  long Axis1EncMinGuide=100;

  // timing related
  volatile uint32_t T0=0;
  volatile uint32_t T1=0;
  volatile uint32_t Telapsed=0;

  #define MIN_ENC_PERIOD 0.2
  #define MAX_ENC_PERIOD 5.0
  float arcSecondsPerTick=(1.0/Axis1EncTicksPerDeg)*3600.0; // (0.0018)*3600 = 6.48
  float usPerTick=(arcSecondsPerTick/15.041)*1000000.0;     // 6.48/15.041 = 0.4308 seconds per tick

  unsigned long msPerTickMax =(arcSecondsPerTick/15.041)*1000.0*MAX_ENC_PERIOD;
#if AXIS1_ENC_BIN_AVG > 0
  volatile uint32_t usPerBinTickMin =(double)usPerTick*(double)AXIS1_ENC_BIN_AVG*MIN_ENC_PERIOD;
  volatile uint32_t usPerBinTickMax =(double)usPerTick*(double)AXIS1_ENC_BIN_AVG*MAX_ENC_PERIOD;
#endif
#if defined(ESP8266) || defined(ESP32)
  volatile uint32_t clocksPerTickMin=(double)usPerTick*(double)ESP.getCpuFreqMHz()*MIN_ENC_PERIOD;
  volatile uint32_t clocksPerTickMax=(double)usPerTick*(double)ESP.getCpuFreqMHz()*MAX_ENC_PERIOD;
  #define GetClockCount ESP.getCycleCount()
  #define ClockCountToMicros ((uint32_t)ESP.getCpuFreqMHz())
#elif defined(__MK20DX256__)
  volatile uint32_t clocksPerTickMin=(double)usPerTick*(double)(F_CPU/1000000L)*MIN_ENC_PERIOD;
  volatile uint32_t clocksPerTickMax=(double)usPerTick*(double)(F_CPU/1000000L)*MAX_ENC_PERIOD;
  #define GetClockCount ARM_DWT_CYCCNT
  #define ClockCountToMicros (F_CPU/1000000L)
#else
  volatile uint32_t clocksPerTickMin=(double)usPerTick*MIN_ENC_PERIOD;
  volatile uint32_t clocksPerTickMax=(double)usPerTick*MAX_ENC_PERIOD;
  #define GetClockCount micros()
  #define ClockCountToMicros (1L)
#endif
  
  // averages & rate calculation
  volatile long Axis1EncStaSamples=20;
  volatile long Axis1EncLtaSamples=200;
  volatile int32_t Tsta=0;
  volatile int32_t Tlta=0;
#if AXIS1_ENC_BIN_AVG > 0
  volatile uint32_t StaBins[AXIS1_ENC_BIN_AVG];
  volatile uint32_t LtaBins[AXIS1_ENC_BIN_AVG];
  volatile uint32_t T1Bins[AXIS1_ENC_BIN_AVG];
#endif
  float axis1EncRateSta=1.0;
  float axis1EncRateLta=1.0;
  float axis1EncRateComp=0.0;
  float axis1Rate=1.0;

#if AXIS1_ENC_INTPOL_COS == ON
  long Axis1EncIntPolPeriod=AXIS1_ENC_BIN_AVG;
  long Axis1EncIntPolPhase=1;
  long Axis1EncIntPolMag=0;
  float intpolComp=0;
  float intpolPhase=0;
#endif

#if AXIS1_ENC_RATE_AUTO > 0
  static unsigned long nextWormPeriod=0;
  static float axis1RateDelta=0;
#endif

  // guiding
  float guideCorrection=0;
  long guideCorrectionMillis=0;

#endif

// ----------------------------------------------------------------------------------------------------------------
// background process position/rate control for encoders 
class Encoders {
  public:
    void init() {
    }

    // automatically sync the encoders from OnStep's position when at home or parked
    void syncFromOnStep() {
      // don't sync if the Encoders vs. OnStep disagree by too much
      if (Axis1EncDiffFrom != OFF && fabs(_osAxis1-_enAxis1) > (double)(Axis1EncDiffFrom/3600.0)) return;
      if (Axis2EncDiffFrom != OFF && fabs(_osAxis1-_enAxis1) > (double)(Axis1EncDiffFrom/3600.0)) return;
        
      if (Axis1EncRev == ON)
        axis1Pos.write(-_osAxis1*(double)Axis1EncTicksPerDeg);
      else
        axis1Pos.write(_osAxis1*(double)Axis1EncTicksPerDeg);

      if (Axis2EncRev == ON)
        axis2Pos.write(-_osAxis2*(double)Axis2EncTicksPerDeg);
      else
        axis2Pos.write(_osAxis2*(double)Axis2EncTicksPerDeg);

    }
    
    // zero absolute encoders from OnStep's position
#ifdef ENC_HAS_ABSOLUTE
    void zeroFromOnStep() {
  #ifdef ENC_HAS_ABSOLUTE_AXIS1
      axis1Pos.write(_osAxis1*(double)Axis1EncTicksPerDeg);
  #endif
  #ifdef ENC_HAS_ABSOLUTE_AXIS2
      axis2Pos.write(_osAxis2*(double)Axis2EncTicksPerDeg);
  #endif
    }
#endif
    
    void syncToOnStep() {
        char s[22];
        // automatically sync OnStep to the encoders' position
        Ser.print(":SX40,"); Ser.print(_enAxis1,6); Ser.print("#"); Ser.readBytes(s,1);
        Ser.print(":SX41,"); Ser.print(_enAxis2,6); Ser.print("#"); Ser.readBytes(s,1);
        Ser.print(":SX42,1#"); Ser.readBytes(s,1);
    }
    void poll() {
      // check encoders and sync OnStep if diff is too great, checks every 2 seconds
      static unsigned long nextEncCheckMs=millis()+(unsigned long)(POLLING_RATE*1000.0);
      unsigned long temp=millis();
      char *conv_end;
      if ((long)(temp-nextEncCheckMs)>0) {
        nextEncCheckMs=temp+(unsigned long)(POLLING_RATE*1000.0);
        char s[22];
        if (command(":GX42#",s) && strlen(s) > 1) {
          double f=strtod(s,&conv_end);
          if (&s[0] != conv_end && f >= -999.9 && f <= 999.9) _osAxis1=f;
        }
        if (command(":GX43#",s) && strlen(s) > 1) {
          double f=strtod(s,&conv_end);
          if (&s[0] != conv_end && f >= -999.9 && f <= 999.9) _osAxis2=f;
        }
        
        long pos=axis1Pos.read();
        if (pos == INT32_MAX) _enAxis1Fault = true; else _enAxis1Fault=false;
        _enAxis1=(double)pos/(double)Axis1EncTicksPerDeg;
        if (Axis1EncRev == ON) _enAxis1=-_enAxis1;

        pos=axis2Pos.read();
        if (pos == INT32_MAX) _enAxis2Fault = true; else _enAxis2Fault=false;
        _enAxis2=(double)pos/(double)Axis2EncTicksPerDeg;
        if (Axis2EncRev == ON) _enAxis2=-_enAxis2;

        mountStatus.update();
        if (encAutoSync && mountStatus.valid() && !_enAxis1Fault && !_enAxis2Fault) {
          if (mountStatus.atHome() || mountStatus.parked() || mountStatus.aligning() || mountStatus.syncToEncodersOnly()) {
            syncFromOnStep();
            // re-enable normal operation once we're updated here
            if (mountStatus.syncToEncodersOnly()) { Ser.print(":SX43,1#"); Ser.readBytes(s,1); }
          } else
            if (!mountStatus.slewing() && !mountStatus.guiding()) {
              if ((fabs(_osAxis1-_enAxis1)>(double)(Axis1EncDiffTo/3600.0)) ||
                  (fabs(_osAxis2-_enAxis2)>(double)(Axis2EncDiffTo/3600.0))) syncToOnStep();
            }
        }

        // automatic rate compensation
#if AXIS1_ENC_RATE_CONTROL == ON

        // get the averages
#if AXIS1_ENC_BIN_AVG > 0
        Tsta=0; Tlta=0;
        for (int i=0; i<AXIS1_ENC_BIN_AVG; i++) { Tsta+=StaBins[i]; Tlta+=LtaBins[i]; }
        Tsta/=AXIS1_ENC_BIN_AVG; // average
        Tlta/=AXIS1_ENC_BIN_AVG;
        Tsta/=AXIS1_ENC_BIN_AVG; // each period is AXIS1_ENC_BIN_AVG X longer than the step to step frequency
        Tlta/=AXIS1_ENC_BIN_AVG;
#endif
        axis1EncRateSta=(usPerTick/Tsta)+axis1EncRateComp;
        axis1EncRateLta=(usPerTick/Tlta)+axis1EncRateComp;

        // get the tracking rate OnStep thinks it has once every ten seconds
        static int pass=-1;
        pass++;
        if (pass%5==0) {
          Ser.print(":GX49#"); s[Ser.readBytesUntil('#',s,20)]=0;
          if (strlen(s)>1) axis1Rate=atof(s); else axis1Rate=0;
        }

        // reset averages if rate is too fast or too slow
        static unsigned long resetTimeout=0;
        if (fastMotion() || slowMotion()) resetTimeout=millis();

        // keep things reset for 15 seconds if just starting up again
        if ((long)(millis()-resetTimeout)<15000L) { clearAverages(); return; }

        // encoder rate control disabled
        if (!encRateControl) return;
          
#if AXIS1_ENC_INTPOL_COS == ON
        long a1=axis1Pos.read();
        intpolPhase=(a1+Axis1EncIntPolPhase) % Axis1EncIntPolPeriod;
        float a3=(intpolPhase/(float)Axis1EncIntPolPeriod)*3.1415*2.0;
        intpolComp=cos(a3)*(Axis1EncIntPolMag/1000000.0);
        axis1EncRateSta=axis1EncRateSta+intpolComp;
#endif

#if AXIS1_ENC_RATE_AUTO > 0
        if ((long)(millis()-nextWormPeriod)>=0) {
          nextWormPeriod=millis()+(unsigned long)(AXIS1_ENC_RATE_AUTO)*997UL;
          axis1EncRateComp+=axis1RateDelta/(double)(AXIS1_ENC_RATE_AUTO);
          if (axis1EncRateComp>+0.01) axis1EncRateComp=+0.01;
          if (axis1EncRateComp<-0.01) axis1EncRateComp=-0.01;
          axis1RateDelta=0;
        }
        axis1RateDelta+=(axis1Rate-axis1EncRateSta)*POLLING_RATE;
#endif

        // accumulate tracking rate departures for pulse-guide, rate delta * 2 seconds
        guideCorrection+=(axis1Rate-axis1EncRateSta)*((float)Axis1EncProp/100.0)*POLLING_RATE;

        if (guideCorrection>POLLING_RATE) clearAverages(); else
        if (guideCorrection<-POLLING_RATE) clearAverages(); else
        if (guideCorrection>Axis1EncMinGuide/1000.0) {
          guideCorrectionMillis=round(guideCorrection*1000.0);
          Ser.print(":Mgw"); Ser.print(guideCorrectionMillis); Ser.print("#");
          guideCorrection=0;
        } else
        if (guideCorrection<-Axis1EncMinGuide/1000.0) {
          guideCorrectionMillis=round(guideCorrection*1000.0);
          Ser.print(":Mge"); Ser.print(-guideCorrectionMillis); Ser.print("#");
          guideCorrection=0;
        } else 
          guideCorrectionMillis=0;
#endif
      }
    }

#if AXIS1_ENC_RATE_CONTROL == ON
    void clearAverages() {
      double d=usPerTick*axis1Rate;
#if AXIS1_ENC_BIN_AVG > 0
      for (int i=0; i<AXIS1_ENC_BIN_AVG; i++) { StaBins[i]=d*AXIS1_ENC_BIN_AVG; LtaBins[i]=d*AXIS1_ENC_BIN_AVG; }
#endif
      Tsta=d;
      Tlta=d;
      axis1EncRateSta=usPerTick/d;
      axis1EncRateLta=usPerTick/d;
      guideCorrection=0.0;
      guideCorrectionMillis=0;
#if AXIS1_ENC_RATE_AUTO > 0
      axis1EncRateComp=0.0;
      axis1RateDelta=0;
      nextWormPeriod=millis()+(unsigned long)(AXIS1_ENC_RATE_AUTO)*997UL;;
#endif
    }
#endif
  
    double getAxis1() { return _enAxis1; }
    double getAxis2() { return _enAxis2; }
    bool validAxis1() { return !_enAxis1Fault; }
    bool validAxis2() { return !_enAxis2Fault; }
    double getOnStepAxis1() { return _osAxis1; }
    double getOnStepAxis2() { return _osAxis2; }

  private:
    double _osAxis1=0;
    double _osAxis2=0;
    double _enAxis1=0;
    double _enAxis2=0;
    bool _enAxis1Fault=false;
    bool _enAxis2Fault=false;
};
#endif
