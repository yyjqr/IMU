/* VN-100 Registers */
#define     VN100_REG_MODEL     1
#define     VN100_REG_HWREV     2
#define     VN100_REG_SN        3
#define     VN100_REG_FWVER     4
#define     VN100_REG_SBAUD     5
#define     VN100_REG_ADOR      6
#define     VN100_REG_ADOF      7
#define     VN100_REG_YPR       8
#define     VN100_REG_QTN       9
#define     VN100_REG_QTM       10
#define     VN100_REG_QTA       11
#define     VN100_REG_QTR       12
#define     VN100_REG_QMA       13
#define     VN100_REG_QAR       14
#define     VN100_REG_QMR       15
#define     VN100_REG_DCM       16
#define     VN100_REG_MAG       17
#define     VN100_REG_ACC       18
#define     VN100_REG_GYR       19
#define     VN100_REG_MAR       20
#define     VN100_REG_REF       21
#define     VN100_REG_SIG       22
#define     VN100_REG_HSI       23
#define     VN100_REG_ATP       24
#define     VN100_REG_ACT       25
#define     VN100_REG_RFR       26
#define     VN100_REG_YMR       27
#define     VN100_REG_ACG       28



 /* System Error */
  #define VN100_Error_None                     0
  #define VN100_Error_HardFaultException       1
  #define VN100_Error_InputBufferOverflow      2
  #define VN100_Error_InvalidChecksum          3
  #define VN100_Error_InvalidCommand           4
  #define VN100_Error_NotEnoughParameters      5
  #define VN100_Error_TooManyParameters        6
  #define VN100_Error_InvalidParameter         7
  #define VN100_Error_InvalidRegister          8
  #define VN100_Error_UnauthorizedAccess       9
  #define VN100_Error_WatchdogReset            10
  #define VN100_Error_OutputBufferOverflow     11
  #define VN100_Error_InsufficientBandwidth    12
