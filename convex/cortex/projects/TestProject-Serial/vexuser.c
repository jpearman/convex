#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "ch.h"  		// needs for all ChibiOS programs
#include "hal.h" 		// hardware abstraction layer header
#include "vex.h"		// vex library header

#include "smartmotor.h"
#define SHELL_WA_SIZE   THD_WA_SIZE(2048)

// Digi IO configuration
vexDigiCfg  dConfig[kVexDigital_Num] = {
    { kVexDigital_1,    kVexSensorSonarCm,       kVexConfigSonarIn,     kVexSonar_1  },
    { kVexDigital_2,    kVexSensorSonarCm,       kVexConfigSonarOut,    kVexSonar_1  },
    { kVexDigital_3,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
    { kVexDigital_4,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
    { kVexDigital_5,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
    { kVexDigital_6,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
    { kVexDigital_7,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
    { kVexDigital_8,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
    { kVexDigital_9,    kVexSensorDigitalInput,  kVexConfigInput,       0 },
    { kVexDigital_10,   kVexSensorDigitalInput,  kVexConfigInput,       0 },
    { kVexDigital_11,   kVexSensorQuadEncoder,   kVexConfigQuadEnc1,    kVexQuadEncoder_1 },
    { kVexDigital_12,   kVexSensorQuadEncoder,   kVexConfigQuadEnc2,    kVexQuadEncoder_1 }
};

vexMotorCfg mConfig[kVexMotorNum] = {
  { kVexMotor_1,  kVexMotor393T,  kVexMotorNormal,   kVexSensorNone,        0 },
  { kVexMotor_2,  kVexMotor393T,  kVexMotorNormal,   kVexSensorNone,        0 },
  { kVexMotor_3,  kVexMotor393T,  kVexMotorNormal,   kVexSensorNone,        0 },
  { kVexMotor_4,  kVexMotor393T,  kVexMotorNormal,   kVexSensorNone,        0 },
  { kVexMotor_5,  kVexMotor393T,  kVexMotorNormal,   kVexSensorNone,        0 },
  { kVexMotor_6,  kVexMotor393T,  kVexMotorNormal,   kVexSensorNone,        0 },
  { kVexMotor_7,  kVexMotor393T,  kVexMotorNormal,   kVexSensorNone,        0 },
  { kVexMotor_8,  kVexMotor393T,  kVexMotorNormal,   kVexSensorNone,        0 },
  { kVexMotor_9,  kVexMotor393T,  kVexMotorNormal,   kVexSensorNone,        0 },
  { kVexMotor_10, kVexMotor393T,  kVexMotorNormal,   kVexSensorNone,        0 }
};


static int16_t speeds[kVexMotorNum];

/*-----------------------------------------------------------------------------*/
/* Command line related.                                                       */
/*-----------------------------------------------------------------------------*/

static void
noOp( vexStream *chp, int argc, char *argv[])
{
    (void)argv;
    (void)chp;
    (void)argc;
 
}

static void
servexMotor( vexStream *chp, int argc, char *argv[])
{
  int num_motors=argc/2;
  int16_t speed=0;
  int i,motor;

  vex_chprintf(chp, "processing %d motor commands\r\n",num_motors);
  
  if(*(argv[0])=='0')
  {
    for(i=0;i<9;i++)
    {
      vex_chprintf(chp, "< motor %d set to OFF\r\n",i);
      speeds[i]=0;
    }
    return;
  }
  if(argc%2)
  {
    vex_chprintf(chp, "invalid args\r\n");
  }
  
  if(num_motors>10) num_motors=10;
  for(i=0;i<argc;i=i+2)
  {
    motor=atoi(argv[i])-1;
    speed=(int16_t)atoi(argv[i+1]);
    speeds[motor]=speed;
    vex_chprintf(chp, "< motor %d set to %d\r\n",motor,speeds[motor]);
  }
  return;
}

static void
showThreads(vexStream *chp, int argc, char *argv[])
{
  static const char *states[] = {THD_STATE_NAMES };
  Thread *tp;
  (void)argv;
  (void)argc;
  
  tp = chRegFirstThread();
  do {
    if( tp->p_name != NULL )
      vex_chprintf(chp, "<st|%16s ", tp->p_name);
    else vex_chprintf(chp,"unknown");
    vex_chprintf(chp, "|%.8lx|%.8lx|%4lu|%4lu|%9s|%lu\r\n",
                 (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
                 (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
                 states[tp->p_state], (uint32_t)tp->p_time);

    tp = chRegNextThread(tp);
    chThdSleepMilliseconds(20);
  }
  while (tp != NULL);
}


static int
parseSRCPline(char* str,char* delim,char* args[])
{
   char *token;
   int i =0;
   
   /* get the first token */
   token = strtok(str, delim);
   args[i]=token;
   i++; 

   /* walk through other tokens */
   while( token != NULL ) 
   {
      token = strtok(NULL, delim);
      args[i]=token;
      i++; 
   }
   return i;
}


int
lineCallback(vexStream *chp, char* line)
{
  char* args[32];

  int count=parseSRCPline(line," ",args);

  for(int x=0;x<count;x++)
  {
    vex_chprintf(chp,"[%s]\r\n",args[x]);
    

  }

  return 0;
}

/*-----------------------------------------------------------------------------*/



static const SerialCommand commands[] = {
  {"ms",       servexMotor},
  {"do",       noOp},
  {"di",       noOp},
  {"a",        noOp},
  {"st",        showThreads},
  {NULL, NULL}
};

const SerProtoConfig serial_cfg1 =
{
  (vexStream *)SD_CONSOLE,
  lineCallback,
  ">",
  "*end*",
  120
};

void
vexUserSetup()
{
    vexDigitalConfigure( dConfig, DIG_CONFIG_SIZE( dConfig ) );
    vexMotorConfigure( mConfig, MOT_CONFIG_SIZE( mConfig ) );
}

// called before either autonomous or user control
void
vexUserInit()
{
  SmartMotorsInit();
  SmartMotorCurrentMonitorEnable();
  SmartMotorRun();

  for(int i=0;i<kVexMotorNum;i++)
  {
    speeds[i]=0;
  }

}

// Autonomous control task
msg_t
vexAutonomous( void *arg )
{
    (void)arg;

    // Must call this
    vexTaskRegister("auton");

    while(1)
        {
        // Don't hog cpu
        vexSleep( 25 );
        }

    return (msg_t)0;
}


// Driver control task
msg_t
vexOperator( void *arg )
{
	(void)arg;
	Thread *shelltp = NULL;
  int i=0;
  
  
	// Must call this
	vexTaskRegister("operator");

  // Shell manager initialization.
  shellInit();
  
  while (TRUE)
  {
    if (!shelltp) shelltp = serialCreate(&serial_cfg1, SHELL_WA_SIZE, NORMALPRIO);
    else if (chThdTerminated(shelltp))
    {
      chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
      shelltp = NULL;           /* Triggers spawning of a new shell.        */
    }
   
    for(i=0;i<kVexMotorNum;i++)
    {
      vexMotorSet(i,speeds[i]);
    }
    chThdSleepMilliseconds(50);
  }
	return (msg_t)0;
}



