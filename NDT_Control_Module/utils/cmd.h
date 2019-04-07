#define CMDLINE_MAX_ARGS 				3
#define MASTER_TO_SLAVE_PACKET_SIZE 	3
#define CMD_GAIN_DEFINITION				0x62
#define CMD_DIAGNOSE_DEFINITION			0x63
#define CMD_PWDN_DEFINITION				0x65
#define CMD_PWUP_DEFINITION				0x66


extern uint8_t masterToSlaveByteCtr;
extern uint8_t masterToSlaveIndex;
extern char masterToSlavePacket[MASTER_TO_SLAVE_PACKET_SIZE];
extern FILE *src, *dst;
extern const char daq1[];

//*****************************************************************************
//
// Declaration for the callback functions that will implement the command line
// functionality.  These functions get called by the command line interpreter
// when the corresponding command is typed into the command line.
//
//*****************************************************************************
extern int CMD_help(int argc, char **argv);
extern int CMD_mode(int argc, char **argv);
extern int CMD_diagnose(int argc, char **argv);
extern int CMD_pwdn(int argc, char **argv);
extern int CMD_pwup(int argc, char **argv);
extern int CMD_getData(int argc, char ** argv);
extern int CMD_setGain(int argc, char ** argv);
extern int CMD_QuitProcess(int argc, char **argv);
extern int CMD_read(int argc, char **argv);

