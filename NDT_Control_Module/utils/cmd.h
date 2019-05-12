/********************************/
//		Defines					//
/********************************/

#define CMDLINE_MAX_ARGS 				3
#define I2C_PACKET_HEADER				0xCA
#define CMD_SET_SAMPLING_FREQ			0x61
#define CMD_GAIN_DEFINITION				0x62
#define CMD_DIAGNOSE_DEFINITION			0x63
#define CMD_SET_CUTOFF_FREQ_DEFINITION	0x64
#define CMD_DURATION_DEFINITION			0x65

/********************************/
//		External variables		//
//		declared in main.c		//
/********************************/

extern FILE *src, *dst;
extern const char daq1[], daq2[], daq3[], daq4[], daq5[], daq6[], daq7[], daq8[];

//*****************************************************************************
//
// Declaration for the callback functions that will implement the command line
// functionality.  These functions get called by the command line interpreter
// when the corresponding command is typed into the command line.
//
//*****************************************************************************
extern int CMD_help(int argc, char **argv);
extern int CMD_diagnose(int argc, char **argv);
extern int CMD_getData(int argc, char ** argv);
extern int CMD_setGain(int argc, char ** argv);
extern int CMD_setCutoffFreq(int argc, char ** argv);
extern int CMD_start(int argc, char ** argv);
extern int CMD_setSamplingFreq(int argc, char **argv);
extern int CMD_create(int argc, char **argv);
extern int CMD_read(int argc, char **argv);
extern int CMD_write(int argc, char **argv);
extern int CMD_reset(int argc, char **argv);
extern int CMD_duration(int argc, char **argv);

