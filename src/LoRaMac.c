/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech
 ___ _____ _   ___ _  _____ ___  ___  ___ ___
/ __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
\__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
|___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
embedded.connectivity.solutions===============

Description: LoRa MAC layer implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis ( Semtech ), Gregory Cristian ( Semtech ) and Daniel J闂傚倸鍊搁崐鐑芥嚄閸洖纾块柣銏㈩焾閻ょ偓绻濋棃娑卞剱闁稿﹨娉涢埥澶愬籍閸屾粈鍠奺 ( STACKFORCE )
*/
#include <math.h>
#include "board.h"

#include "LoRaMacCrypto.h"
#include "LoRaMac.h"
#include "LoRaMacTest.h"
#include "board.h"

#include "Arduino.h"
extern  void lora_printf(const char *format, ...);
extern hw_timer_t * timer;
/*!
 * Maximum PHY layer payload size
 */
#define LORAMAC_PHY_MAXPAYLOAD                      255

/*!
 * Maximum MAC commands buffer size
 */
#define LORA_MAC_COMMAND_MAX_LENGTH                 15

/*!
 * FRMPayload overhead to be used when setting the Radio.SetMaxPayloadLength
 * in RxWindowSetup function.
 * Maximum PHYPayload = MaxPayloadOfDatarate/MaxPayloadOfDatarateRepeater + LORA_MAC_FRMPAYLOAD_OVERHEAD
 */
#define LORA_MAC_FRMPAYLOAD_OVERHEAD                13 // MHDR(1) + FHDR(7) + Port(1) + MIC(4)

/*!
 * LoRaMac duty cycle for the back-off procedure during the first hour.
 */
#define BACKOFF_DC_1_HOUR                           100

/*!
 * LoRaMac duty cycle for the back-off procedure during the next 10 hours.
 */
#define BACKOFF_DC_10_HOURS                         1000

/*!
 * LoRaMac duty cycle for the back-off procedure during the next 24 hours.
 */
#define BACKOFF_DC_24_HOURS                         10000

/*!
 * Device IEEE EUI
 */
static uint8_t *LoRaMacDevEui;

/*!
 * Application IEEE EUI
 */
 uint8_t *LoRaMacAppEui;

/*!
 * AES encryption/decryption cipher application key
 */
static uint8_t *LoRaMacAppKey;

/*!
 * AES encryption/decryption cipher network session key
 */
//闂傚倸鍊搁崐椋庣矆娓氾拷楠炴牠顢曚綅閸ヮ剦鏁冮柨鏇嫹闁汇倗鍋撶换婵囩節閸屾粌顤�闂佺顑戠换婵嬪蓟閺囥垹閱囨繝闈涙搐椤︹晠姊虹粙娆惧剱闁绘濮撮锝夊醇閺囩偤鍞跺┑鐐村灦閿氭い顒�顑夊鍝勭暦閸モ晛绗″┑顔硷工缂嶅﹥淇婇悽绋跨妞ゆ柨澧介弶鎼佹⒑鐟欏嫬绀冮柟娲讳簽濡叉劙鎮欏ù瀣杸闂佺粯鍔樼亸娆徯掗悙鐑樼厸闁告侗鍨板瓭濡炪値鍋勭换姗�骞冨鍏剧喓鎷犻幓鎺嶇礋闂傚倷绀佺紞濠囧磻婵犲洤绀堟繛鍡樺姈閸欏繒锟藉箍鍎卞ú銊у閽樺褰掓晲閸偄娈愬┑鐐茬墛濮婂綊濡甸崟顖涙櫇闁稿本绋掗柨顓烆渻閵堝棙绌跨紒鎻掑⒔閿熶粙娼ч妶鎼佺嵁閺嶎収鏁囬柣鏂挎啞閻濐偄鈹戦悩鍨毄濠殿喗鎸抽幃妯侯潩鐠鸿櫣顦梺鍝勬储閸ㄥ湱绮婚弽顬″綊鏁愰崼鐕佷哗缂佺偓宕樺Λ鍕箒闂佹寧绻傞幊蹇涘箚閸儱鍐�闁炽儲鍓氬〒濠氭煏閸繂鏆欏┑鈥炽偢閺岀喖鎼归锟介鍫熷仼闁绘垼濮ら崑銊╂煕閹惧啿绾ч柡鍜冪秮濮婃椽妫冨☉姘鳖唺婵犳鍣ｉ弨閬嶃�冮妷鈺佺倞妞ゆ帊璁查幏娲⒒閸屾氨澧涢柛鎰吹濡叉劙鏁撻悩宕囧幈闂佸搫娲㈤崝宀勭嵁濡や胶绠鹃柛顐犲焺閻掗箖鏌嶇憴鍕伌闁诡喗鐟╁鍫曞箣閻樿鲸顢橀梻鍌欑閹碱偆鎮锕�绀夐柟杈剧畱缁犳牜鎲搁弮鍫濈畺婵炲棙鎸搁崹鍌涖亜閹伴潧浜濈紓宥嗩殜濮婄粯鎷呴懞銉с�婇梺闈╃秶缁犳捇鐛箛娑欐櫢闁跨噦鎷�
RTC_DATA_ATTR static  uint8_t LoRaMacNwkSKey[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//static  uint8_t A=0;

/*!
 * AES encryption/decryption cipher application session key
 */
RTC_DATA_ATTR static uint8_t LoRaMacAppSKey[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*!
 * Device nonce is a random value extracted by issuing a sequence of RSSI
 * measurements
 */
static uint16_t LoRaMacDevNonce;

/*!
 * Network ID ( 3 bytes )
 */
static uint32_t LoRaMacNetID;

/*!
 * Mote Address
 */
RTC_DATA_ATTR static uint32_t LoRaMacDevAddr;

/*!
 * Multicast channels linked list
 */
static MulticastParams_t *MulticastChannels = NULL;

/*!
 * Actual device class
 */
static DeviceClass_t LoRaMacDeviceClass;

/*!
 * Indicates if the node is connected to a private or public network
 */
static bool PublicNetwork;

/*!
 * Indicates if the node supports repeaters
 */
static bool RepeaterSupport;

/*!
 * Buffer containing the data to be sent or received.
 */
static uint8_t LoRaMacBuffer[LORAMAC_PHY_MAXPAYLOAD];

/*!
 * Length of packet in LoRaMacBuffer
 */
static uint16_t LoRaMacBufferPktLen = 0;

/*!
 * Length of the payload in LoRaMacBuffer
 */
static uint8_t LoRaMacTxPayloadLen = 0;

/*!
 * Buffer containing the upper layer data.
 */
static uint8_t LoRaMacRxPayload[LORAMAC_PHY_MAXPAYLOAD];

/*!
 * LoRaMAC frame counter. Each time a packet is sent the counter is incremented.
 * Only the 16 LSB bits are sent
 */
RTC_DATA_ATTR static uint32_t UpLinkCounter = 0;

/*!
 * LoRaMAC frame counter. Each time a packet is received the counter is incremented.
 * Only the 16 LSB bits are received
 */
RTC_DATA_ATTR static uint32_t DownLinkCounter = 0;

/*!
 * IsPacketCounterFixed enables the MIC field tests by fixing the
 * UpLinkCounter value
 */
static bool IsUpLinkCounterFixed = false;

/*!
 * Used for test purposes. Disables the opening of the reception windows.
 */
static bool IsRxWindowsEnabled = true;

/*!
 * Indicates if the MAC layer has already joined a network.
 */
RTC_DATA_ATTR  bool IsLoRaMacNetworkJoined = false;

/*!
 * LoRaMac ADR control status
 */
static bool AdrCtrlOn = false;

/*!
 * Counts the number of missed ADR acknowledgements
 */
RTC_DATA_ATTR static uint32_t AdrAckCounter = 0;

/*!
 * If the node has sent a FRAME_TYPE_DATA_CONFIRMED_UP this variable indicates
 * if the nodes needs to manage the server acknowledgement.
 */
static bool NodeAckRequested = false;

/*!
 * If the server has sent a FRAME_TYPE_DATA_CONFIRMED_DOWN this variable indicates
 * if the ACK bit must be set for the next transmission
 */
static bool SrvAckRequested = false;

/*!
 * Indicates if the MAC layer wants to send MAC commands
 */
static bool MacCommandsInNextTx = false;

/*!
 * Contains the current MacCommandsBuffer index
 */
static uint8_t MacCommandsBufferIndex = 0;

/*!
 * Contains the current MacCommandsBuffer index for MAC commands to repeat
 */
static uint8_t MacCommandsBufferToRepeatIndex = 0;

/*!
 * Buffer containing the MAC layer commands
 */
static uint8_t MacCommandsBuffer[LORA_MAC_COMMAND_MAX_LENGTH];

/*!
 * Buffer containing the MAC layer commands which must be repeated
 */
static uint8_t MacCommandsBufferToRepeat[LORA_MAC_COMMAND_MAX_LENGTH];

#if defined( USE_BAND_433 )
/*!
 * Data rates table definition
 */
const uint8_t Datarates[]  = { 12, 11, 10,  9,  8,  7,  7, 50 };

/*!
 * Bandwidths table definition in Hz
 */
const uint32_t Bandwidths[] = { 125e3, 125e3, 125e3, 125e3, 125e3, 125e3, 250e3, 0 };

/*!
 * Maximum payload with respect to the datarate index. Cannot operate with repeater.
 */
const uint8_t MaxPayloadOfDatarate[] = { 51, 51, 51, 115, 242, 242, 242, 242 };

/*!
 * Maximum payload with respect to the datarate index. Can operate with repeater.
 */
const uint8_t MaxPayloadOfDatarateRepeater[] = { 51, 51, 51, 115, 222, 222, 222, 222 };

/*!
 * Tx output powers table definition
 */
const int8_t TxPowers[]    = { 10, 7, 4, 1, -2, -5 };

/*!
 * LoRaMac bands
 */
static Band_t Bands[LORA_MAX_NB_BANDS] =
{
    BAND0,
};

/*!
 * LoRaMAC channels
 */
static ChannelParams_t Channels[LORA_MAX_NB_CHANNELS] =
{
    LC1,
    LC2,
    LC3,
};
#elif defined( USE_BAND_470 ) || defined( USE_BAND_470PREQUEL )

/*!
 * Data rates table definition
 */
const uint8_t Datarates[]  = { 12, 11, 10,  9,  8,  7 };

/*!
 * Bandwidths table definition in Hz
 */
const uint32_t Bandwidths[] = { 125e3, 125e3, 125e3, 125e3, 125e3, 125e3 };

/*!
 * Maximum payload with respect to the datarate index. Cannot operate with repeater.
 */
const uint8_t MaxPayloadOfDatarate[] = { 51, 51, 51, 115, 222, 222 };

/*!
 * Maximum payload with respect to the datarate index. Can operate with repeater.
 */
const uint8_t MaxPayloadOfDatarateRepeater[] = { 51, 51, 51, 115, 222, 222 };

/*!
 * Tx output powers table definition
 */
const int8_t TxPowers[]    = { 17, 16, 14, 12, 10, 7, 5, 2 };//

/*!
 * LoRaMac bands
 */
static Band_t Bands[LORA_MAX_NB_BANDS] =
{
    BAND0,
};

/*!
 * LoRaMAC channels
 */
static ChannelParams_t Channels[LORA_MAX_NB_CHANNELS];

#if defined( USE_BAND_470PREQUEL )
//LoRaMac-definitions.h闂傚倸鍊搁崐椋庣矆娓氾拷楠炴牠顢曢敃锟界壕鍦磼鐎ｎ偓绱╂繛宸簼閺呮煡鏌涘☉鍙樼凹闁诲骸顭峰铏规崉閵娿儲鐝㈤梺鐟板殩閹凤拷140闂傚倸鍊峰ù鍥х暦閻㈢纾婚柣鎰暩閻瑩鏌熸潏鍓х暠閻庢艾顦伴妵鍕箳閻愭潙顏堕梻浣瑰濞插繘宕瑰ú顏勭闁告稑鐡ㄩ崐缁樹繆椤栨繃顏犵憸鐗堟尦濮婄粯鎷呴悷閭﹀殝闂佸搫琚崝宥夊疾閼哥數顩烽悗锝庡亜閸撳綊鏌熼崗鑲╂殬闁告柨绉瑰畷鎴︽晲婢跺鍘遍梺闈涱槹閸ㄧ敻骞婅箛娑樼疅闁绘挸绨堕弨浠嬫⒔閸ヮ剦鏁嬫繝濠傜墕缁狀垶鏌ㄩ悤鍌涘
/*!
 * Defines the first channel for RX window 1 for CN470 band
 */
#define LORAMAC_FIRST_RX1_CHANNEL           ( (uint32_t) 471.5e6 )

/*!
 * Defines the last channel for RX window 1 for CN470 band
 */
#define LORAMAC_LAST_RX1_CHANNEL            ( (uint32_t) 472.9e6 )//ast c

#elif defined( USE_BAND_470 )
/*!
 * Defines the first channel for RX window 1 for CN470 band
 */
#define LORAMAC_FIRST_RX1_CHANNEL           ( (uint32_t) 500.3e6 )

/*!
 * Defines the last channel for RX window 1 for CN470 band
 */
#define LORAMAC_LAST_RX1_CHANNEL            ( (uint32_t) 509.7e6 )//ast c

#else 
  #error "please define USE_BAND_470 OR USE_BAND_470PREQUEL."
#endif

/*!
 * Defines the step width of the channels for RX window 1
 */
#define LORAMAC_STEPWIDTH_RX1_CHANNEL       ( (uint32_t) 200e3 )

#elif defined( USE_BAND_780 )
/*!
 * Data rates table definition
 */
const uint8_t Datarates[]  = { 12, 11, 10,  9,  8,  7,  7, 50 };

/*!
 * Bandwidths table definition in Hz
 */
const uint32_t Bandwidths[] = { 125e3, 125e3, 125e3, 125e3, 125e3, 125e3, 250e3, 0 };

/*!
 * Maximum payload with respect to the datarate index. Cannot operate with repeater.
 */
const uint8_t MaxPayloadOfDatarate[] = { 51, 51, 51, 115, 242, 242, 242, 242 };

/*!
 * Maximum payload with respect to the datarate index. Can operate with repeater.
 */
const uint8_t MaxPayloadOfDatarateRepeater[] = { 51, 51, 51, 115, 222, 222, 222, 222 };

/*!
 * Tx output powers table definition
 */
const int8_t TxPowers[]    = { 10, 7, 4, 1, -2, -5 };

/*!
 * LoRaMac bands
 */
static Band_t Bands[LORA_MAX_NB_BANDS] =
{
    BAND0,
};

/*!
 * LoRaMAC channels
 */
static ChannelParams_t Channels[LORA_MAX_NB_CHANNELS] =
{
    LC1,
    LC2,
    LC3,
};
#elif defined( USE_BAND_868 )
/*!
 * Data rates table definition
 */
const uint8_t Datarates[]  = { 12, 11, 10,  9,  8,  7,  7, 50 };

/*!
 * Bandwidths table definition in Hz
 */
const uint32_t Bandwidths[] = { 125e3, 125e3, 125e3, 125e3, 125e3, 125e3, 250e3, 0 };

/*!
 * Maximum payload with respect to the datarate index. Cannot operate with repeater.
 */
const uint8_t MaxPayloadOfDatarate[] = { 51, 51, 51, 115, 242, 242, 242, 242 };

/*!
 * Maximum payload with respect to the datarate index. Can operate with repeater.
 */
const uint8_t MaxPayloadOfDatarateRepeater[] = { 51, 51, 51, 115, 222, 222, 222, 222 };

/*!
 * Tx output powers table definition
 */
const int8_t TxPowers[]    = { 20, 14, 11,  8,  5,  2 };

/*!
 * LoRaMac bands
 */
static Band_t Bands[LORA_MAX_NB_BANDS] =
{
    BAND0,
    BAND1,
    BAND2,
    BAND3,
    BAND4,
};

/*!
 * LoRaMAC channels
 */
static ChannelParams_t Channels[LORA_MAX_NB_CHANNELS] =
{
    LC1,
    LC2,
    LC3,
};
#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
/*!
 * Data rates table definition
 */
const uint8_t Datarates[]  = { 10, 9, 8,  7,  8,  0,  0, 0, 12, 11, 10, 9, 8, 7, 0, 0 };

/*!
 * Bandwidths table definition in Hz
 */
const uint32_t Bandwidths[] = { 125e3, 125e3, 125e3, 125e3, 500e3, 0, 0, 0, 500e3, 500e3, 500e3, 500e3, 500e3, 500e3, 0, 0 };

/*!
 * Up/Down link data rates offset definition
 */
const int8_t DatarateOffsets[5][4] =
{
    { DR_10, DR_9 , DR_8 , DR_8  }, // DR_0
    { DR_11, DR_10, DR_9 , DR_8  }, // DR_1
    { DR_12, DR_11, DR_10, DR_9  }, // DR_2
    { DR_13, DR_12, DR_11, DR_10 }, // DR_3
    { DR_13, DR_13, DR_12, DR_11 }, // DR_4
};

/*!
 * Maximum payload with respect to the datarate index. Cannot operate with repeater.
 */
const uint8_t MaxPayloadOfDatarate[] = { 11, 53, 125, 242, 242, 0, 0, 0, 53, 129, 242, 242, 242, 242, 0, 0 };

/*!
 * Maximum payload with respect to the datarate index. Can operate with repeater.
 */
const uint8_t MaxPayloadOfDatarateRepeater[] = { 11, 53, 125, 242, 242, 0, 0, 0, 33, 109, 222, 222, 222, 222, 0, 0 };

/*!
 * Tx output powers table definition
 */
const int8_t TxPowers[]    = { 30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10 };

/*!
 * LoRaMac bands
 */
static Band_t Bands[LORA_MAX_NB_BANDS] =
{
    BAND0,
};

/*!
 * LoRaMAC channels
 */
static ChannelParams_t Channels[LORA_MAX_NB_CHANNELS];

/*!
 * Contains the channels which remain to be applied.
 */
static uint16_t ChannelsMaskRemaining[6];

/*!
 * Defines the first channel for RX window 1 for US band
 */
#define LORAMAC_FIRST_RX1_CHANNEL           ( (uint32_t) 923.3e6 )

/*!
 * Defines the last channel for RX window 1 for US band
 */
#define LORAMAC_LAST_RX1_CHANNEL            ( (uint32_t) 927.5e6 )

/*!
 * Defines the step width of the channels for RX window 1
 */
#define LORAMAC_STEPWIDTH_RX1_CHANNEL       ( (uint32_t) 600e3 )

#else
    #error "Please define a frequency band in the compiler options."
#endif

/*!
 * LoRaMac parameters
 */
LoRaMacParams_t LoRaMacParams;

/*!
 * LoRaMac default parameters
 */
LoRaMacParams_t LoRaMacParamsDefaults;

/*!
 * Uplink messages repetitions counter
 */
static uint8_t ChannelsNbRepCounter = 0;

/*!
 * Maximum duty cycle
 * \remark Possibility to shutdown the device.
 */
static uint8_t MaxDCycle = 0;

/*!
 * Aggregated duty cycle management
 */
static uint16_t AggregatedDCycle;
static TimerTime_t AggregatedLastTxDoneTime;
static TimerTime_t AggregatedTimeOff;

/*!
 * Enables/Disables duty cycle management (Test only)
 */
static bool DutyCycleOn;

/*!
 * Current channel index
 */
static uint8_t Channel;

/*!
 * Stores the time at LoRaMac initialization.
 *
 * \remark Used for the BACKOFF_DC computation.
 */
static TimerTime_t LoRaMacInitializationTime = 0;

/*!
 * LoRaMac internal states
 */
enum eLoRaMacState
{
    LORAMAC_IDLE          = 0x00000000,
    LORAMAC_TX_RUNNING    = 0x00000001,
    LORAMAC_RX            = 0x00000002,
    LORAMAC_ACK_REQ       = 0x00000004,
    LORAMAC_ACK_RETRY     = 0x00000008,
    LORAMAC_TX_DELAYED    = 0x00000010,
    LORAMAC_TX_CONFIG     = 0x00000020,
    LORAMAC_RX_ABORT      = 0x00000040,
};

/*!
 * LoRaMac internal state
 */
uint32_t LoRaMacState = LORAMAC_IDLE;

/*!
 * LoRaMac timer used to check the LoRaMacState (runs every second)
 */
TimerEvent_t MacStateCheckTimer;

/*!
 * LoRaMac upper layer event functions
 */
static LoRaMacPrimitives_t *LoRaMacPrimitives;

/*!
 * LoRaMac upper layer callback functions
 */
static LoRaMacCallback_t *LoRaMacCallbacks;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * LoRaMac duty cycle delayed Tx timer
 */
static TimerEvent_t TxDelayedTimer;

/*!
 * LoRaMac reception windows timers
 */
static TimerEvent_t RxWindowTimer1;
static TimerEvent_t RxWindowTimer2;

/*!
 * LoRaMac reception windows delay
 * \remark normal frame: RxWindowXDelay = ReceiveDelayX - RADIO_WAKEUP_TIME
 *         join frame  : RxWindowXDelay = JoinAcceptDelayX - RADIO_WAKEUP_TIME
 */
static uint32_t RxWindow1Delay;
static uint32_t RxWindow2Delay;

/*!
 * Rx window parameters
 */
typedef struct
{
    int8_t Datarate;
    uint8_t Bandwidth;
    uint32_t RxWindowTimeout;
    int32_t RxOffset;
}RxConfigParams_t;

/*!
 * Rx windows params
 */
static RxConfigParams_t RxWindowsParams[2];

/*!
 * Acknowledge timeout timer. Used for packet retransmissions.
 */
 static TimerEvent_t AckTimeoutTimer;

/*!
 * Number of trials to get a frame acknowledged
 */
static uint8_t AckTimeoutRetries = 1;

/*!
 * Number of trials to get a frame acknowledged
 */
static uint8_t AckTimeoutRetriesCounter = 1;

/*!
 * Indicates if the AckTimeout timer has expired or not
 */
static bool AckTimeoutRetry = false;

/*!
 * Last transmission time on air
 */
TimerTime_t TxTimeOnAir = 0;

/*!
 * Number of trials for the Join Request
 */
static uint8_t JoinRequestTrials;

/*!
 * Maximum number of trials for the Join Request
 */
static uint8_t MaxJoinRequestTrials;

/*!
 * Structure to hold an MCPS indication data.
 */
static McpsIndication_t McpsIndication;

/*!
 * Structure to hold MCPS confirm data.
 */
static McpsConfirm_t McpsConfirm;

/*!
 * Structure to hold MLME confirm data.
 */
static MlmeConfirm_t MlmeConfirm;

/*!
 * Holds the current rx window slot
 */
static uint8_t RxSlot = 0;

/*!
 * LoRaMac tx/rx operation state
 */
LoRaMacFlags_t LoRaMacFlags;

/*!
 * \brief Function to be executed on Radio Tx Done event
 */
static void OnRadioTxDone( void );

/*!
 * \brief This function prepares the MAC to abort the execution of function
 *        OnRadioRxDone in case of a reception error.
 */
static void PrepareRxDoneAbort( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
static void OnRadioRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
static void OnRadioTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx error event
 */
static void OnRadioRxError( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
static void OnRadioRxTimeout( void );

/*!
 * \brief Function executed on Resend Frame timer event.
 */

static void OnMacStateCheckTimerEvent( void );

/*!
 * \brief Function executed on duty cycle delayed Tx  timer event
 */
static void OnTxDelayedTimerEvent( void );

/*!
 * \brief Function executed on first Rx window timer event
 */
static void OnRxWindow1TimerEvent( void );

/*!
 * \brief Function executed on second Rx window timer event
 */
static void OnRxWindow2TimerEvent( void );

/*!
 * \brief Function executed on AckTimeout timer event
 */
static void OnAckTimeoutTimerEvent( void );

/*!
 * \brief Searches and set the next random available channel
 *
 * \param [OUT] Time to wait for the next transmission according to the duty
 *              cycle.
 *
 * \retval status  Function status [1: OK, 0: Unable to find a channel on the
 *                                  current datarate]
 */
static bool SetNextChannel( TimerTime_t* time );

/*!
 * \brief Initializes and opens the reception window
 *
 * \param [IN] freq window channel frequency
 * \param [IN] datarate window channel datarate
 * \param [IN] bandwidth window channel bandwidth
 * \param [IN] timeout window channel timeout
 *
 * \retval status Operation status [true: Success, false: Fail]
 */
static bool RxWindowSetup( uint32_t freq, int8_t datarate, uint32_t bandwidth, uint16_t timeout, bool rxContinuous );

/*!
 * \brief Verifies if the RX window 2 frequency is in range
 *
 * \param [IN] freq window channel frequency
 *
 * \retval status  Function status [1: OK, 0: Frequency not applicable]
 */
static bool Rx2FreqInRange( uint32_t freq );

/*!
 * \brief Adds a new MAC command to be sent.
 *
 * \Remark MAC layer internal function
 *
 * \param [in] cmd MAC command to be added
 *                 [MOTE_MAC_LINK_CHECK_REQ,
 *                  MOTE_MAC_LINK_ADR_ANS,
 *                  MOTE_MAC_DUTY_CYCLE_ANS,
 *                  MOTE_MAC_RX2_PARAM_SET_ANS,
 *                  MOTE_MAC_DEV_STATUS_ANS
 *                  MOTE_MAC_NEW_CHANNEL_ANS]
 * \param [in] p1  1st parameter ( optional depends on the command )
 * \param [in] p2  2nd parameter ( optional depends on the command )
 *
 * \retval status  Function status [0: OK, 1: Unknown command, 2: Buffer full]
 */
static LoRaMacStatus_t AddMacCommand( uint8_t cmd, uint8_t p1, uint8_t p2 );

/*!
 * \brief Parses the MAC commands which must be repeated.
 *
 * \Remark MAC layer internal function
 *
 * \param [IN] cmdBufIn  Buffer which stores the MAC commands to send
 * \param [IN] length  Length of the input buffer to parse
 * \param [OUT] cmdBufOut  Buffer which stores the MAC commands which must be
 *                         repeated.
 *
 * \retval Size of the MAC commands to repeat.
 */
static uint8_t ParseMacCommandsToRepeat( uint8_t* cmdBufIn, uint8_t length, uint8_t* cmdBufOut );

/*!
 * \brief Validates if the payload fits into the frame, taking the datarate
 *        into account.
 *
 * \details Refer to chapter 4.3.2 of the LoRaWAN specification, v1.0
 *
 * \param lenN Length of the application payload. The length depends on the
 *             datarate and is region specific
 *
 * \param datarate Current datarate
 *
 * \param fOptsLen Length of the fOpts field
 *
 * \retval [false: payload does not fit into the frame, true: payload fits into
 *          the frame]
 */
static bool ValidatePayloadLength( uint8_t lenN, int8_t datarate, uint8_t fOptsLen );

/*!
 * \brief Counts the number of bits in a mask.
 *
 * \param [IN] mask A mask from which the function counts the active bits.
 * \param [IN] nbBits The number of bits to check.
 *
 * \retval Number of enabled bits in the mask.
 */
static uint8_t CountBits( uint16_t mask, uint8_t nbBits );

#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
/*!
 * \brief Counts the number of enabled 125 kHz channels in the channel mask.
 *        This function can only be applied to US915 band.
 *
 * \param [IN] channelsMask Pointer to the first element of the channel mask
 *
 * \retval Number of enabled channels in the channel mask
 */
static uint8_t CountNbEnabled125kHzChannels( uint16_t *channelsMask );

#if defined( USE_BAND_915_HYBRID )
/*!
 * \brief Validates the correctness of the channel mask for US915, hybrid mode.
 *
 * \param [IN] mask Block definition to set.
 * \param [OUT] channelsMask Pointer to the first element of the channel mask
 */
static void ReenableChannels( uint16_t mask, uint16_t* channelsMask );

/*!
 * \brief Validates the correctness of the channel mask for US915, hybrid mode.
 *
 * \param [IN] channelsMask Pointer to the first element of the channel mask
 *
 * \retval [true: channel mask correct, false: channel mask not correct]
 */
static bool ValidateChannelMask( uint16_t* channelsMask );
#endif

#endif

/*!
 * \brief Validates the correctness of the datarate against the enable channels.
 *
 * \param [IN] datarate Datarate to be check
 * \param [IN] channelsMask Pointer to the first element of the channel mask
 *
 * \retval [true: datarate can be used, false: datarate can not be used]
 */
static bool ValidateDatarate( int8_t datarate, uint16_t* channelsMask );

/*!
 * \brief Limits the Tx power according to the number of enabled channels
 *
 * \param [IN] txPower txPower to limit
 * \param [IN] maxBandTxPower Maximum band allowed TxPower
 *
 * \retval Returns the maximum valid tx power
 */
static int8_t LimitTxPower( int8_t txPower, int8_t maxBandTxPower );

/*!
 * \brief Verifies, if a value is in a given range.
 *
 * \param value Value to verify, if it is in range
 *
 * \param min Minimum possible value
 *
 * \param max Maximum possible value
 *
 * \retval Returns the maximum valid tx power
 */
static bool ValueInRange( int8_t value, int8_t min, int8_t max );

/*!
 * \brief Calculates the next datarate to set, when ADR is on or off
 *
 * \param [IN] adrEnabled Specify whether ADR is on or off
 *
 * \param [IN] updateChannelMask Set to true, if the channel masks shall be updated
 *
 * \param [OUT] datarateOut Reports the datarate which will be used next
 *
 * \retval Returns the state of ADR ack request
 */
static bool AdrNextDr( bool adrEnabled, bool updateChannelMask, int8_t* datarateOut );

/*!
 * \brief Disables channel in a specified channel mask
 *
 * \param [IN] id - Id of the channel
 *
 * \param [IN] mask - Pointer to the channel mask to edit
 *
 * \retval [true, if disable was successful, false if not]
 */
static bool DisableChannelInMask( uint8_t id, uint16_t* mask );

/*!
 * \brief Decodes MAC commands in the fOpts field and in the payload
 */
static void ProcessMacCommands( uint8_t *payload, uint8_t macIndex, uint8_t commandsSize, uint8_t snr );

/*!
 * \brief LoRaMAC layer generic send frame
 *
 * \param [IN] macHdr      MAC header field
 * \param [IN] fPort       MAC payload port
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t Send( LoRaMacHeader_t *macHdr, uint8_t fPort, void *fBuffer, uint16_t fBufferSize );

/*!
 * \brief LoRaMAC layer frame buffer initialization
 *
 * \param [IN] macHdr      MAC header field
 * \param [IN] fCtrl       MAC frame control field
 * \param [IN] fOpts       MAC commands buffer
 * \param [IN] fPort       MAC payload port
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t PrepareFrame( LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t fPort, void *fBuffer, uint16_t fBufferSize );

/*
 * \brief Schedules the frame according to the duty cycle
 *
 * \retval Status of the operation
 */
static LoRaMacStatus_t ScheduleTx( void );

/*
 * \brief Sets the duty cycle for the join procedure.
 *
 * \retval Duty cycle
 */
static uint16_t JoinDutyCycle( void );

/*
 * \brief Calculates the back-off time for the band of a channel.
 *
 * \param [IN] channel     The last Tx channel index
 */
static void CalculateBackOff( uint8_t channel );

/*
 * \brief Alternates the datarate of the channel for the join request.
 *
 * \param [IN] nbTrials    Number of performed join requests.
 * \retval Datarate to apply
 */
static int8_t AlternateDatarate( uint16_t nbTrials );

/*!
 * \brief LoRaMAC layer prepared frame buffer transmission with channel specification
 *
 * \remark PrepareFrame must be called at least once before calling this
 *         function.
 *
 * \param [IN] channel     Channel parameters
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t SendFrameOnChannel( ChannelParams_t channel );

/*!
 * \brief Sets the radio in continuous transmission mode
 *
 * \remark Uses the radio parameters set on the previous transmission.
 *
 * \param [IN] timeout     Time in seconds while the radio is kept in continuous wave mode
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t SetTxContinuousWave( uint16_t timeout );

/*!
 * \brief Sets the radio in continuous transmission mode
 *
 * \remark Uses the radio parameters set on the previous transmission.
 *
 * \param [IN] timeout     Time in seconds while the radio is kept in continuous wave mode
 * \param [IN] frequency   RF frequency to be set.
 * \param [IN] power       RF ouptput power to be set.
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t SetTxContinuousWave1( uint16_t timeout, uint32_t frequency, uint8_t power );

/*!
 * \brief Resets MAC specific parameters to default
 */
static void ResetMacParameters( void );

/*
 * Rx window precise timing
 *
 * For more details please consult the following document, chapter 3.1.2.
 * http://www.semtech.com/images/datasheet/SX1272_settings_for_LoRaWAN_v2.0.pdf
 * or
 * http://www.semtech.com/images/datasheet/SX1276_settings_for_LoRaWAN_v2.0.pdf
 *
 *                 Downlink start: T = Tx + 1s (+/- 20 us)
 *                            |
 *             TRxEarly       |        TRxLate
 *                |           |           |
 *                |           |           +---+---+---+---+---+---+---+---+
 *                |           |           |       Latest Rx window        |
 *                |           |           +---+---+---+---+---+---+---+---+
 *                |           |           |
 *                +---+---+---+---+---+---+---+---+
 *                |       Earliest Rx window      |
 *                +---+---+---+---+---+---+---+---+
 *                            |
 *                            +---+---+---+---+---+---+---+---+
 *Downlink preamble 8 symbols |   |   |   |   |   |   |   |   |
 *                            +---+---+---+---+---+---+---+---+
 *
 *                     Worst case Rx window timings
 *
 * TRxLate  = DEFAULT_MIN_RX_SYMBOLS * tSymbol - RADIO_WAKEUP_TIME
 * TRxEarly = 8 - DEFAULT_MIN_RX_SYMBOLS * tSymbol - RxWindowTimeout - RADIO_WAKEUP_TIME
 *
 * TRxLate - TRxEarly = 2 * DEFAULT_SYSTEM_MAX_RX_ERROR
 *
 * RxOffset = ( TRxLate + TRxEarly ) / 2
 *
 * RxWindowTimeout = ( 2 * DEFAULT_MIN_RX_SYMBOLS - 8 ) * tSymbol + 2 * DEFAULT_SYSTEM_MAX_RX_ERROR
 * RxOffset = 4 * tSymbol - RxWindowTimeout / 2 - RADIO_WAKE_UP_TIME
 *
 * Minimal value of RxWindowTimeout must be 5 symbols which implies that the system always tolerates at least an error of 1.5 * tSymbol
 */
/*!
 * Computes the Rx window parameters.
 *
 * \param [IN] datarate     Rx window datarate to be used
 * \param [IN] rxError      Maximum timing error of the receiver. in milliseconds
 *                          The receiver will turn on in a [-rxError : +rxError] ms
 *                          interval around RxOffset
 *
 * \retval rxConfigParams   Returns a RxConfigParams_t structure.
 */
static RxConfigParams_t ComputeRxWindowParameters( int8_t datarate, uint32_t rxError );

static void OnRadioTxDone( void )
{
    TimerTime_t curTime = TimerGetCurrentTime( );

   // lora_printf("OnRadioTxDone\n");
    if( LoRaMacDeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    else
    {
        OnRxWindow2TimerEvent( );
    }

    // Setup timers
    //濠电姷鏁告慨鐑藉极閹间礁纾婚柣鎰惈閸ㄥ倿鏌涢锝嗙缂佺姳鍗抽弻鐔兼⒒鐎垫瓕绐楅梺杞扮鐎氫即寮诲☉銏╂晝闁绘ɑ褰冩慨宀勬⒑鐠恒劌鏋戦柛銊︾箘濞嗐垹顫濋崜浣哥ウ闁诲函缍嗛崰妤呭疾濠靛鐓ラ柣鏂垮閻瑧锟芥鍠栭幉锛勭箔閻斿吋鏅稿ù鐘差儏閽冪喖鏌ㄥ┑鍡樺晵闁哄啫鐗婇弲绋棵归悩宸剱闁稿顑夐弻娑㈠焺閸愵亝鍣梺娲诲幖閻楁捇寮婚妸鈺佸嵆闁绘劘顕滈幏鐤亹閹烘挸浜楅梺缁樻煥閸氬鎮¤箛鎾斀闁绘劘灏欐禒銏ゆ煕閺冿拷绾板秹濡甸崟顔剧杸濠电姴鍟悵鏍磽娴ｄ粙鍝洪悽顖涱殔椤洩绠涘☉妯溾晠鏌曟竟顖氬�归、姗�姊婚崒娆戠獢闁跨喕濮ょ粙鎾诲煘閹烘鐓曢柡鍌氭健閸欏嫰鏌℃担瑙勫磳鐎殿噮鍓熸俊鍫曞炊瑜嶉悡鍌氣攽閻橆偅濯伴柛鎰ㄦ櫆瀹曞磭绱撴笟鍥ф灈闁绘锕ら銉╁礋椤栨氨鐤�濡炪倖鎸炬慨鎾嵁閹扮増鈷掑ù锝呮啞閸熺偞绻涢懠顒�鏋庢い顓炴穿椤﹀綊鏌熼鏂よ�块柟顔规櫅閻ｇ兘宕堕埡鍐╊潓闂傚倷娴囧▔鏇㈠窗瀹ュ洤鍨濇繛鍡樻尪閿熸垝绶氬顕�宕煎┑瀣暪婵犵數濞�濞佳囨偋韫囨稑鍌ㄦい鎺戝閳锋帒霉閿濆嫯顒熼柡锟芥导瀛樼厵婵炶尪顔婇柇顖涱殽閻愯尙绠婚柡浣规崌閺佹捇鏁撻敓锟�?婵犵數濮烽弫鎼佸磻閻樿绠垫い蹇撴缁躲倕霉閻樺樊鍎忕紒鐘靛劋缁绘繃绻濋崒婊冾暫濠殿喚鎳撳鍫曞箞閵娿儙鐔煎垂椤旀儳甯块梻浣虹帛閹稿鎮烽埡鍛畺婵°倧鎷烽柍钘夘槸閳诲秹顢樿缁ㄥジ鏌熸笟鍨鐎规洖鐖奸、妤佹媴閸濆嫬绠ュ┑锛勫亼閸婃牕顫忔繝姘仱闁哄倸绨遍弸鏃�绻濇繝鍌滃闁绘挻绋戦湁闁挎繂鎳忛幉鎼佹嚃閺嶃劎绠鹃悗鐢殿焾椤庢挾绱掗悩铏磳妤犵偛锕幃娆撳传閸曞簺鍔嶉妵鍕籍閸噦绱电紓鍌氬�归幑鍥ь潖婵犳艾纾兼繛鍡樺焾濡差噣姊洪崷顓涙嫛闁稿锕ら悾鐑筋敍閻戝棙鏅┑顔缴戝畷姗�鏁撻挊澶嬪仴闁哄本鐩弫鍌滄嫚閹绘帞顔掗梻浣告啞椤ㄥ棗鐣濋幖浣歌摕闁斥晛鍟欢鐐烘倵閿濆簼绨藉ù婊冪埣濮婃椽宕崟闈涘壉濠碘槅鍋勯崯鏉戭嚕鐠囨祴妲堥柍鍨涙櫔閹风兘鏁冮崒姘鳖吅濠电娀娼ч幊鎰板礄鐟欏嫪绻嗛柣鎰典簷缁辨牕霉閿濆懏鍟炲璺哄閺屽秹鏌ㄧ�ｎ亝璇為梺鍝勬湰缁嬫挻绂掗敃鍌氱鐟滃繘鎮惧ú顏呭仭婵犲﹤瀚儫闂佸憡渚楅崹鎶芥晬濠婂牊鐓涘璺猴功婢ф垿鏌涢弬鎸庢崳缂侇喖顭烽幃娆戝枈婢跺顏堕柣鐔哥懃鐎氼厾绮堥崘顏庢嫹閻у憡瀚圭紓浣割儓濞夋洟寮抽敃鍌涚厱婵°倕鍟敓鏂ょ秮瀵噣宕奸悢鍛婎仧濠电偛顕慨鎾敄閸℃稑纾归柛褎顨嗛悡鐘崇箾閺夋埈鍎愭繛鍛閺岀喖鎮滅粵瀣棖闂佸摜濮撮敃銉ヮ嚗閸曨叏鎷烽敐搴濇喚濞寸》鎷�?闂傚倸鍊搁崐鎼佸磹妞嬪海鐭嗗〒姘炬嫹妤犵偛顦甸崹楣冨箛娴ｇ懓鍏婇梻渚�娼ц墝闁哄懏鐩幏鎴︽偄闂堢偛閰ｅ畷鍫曟煥鐎ｎ亶浼�1闂傚倸鍊搁崐鎼佸磹妞嬪海鐭嗗〒姘炬嫹妤犵偛顦甸弫宥夊礋椤愩垻浜伴柣搴ｆ閹风兘鏌涢弴鐐典粵闁哄懌鍨藉鍝勭暦閸パ呮闂佺懓鍤栭幏锟�2闂傚倸鍊搁崐鎼佸磹瀹勬噴褰掑炊椤掑﹦绋忔繝銏ｆ硾椤戝洭銆呴幓鎹楀綊鎮╁顔煎壈缂佺偓鍎冲锟犲蓟閿涘嫪娌悹鍥ㄥ絻婵酣姊洪崫鍕靛剮缂佽埖宀稿璇测槈閵忊剝娅嗛柣鐘叉处瑜板啰绮绘繝姘拺缂備焦顭囬惌宀�绱掗鍛仭缂佸矁椴哥换婵嬪炊椤儸鍥╁彄闁搞儯鍔嶉悡銉︺亜韫囨挾鍩ｉ柡宀嬬秮閹垽宕妷褏鍘愰梺鍦帶閻°劎鍒掗幘鑽ゅ祦闊洦绋掗弲鎼佹煥閻曞倹瀚�
    if( IsRxWindowsEnabled == true )
    {
    	//lora_printf("RxWindow1Delay:  %d\n",RxWindow1Delay);
        TimerSetValue( &RxWindowTimer1, RxWindow1Delay );
        TimerStart( &RxWindowTimer1 );
        if( LoRaMacDeviceClass != CLASS_C )
        {
            TimerSetValue( &RxWindowTimer2, RxWindow2Delay );
            TimerStart( &RxWindowTimer2 );
        }
        if( ( LoRaMacDeviceClass == CLASS_C ) || ( NodeAckRequested == true ) )
        {
            TimerSetValue( &AckTimeoutTimer, RxWindow2Delay + ACK_TIMEOUT +
                                             randr( -ACK_TIMEOUT_RND, ACK_TIMEOUT_RND ) );
            TimerStart( &AckTimeoutTimer );
        }
    }
    else
    {
        McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;
        MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT;

        if( LoRaMacFlags.Value == 0 )
        {
            LoRaMacFlags.Bits.McpsReq = 1;
        }
        LoRaMacFlags.Bits.MacDone = 1;
    }

    // Update last tx done time for the current channel
    Bands[Channels[Channel].Band].LastTxDoneTime = curTime;
    // Update Aggregated last tx done time
    AggregatedLastTxDoneTime = curTime;
    // Update Backoff
    CalculateBackOff( Channel );

    if( NodeAckRequested == false )
    {
        McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;
        ChannelsNbRepCounter++;
    }
}

static void PrepareRxDoneAbort( void )
{
    LoRaMacState |= LORAMAC_RX_ABORT;

    if( NodeAckRequested )
    {
        OnAckTimeoutTimerEvent( );
    }

    LoRaMacFlags.Bits.McpsInd = 1;
    LoRaMacFlags.Bits.MacDone = 1;

    // Trig OnMacCheckTimerEvent call as soon as possible
//    TimerSetValue( &MacStateCheckTimer, 1 );
//    TimerStart( &MacStateCheckTimer );
    OnMacStateCheckTimerEvent();
}
//OnRadioRxDone闂傚倸鍊搁崐鎼佸磹妞嬪海鐭嗗ù锝夋交閼板潡姊洪锟界粔鎾磼閵娧勫枑闁哄啫鐗忓畵渚�鏌″搴″季闁轰礁鍟撮弻銊╁籍閸ヮ煈妫勬繛瀵稿Т閹碱偊鈥旈崘顔嘉ч柛鈩冾殔琛奸梻鍌氬�哥�氼剛锟芥碍婢橀悾鐑藉即閵忊剝娅㈤梺璺ㄥ櫐閹凤拷?MAC闂傚倸鍊搁崐鐑芥倿閿曪拷椤啴宕归鍛數婵炴挻鐔幏閿嬵殽閻愯尙绠婚柡浣规崌閺佹捇鏁撻敓锟�?
static void OnRadioRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
	//lora_printf("OnRadioRxDone\n");

    LoRaMacHeader_t macHdr;
    LoRaMacFrameCtrl_t fCtrl;
    bool skipIndication = false;

    uint8_t pktHeaderLen = 0;
    uint32_t address = 0;
    uint8_t appPayloadStartIndex = 0;
    uint8_t port = 0xFF;
    uint8_t frameLen = 0;
    uint32_t mic = 0;
    uint32_t micRx = 0;

    uint16_t sequenceCounter = 0;
    uint16_t sequenceCounterPrev = 0;
    uint16_t sequenceCounterDiff = 0;
    uint32_t downLinkCounter = 0;

    MulticastParams_t *curMulticastParams = NULL;
    uint8_t *nwkSKey = LoRaMacNwkSKey;
    uint8_t *appSKey = LoRaMacAppSKey;

    uint8_t multicast = 0;

    bool isMicOk = false;

    McpsConfirm.AckReceived = false;
    McpsIndication.Rssi = rssi;
    McpsIndication.Snr = snr;
    McpsIndication.RxSlot = RxSlot;
    McpsIndication.Port = 0;
    McpsIndication.Multicast = 0;
    McpsIndication.FramePending = 0;
    McpsIndication.Buffer = NULL;
    McpsIndication.BufferSize = 0;
    McpsIndication.RxData = false;
    McpsIndication.AckReceived = false;
    McpsIndication.DownLinkCounter = 0;
    McpsIndication.McpsIndication = MCPS_UNCONFIRMED;

    Radio.Sleep( );
    TimerStop( &RxWindowTimer2 );

    macHdr.Value = payload[pktHeaderLen++];
    //lora_printf("macHdr.Value:%d\n",macHdr.Value);
    //lora_printf("macHdr.Value:%d\n",SX1276Read(0x00));
    switch( macHdr.Bits.MType )
    {
        case FRAME_TYPE_JOIN_ACCEPT:
            if( IsLoRaMacNetworkJoined == true )
            {
                McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
                PrepareRxDoneAbort( );
                return;
            }
            LoRaMacJoinDecrypt( payload + 1, size - 1, LoRaMacAppKey, LoRaMacRxPayload + 1 );

            LoRaMacRxPayload[0] = macHdr.Value;

            LoRaMacJoinComputeMic( LoRaMacRxPayload, size - LORAMAC_MFR_LEN, LoRaMacAppKey, &mic );

            micRx |= ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN];
            micRx |= ( ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN + 1] << 8 );
            micRx |= ( ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN + 2] << 16 );
            micRx |= ( ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN + 3] << 24 );

            if( micRx == mic )
            {
                LoRaMacJoinComputeSKeys( LoRaMacAppKey, LoRaMacRxPayload + 1, LoRaMacDevNonce, LoRaMacNwkSKey, LoRaMacAppSKey );

                LoRaMacNetID = ( uint32_t )LoRaMacRxPayload[4];
                LoRaMacNetID |= ( ( uint32_t )LoRaMacRxPayload[5] << 8 );
                LoRaMacNetID |= ( ( uint32_t )LoRaMacRxPayload[6] << 16 );

                LoRaMacDevAddr = ( uint32_t )LoRaMacRxPayload[7];
                LoRaMacDevAddr |= ( ( uint32_t )LoRaMacRxPayload[8] << 8 );
                LoRaMacDevAddr |= ( ( uint32_t )LoRaMacRxPayload[9] << 16 );
                LoRaMacDevAddr |= ( ( uint32_t )LoRaMacRxPayload[10] << 24 );

                // DLSettings
                LoRaMacParams.Rx1DrOffset = ( LoRaMacRxPayload[11] >> 4 ) & 0x07;
                LoRaMacParams.Rx2Channel.Datarate = LoRaMacRxPayload[11] & 0x0F;

                // RxDelay
                LoRaMacParams.ReceiveDelay1 = ( LoRaMacRxPayload[12] & 0x0F );
                if( LoRaMacParams.ReceiveDelay1 == 0 )
                {
                    LoRaMacParams.ReceiveDelay1 = 1;
                }
                LoRaMacParams.ReceiveDelay1 *= 1e3;
                LoRaMacParams.ReceiveDelay2 = LoRaMacParams.ReceiveDelay1 + 1e3;

#if !( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) )
                //CFList
                if( ( size - 1 ) > 16 )
                {
                    ChannelParams_t param;
                    param.DrRange.Value = ( DR_5 << 4 ) | DR_0;

                    LoRaMacState |= LORAMAC_TX_CONFIG;
                    for( uint8_t i = 3, j = 0; i < ( 5 + 3 ); i++, j += 3 )
                    {
                        param.Frequency = ( ( uint32_t )LoRaMacRxPayload[13 + j] | ( ( uint32_t )LoRaMacRxPayload[14 + j] << 8 ) | ( ( uint32_t )LoRaMacRxPayload[15 + j] << 16 ) ) * 100;
                        if( param.Frequency != 0 )
                        {
                            LoRaMacChannelAdd( i, param );
                        }
                        else
                        {
                            LoRaMacChannelRemove( i );
                        }
                    }
                    LoRaMacState &= ~LORAMAC_TX_CONFIG;
                }
#endif
                MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                IsLoRaMacNetworkJoined = true;
                LoRaMacParams.ChannelsDatarate = LoRaMacParamsDefaults.ChannelsDatarate;
            }
            else
            {
                MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL;
            }
            break;
        case FRAME_TYPE_DATA_CONFIRMED_DOWN:
        case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
            {
                address = payload[pktHeaderLen++];
                address |= ( (uint32_t)payload[pktHeaderLen++] << 8 );
                address |= ( (uint32_t)payload[pktHeaderLen++] << 16 );
                address |= ( (uint32_t)payload[pktHeaderLen++] << 24 );

                if( address != LoRaMacDevAddr )
                {
                    curMulticastParams = MulticastChannels;
                    while( curMulticastParams != NULL )
                    {
                        if( address == curMulticastParams->Address )
                        {
                            multicast = 1;
                            nwkSKey = curMulticastParams->NwkSKey;
                            appSKey = curMulticastParams->AppSKey;
                            downLinkCounter = curMulticastParams->DownLinkCounter;
                            break;
                        }
                        curMulticastParams = curMulticastParams->Next;
                    }
                    if( multicast == 0 )
                    {
                        // We are not the destination of this frame.
                        McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL;
                        PrepareRxDoneAbort( );
                        return;
                    }
                }
                else
                {
                    multicast = 0;
                    nwkSKey = LoRaMacNwkSKey;
                    appSKey = LoRaMacAppSKey;
                    downLinkCounter = DownLinkCounter;
                }

                fCtrl.Value = payload[pktHeaderLen++];

                sequenceCounter = ( uint16_t )payload[pktHeaderLen++];
                sequenceCounter |= ( uint16_t )payload[pktHeaderLen++] << 8;

                appPayloadStartIndex = 8 + fCtrl.Bits.FOptsLen;

                micRx |= ( uint32_t )payload[size - LORAMAC_MFR_LEN];
                micRx |= ( ( uint32_t )payload[size - LORAMAC_MFR_LEN + 1] << 8 );
                micRx |= ( ( uint32_t )payload[size - LORAMAC_MFR_LEN + 2] << 16 );
                micRx |= ( ( uint32_t )payload[size - LORAMAC_MFR_LEN + 3] << 24 );

                sequenceCounterPrev = ( uint16_t )downLinkCounter;
                sequenceCounterDiff = ( sequenceCounter - sequenceCounterPrev );

                if( sequenceCounterDiff < ( 1 << 15 ) )
                {
                    downLinkCounter += sequenceCounterDiff;
                    LoRaMacComputeMic( payload, size - LORAMAC_MFR_LEN, nwkSKey, address, DOWN_LINK, downLinkCounter, &mic );
                    if( micRx == mic )
                    {
                        isMicOk = true;
                    }
                }
                else
                {
                    // check for sequence roll-over
                    uint32_t  downLinkCounterTmp = downLinkCounter + 0x10000 + ( int16_t )sequenceCounterDiff;
                    LoRaMacComputeMic( payload, size - LORAMAC_MFR_LEN, nwkSKey, address, DOWN_LINK, downLinkCounterTmp, &mic );
                    if( micRx == mic )
                    {
                        isMicOk = true;
                        downLinkCounter = downLinkCounterTmp;
                    }
                }

                // Check for a the maximum allowed counter difference
                if( sequenceCounterDiff >= MAX_FCNT_GAP )
                {
                    McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_DOWNLINK_TOO_MANY_FRAMES_LOSS;
                    McpsIndication.DownLinkCounter = downLinkCounter;
                    PrepareRxDoneAbort( );
                    return;
                }

                if( isMicOk == true )
                {
                    McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                    McpsIndication.Multicast = multicast;
                    McpsIndication.FramePending = fCtrl.Bits.FPending;
                    McpsIndication.Buffer = NULL;
                    McpsIndication.BufferSize = 0;
                    McpsIndication.DownLinkCounter = downLinkCounter;

                    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;

                    AdrAckCounter = 0;
                    MacCommandsBufferToRepeatIndex = 0;

                    // Update 32 bits downlink counter
                    if( multicast == 1 )
                    {
                        McpsIndication.McpsIndication = MCPS_MULTICAST;

                        if( ( curMulticastParams->DownLinkCounter == downLinkCounter ) &&
                            ( curMulticastParams->DownLinkCounter != 0 ) )
                        {
                            McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED;
                            McpsIndication.DownLinkCounter = downLinkCounter;
                            PrepareRxDoneAbort( );
                            return;
                        }
                        curMulticastParams->DownLinkCounter = downLinkCounter;
                    }
                    else
                    {
                        if( macHdr.Bits.MType == FRAME_TYPE_DATA_CONFIRMED_DOWN )
                        {
                            SrvAckRequested = true;
                            McpsIndication.McpsIndication = MCPS_CONFIRMED;

                            if( ( DownLinkCounter == downLinkCounter ) &&
                                ( DownLinkCounter != 0 ) )
                            {
                                // Duplicated confirmed downlink. Skip indication.
                                // In this case, the MAC layer shall accept the MAC commands
                                // which are included in the downlink retransmission.
                                // It should not provide the same frame to the application
                                // layer again.
                                skipIndication = true;
                            }
                        }
                        else
                        {
                            SrvAckRequested = false;
                            McpsIndication.McpsIndication = MCPS_UNCONFIRMED;

                            if( ( DownLinkCounter == downLinkCounter ) &&
                                ( DownLinkCounter != 0 ) )
                            {
                                McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED;
                                McpsIndication.DownLinkCounter = downLinkCounter;
                                PrepareRxDoneAbort( );
                                return;
                            }
                        }
                        DownLinkCounter = downLinkCounter;
                    }

                    // This must be done before parsing the payload and the MAC commands.
                    // We need to reset the MacCommandsBufferIndex here, since we need
                    // to take retransmissions and repititions into account. Error cases
                    // will be handled in function OnMacStateCheckTimerEvent.
                    if( McpsConfirm.McpsRequest == MCPS_CONFIRMED )
                    {
                        if( fCtrl.Bits.Ack == 1 )
                        {// Reset MacCommandsBufferIndex when we have received an ACK.
                            MacCommandsBufferIndex = 0;
                        }
                    }
                    else
                    {// Reset the variable if we have received any valid frame.
                        MacCommandsBufferIndex = 0;
                    }

                    // Process payload and MAC commands
                    if( ( ( size - 4 ) - appPayloadStartIndex ) > 0 )
                    {
                        port = payload[appPayloadStartIndex++];
                        frameLen = ( size - 4 ) - appPayloadStartIndex;

                        McpsIndication.Port = port;
                        //MAC闂傚倸鍊搁崐鎼佸磹妞嬪海鐭嗗〒姘炬嫹妤犵偛顦甸弫宥夊礋椤撶姷鍘梻浣告贡閸庛倝銆冮崨鏉戝瀭闁稿本绋撶弧锟介梻鍌氱墛娓氭宕曡箛鏇犵＜闁绘瑱鎷烽柛锝忕秮瀵寮撮姀鐘诲敹濠电姴鐏氶崝鏍ㄦ櫏闂傚倷绀侀幖顐﹀箯鐎ｎ喖绀堟繛鎴欏灪閺呮悂鏌ｉ敐鍛拱妞ゅ繒鍠栭弻鐔割槹鎼粹�冲箣闂佸搫鏈惄顖炵嵁閸ヮ剦鏁婇梻鍌涙綑椤ユ岸姊绘担瑙勩仧闁告ü绮欓妴鍐幢濞嗘劕搴婂┑鐐村灟閸ㄥ湱绮婚敐澶嬬厽闁归偊鍓﹂崵鐔兼煕濞嗗繑顥㈡慨濠呮缁辨帒顫滈崱妯兼殽闂備胶鎳撻崯璺ㄦ崲閹烘绠查柕蹇曞Л濡插牓鏌曡箛鏇炐ラ柛濠勫仜椤啴濡堕崱妤冪憪缂備緤鎷峰ù锝呮贡椤╅攱绻涘顔荤凹闁抽攱鍨块弻锝夊箛椤栨侗妫勯梺鍛婃煟閸婃牠濡甸崟顖毼╅柕澹拋妲穜t0闂傚倸鍊搁崐鎼佸磹妞嬪海鐭嗗〒姘炬嫹妤犵偞鐗犻、鏇氱秴闁搞儺鍓﹂弫鍥煏韫囨洖啸闁挎稓鍠栧娲川婵犲孩鐣奸梺绋款儑閸犳牕顕ｉ幎鑺ユ櫇闁稿本绋撻崢浠嬫⒑閸濆嫬锟芥悂鎮樺┑瀣畺闁硅揪闄勯悡鏇㈡倵閿濆骸浜滈柣蹇ョ畵閺屾洟宕惰椤忣厽顨ラ悙宸剶闁轰焦鍔欏畷鍗炩枎瀹ュ繑瀚瑰┑鐘叉处閳锋垹绱撴担濮戭亝鎱ㄩ崶顒佺叆闁哄洢鍔嬬花缁樸亜閺囶亞绉�规洖銈稿鎾倷閸濆嫭鏆梻鍌欒兌缁垶寮婚妸鈺佺疅闁跨喓濮寸壕鍧楁煙閹冾暢缁炬崘妫勯湁闁挎繂瀚惌娆撴煕濠靛﹥顏犵紒杈ㄥ笧閹风娀骞撻幒婵呯磾婵°倗濮烽崑娑⑺囬悽绋跨畺鐟滄柨鐣锋總鍛婂亜濠靛倸顧�閹风兘骞樼紙鐘电畾闂佺粯鍔曞Ο濠囧磿韫囨梻绠鹃悗鐢殿焾閸樺锟芥鍠栭悥濂稿箠閻愬搫唯鐟滃繘顢欓弴銏＄厸濠㈣泛锕︽晶鏇烆熆瑜忛弲顐ゅ垝閺冨牜鏁嗗ù锝堟椤旀洘绻濋姀锝嗙【妞ゆ垵妫涚划鍫拷锝庡墾閹峰嘲鈻撻崹顔界彯闂佺顑呴敃銉︾┍婵犲洦鍤嬮梻鍫熺〒缁愮偤鏌ｆ惔顖滃埌闁诲繑宀歌棢闁跨噦鎷�?闂傚倸鍊搁崐鎼佸磹閻戣姤鍊块柨鏇炲�归崕鎴犳喐閻楀牆绗掔紒锟芥径宀嬫嫹閻у憡瀚归梺鍛婃处閸嬪嫰骞嗛悙鐑樷拺缁绢厼鎳忕涵鑸典繆椤愩値娼╰闂傚倸鍊搁崐鎼佸磹妞嬪海鐭嗗〒姘炬嫹妤犵偞鐗犻、鏇氱秴闁搞儺鍓﹂弫鍥煏韫囨洖啸闁挎稓鍠栧缁樼瑹閸パ冾潻缂備礁顦顓犲垝婵犳凹鏁嶆慨锝庡幖缂嶅﹤鐣峰Δ鍛闁兼祴鏅濋埢蹇涙⒒娴ｅ憡鎯堟い鎴濆濞嗐垹顫濈捄楦挎憰闂佹寧绻傞幉姗�寮崼鐔告婵炲濮撮鍡涘磻鐎ｎ喗鐓曢柍鈺佸暟閹冲懘鏌ｉ鐐靛ⅵ闁哄被鍔戝鎾敂閸涱収浼撻梻鍌欑閻ゅ洤螞閸曨垁鍥偨缁嬫寧鐎梺鐟板⒔缁垶寮查幖浣圭叆闁绘洖鍊圭�氾拷?闂傚倸鍊搁崐宄懊归崶褏鏆﹂柛顭戝亝閸欏繒锟藉箍鍎遍ˇ顖炵嵁閵忊�茬箚妞ゆ牗姘ㄦ禒銏ゆ煙椤斿吋鍣界紒杈ㄦ崌瀹曟帒鈻庨幇顔哄仒缂傚倷绶￠崳锝囩不閺嵮呮殾婵炲樊浜滈崘锟介悷婊勭矒閹偞绂掔�ｎ偆鍙嗛梺缁樻礀閸婂湱锟芥熬鎷�?
                        if( port == 0 )
                        {
                            // Only allow frames which do not have fOpts
                            if( fCtrl.Bits.FOptsLen == 0 )
                            {
                                LoRaMacPayloadDecrypt( payload + appPayloadStartIndex,
                                                       frameLen,
                                                       nwkSKey,
                                                       address,
                                                       DOWN_LINK,
                                                       downLinkCounter,
                                                       LoRaMacRxPayload );

                                // Decode frame payload MAC commands
                                ProcessMacCommands( LoRaMacRxPayload, 0, frameLen, snr );
                            }
                            else
                            {
                                skipIndication = true;
                            }
                        }
                        else//闂傚倸鍊搁崐椋庣矆娓氾拷楠炴牠顢曚綅閸ヮ剦鏁冮柨鏇嫹闁汇倗鍋撶换婵囩節閸屾粌顤�闂佺顑戠换婵嬪蓟瀹ュ浼犻柛鏇ㄥ墮濞咃綁姊婚崒姘簽闁搞劋鍗抽垾鏃堝礃椤忓孩瀚归柨婵嗙凹缁ㄤ粙鏌涢幋婵堢Ш闁哄矉缍侀獮鎺楀箣椤栨繂鎯堥柣搴㈩問閸犳牠鎮ラ悡搴ｆ殾闁绘挸绨跺Σ鍫熶繆椤栨瑨顒熼柣搴☆嚟缁辨挻鎷呯拠鈩冪暦闂佹悶鍔屽鈥愁嚕婵犳碍鏅插璺侯儏娴滄粓姊洪崨濠勨槈闁挎洩绠撻弫宥呪槈濡攱鏂�闂佹寧绋戠�氼剚绂掗敃鍌涘仺妞ゆ牗绮屾禒褔鏌涢幒鎾崇瑲缂佺粯绻傞～婵嬵敇閻戝洦瀚归柣鎴ｅГ閸婂灚绻涢幋鐑嗕痪妞ゅ繐鐗滈弫渚�鏌熼崜褏甯涢柣鎾跺枛閺岋絽螣閸濆嫬绗梺鐓庡娴滎亪寮诲☉銏犳閻犳亽鍓辫閿熻姤顔栭崰鏍偪閸モ晝涓嶆繛鎴炃氬Σ鍫熺箾閸℃婀伴悗姘矙濮婄粯鎷呴懞銉с�婇梺闈╃秶缁犳捇鐛箛娑欐櫢闁跨噦鎷�?闂傚倸鍊搁崐鎼佸磹閻戣姤鍊块柨鏇炲�归崕鎴犳喐閻楀牆绗掔紒锟芥径宀嬫嫹閻у憡瀚归梺鍛婃处閸嬪嫰骞嗛悙鐑樷拺缁绢厼鎳忕涵鑸典繆椤愩値娼╰闂傚倸鍊搁崐鎼佸磹妞嬪海鐭嗗〒姘炬嫹妤犵偞鐗犻、鏇氱秴闁搞儺鍓﹂弫鍥煏韫囨洖啸闁挎稓鍠栧缁樼瑹閸パ冾潻缂備礁顦顓犲垝婵犳凹鏁嶆慨锝庡幖缂嶅﹤鐣峰Δ鍛闁兼祴鏅濋埢蹇涙⒒娴ｅ憡鎯堟い鎴濆濞嗐垹顫濈捄楦挎憰闂佹寧绻傞幉姗�寮崼鐔告婵炲濮撮鍡涘磻鐎ｎ喗鐓曢柍鈺佸暟閹冲懘鏌ｉ鐐靛ⅵ闁哄被鍔戝鎾敂閸涱収浼撻梻鍌欑閻ゅ洤螞閸曨垁鍥偨缁嬫寧鐎梺鐟板⒔缁垶寮查幖浣圭叆闁绘洖鍊圭�氾拷?闂傚倸鍊搁崐宄懊归崶褏鏆﹂柛顭戝亝閸欏繒锟藉箍鍎遍ˇ顖炵嵁閵忊�茬箚妞ゆ牗姘ㄦ禒銏ゆ煙椤斿吋鍣界紒杈ㄦ崌瀹曟帒鈻庨幇顔哄仒缂傚倷绶￠崳锝囩不閺嵮呮殾婵炲樊浜滈崘锟介悷婊勭矒閹偞绂掔�ｎ偆鍙嗛梺缁樻礀閸婂湱锟芥熬鎷�?
                        {
                            if( fCtrl.Bits.FOptsLen > 0 )
                            {
                                // Decode Options field MAC commands. Omit the fPort.
                                ProcessMacCommands( payload, 8, appPayloadStartIndex - 1, snr );
                            }

                            LoRaMacPayloadDecrypt( payload + appPayloadStartIndex,
                                                   frameLen,
                                                   appSKey,
                                                   address,
                                                   DOWN_LINK,
                                                   downLinkCounter,
                                                   LoRaMacRxPayload );

                            if( skipIndication == false )
                            {
                                McpsIndication.Buffer = LoRaMacRxPayload;
                                McpsIndication.BufferSize = frameLen;
                                McpsIndication.RxData = true;
                            }
                        }
                    }
                    else
                    {
                        if( fCtrl.Bits.FOptsLen > 0 )
                        {
                            // Decode Options field MAC commands MAC闂傚倸鍊搁崐鐑芥倿閿曞倹鍎戠憸鐗堝笒閺勩儵鏌涢弴銊ョ仩闁搞劌鍊块獮鏍庨锟芥俊鍏碱殽閻愵亜鐏紒缁樼洴瀹曞崬螣閾忕懓濮奸梻浣告憸閸犲酣顢栨径濠庢綎缂備焦蓱婵挳鏌涘☉姗堟敾闁稿孩鎸搁—鍐Χ韫囨挾妲ｉ梺鍛婄懃閸燁偊鎮鹃悜钘夊嵆闁靛繆锟藉厖缃曢梻浣稿閸嬩線宕洪崟顐�娲箹娴ｅ厜鎷洪梺闈╁瘜閸樺ジ宕濈�ｎ偁浜滈柕濞垮劜閸ｄ粙鏌熼崣澶嬪唉闁轰礁鍟村畷鎺戭潩妲屾牕鏅ｉ梻鍌欑濠�閬嶅煕閸儱鍌ㄥ┑鍌氭啞閸婂爼鏌ㄥ┑鍡樺窛缁炬儳銈搁弻銊╂偄鐠囨畫鎾讹拷鍨緲鐎氫即鐛崶顒夋晣婵炴垶鐟ラ鐟扳攽閿涘嫬浜奸柛濠冪墵瀹曟繆顦寸�垫澘锕畷妤呮嚃閳哄﹥閿ら梻渚�娼х换鍫ュ垂閼姐倕顥氬┑鍌氭啞閸嬶綁鏌涢妷锝呭缂佹彃顭烽弻锟犲幢濞嗗繑鐎婚梻鍥ь樀閺岋絽鈻庤箛鏂挎闂佷紮绲块弫鎼佹晸閸婄噥娼愭繛鍙夘焽閺侇噣骞掑Δ瀣◤濠电娀娼уú锔剧礊閸ヮ剚鐓忓┑鐐茬仢閸旀岸鏌ｉ悢閿嬫崳缂佽鲸鎹囧畷鎺戭潩椤掑﹥瀚归柣鎴ｅГ閸婂潡鏌ㄩ弬鍨挃闁活厽鐟╅弻鐔兼倻濮楀棙鐣烽梺绋块闁帮綁寮婚妸鈺傚亞闁稿本绋戦锟�?婵犵數濮烽弫鍛婃叏娴兼潙鍨傚┑鍌滎焾閺勩儵鏌″搴′簽闁活厽鐟╅弻鐔煎箚瑜嶉。鎶芥煟椤撶偞顥滄い顓炴健閹虫粓妫冨☉姗嗘綌闂備胶绮幐濠氭儎椤栫偛钃熼柕濞炬櫆閸嬪棝鏌涚仦鍓р槈妞ゅ骏鎷�?缂傚倸鍊搁崐鎼佸磹閹间礁纾归柟闂寸绾剧懓顪冪�ｎ亝鎹ｉ柣顓炴閵嗘帒顫濋悙顒�顏堕梻浣筋嚃閸燁偊宕惰閼板灝鈹戦悙鏉戠仸閽冭鲸淇婂顔煎⒋闁哄矉缍�缁犳盯寮撮悙鐗堝煕闂備胶顭堥鍡涘箰閸撗勵潟闁绘劕鎼悙濠冦亜閹哄秷鍏岄柛妯兼暬閺岋綁鎮㈤崫銉﹀櫑闁诲孩鍑归崜姘跺箞閵婏箑绶炵�癸拷閿熺晫澹曟總鍛婄厽婵☆垵娅ｆ禒娑㈡煕閵堝繑瀚归梻鍌欑閻ゅ洤螞閸曨垁鍥偨閸濄儱绁﹀┑鐐叉閹稿摜绮堥崘顏呭枑闊洦绋掗崕搴ㄥ箹濞ｎ剙濡介柍閿嬪灴閺岀喖鎳栭埡浣风捕闂佽法鍠曟慨銈囨崲閸愵亞鐭夌�广儱娲﹀畷澶娿�掑鐓庣仩婵炲牄鍔岄—鍐Χ閸℃衼闂傚倸瀚�氭澘鐣烽幋鐙呮嫹閿濆骸鏋熼柣鎾存礃閵囧嫰骞囬埡渚婃嫹閺囥垹鍑犳繛鎴炵懁缁诲棗螖閿濆懎鏆為柡浣戒含缁辨帞绱掑Ο鑲╃杽婵犳鍣幏鐑芥⒑閹稿海绠撴俊顐ｇ洴婵℃挳骞囬悧鍫㈠幗闁瑰吋鐣崹褰掑吹椤掑嫭鐓曢柨婵嗙箳缁夘噣鏌ｅ☉鍗炴珝鐎规洘锕㈤崺锟犲礃椤旂瓔锟藉秵绻濆▓鍨灍妞ゃ劌妫濋獮锟介柨鐔剁矙濮婂宕掑顒婃嫹妞嬪海鐭嗗〒姘炬嫹妤犵偛顦甸弫宥夊礋椤撶姷鍘梻浣告贡閸庛倝銆冮崨鏉戝瀭闁稿本绋撶弧锟介梻鍌氱墛娓氭宕曡箛鏇犵＜闁绘瑱鎷烽柛锝忕秮瀵濡搁埡鍌氫簽闂佺鏈粙鎴︻敂閿燂拷
                            ProcessMacCommands( payload, 8, appPayloadStartIndex, snr );
                        }
                    }

                    if( skipIndication == false )
                    {
                        // Check if the frame is an acknowledgement
                        if( fCtrl.Bits.Ack == 1 )
                        {
                            McpsConfirm.AckReceived = true;
                            McpsIndication.AckReceived = true;

                            // Stop the AckTimeout timer as no more retransmissions
                            // are needed.
                            TimerStop( &AckTimeoutTimer );
                        }
                        else
                        {
                            McpsConfirm.AckReceived = false;

                            if( AckTimeoutRetriesCounter > AckTimeoutRetries )
                            {
                                // Stop the AckTimeout timer as no more retransmissions
                                // are needed.
                                TimerStop( &AckTimeoutTimer );
                            }
                        }
                    }
                    // Provide always an indication, skip the callback to the user application,
                    // in case of a confirmed downlink retransmission.
                    LoRaMacFlags.Bits.McpsInd = 1;
                    LoRaMacFlags.Bits.McpsIndSkip = skipIndication;
                }
                else
                {
                    McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_MIC_FAIL;

                    PrepareRxDoneAbort( );
                    return;
                }
            }
            break;
        case FRAME_TYPE_PROPRIETARY:
            {
                memcpy1( LoRaMacRxPayload, &payload[pktHeaderLen], size );

                McpsIndication.McpsIndication = MCPS_PROPRIETARY;
                McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                McpsIndication.Buffer = LoRaMacRxPayload;
                McpsIndication.BufferSize = size - pktHeaderLen;

                LoRaMacFlags.Bits.McpsInd = 1;
                break;
            }
        default:
            McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            PrepareRxDoneAbort( );
            break;
    }
    LoRaMacFlags.Bits.MacDone = 1;

    // Trig OnMacCheckTimerEvent call as soon as possible
//    TimerSetValue( &MacStateCheckTimer, 1 );
//    TimerStart( &MacStateCheckTimer );
    OnMacStateCheckTimerEvent();
}

static void OnRadioTxTimeout( void )
{
    if( LoRaMacDeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    else
    {
        OnRxWindow2TimerEvent( );
    }

    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT;
    MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT;
    LoRaMacFlags.Bits.MacDone = 1;
}

static void OnRadioRxError( void )
{
    if( LoRaMacDeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    else
    {
        OnRxWindow2TimerEvent( );
    }

    if( RxSlot == 0 )
    {
        if( NodeAckRequested == true )
        {
            McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX1_ERROR;
        }
        MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX1_ERROR;

        if( TimerGetElapsedTime( AggregatedLastTxDoneTime ) >= RxWindow2Delay )
        {
            LoRaMacFlags.Bits.MacDone = 1;
        }
    }
    else
    {
        if( NodeAckRequested == true )
        {
            McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_ERROR;
        }
        MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_ERROR;
        LoRaMacFlags.Bits.MacDone = 1;
    }
}

static void OnRadioRxTimeout( void )
{
	//lora_printf("OnRadioRxTimeout\n");

    if( LoRaMacDeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    else
    {
        OnRxWindow2TimerEvent( );
    }

    if( RxSlot == 1 )
    {
        if( NodeAckRequested == true )
        {
            McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT;
        }
        MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT;
        LoRaMacFlags.Bits.MacDone = 1;
    }
}



static void OnMacStateCheckTimerEvent( void )
{
//lora_printf("A=%d\r\n",A);
//A++;
#ifdef debug
	lora_printf("OnMacStateCheckTimerEvent\n");
	//lora_printf("%d\n",millis());
	uint64_t tim=0;
	uint32_t tim1=0;
	uint32_t tim2=0;
	tim=timerRead(timer);
	tim1=tim&0XFFFFFFFF;
	tim=tim>>32;
	tim2=tim&0XFFFFFFFF;
	lora_printf("%d  %ld  %ld\n",millis(),tim2,tim1);
#endif
    TimerStop( &MacStateCheckTimer );

    bool txTimeout = false;

    if( LoRaMacFlags.Bits.MacDone == 1 )
    {
    	//lora_printf("\r\nLoRaMacFlags.Bits.MacDone == 1 \n");
        if( ( LoRaMacState & LORAMAC_RX_ABORT ) == LORAMAC_RX_ABORT )//LORAMAC_RX_ABORT 闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮婚敐澶婄闁挎繂鎲涢幘缁樼厸闁告侗鍠楅崐鎰版煙椤旀寧纭鹃柍钘夘槸閳诲骸螣閼姐倖娈鹃梻鍌欑閹碱偊顢栭崨绮癸拷锕傛倻閻ｅ苯绁﹂棅顐㈡处缁嬫帡宕愭繝姘厾闁诡厽甯掗崝婊勭箾閸涱偄鐏紒杈ㄦ尰閹峰懘宕滈崣澹囨⒑閻戔晛澧叉繛鑼枛閻涱噣骞樺ú缁樻櫆闂佸壊鍋呯换鍕敊閸℃稒鈷戠紓浣姑悘鍗炩攽椤斿搫锟芥繂顕ｉ弻銉ョ濞达絽婀遍崢鎼佹⒑缁嬫寧婀伴柨姘舵煥閻旇袚闁绘绮撻弫鎾绘嚍閵壯冩殹闂佺懓鍤栭幏锟� 闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮婚敐澶婄闁挎繂鎲涢幘缁樼厸闁告侗鍠楅崐鎰版煛瀹�瀣瘈鐎规洘甯掕灒闁绘艾鐡ㄧ�氬綊鏌″搴′壕闁瑰嘲鍢查～婵嬵敆閸屾簽銊╂⒑閸濆嫯顫﹂柛鏃�鍨块獮鍐閵堝懎绐涙繝鐢靛Т鐎氼亞妲愰弴銏♀拻濞达絽鎲＄拹锟犳煕韫囨棑鑰块挊鐔兼煙閹呮憼闁告瑥绻橀弻鐔兼⒒鐎靛壊妲紓浣哄У閸ㄥ潡寮婚妸鈺傚亞闁稿本绋戦锟�
        {
            LoRaMacState &= ~LORAMAC_RX_ABORT;
            LoRaMacState &= ~LORAMAC_TX_RUNNING;   //LORAMAC_TX_RUNNING  闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮婚敐澶婄闁挎繂鎲涢幘缁樼厸闁告侗鍠楅崐鎰版煛瀹�瀣瘈鐎规洘甯掕灒闁绘艾鐡ㄧ�氬綊鏌″搴′壕闁瑰嘲鍢查～婵嬵敆閸屾簽銊╂⒑閸濆嫯顫﹂柛鏃�鍨块獮鍐閵堝懎绐涙繝鐢靛Т鐎氼亞妲愰弴銏♀拻濞达絽鎽滅粔鐑樸亜閵夛附宕岀�规洘顨呴～婊堟晸閽樺鍤曟い鎰剁畱缁犲鎮归崶顏勭毢妞ゆ柨顦靛娲倷椤掑啫顥濋梺绋匡功缁垳绮欐径鎰摕闁靛濡囬崢鍨繆閻愬樊鍎忓Δ鐘虫倐瀹曘垽骞橀鐣屽幐婵炶揪缍�椤鐣峰畝锟介敓鑺ヮ問閸ｎ噣宕滈悢闀愮箚闁割偅娲栭獮銏′繆閵堝拑姊楃紒鎲嬬畵濮婂宕掑▎鎰偘濠电偛顦板ú鐔风暦娴兼潙绠涢柡澶婄仢閸ゆ垶绻濋悽闈浶ｉ柤鐟板⒔缁顢涢悙瀵稿幗闂佺鎻徊鍊燁暱闂備礁鎼幊蹇涘箖閸岀偛钃熼柨鐔哄Т绾惧吋淇婇婊冪殤闁归鍏樺浠嬵敃椤掍礁顏跺┑顔斤供閸撴稓绮斿ú顏呯厸閻忕偠顕ф慨鍌炴煙椤斿搫鐏茬�规洏鍔嶇换婵嬪礋椤忓棛锛熼梻鍌氬�风粈浣虹礊婵犲泚澶愬箻鐠囪尙顦у┑顔筋殣閹烽锟芥鍣崑濠囩嵁濡吋瀚氶柛娆忣樈濡喖姊绘笟锟藉褔鎮ч崱妞㈡稑螖閸滀焦鏅滈梺鍓插亝濞叉﹢鍩涢幋锔藉仯闁诡厽甯掓俊濂告煛鐎ｎ偄鐏撮柡宀嬬磿閿熸枻缍嗘禍鍫曟偂閸忕⒈娈介柣鎰皺缁犲鏌熼鐣岀煉闁瑰磭鍋ゆ俊鐑芥晜缁涘鎴烽梻鍌氬�风粈渚�骞栭锕�纾归柛顐ｆ礀绾惧綊鏌″搴′壕闁瑰嘲鍢查～婵嬫倷椤掞拷椤忥拷 闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮婚敐澶婄闁挎繂鎲涢幘缁樼厸闁告侗鍠楅崐鎰版煛瀹�瀣瘈鐎规洘甯掕灒闁绘艾鐡ㄧ�氬綊鏌″搴′壕闁瑰嘲鍢查～婵嬵敆閸屾簽銊╂⒑鐠団�虫灀闁哄懐濞�楠炲﹤顭ㄩ崨顓熺�虫繝銏ｆ硾椤戝懘顢旈崼鏇熲拺閻犳亽鍔屽▍鎰版煙閸戙倖瀚�
        }

        if( ( LoRaMacFlags.Bits.MlmeReq == 1 ) || ( ( LoRaMacFlags.Bits.McpsReq == 1 ) ) )//LoRaMacFlags.Bits.MlmeReq == 1  LoraMacMlmeRequest LORAMAC_STATUS_OK闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮婚敐澶婄闁挎繂鎲涢幘缁樼厸闁告侗鍠楅崐鎰版煛瀹�瀣瘈鐎规洘甯掕灒闁绘艾鐡ㄧ�氬綊鏌″搴′壕闁瑰嘲鍢查～婵嬵敆閸屾簽銊╂⒑閸濆嫯顫﹂柛鏃�鍨块獮鍐閵堝棙鍎梺鑽ゅ枑濠㈡寰婃總鍛娾拻濞达絿鎳撻婊呯磼鐠囨彃锟藉骞戦姀銈呯闁绘瑥鎳忕�氳鎱ㄥΟ鐓庝壕閻庢熬鎷�//LoRaMacFlags.Bits.McpsReq == 1 LoRaMacMcpsRequest LORAMAC_STATUS_OK闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮婚敐澶婄闁挎繂鎲涢幘缁樼厸闁告侗鍠楅崐鎰版煛瀹�瀣瘈鐎规洘甯掕灒闁绘艾鐡ㄧ�氬綊鏌″搴′壕闁瑰嘲鍢查～婵嬵敆閸屾簽銊╂⒑閸濆嫯顫﹂柛鏃�鍨块獮鍐閵堝懎绐涙繝鐢靛Т鐎氼亞妲愰弴銏♀拻濞达絿鍎ら崵锟介梺鎼炲�栭悧鐘荤嵁韫囨稒鏅搁柨鐕傛嫹
        {
            if( ( McpsConfirm.Status == LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT ) ||
                ( MlmeConfirm.Status == LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT ) )//闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮婚敐澶婄闁挎繂鎲涢幘缁樼厸闁告侗鍠楅崐鎰版煛瀹�瀣瘈鐎规洘甯掕灒闁绘艾鐡ㄧ�氬綊鏌″搴′壕闁瑰嘲鍢查～婵嬵敆閸屾簽銊╂⒑鐠団�虫灀闁哄懐濞�楠炲﹤顭ㄩ崨顓熺�虫繝銏ｆ硾椤戝懘顢旈崼鏇熲拺閻犳亽鍔屽▍鎰版煙閸戙倖瀚� OnRadioTxTimeout 闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮婚敐澶婄闁挎繂鎲涢幘缁樼厸闁告侗鍠楅崐鎰版煛瀹�瀣瘈鐎规洘甯掕灒闁绘艾鐡ㄧ�氬綊鏌″搴′壕闁瑰嘲鍢查～婵嬫倷椤掞拷椤忥拷
            {
                // Stop transmit cycle due to tx timeout.
                LoRaMacState &= ~LORAMAC_TX_RUNNING;
                MacCommandsBufferIndex = 0;
                McpsConfirm.NbRetries = AckTimeoutRetriesCounter;
                McpsConfirm.AckReceived = false;
                McpsConfirm.TxTimeOnAir = 0;
                txTimeout = true;
            }
        }

        if( ( NodeAckRequested == false ) && ( txTimeout == false ) )//NodeAckRequested 闂傚倸鍊风粈渚�骞夐敓鐘冲仭妞ゆ牗绋撻々鍙夈亜韫囨挾澧曢柡瀣╃窔閺岀喖骞戦幇闈涙濠碘槅鍋呴敃銏ゅ蓟閺囥垹閱囨繝闈涙祩濡偛顪冮妶鍛寸崪闁瑰嚖鎷� PrepareFrame case FRAME_TYPE_DATA_CONFIRMED_UP 闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮婚敐澶婄闁挎繂鎲涢幘缁樼厸闁告侗鍠楅崐鎰版煛瀹�瀣瘈鐎规洘甯掕灒闁绘艾鐡ㄧ�氬綊鏌″搴′壕闁瑰嘲鍢查～婵嬪箛娴ｅ湱绉风紓鍌欑椤戝懘藝闁秵鍤嶉柛妤冨仧閺嗗鐥鐐仱
        {
            if( ( LoRaMacFlags.Bits.MlmeReq == 1 ) || ( ( LoRaMacFlags.Bits.McpsReq == 1 ) ) )
            {
                if( ( LoRaMacFlags.Bits.MlmeReq == 1 ) && ( MlmeConfirm.MlmeRequest == MLME_JOIN ) ) //MlmeConfirm.MlmeRequest == MLME_JOIN    LoRaMacMlmeRequest case MLME_JOIN 闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮婚敐澶婄闁挎繂鎲涢幘缁樼厸闁告侗鍠楅崐鎰版煛瀹�瀣瘈鐎规洘甯掕灒闁绘艾鐡ㄧ�氬綊鏌″搴′壕闁瑰嘲鍢查～婵嬫倷椤掞拷椤忥拷
                {// Procedure for the join request
                    MlmeConfirm.NbRetries = JoinRequestTrials;
                    if( MlmeConfirm.Status == LORAMAC_EVENT_INFO_STATUS_OK )//MlmeConfirm.Status == LORAMAC_EVENT_INFO_STATUS_OK  OnRadioRxDone  case FRAME_TYPE_JOIN_ACCEPT闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮婚敐澶婄闁挎繂鎲涢幘缁樼厸闁告侗鍠楅崐鎰版煛瀹�瀣瘈鐎规洘甯掕灒闁绘艾鐡ㄧ�氬綊鏌″搴′壕闁瑰嘲鍢查～婵嬫倷椤掞拷椤忥拷 闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮婚敐澶婄闁挎繂鎲涢幘缁樼厸闁告侗鍠楅崐鎰版煛瀹�瀣瘈鐎规洘甯掕灒闁绘艾鐡ㄧ�氬綊鏌″搴′壕闁瑰嘲鍢查～婵嬵敆閸屾簽銊╂⒑閸濆嫯顫﹂柛鏃�鍨块獮鍐閵堝懎绐涙繝鐢靛Т鐎氼亞妲愰弴銏♀拻濞达絽鎽滅粔鐑樸亜閵夛附宕岀�规洘顨呴～婊堟晸閽樺鍤曟い鎰剁畱缁犲鏌涢幘鑼跺厡闁绘挸顑夊娲川婵犲嫮绱伴梺绋垮濡炶棄顕ｉ崘宸叆闁割偆鍠撻崢鍨繆閻愬樊鍎忓Δ鐘虫倐瀹曘垽骞橀鐣屽幐婵炶揪缍�椤鐣峰畝锟介敓鑺ヮ問閸ｎ噣宕滈悢闀愮箚闁割偅娲栭獮銏′繆閵堝拑姊楃紒鎲嬬畵濮婂宕掑▎鎰偘濠电偛顦板ú鐔风暦娴兼潙绠涢柡澶婄仢閸ゆ垶绻濋悽闈浶ｉ柤鐟板⒔缁顢涢悙瀵稿幗闂佺鎻徊鍊燁暱闂備礁鎼幊蹇涘箖閸岀偛钃熼柨鐔哄Т绾惧吋淇婇婊冪殤闁归鍏樺浠嬵敃椤掍礁顏跺┑顔斤供閸撴稓绮斿ú顏呯厸閻忕偠顕ф慨鍌炴煙椤斿搫鐏茬�规洏鍔嶇换婵嬪礋椤忓棛锛熼梻鍌氬�风粈浣革耿闁秴绀傛慨妤嬫嫹鐎殿喓鍔戝畷锟犳倻閸℃瑱鎷风捄琛℃斀闁稿本纰嶉崯鐐烘煟閹惧崬鍔﹂柡宀嬬秮瀵剟骞愭惔鈩冩畼闂備浇顕уù姘跺闯閿濆钃熼柣鏂垮濡插綊骞栫划鐧告嫹閸愯尙顓洪梻鍌欐祰濡椼劑鎮為敃鍌氱闁搞儺鍓欓弸渚�鏌熼崜褏甯涢柡鍛倐閺屻劑鎮ら崒娑橆伓
                    {// Node joined successfully
                        UpLinkCounter = 0;
                        ChannelsNbRepCounter = 0;
                        LoRaMacState &= ~LORAMAC_TX_RUNNING;
                    }
                    else//闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮婚敐澶婄闁挎繂鎲涢幘缁樼厸闁告侗鍠楅崐鎰版煛瀹�瀣瘈鐎规洘甯掕灒闁绘艾鐡ㄧ�氬綊鏌″搴′壕闁瑰嘲鍢查～婵嬪箛娴ｅ湱绉烽柣搴＄仛濠㈡锟芥矮鍗冲濠氬幢濡ゅ﹤鎮戦梺绯曞墲钃辨い蹇曞枛濮婄粯鎷呴崨濠冨創闂佺锕ら悥鐓庣暦閹版澘鍐�妞ゆ劦鍓氱�氳鎱ㄥΟ鐓庝壕閻庢熬鎷�
                    {
                        if( JoinRequestTrials >= MaxJoinRequestTrials )//470濠电姷顣藉Σ鍛村磻閸℃ɑ娅犳俊銈呮噹閸ㄥ倿鏌涜椤ㄥ懘寮告笟锟介弻鐔煎箲閹伴潧娈┑鈽嗗亝閿曘垽寮婚弴銏犻唶婵犻潧娴傚Λ鐐差渻閵堝懐绠烘繛澶嬬洴閸┿儲寰勯幇顒傤啌闂佸憡娲﹂崜娆擃敂閿燂拷48
                        {
                            LoRaMacState &= ~LORAMAC_TX_RUNNING;
                        }
                        else
                        {
                            LoRaMacFlags.Bits.MacDone = 0;
                            // Sends the same frame again
                            OnTxDelayedTimerEvent( ); //闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮婚敐澶婄闁挎繂鎲涢幘缁樼厸闁告侗鍠楅崐鎰叏婵犲啯銇濋柟顔界懇椤㈡鎷呴崫鍕闂傚倷鐒﹂幃鍫曞磹閺嶎厽鍋嬮柣妯垮吹瀹撲線鐓崶銊р檨闁稿﹤顭烽弻銈夊箒閹烘垵濮曞┑鐐叉噷閸ㄨ棄顫忓ú顏勪紶闁告洦鍋撻幏鐑藉醇閺囩偟锛欓梺鍝勭▉閸嬫捇骞忓畡閭︽僵妞ゆ帒鍊绘闂備礁鎼悮顐﹀礉閹达箑绠栭柕蹇嬪�曞婵嗏攽閻樻彃顏х紒杈ㄦ礋濮婄粯鎷呴崫銉ㄥ銈冨劜閹瑰洤鐣峰Δ锟介～婊堟晸閽樺鍤曟い鎰剁畱缁犲鎮归崶褍绾фい銉︾箞濮婃椽妫冨☉姘暫濠碘槅鍋呴〃濠囥�侀弮鍫濋唶闁哄洨鍟块幏娲煟閻樺厖鑸柛鏂跨焸瀵悂骞嬮敂鐣屽幐闁诲函缍嗘禍鍫曟偂閸忕⒈娈介柣鎰皺缁犲鏌熼鐣岀煉闁瑰磭鍋ゆ俊鐑芥晜缁涘鎴烽梻鍌氬�风粈渚�骞栭锕�纾归柛顐ｆ礀绾惧綊鏌″搴′壕闁瑰嘲鍢查～婵嬫倷椤掞拷椤忥拷
                        }
                    }
                }
                else
                {// Procedure for all other frames
                    if( ( ChannelsNbRepCounter >= LoRaMacParams.ChannelsNbRep ) || ( LoRaMacFlags.Bits.McpsInd == 1 ) )// OnRadioTxDone NodeAckRequested==false ChannelsNbRepCounter++;
                    {
                        if( LoRaMacFlags.Bits.McpsInd == 0 )//LoRaMacFlags.Bits.McpsInd =1  OnRadioRxDone 闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮婚敐澶婄闁挎繂鎲涢幘缁樼厸闁告侗鍠楅崐鎰版煛瀹�瀣瘈鐎规洘甯掕灒闁绘艾鐡ㄧ�氬綊鏌″搴′壕闁瑰嘲鍢查～婵囶潙閺嵮冃乺epareRxdoneabort
                        {   // Maximum repititions without downlink. Reset MacCommandsBufferIndex. Increase ADR Ack counter.
                            // Only process the case when the MAC did not receive a downlink.
                            MacCommandsBufferIndex = 0;
                            AdrAckCounter++;
                        }

                        ChannelsNbRepCounter = 0;// uplink messages repetitions counter

                        if( IsUpLinkCounterFixed == false )
                        {
                            UpLinkCounter++;
                        }

                        LoRaMacState &= ~LORAMAC_TX_RUNNING;
                    }
                    else//闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮婚敐澶婄闁挎繂鎲涢幘缁樼厸闁告侗鍠楅崐鎰叏婵犲啯銇濋柟顔界懇椤㈡鎷呴崫鍕闂傚倷鐒﹂幃鍫曞磹閺嶎厽鍋嬮柣妯垮吹瀹撲線鐓崶銊р檨闁稿﹤顭烽弻銈夊箒閹烘垵濮曞┑鐐叉噷閸ㄨ棄顫忓ú顏勪紶闁告洦鍋撻幏鐑藉醇閺囩偟锛欓梺鍝勭▉閸嬫捇骞忓畡閭︽僵闁绘劦鍓欓锟�
                    {
                        LoRaMacFlags.Bits.MacDone = 0;
                        // Sends the same frame again
                        OnTxDelayedTimerEvent( );
                    }
                }
            }
        }

        if( LoRaMacFlags.Bits.McpsInd == 1 )
        {// Procedure if we received a frame
            if( ( McpsConfirm.AckReceived == true ) || ( AckTimeoutRetriesCounter > AckTimeoutRetries ) )
            {
                AckTimeoutRetry = false;
                NodeAckRequested = false;
                if( IsUpLinkCounterFixed == false )
                {
                    UpLinkCounter++;
                }
                McpsConfirm.NbRetries = AckTimeoutRetriesCounter;

                LoRaMacState &= ~LORAMAC_TX_RUNNING;
            }
        }

        if( ( AckTimeoutRetry == true ) && ( ( LoRaMacState & LORAMAC_TX_DELAYED ) == 0 ) )
        {// Retransmissions procedure for confirmed uplinks
            AckTimeoutRetry = false;
            if( ( AckTimeoutRetriesCounter < AckTimeoutRetries ) && ( AckTimeoutRetriesCounter <= MAX_ACK_RETRIES ) )
            {
                AckTimeoutRetriesCounter++;

                if( ( AckTimeoutRetriesCounter % 2 ) == 1 )
                {
                    LoRaMacParams.ChannelsDatarate = MAX( LoRaMacParams.ChannelsDatarate - 1, LORAMAC_TX_MIN_DATARATE );
                }
                // Try to send the frame again
                if( ScheduleTx( ) == LORAMAC_STATUS_OK )
                {
                    LoRaMacFlags.Bits.MacDone = 0;
                }
                else
                {
                    // The DR is not applicable for the payload size
                    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_TX_DR_PAYLOAD_SIZE_ERROR;

                    MacCommandsBufferIndex = 0;
                    LoRaMacState &= ~LORAMAC_TX_RUNNING;
                    NodeAckRequested = false;
                    McpsConfirm.AckReceived = false;
                    McpsConfirm.NbRetries = AckTimeoutRetriesCounter;
                    McpsConfirm.Datarate = LoRaMacParams.ChannelsDatarate;
                    if( IsUpLinkCounterFixed == false )
                    {
                        UpLinkCounter++;
                    }
                }
            }
            else
            {
#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
                // Re-enable default channels LC1, LC2, LC3
                LoRaMacParams.ChannelsMask[0] = LoRaMacParams.ChannelsMask[0] | ( LC( 1 ) + LC( 2 ) + LC( 3 ) );
#elif defined( USE_BAND_470 ) || defined( USE_BAND_470PREQUEL )
                // Re-enable default channels
                memcpy1( ( uint8_t* )LoRaMacParams.ChannelsMask, ( uint8_t* )LoRaMacParamsDefaults.ChannelsMask, sizeof( LoRaMacParams.ChannelsMask ) );
#elif defined( USE_BAND_915 )
                // Re-enable default channels
                memcpy1( ( uint8_t* )LoRaMacParams.ChannelsMask, ( uint8_t* )LoRaMacParamsDefaults.ChannelsMask, sizeof( LoRaMacParams.ChannelsMask ) );
#elif defined( USE_BAND_915_HYBRID )
                // Re-enable default channels
                ReenableChannels( LoRaMacParamsDefaults.ChannelsMask[4], LoRaMacParams.ChannelsMask );
#else
    #error "Please define a frequency band in the compiler options."
#endif
                LoRaMacState &= ~LORAMAC_TX_RUNNING;

                MacCommandsBufferIndex = 0;
                NodeAckRequested = false;
                McpsConfirm.AckReceived = false;
                McpsConfirm.NbRetries = AckTimeoutRetriesCounter;
                if( IsUpLinkCounterFixed == false )
                {
                    UpLinkCounter++;
                }
            }
        }
    }
    // Handle reception for Class B and Class C         ???
    if( ( LoRaMacState & LORAMAC_RX ) == LORAMAC_RX )//Class A 闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ磭妲愰幒鏃撴嫹閿濆骸澧柛濠勬搐AMAC_RX闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮婚妸鈺傚亞闁稿本绋戦锟�
    {
        LoRaMacState &= ~LORAMAC_RX;
    }
    if( LoRaMacState == LORAMAC_IDLE )
    {
        if( LoRaMacFlags.Bits.McpsReq == 1 )
        {
            LoRaMacPrimitives->MacMcpsConfirm( &McpsConfirm );
            LoRaMacFlags.Bits.McpsReq = 0;
        }

        if( LoRaMacFlags.Bits.MlmeReq == 1 )
        {
            LoRaMacPrimitives->MacMlmeConfirm( &MlmeConfirm );
            LoRaMacFlags.Bits.MlmeReq = 0;
        }

        // Procedure done. Reset variables.
        LoRaMacFlags.Bits.MacDone = 0;
    }
    else
    {
        // Operation not finished restart timer
        TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );
        TimerStart( &MacStateCheckTimer );
    }

    if( LoRaMacFlags.Bits.McpsInd == 1 )
    {
        if( LoRaMacDeviceClass == CLASS_C )
        {// Activate RX2 window for Class C
            OnRxWindow2TimerEvent( );
        }
        if( LoRaMacFlags.Bits.McpsIndSkip == 0 )//LoRaMacFlags.Bits.McpsIndSkip == 1  闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮婚妸鈺傚亞闁稿本绋戦锟� OnRadioRxDone 闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼幑鎰靛殭缁绢叏鎷烽梻浣筋潐閸庡吋鎱ㄩ妶澶婄？鐎癸拷閸曨剛鍘卞┑鐐村灦閿曨偄顔忛妷锔轰簻闊洦鎸鹃崺锝嗘叏婵犲懏顏犻柟鐟板婵℃悂濡烽敂鎾呮嫹妤ｅ啯鈷戦柛婵嗗閸庡繑銇勯鐘插幋闁绘侗鍠涚粻娑㈠即閻樼數鍘版繝娈垮枟缁诲嫬鈻撳锟�==0闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ磭妲愰幒鏃撴嫹閿濆懐浠涚痪鍓ф７tslen>0闂傚倸鍊风粈渚�骞栭锕�鐤い鎰堕檮閸嬪鈹戦崒婊庣劸闁哄绶氶弻鐔煎箲閹伴潧娈┑鈽嗗亝閿曘垽寮婚弴銏犻唶婵犻潧娴傚Λ鐐差渻閵堝懐绠伴柣鏍帶椤繘鎼圭憴鍕／闂侀潧顭堥崕鏌ユ倵妤ｅ啯鈷戦柛婵嗗閸庡繑銇勯鐘插幋闁绘侗鍠涚粻娑樷槈濞嗘劖顏熼梻浣芥硶閸ｏ箓骞忛敓锟�
            LoRaMacPrimitives->MacMcpsIndication( &McpsIndication );
    }
        LoRaMacFlags.Bits.McpsIndSkip = 0;
        LoRaMacFlags.Bits.McpsInd = 0;
}




static void OnTxDelayedTimerEvent( void )
{
    LoRaMacHeader_t macHdr;
    LoRaMacFrameCtrl_t fCtrl;

    TimerStop( &TxDelayedTimer );
    LoRaMacState &= ~LORAMAC_TX_DELAYED;

    if( ( LoRaMacFlags.Bits.MlmeReq == 1 ) && ( MlmeConfirm.MlmeRequest == MLME_JOIN ) )
    {
        ResetMacParameters( );
        // Add a +1, since we start to count from 0
        LoRaMacParams.ChannelsDatarate = AlternateDatarate( JoinRequestTrials + 1 );//闂傚倸鍊搁崐鎼佸磹閻戣姤鍊块柨鏇氶檷娴滃綊鏌涢幇鍏哥敖闁活厽鎸鹃敓鍊燁潐濞叉牕煤閿曞偊缍栭柡鍥ュ灪閻撱儵鏌ｉ弴鐐诧拷鍦拷姘炬嫹8濠电姷鏁告慨鐑藉极閹间礁纾绘繛鎴烆焸濞戞鏆嗛柛鏇ㄥ墯濞呫垽姊虹粙鍖″姛闁哥姴閰ｉ幆鍐洪鍛幗濡炪倖鎸鹃崰鎾剁不閻愮儤鐓涚�癸拷鐎ｎ剛鐦堥悗瑙勬礀閵堟悂銆侀弴銏℃櫖闁告洦鍋勫В鍫ユ⒒閸屾瑦绁版俊妞煎妿缁牊鎷呴搹鍦槸婵犵數濮撮崐濠氬汲閿旇姤鍙忔俊顖涘绾墽绱掗悪娆忔处閻撴洘銇勯鐔风仴闁哄鍊曢湁闁绘ɑ鐟ュú锕傛偂閻斿吋鐓熸俊顖氭惈缁狙囨煙椤栨氨澧﹂柡灞剧洴閹垻鎷犻幓鎺懶ラ梻渚�娼уú锕傚礉閺嵮屽殫闁告洦鍨扮粻娑欍亜閹捐泛孝濠殿喚鍏樺缁樻媴閾忕懓绗￠梺鍝勮閸旀垿鐛弽顓ф晝闁挎棁妫勫▓鐔兼⒑鐠恒劌娅愰柟鍑ゆ嫹

        macHdr.Value = 0;
        macHdr.Bits.MType = FRAME_TYPE_JOIN_REQ;

        fCtrl.Value = 0;
        fCtrl.Bits.Adr = AdrCtrlOn;

        /* In case of join request retransmissions, the stack must prepare
         * the frame again, because the network server keeps track of the random
         * LoRaMacDevNonce values to prevent reply attacks. */
        PrepareFrame( &macHdr, &fCtrl, 0, NULL, 0 );
    }

    ScheduleTx( );
}

static void OnRxWindow1TimerEvent( void )
{
    TimerStop( &RxWindowTimer1 );

#ifdef debug
    RxSlot = 0;
    lora_printf("OnRxWindow1TimerEvent\n");
	uint64_t tim=0;
	uint32_t tim1=0;
	uint32_t tim2=0;
	tim=timerRead(timer);
	tim1=tim&0XFFFFFFFF;
	tim=tim>>32;
	tim2=tim&0XFFFFFFFF;
	lora_printf("%d  %ld  %ld\n",millis(),tim2,tim1);
#endif
    if( LoRaMacDeviceClass == CLASS_C )
    {
        Radio.Standby( );
    }

#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
    RxWindowSetup( Channels[Channel].Frequency, RxWindowsParams[0].Datarate, RxWindowsParams[0].Bandwidth, RxWindowsParams[0].RxWindowTimeout, false );
    //闂傚倸鍊搁崐鎼佸磹妞嬪海鐭嗗〒姘炬嫹妤犵偛顦甸崹楣冨箛娴ｅ憡娅婂┑鐘垫暩婵敻鎳濇ィ鍐╁仾闁绘劦鍓涚弧锟介梻鍌氱墛娓氭宕曡箛娑欑厸閻庯絺鏅濈粣鏃堟煛鐏炶濮傞柟顔哄�濆畷鎺戔槈濮楀棔绱� ( Channel % 48 ) 闂傚倸鍊搁崐宄懊归崶顒夋晪鐟滃繘骞戦姀銈呯疀妞ゆ棁妫勬惔濠囨⒑瑜版帒浜伴柛搴ㄦ涧閳藉濮�閻樼數娼夐梻浣侯焾閺堫剛鍒掓惔銊﹀仼闁割偅娲橀埛鎴犵磼鐎ｎ偄顕滄繝锟介幍顔剧＜妞ゆ棁鍋愭晥閻庤娲橀崹鐢革綖濠婂牊鏅稿ù鐘差儏缁犳牗鎱ㄥΔ瀣闂佽鍠楅悷鈺呭箖濠婂喚娼ㄩ柛顐犲灩閹偟绱撻崒姘拷宄懊归崶顒夋晪鐟滃秷鐏嬪┑鐘绘涧濡厼顭囬弽銊х鐎瑰壊鍠曢幉楣冩煛娴ｅ憡顥滈棁澶愭煥濠靛棙澶勯柛銈忔嫹缂備胶鍋撳妯肩矓閻熼偊娼栨繛宸簻缁犲綊鏌ｉ幇顓炵祷妞ゎ剙鐗撻弻锝嗘償閵忊懇鏋旈梺鍝勬噺缁诲嫰宕氶幒鎴旀瀻闁规儳鐡ㄥ▍銏ゆ⒑鐠恒劌娅愰柟鍑ゆ嫹?~7闂傚倸鍊搁崐鎼佸磹閻戣姤鍊块柨鏃�鎷濋幏椋庣箔濞戞ɑ鍣介柣顓熺懇閺岀喐娼忔ィ鍐╊�嶉梺鎼炲�栭崝鏍Φ閸曨垰鍐�妞ゆ劦婢�缁爼姊洪幖鐐插闁稿﹤娼￠悰顕�寮介‖銉ラ叄椤㈡鏁撻懞銉ょ箚闁兼祴鏅濈壕濂告煥閻斿墎鐭欑�规洖宕灒闂傦拷閹邦喚娉块梻鍌欐祰椤顭垮Ο缁樻珷閹艰揪绲介ˉ姘舵煕瑜庨〃鍡涙偂閻斿吋鐓熼柡鍐挎嫹婵炲吋鐟╅幃姗�鍩￠崨顔惧帗闂佽姤锚椤﹁棄螣閿熻棄螖閻橀潧浠﹂柨鏇樺灲閻涱噣宕堕妸锕�顎撶紓浣割儏閻忔繈藟閹烘垟鏀介柨娑樺娴滃ジ鏌涙繝鍐ㄧ伌鐎规洜鎳撶叅妞ゅ繐瀚悗顓㈡⒑閸涘﹥灏柤鍐茬埣閹苯顫濋懜纰樻嫼濡炪倖甯幏椋庣磼婢跺本鏆�殿喗鐓￠獮鏍ㄦ媴閸︻厼骞楁繝纰樻閸ㄤ即骞栭锔藉殝閻熸瑥瀚ㄦ禍婊堟煏婢舵稓鐣遍柛婵堝劋閹便劍绻濋崟顐㈠闁诲海鏁搁崢褔鍩㈡惔銊ョ疀妞ゆ帒顦槐锟�?~7闂傚倸鍊搁崐宄懊归崶顒夋晪鐟滃秹婀侀梺缁樺灱濡嫰寮告笟锟介弻鐔兼⒒鐎靛壊妲紓浣哄Х婵烇拷闁哄瞼鍠栭幃娆擃敆閿熶粙宕濈捄琛℃斀妞ゆ棁妫勯埢鏇㈡煛鐏炶濮傞柟顔哄�濆畷鎺戔槈濮楀棔绱�
#elif defined( USE_BAND_470 ) || defined( USE_BAND_470PREQUEL )
    //lora_printf("first Rx feq: %d\n",LORAMAC_FIRST_RX1_CHANNEL + ( Channel % 48 ) * LORAMAC_STEPWIDTH_RX1_CHANNEL);
    RxWindowSetup( LORAMAC_FIRST_RX1_CHANNEL + ( Channel % 48 ) * LORAMAC_STEPWIDTH_RX1_CHANNEL, RxWindowsParams[0].Datarate, RxWindowsParams[0].Bandwidth, RxWindowsParams[0].RxWindowTimeout, false );
#elif ( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) )
    RxWindowSetup( LORAMAC_FIRST_RX1_CHANNEL + ( Channel % 8 ) * LORAMAC_STEPWIDTH_RX1_CHANNEL, RxWindowsParams[0].Datarate, RxWindowsParams[0].Bandwidth, RxWindowsParams[0].RxWindowTimeout, false );
#else
    #error "Please define a frequency band in the compiler options."
#endif
}

static void OnRxWindow2TimerEvent( void )
{
    bool rxContinuousMode = false;
    //lora_printf("OnRxWindow2TimerEvent\n");

    TimerStop( &RxWindowTimer2 );

    if( LoRaMacDeviceClass == CLASS_C )
    {
        rxContinuousMode = true;
    }
    //lora_printf("second Rx feq: %d\r\n",LoRaMacParams.Rx2Channel.Frequency);
    if( RxWindowSetup( LoRaMacParams.Rx2Channel.Frequency, RxWindowsParams[1].Datarate, RxWindowsParams[1].Bandwidth, RxWindowsParams[1].RxWindowTimeout, rxContinuousMode ) == true )
    {
        RxSlot = 1;
    }
}

static void OnAckTimeoutTimerEvent( void )
{
	//lora_printf("OnAckTimeoutTimerEvent\n");

    TimerStop( &AckTimeoutTimer );

    if( NodeAckRequested == true )
    {
        AckTimeoutRetry = true;
        LoRaMacState &= ~LORAMAC_ACK_REQ;
    }
    if( LoRaMacDeviceClass == CLASS_C )
    {
        LoRaMacFlags.Bits.MacDone = 1;
    }
}

static bool SetNextChannel( TimerTime_t* time )
{
    uint8_t nbEnabledChannels = 0;
    uint8_t delayTx = 0;
    uint8_t enabledChannels[LORA_MAX_NB_CHANNELS];
    TimerTime_t nextTxDelay = ( TimerTime_t )( -1 );

    memset1( enabledChannels, 0, LORA_MAX_NB_CHANNELS );

#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    if( CountNbEnabled125kHzChannels( ChannelsMaskRemaining ) == 0 )
    { // Restore default channels
        memcpy1( ( uint8_t* ) ChannelsMaskRemaining, ( uint8_t* ) LoRaMacParams.ChannelsMask, 8 );
    }
    if( ( LoRaMacParams.ChannelsDatarate >= DR_4 ) && ( ( ChannelsMaskRemaining[4] & 0x00FF ) == 0 ) )
    { // Make sure, that the channels are activated
        ChannelsMaskRemaining[4] = LoRaMacParams.ChannelsMask[4];
    }
#elif defined( USE_BAND_470 ) || defined( USE_BAND_470PREQUEL )
    if( ( CountBits( LoRaMacParams.ChannelsMask[0], 16 ) == 0 ) &&
        ( CountBits( LoRaMacParams.ChannelsMask[1], 16 ) == 0 ) &&
        ( CountBits( LoRaMacParams.ChannelsMask[2], 16 ) == 0 ) &&
        ( CountBits( LoRaMacParams.ChannelsMask[3], 16 ) == 0 ) &&
        ( CountBits( LoRaMacParams.ChannelsMask[4], 16 ) == 0 ) &&
        ( CountBits( LoRaMacParams.ChannelsMask[5], 16 ) == 0 ) )
    {
        memcpy1( ( uint8_t* )LoRaMacParams.ChannelsMask, ( uint8_t* )LoRaMacParamsDefaults.ChannelsMask, sizeof( LoRaMacParams.ChannelsMask ) );
    }
#else
    if( CountBits( LoRaMacParams.ChannelsMask[0], 16 ) == 0 )
    {
        // Re-enable default channels, if no channel is enabled
        LoRaMacParams.ChannelsMask[0] = LoRaMacParams.ChannelsMask[0] | ( LC( 1 ) + LC( 2 ) + LC( 3 ) );
    }
#endif

    // Update Aggregated duty cycle
    if( AggregatedTimeOff <= TimerGetElapsedTime( AggregatedLastTxDoneTime ) )
    {
        AggregatedTimeOff = 0;

        // Update bands Time OFF
        for( uint8_t i = 0; i < LORA_MAX_NB_BANDS; i++ )
        {
            if( ( IsLoRaMacNetworkJoined == false ) || ( DutyCycleOn == true ) )
            {
                if( Bands[i].TimeOff <= TimerGetElapsedTime( Bands[i].LastTxDoneTime ) )
                {
                    Bands[i].TimeOff = 0;
                }
                if( Bands[i].TimeOff != 0 )
                {
                    nextTxDelay = MIN( Bands[i].TimeOff - TimerGetElapsedTime( Bands[i].LastTxDoneTime ), nextTxDelay );
                }
            }
            else
            {
                if( DutyCycleOn == false )
                {
                    Bands[i].TimeOff = 0;
                }
            }
        }

        // Search how many channels are enabled
        for( uint8_t i = 0, k = 0; i < LORA_MAX_NB_CHANNELS; i += 16, k++ )
        {
            for( uint8_t j = 0; j < 16; j++ )
            {
#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
                if( ( ChannelsMaskRemaining[k] & ( 1 << j ) ) != 0 )
#else
                if( ( LoRaMacParams.ChannelsMask[k] & ( 1 << j ) ) != 0 )
#endif
                {
                    if( Channels[i + j].Frequency == 0 )
                    { // Check if the channel is enabled
                        continue;
                    }
#if defined( USE_BAND_868 ) || defined( USE_BAND_433 ) || defined( USE_BAND_780 )
                    if( IsLoRaMacNetworkJoined == false )
                    {
                        if( ( JOIN_CHANNELS & ( 1 << j ) ) == 0 )
                        {
                            continue;
                        }
                    }
#endif
                    if( ( ( Channels[i + j].DrRange.Fields.Min <= LoRaMacParams.ChannelsDatarate ) &&
                          ( LoRaMacParams.ChannelsDatarate <= Channels[i + j].DrRange.Fields.Max ) ) == false )
                    { // Check if the current channel selection supports the given datarate
                        continue;
                    }
                    if( Bands[Channels[i + j].Band].TimeOff > 0 )
                    { // Check if the band is available for transmission
                        delayTx++;
                        continue;
                    }
                    enabledChannels[nbEnabledChannels++] = i + j;
                }
            }
        }
    }
    else
    {
        delayTx++;
        nextTxDelay = AggregatedTimeOff - TimerGetElapsedTime( AggregatedLastTxDoneTime );
    }

    if( nbEnabledChannels > 0 )
    {
        Channel = enabledChannels[randr( 0, nbEnabledChannels - 1 )];
#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
        if( Channel < ( LORA_MAX_NB_CHANNELS - 8 ) )
        {
            DisableChannelInMask( Channel, ChannelsMaskRemaining );
        }
#endif
        *time = 0;
        return true;
    }
    else
    {
        if( delayTx > 0 )
        {
            // Delay transmission due to AggregatedTimeOff or to a band time off
            *time = nextTxDelay;
            return true;
        }
        // Datarate not supported by any channel
        *time = 0;
        return false;
    }
}

static bool RxWindowSetup( uint32_t freq, int8_t datarate, uint32_t bandwidth, uint16_t timeout, bool rxContinuous )
{
    uint8_t downlinkDatarate = Datarates[datarate];
    RadioModems_t modem;

    if( Radio.GetStatus( ) == RF_IDLE )
    {
        Radio.SetChannel( freq );

        // Store downlink datarate
        McpsIndication.RxDatarate = ( uint8_t ) datarate;

#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
        if( datarate == DR_7 )
        {
            modem = MODEM_FSK;
            Radio.SetRxConfig( modem, 50e3, downlinkDatarate * 1e3, 0, 83.333e3, 5, timeout, false, 0, true, 0, 0, false, rxContinuous );
        }
        else
        {
            modem = MODEM_LORA;
            Radio.SetRxConfig( modem, bandwidth, downlinkDatarate, 1, 0, 8, timeout, false, 0, false, 0, 0, true, rxContinuous );
        }
#elif defined( USE_BAND_470 ) || defined( USE_BAND_470PREQUEL ) || defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
        modem = MODEM_LORA;
        Radio.SetRxConfig( modem, bandwidth, downlinkDatarate, 1, 0, 8, timeout, false, 0, false, 0, 0, true, rxContinuous );
#endif

        if( RepeaterSupport == true )
        {
            Radio.SetMaxPayloadLength( modem, MaxPayloadOfDatarateRepeater[datarate] + LORA_MAC_FRMPAYLOAD_OVERHEAD );
        }
        else
        {
            Radio.SetMaxPayloadLength( modem, MaxPayloadOfDatarate[datarate] + LORA_MAC_FRMPAYLOAD_OVERHEAD );
        }

        if( rxContinuous == false )
        {
            Radio.Rx( LoRaMacParams.MaxRxWindow );
        }
        else
        {
            Radio.Rx( 0 ); // Continuous mode
        }
        return true;
    }
    return false;
}

static bool Rx2FreqInRange( uint32_t freq )
{
#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
    if( Radio.CheckRfFrequency( freq ) == true )
#elif defined( USE_BAND_470 ) || defined( USE_BAND_470PREQUEL ) || defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    if( ( Radio.CheckRfFrequency( freq ) == true ) &&
        ( freq >= LORAMAC_FIRST_RX1_CHANNEL ) &&
        ( freq <= LORAMAC_LAST_RX1_CHANNEL ) &&
        ( ( ( freq - ( uint32_t ) LORAMAC_FIRST_RX1_CHANNEL ) % ( uint32_t ) LORAMAC_STEPWIDTH_RX1_CHANNEL ) == 0 ) )
#endif
    {
        return true;
    }
    return false;
}

static bool ValidatePayloadLength( uint8_t lenN, int8_t datarate, uint8_t fOptsLen )
{
    uint16_t maxN = 0;
    uint16_t payloadSize = 0;

    // Get the maximum payload length
    if( RepeaterSupport == true )
    {
        maxN = MaxPayloadOfDatarateRepeater[datarate];
    }
    else
    {
        maxN = MaxPayloadOfDatarate[datarate];
    }

    // Calculate the resulting payload size
    payloadSize = ( lenN + fOptsLen );

    // Validation of the application payload size
    if( ( payloadSize <= maxN ) && ( payloadSize <= LORAMAC_PHY_MAXPAYLOAD ) )
    {
        return true;
    }
    return false;
}

static uint8_t CountBits( uint16_t mask, uint8_t nbBits )
{
    uint8_t nbActiveBits = 0;

    for( uint8_t j = 0; j < nbBits; j++ )
    {
        if( ( mask & ( 1 << j ) ) == ( 1 << j ) )
        {
            nbActiveBits++;
        }
    }
    return nbActiveBits;
}

#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
static uint8_t CountNbEnabled125kHzChannels( uint16_t *channelsMask )
{
    uint8_t nb125kHzChannels = 0;

    for( uint8_t i = 0, k = 0; i < LORA_MAX_NB_CHANNELS - 8; i += 16, k++ )
    {
        nb125kHzChannels += CountBits( channelsMask[k], 16 );
    }

    return nb125kHzChannels;
}

#if defined( USE_BAND_915_HYBRID )
static void ReenableChannels( uint16_t mask, uint16_t* channelsMask )
{
    uint16_t blockMask = mask;

    for( uint8_t i = 0, j = 0; i < 4; i++, j += 2 )
    {
        channelsMask[i] = 0;
        if( ( blockMask & ( 1 << j ) ) != 0 )
        {
            channelsMask[i] |= 0x00FF;
        }
        if( ( blockMask & ( 1 << ( j + 1 ) ) ) != 0 )
        {
            channelsMask[i] |= 0xFF00;
        }
    }
    channelsMask[4] = blockMask;
    channelsMask[5] = 0x0000;
}

static bool ValidateChannelMask( uint16_t* channelsMask )
{
    bool chanMaskState = false;
    uint16_t block1 = 0;
    uint16_t block2 = 0;
    uint8_t index = 0;

    for( uint8_t i = 0; i < 4; i++ )
    {
        block1 = channelsMask[i] & 0x00FF;
        block2 = channelsMask[i] & 0xFF00;

        if( ( CountBits( block1, 16 ) > 5 ) && ( chanMaskState == false ) )
        {
            channelsMask[i] &= block1;
            channelsMask[4] = 1 << ( i * 2 );
            chanMaskState = true;
            index = i;
        }
        else if( ( CountBits( block2, 16 ) > 5 ) && ( chanMaskState == false ) )
        {
            channelsMask[i] &= block2;
            channelsMask[4] = 1 << ( i * 2 + 1 );
            chanMaskState = true;
            index = i;
        }
    }

    // Do only change the channel mask, if we have found a valid block.
    if( chanMaskState == true )
    {
        for( uint8_t i = 0; i < 4; i++ )
        {
            if( i != index )
            {
                channelsMask[i] = 0;
            }
        }
    }
    return chanMaskState;
}
#endif
#endif

static bool ValidateDatarate( int8_t datarate, uint16_t* channelsMask )
{
    if( ValueInRange( datarate, LORAMAC_TX_MIN_DATARATE, LORAMAC_TX_MAX_DATARATE ) == false )
    {
        return false;
    }
    for( uint8_t i = 0, k = 0; i < LORA_MAX_NB_CHANNELS; i += 16, k++ )
    {
        for( uint8_t j = 0; j < 16; j++ )
        {
            if( ( ( channelsMask[k] & ( 1 << j ) ) != 0 ) )
            {// Check datarate validity for enabled channels
                if( ValueInRange( datarate, Channels[i + j].DrRange.Fields.Min, Channels[i + j].DrRange.Fields.Max ) == true )
                {
                    // At least 1 channel has been found we can return OK.
                    return true;
                }
            }
        }
    }
    return false;
}

static int8_t LimitTxPower( int8_t txPower, int8_t maxBandTxPower )
{
    int8_t resultTxPower = txPower;

    // Limit tx power to the band max
    resultTxPower =  MAX( txPower, maxBandTxPower );

#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    if( ( LoRaMacParams.ChannelsDatarate == DR_4 ) ||
        ( ( LoRaMacParams.ChannelsDatarate >= DR_8 ) && ( LoRaMacParams.ChannelsDatarate <= DR_13 ) ) )
    {// Limit tx power to max 26dBm
        resultTxPower =  MAX( txPower, TX_POWER_26_DBM );
    }
    else
    {
        if( CountNbEnabled125kHzChannels( LoRaMacParams.ChannelsMask ) < 50 )
        {// Limit tx power to max 21dBm
            resultTxPower = MAX( txPower, TX_POWER_20_DBM );
        }
    }
#endif
    return resultTxPower;
}

static bool ValueInRange( int8_t value, int8_t min, int8_t max )
{
    if( ( value >= min ) && ( value <= max ) )
    {
        return true;
    }
    return false;
}

static bool DisableChannelInMask( uint8_t id, uint16_t* mask )
{
    uint8_t index = 0;
    index = id / 16;

    if( ( index > 4 ) || ( id >= LORA_MAX_NB_CHANNELS ) )
    {
        return false;
    }

    // Deactivate channel
    mask[index] &= ~( 1 << ( id % 16 ) );

    return true;
}
//濠电姷鏁告慨鐑藉极閹间礁纾婚柣鎰惈閺勩儱鈹戦悩鍙夋悙缂佺媭鍨堕弻銊╂偆閸屾稑顏�?闂傚倸鍊搁崐鎼佸磹妞嬪海鐭嗗〒姘炬嫹妤犵偞鐗犻、鏇㈡晝閿熺晫绮ｅΔ浣瑰弿婵妫楅崝顕�鏌熼悜姗嗘畷闁哄懎鎽滅槐鎺撴綇閳哄啯缍�
static bool AdrNextDr( bool adrEnabled, bool updateChannelMask, int8_t* datarateOut )
{
    bool adrAckReq = false;
    int8_t datarate = LoRaMacParams.ChannelsDatarate;

    if( adrEnabled == true )
    {
        if( datarate == LORAMAC_TX_MIN_DATARATE )
        {
            AdrAckCounter = 0;
            adrAckReq = false;
        }
        else
        {
            if( AdrAckCounter >= ADR_ACK_LIMIT )
            {
                adrAckReq = true;
                LoRaMacParams.ChannelsTxPower = LORAMAC_MAX_TX_POWER;
            }
            else
            {
                adrAckReq = false;
            }
            if( AdrAckCounter >= ( ADR_ACK_LIMIT + ADR_ACK_DELAY ) )
            {
                if( ( AdrAckCounter % ADR_ACK_DELAY ) == 1 )
                {
#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
                    if( datarate > LORAMAC_TX_MIN_DATARATE )
                    {
                        datarate--;
                    }
                    if( datarate == LORAMAC_TX_MIN_DATARATE )
                    {
                        if( updateChannelMask == true )
                        {
                            // Re-enable default channels LC1, LC2, LC3
                            LoRaMacParams.ChannelsMask[0] = LoRaMacParams.ChannelsMask[0] | ( LC( 1 ) + LC( 2 ) + LC( 3 ) );
                        }
                    }
#elif defined( USE_BAND_470 ) || defined( USE_BAND_470PREQUEL )
                    if( datarate > LORAMAC_TX_MIN_DATARATE )
                    {
                        datarate--;
                    }
                    if( datarate == LORAMAC_TX_MIN_DATARATE )
                    {
                        if( updateChannelMask == true )
                        {
                            // Re-enable default channels
                            memcpy1( ( uint8_t* )LoRaMacParams.ChannelsMask, ( uint8_t* )LoRaMacParamsDefaults.ChannelsMask, sizeof( LoRaMacParams.ChannelsMask ) );
                        }
                    }
#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
                    if( ( datarate > LORAMAC_TX_MIN_DATARATE ) && ( datarate == DR_8 ) )
                    {
                        datarate = DR_4;
                    }
                    else if( datarate > LORAMAC_TX_MIN_DATARATE )
                    {
                        datarate--;
                    }
                    if( datarate == LORAMAC_TX_MIN_DATARATE )
                    {
                        if( updateChannelMask == true )
                        {
#if defined( USE_BAND_915 )
                            // Re-enable default channels
                            memcpy1( ( uint8_t* )LoRaMacParams.ChannelsMask, ( uint8_t* )LoRaMacParamsDefaults.ChannelsMask, sizeof( LoRaMacParams.ChannelsMask ) );
#else // defined( USE_BAND_915_HYBRID )
                            // Re-enable default channels
                            ReenableChannels( LoRaMacParamsDefaults.ChannelsMask[4], LoRaMacParams.ChannelsMask );
#endif
                        }
                    }
#else
#error "Please define a frequency band in the compiler options."
#endif
                }
            }
        }
    }

    *datarateOut = datarate;

    return adrAckReq;
}

static LoRaMacStatus_t AddMacCommand( uint8_t cmd, uint8_t p1, uint8_t p2 )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_BUSY;
    // The maximum buffer length must take MAC commands to re-send into account.
    uint8_t bufLen = LORA_MAC_COMMAND_MAX_LENGTH - MacCommandsBufferToRepeatIndex;

    switch( cmd )
    {
        case MOTE_MAC_LINK_CHECK_REQ:
            if( MacCommandsBufferIndex < bufLen )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this command
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_LINK_ADR_ANS:
            if( MacCommandsBufferIndex < ( bufLen - 1 ) )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Margin
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_DUTY_CYCLE_ANS:
            if( MacCommandsBufferIndex < bufLen )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_RX_PARAM_SETUP_ANS:
            if( MacCommandsBufferIndex < ( bufLen - 1 ) )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Datarate ACK, Channel ACK
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_DEV_STATUS_ANS:
            if( MacCommandsBufferIndex < ( bufLen - 2 ) )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // 1st byte Battery
                // 2nd byte Margin
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                MacCommandsBuffer[MacCommandsBufferIndex++] = p2;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_NEW_CHANNEL_ANS:
            if( MacCommandsBufferIndex < ( bufLen - 1 ) )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Datarate range OK, Channel frequency OK
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_RX_TIMING_SETUP_ANS:
            if( MacCommandsBufferIndex < bufLen )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
                status = LORAMAC_STATUS_OK;
            }
            break;
        default:
            return LORAMAC_STATUS_SERVICE_UNKNOWN;
    }
    if( status == LORAMAC_STATUS_OK )
    {
        MacCommandsInNextTx = true;
    }
    return status;
}

static uint8_t ParseMacCommandsToRepeat( uint8_t* cmdBufIn, uint8_t length, uint8_t* cmdBufOut )
{
    uint8_t i = 0;
    uint8_t cmdCount = 0;

    if( ( cmdBufIn == NULL ) || ( cmdBufOut == NULL ) )
    {
        return 0;
    }

    for( i = 0; i < length; i++ )
    {
        switch( cmdBufIn[i] )
        {
            // STICKY
            case MOTE_MAC_RX_PARAM_SETUP_ANS:
            {
                cmdBufOut[cmdCount++] = cmdBufIn[i++];
                cmdBufOut[cmdCount++] = cmdBufIn[i];
                break;
            }
            case MOTE_MAC_RX_TIMING_SETUP_ANS:
            {
                cmdBufOut[cmdCount++] = cmdBufIn[i];
                break;
            }
            // NON-STICKY
            case MOTE_MAC_DEV_STATUS_ANS:
            { // 2 bytes payload
                i += 2;
                break;
            }
            case MOTE_MAC_LINK_ADR_ANS:
            case MOTE_MAC_NEW_CHANNEL_ANS:
            { // 1 byte payload
                i++;
                break;
            }
            case MOTE_MAC_DUTY_CYCLE_ANS:
            case MOTE_MAC_LINK_CHECK_REQ:
            { // 0 byte payload
                break;
            }
            default:
                break;
        }
    }

    return cmdCount;
}

static void ProcessMacCommands( uint8_t *payload, uint8_t macIndex, uint8_t commandsSize, uint8_t snr )
{
    while( macIndex < commandsSize )
    {
        // Decode Frame MAC commands
        switch( payload[macIndex++] )
        {
            case SRV_MAC_LINK_CHECK_ANS:
                MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                MlmeConfirm.DemodMargin = payload[macIndex++];
                MlmeConfirm.NbGateways = payload[macIndex++];
                break;
            case SRV_MAC_LINK_ADR_REQ:
                {
                    uint8_t i;
                    uint8_t status = 0x07;
                    uint16_t chMask;
                    int8_t txPower = 0;
                    int8_t datarate = 0;
                    uint8_t nbRep = 0;
                    uint8_t chMaskCntl = 0;
                    uint16_t channelsMask[6] = { 0, 0, 0, 0, 0, 0 };

                    // Initialize local copy of the channels mask array
                    for( i = 0; i < 6; i++ )
                    {
                        channelsMask[i] = LoRaMacParams.ChannelsMask[i];
                    }
                    datarate = payload[macIndex++];
                    txPower = datarate & 0x0F;
                    datarate = ( datarate >> 4 ) & 0x0F;

                    if( ( AdrCtrlOn == false ) &&
                        ( ( LoRaMacParams.ChannelsDatarate != datarate ) || ( LoRaMacParams.ChannelsTxPower != txPower ) ) )
                    { // ADR disabled don't handle ADR requests if server tries to change datarate or txpower
                        // Answer the server with fail status
                        // Power ACK     = 0
                        // Data rate ACK = 0
                        // Channel mask  = 0
                        AddMacCommand( MOTE_MAC_LINK_ADR_ANS, 0, 0 );
                        macIndex += 3;  // Skip over the remaining bytes of the request
                        break;
                    }
                    chMask = ( uint16_t )payload[macIndex++];
                    chMask |= ( uint16_t )payload[macIndex++] << 8;

                    nbRep = payload[macIndex++];
                    chMaskCntl = ( nbRep >> 4 ) & 0x07;
                    nbRep &= 0x0F;
                    if( nbRep == 0 )
                    {
                        nbRep = 1;
                    }
#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
                    if( ( chMaskCntl == 0 ) && ( chMask == 0 ) )
                    {
                        status &= 0xFE; // Channel mask KO
                    }
                    else if( ( ( chMaskCntl >= 1 ) && ( chMaskCntl <= 5 )) ||
                             ( chMaskCntl >= 7 ) )
                    {
                        // RFU
                        status &= 0xFE; // Channel mask KO
                    }
                    else
                    {
                        for( i = 0; i < LORA_MAX_NB_CHANNELS; i++ )
                        {
                            if( chMaskCntl == 6 )
                            {
                                if( Channels[i].Frequency != 0 )
                                {
                                    chMask |= 1 << i;
                                }
                            }
                            else
                            {
                                if( ( ( chMask & ( 1 << i ) ) != 0 ) &&
                                    ( Channels[i].Frequency == 0 ) )
                                {// Trying to enable an undefined channel
                                    status &= 0xFE; // Channel mask KO
                                }
                            }
                        }
                        channelsMask[0] = chMask;
                    }
#elif defined( USE_BAND_470 ) || defined( USE_BAND_470PREQUEL )
                    if( chMaskCntl == 6 )
                    {
                        // Enable all 125 kHz channels
                        for( uint8_t i = 0, k = 0; i < LORA_MAX_NB_CHANNELS; i += 16, k++ )
                        {
                            for( uint8_t j = 0; j < 16; j++ )
                            {
                                if( Channels[i + j].Frequency != 0 )
                                {
                                    channelsMask[k] |= 1 << j;
                                }
                            }
                        }
                    }
                    else if( chMaskCntl == 7 )
                    {
                        status &= 0xFE; // Channel mask KO
                    }
                    else
                    {
                        for( uint8_t i = 0; i < 16; i++ )
                        {
                            if( ( ( chMask & ( 1 << i ) ) != 0 ) &&
                                ( Channels[chMaskCntl * 16 + i].Frequency == 0 ) )
                            {// Trying to enable an undefined channel
                                status &= 0xFE; // Channel mask KO
                            }
                        }
                        channelsMask[chMaskCntl] = chMask;
                    }
#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
                    if( chMaskCntl == 6 )
                    {
                        // Enable all 125 kHz channels
                        channelsMask[0] = 0xFFFF;
                        channelsMask[1] = 0xFFFF;
                        channelsMask[2] = 0xFFFF;
                        channelsMask[3] = 0xFFFF;
                        // Apply chMask to channels 64 to 71
                        channelsMask[4] = chMask;
                    }
                    else if( chMaskCntl == 7 )
                    {
                        // Disable all 125 kHz channels
                        channelsMask[0] = 0x0000;
                        channelsMask[1] = 0x0000;
                        channelsMask[2] = 0x0000;
                        channelsMask[3] = 0x0000;
                        // Apply chMask to channels 64 to 71
                        channelsMask[4] = chMask;
                    }
                    else if( chMaskCntl == 5 )
                    {
                        // RFU
                        status &= 0xFE; // Channel mask KO
                    }
                    else
                    {
                        channelsMask[chMaskCntl] = chMask;

                        // FCC 15.247 paragraph F mandates to hop on at least 2 125 kHz channels
                        if( ( datarate < DR_4 ) && ( CountNbEnabled125kHzChannels( channelsMask ) < 2 ) )
                        {
                            status &= 0xFE; // Channel mask KO
                        }

#if defined( USE_BAND_915_HYBRID )
                        if( ValidateChannelMask( channelsMask ) == false )
                        {
                            status &= 0xFE; // Channel mask KO
                        }
#endif
                    }
#else
    #error "Please define a frequency band in the compiler options."
#endif
                    if( ValidateDatarate( datarate, channelsMask ) == false )
                    {
                        status &= 0xFD; // Datarate KO
                    }

                    //
                    // Remark MaxTxPower = 0 and MinTxPower = 5
                    //
                    if( ValueInRange( txPower, LORAMAC_MAX_TX_POWER, LORAMAC_MIN_TX_POWER ) == false )
                    {
                        status &= 0xFB; // TxPower KO
                    }
                    if( ( status & 0x07 ) == 0x07 )
                    {
                        LoRaMacParams.ChannelsDatarate = datarate;
                        LoRaMacParams.ChannelsTxPower = txPower;

                        memcpy1( ( uint8_t* )LoRaMacParams.ChannelsMask, ( uint8_t* )channelsMask, sizeof( LoRaMacParams.ChannelsMask ) );

                        LoRaMacParams.ChannelsNbRep = nbRep;
#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
                        // Reset ChannelsMaskRemaining to the new ChannelsMask
                        ChannelsMaskRemaining[0] &= channelsMask[0];
                        ChannelsMaskRemaining[1] &= channelsMask[1];
                        ChannelsMaskRemaining[2] &= channelsMask[2];
                        ChannelsMaskRemaining[3] &= channelsMask[3];
                        ChannelsMaskRemaining[4] = channelsMask[4];
                        ChannelsMaskRemaining[5] = channelsMask[5];
#endif
                    }
                    AddMacCommand( MOTE_MAC_LINK_ADR_ANS, status, 0 );
                }
                break;
            case SRV_MAC_DUTY_CYCLE_REQ:
                MaxDCycle = payload[macIndex++];
                AggregatedDCycle = 1 << MaxDCycle;
                AddMacCommand( MOTE_MAC_DUTY_CYCLE_ANS, 0, 0 );
                break;
            case SRV_MAC_RX_PARAM_SETUP_REQ:
                {
                    uint8_t status = 0x07;
                    int8_t datarate = 0;
                    int8_t drOffset = 0;
                    uint32_t freq = 0;

                    drOffset = ( payload[macIndex] >> 4 ) & 0x07;
                    datarate = payload[macIndex] & 0x0F;
                    macIndex++;

                    freq =  ( uint32_t )payload[macIndex++];
                    freq |= ( uint32_t )payload[macIndex++] << 8;
                    freq |= ( uint32_t )payload[macIndex++] << 16;
                    freq *= 100;

                    if( Rx2FreqInRange( freq ) == false )
                    {
                        status &= 0xFE; // Channel frequency KO
                    }

                    if( ValueInRange( datarate, LORAMAC_RX_MIN_DATARATE, LORAMAC_RX_MAX_DATARATE ) == false )
                    {
                        status &= 0xFD; // Datarate KO
                    }
#if ( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) )
                    if( ( ValueInRange( datarate, DR_5, DR_7 ) == true ) ||
                        ( datarate > DR_13 ) )
                    {
                        status &= 0xFD; // Datarate KO
                    }
#endif
                    if( ValueInRange( drOffset, LORAMAC_MIN_RX1_DR_OFFSET, LORAMAC_MAX_RX1_DR_OFFSET ) == false )
                    {
                        status &= 0xFB; // Rx1DrOffset range KO
                    }

                    if( ( status & 0x07 ) == 0x07 )
                    {
                        LoRaMacParams.Rx2Channel.Datarate = datarate;
                        LoRaMacParams.Rx2Channel.Frequency = freq;
                        LoRaMacParams.Rx1DrOffset = drOffset;
                    }
                    AddMacCommand( MOTE_MAC_RX_PARAM_SETUP_ANS, status, 0 );
                }
                break;
            case SRV_MAC_DEV_STATUS_REQ:
                {
                    uint8_t batteryLevel = BAT_LEVEL_NO_MEASURE;
                    if( ( LoRaMacCallbacks != NULL ) && ( LoRaMacCallbacks->GetBatteryLevel != NULL ) )
                    {
                        batteryLevel = LoRaMacCallbacks->GetBatteryLevel( );
                    }
                    AddMacCommand( MOTE_MAC_DEV_STATUS_ANS, batteryLevel, snr );
                    break;
                }
            case SRV_MAC_NEW_CHANNEL_REQ:
                {
                    uint8_t status = 0x03;

#if defined( USE_BAND_470 ) || defined( USE_BAND_470PREQUEL ) || defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
                    status &= 0xFC; // Channel frequency and datarate KO
                    macIndex += 5;
#else
                    int8_t channelIndex = 0;
                    ChannelParams_t chParam;

                    channelIndex = payload[macIndex++];
                    chParam.Frequency = ( uint32_t )payload[macIndex++];
                    chParam.Frequency |= ( uint32_t )payload[macIndex++] << 8;
                    chParam.Frequency |= ( uint32_t )payload[macIndex++] << 16;
                    chParam.Frequency *= 100;
                    chParam.DrRange.Value = payload[macIndex++];

                    LoRaMacState |= LORAMAC_TX_CONFIG;
                    if( chParam.Frequency == 0 )
                    {
                        if( channelIndex < 3 )
                        {
                            status &= 0xFC;
                        }
                        else
                        {
                            if( LoRaMacChannelRemove( channelIndex ) != LORAMAC_STATUS_OK )
                            {
                                status &= 0xFC;
                            }
                        }
                    }
                    else
                    {
                        switch( LoRaMacChannelAdd( channelIndex, chParam ) )
                        {
                            case LORAMAC_STATUS_OK:
                            {
                                break;
                            }
                            case LORAMAC_STATUS_FREQUENCY_INVALID:
                            {
                                status &= 0xFE;
                                break;
                            }
                            case LORAMAC_STATUS_DATARATE_INVALID:
                            {
                                status &= 0xFD;
                                break;
                            }
                            case LORAMAC_STATUS_FREQ_AND_DR_INVALID:
                            {
                                status &= 0xFC;
                                break;
                            }
                            default:
                            {
                                status &= 0xFC;
                                break;
                            }
                        }
                    }
                    LoRaMacState &= ~LORAMAC_TX_CONFIG;
#endif
                    AddMacCommand( MOTE_MAC_NEW_CHANNEL_ANS, status, 0 );
                }
                break;
            case SRV_MAC_RX_TIMING_SETUP_REQ:
                {
                    uint8_t delay = payload[macIndex++] & 0x0F;

                    if( delay == 0 )
                    {
                        delay++;
                    }
                    LoRaMacParams.ReceiveDelay1 = delay * 1e3;
                    LoRaMacParams.ReceiveDelay2 = LoRaMacParams.ReceiveDelay1 + 1e3;
                    AddMacCommand( MOTE_MAC_RX_TIMING_SETUP_ANS, 0, 0 );
                }
                break;
            default:
                // Unknown command. ABORT MAC commands processing
                return;
        }
    }
}

LoRaMacStatus_t Send( LoRaMacHeader_t *macHdr, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    LoRaMacFrameCtrl_t fCtrl;
    LoRaMacStatus_t status = LORAMAC_STATUS_PARAMETER_INVALID;

    fCtrl.Value = 0;
    fCtrl.Bits.FOptsLen      = 0;
    fCtrl.Bits.FPending      = 0;
    fCtrl.Bits.Ack           = false;
    fCtrl.Bits.AdrAckReq     = false;
    fCtrl.Bits.Adr           = AdrCtrlOn;

    // Prepare the frame
    status = PrepareFrame( macHdr, &fCtrl, fPort, fBuffer, fBufferSize );

    // Validate status
    if( status != LORAMAC_STATUS_OK )
    {
        return status;
    }

    // Reset confirm parameters
    McpsConfirm.NbRetries = 0;
    McpsConfirm.AckReceived = false;
    McpsConfirm.UpLinkCounter = UpLinkCounter;
//    lora_printf("status = ScheduleTx( );\n");
    status = ScheduleTx( );

    return status;
}

static LoRaMacStatus_t ScheduleTx( void )
{
    TimerTime_t dutyCycleTimeOff = 0;

    // Check if the device is off
    if( MaxDCycle == 255 )
    {
        return LORAMAC_STATUS_DEVICE_OFF;
    }
    if( MaxDCycle == 0 )
    {
        AggregatedTimeOff = 0;
    }

    // Select channel
    while( SetNextChannel( &dutyCycleTimeOff ) == false )
    {
        // Set the default datarate
        LoRaMacParams.ChannelsDatarate = LoRaMacParamsDefaults.ChannelsDatarate;

#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
        // Re-enable default channels LC1, LC2, LC3
        LoRaMacParams.ChannelsMask[0] = LoRaMacParams.ChannelsMask[0] | ( LC( 1 ) + LC( 2 ) + LC( 3 ) );
#endif
    }

    // Compute Rx1 windows parameters
#if ( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) )
    RxWindowsParams[0] = ComputeRxWindowParameters( DatarateOffsets[LoRaMacParams.ChannelsDatarate][LoRaMacParams.Rx1DrOffset], LoRaMacParams.SystemMaxRxError );
#else
    RxWindowsParams[0] = ComputeRxWindowParameters( MAX( DR_0, LoRaMacParams.ChannelsDatarate - LoRaMacParams.Rx1DrOffset ), LoRaMacParams.SystemMaxRxError );
#endif
    // Compute Rx2 windows parameters
    RxWindowsParams[1] = ComputeRxWindowParameters( LoRaMacParams.Rx2Channel.Datarate, LoRaMacParams.SystemMaxRxError );

    if( IsLoRaMacNetworkJoined == false )
    {
        RxWindow1Delay = LoRaMacParams.JoinAcceptDelay1 + RxWindowsParams[0].RxOffset;
        RxWindow2Delay = LoRaMacParams.JoinAcceptDelay2 + RxWindowsParams[1].RxOffset;
    }
    else
    {
        if( ValidatePayloadLength( LoRaMacTxPayloadLen, LoRaMacParams.ChannelsDatarate, MacCommandsBufferIndex ) == false )
        {
            return LORAMAC_STATUS_LENGTH_ERROR;
        }
        RxWindow1Delay = LoRaMacParams.ReceiveDelay1 + RxWindowsParams[0].RxOffset;
        RxWindow2Delay = LoRaMacParams.ReceiveDelay2 + RxWindowsParams[1].RxOffset;
    }
    // Schedule transmission of frame
    if( dutyCycleTimeOff == 0 )
    {
        // Try to send now
        return SendFrameOnChannel( Channels[Channel] );
    }
    else
    {
        // Send later - prepare timer
        LoRaMacState |= LORAMAC_TX_DELAYED;
        TimerSetValue( &TxDelayedTimer, dutyCycleTimeOff );
        TimerStart( &TxDelayedTimer );

        return LORAMAC_STATUS_OK;
    }
}

static uint16_t JoinDutyCycle( void )
{
    uint16_t dutyCycle = 0;
    TimerTime_t timeElapsed = TimerGetElapsedTime( LoRaMacInitializationTime );

    if( timeElapsed < 3600e3 )
    {
        dutyCycle = BACKOFF_DC_1_HOUR;
    }
    else if( timeElapsed < ( 3600e3 + 36000e3 ) )
    {
        dutyCycle = BACKOFF_DC_10_HOURS;
    }
    else
    {
        dutyCycle = BACKOFF_DC_24_HOURS;
    }
    return dutyCycle;
}

static void CalculateBackOff( uint8_t channel )
{
    uint16_t dutyCycle = Bands[Channels[channel].Band].DCycle;
    uint16_t joinDutyCycle = 0;

    // Reset time-off to initial value.
    Bands[Channels[channel].Band].TimeOff = 0;

    if( IsLoRaMacNetworkJoined == false )
    {
        // The node has not joined yet. Apply join duty cycle to all regions.
        joinDutyCycle = JoinDutyCycle( );
        dutyCycle = MAX( dutyCycle, joinDutyCycle );

        // Update Band time-off.
        Bands[Channels[channel].Band].TimeOff = TxTimeOnAir * dutyCycle - TxTimeOnAir;
    }
    else
    {
        if( DutyCycleOn == true )
        {
            Bands[Channels[channel].Band].TimeOff = TxTimeOnAir * dutyCycle - TxTimeOnAir;
        }
    }

    // Update Aggregated Time OFF
    AggregatedTimeOff = AggregatedTimeOff + ( TxTimeOnAir * AggregatedDCycle - TxTimeOnAir );
}

static int8_t AlternateDatarate( uint16_t nbTrials )
{
    int8_t datarate = LORAMAC_TX_MIN_DATARATE;
#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
#if defined( USE_BAND_915 )
    // Re-enable 500 kHz default channels
    LoRaMacParams.ChannelsMask[4] = 0x00FF;
#else // defined( USE_BAND_915_HYBRID )
    // Re-enable 500 kHz default channels
    ReenableChannels( LoRaMacParamsDefaults.ChannelsMask[4], LoRaMacParams.ChannelsMask );
#endif

    if( ( nbTrials & 0x01 ) == 0x01 )
    {
        datarate = DR_4;
    }
    else
    {
        datarate = DR_0;
    }
#else
    if( ( nbTrials % 48 ) == 0 )
    {
        datarate = DR_0;
    }
    else if( ( nbTrials % 32 ) == 0 )
    {
        datarate = DR_1;
    }
    else if( ( nbTrials % 24 ) == 0 )
    {
        datarate = DR_2;
    }
    else if( ( nbTrials % 16 ) == 0 )
    {
        datarate = DR_3;
    }
    else if( ( nbTrials % 8 ) == 0 )
    {
        datarate = DR_4;
    }
    else
    {
        datarate = DR_5;
    }
#endif
    return datarate;
}

static void ResetMacParameters( void )
{
    IsLoRaMacNetworkJoined = false;

    // Counters
    UpLinkCounter = 0;
    DownLinkCounter = 0;
    AdrAckCounter = 0;

    ChannelsNbRepCounter = 0;

    AckTimeoutRetries = 1;
    AckTimeoutRetriesCounter = 1;
    AckTimeoutRetry = false;

    MaxDCycle = 0;
    AggregatedDCycle = 1;

    MacCommandsBufferIndex = 0;
    MacCommandsBufferToRepeatIndex = 0;

    IsRxWindowsEnabled = true;

    LoRaMacParams.ChannelsTxPower = LoRaMacParamsDefaults.ChannelsTxPower;
    LoRaMacParams.ChannelsDatarate = LoRaMacParamsDefaults.ChannelsDatarate;

    LoRaMacParams.Rx1DrOffset = LoRaMacParamsDefaults.Rx1DrOffset;
    LoRaMacParams.Rx2Channel = LoRaMacParamsDefaults.Rx2Channel;

    memcpy1( ( uint8_t* ) LoRaMacParams.ChannelsMask, ( uint8_t* ) LoRaMacParamsDefaults.ChannelsMask, sizeof( LoRaMacParams.ChannelsMask ) );

#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    memcpy1( ( uint8_t* ) ChannelsMaskRemaining, ( uint8_t* ) LoRaMacParamsDefaults.ChannelsMask, sizeof( LoRaMacParams.ChannelsMask ) );
#endif


    NodeAckRequested = false;
    SrvAckRequested = false;
    MacCommandsInNextTx = false;

    // Reset Multicast downlink counters
    MulticastParams_t *cur = MulticastChannels;
    while( cur != NULL )
    {
        cur->DownLinkCounter = 0;
        cur = cur->Next;
    }

    // Initialize channel index.
    Channel = LORA_MAX_NB_CHANNELS;
}

LoRaMacStatus_t PrepareFrame( LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    uint16_t i;
    uint8_t pktHeaderLen = 0;
    uint32_t mic = 0;
    const void* payload = fBuffer;
    uint8_t framePort = fPort;

    LoRaMacBufferPktLen = 0;

    NodeAckRequested = false;

    if( fBuffer == NULL )
    {
        fBufferSize = 0;
    }

    LoRaMacTxPayloadLen = fBufferSize;

    LoRaMacBuffer[pktHeaderLen++] = macHdr->Value;

    switch( macHdr->Bits.MType )
    {
        case FRAME_TYPE_JOIN_REQ:
            LoRaMacBufferPktLen = pktHeaderLen;

            memcpyr( LoRaMacBuffer + LoRaMacBufferPktLen, LoRaMacAppEui, 8 );
            LoRaMacBufferPktLen += 8;
            memcpyr( LoRaMacBuffer + LoRaMacBufferPktLen, LoRaMacDevEui, 8 );
            LoRaMacBufferPktLen += 8;

            LoRaMacDevNonce = Radio.Random( );

            LoRaMacBuffer[LoRaMacBufferPktLen++] = LoRaMacDevNonce & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( LoRaMacDevNonce >> 8 ) & 0xFF;

            LoRaMacJoinComputeMic( LoRaMacBuffer, LoRaMacBufferPktLen & 0xFF, LoRaMacAppKey, &mic );

            LoRaMacBuffer[LoRaMacBufferPktLen++] = mic & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 8 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 16 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 24 ) & 0xFF;

            break;
        case FRAME_TYPE_DATA_CONFIRMED_UP:
            NodeAckRequested = true;
            //Intentional fallthrough
        case FRAME_TYPE_DATA_UNCONFIRMED_UP:
            if( IsLoRaMacNetworkJoined == false )
            {
                return LORAMAC_STATUS_NO_NETWORK_JOINED; // No network has been joined yet
            }

            fCtrl->Bits.AdrAckReq = AdrNextDr( fCtrl->Bits.Adr, true, &LoRaMacParams.ChannelsDatarate );

            if( SrvAckRequested == true )
            {
                SrvAckRequested = false;
                fCtrl->Bits.Ack = 1;
            }

            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 8 ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 16 ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 24 ) & 0xFF;

            LoRaMacBuffer[pktHeaderLen++] = fCtrl->Value;

            LoRaMacBuffer[pktHeaderLen++] = UpLinkCounter & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( UpLinkCounter >> 8 ) & 0xFF;

            // Copy the MAC commands which must be re-send into the MAC command buffer
            memcpy1( &MacCommandsBuffer[MacCommandsBufferIndex], MacCommandsBufferToRepeat, MacCommandsBufferToRepeatIndex );
            MacCommandsBufferIndex += MacCommandsBufferToRepeatIndex;

            if( ( payload != NULL ) && ( LoRaMacTxPayloadLen > 0 ) )
            {
                if( ( MacCommandsBufferIndex <= LORA_MAC_COMMAND_MAX_LENGTH ) && ( MacCommandsInNextTx == true ) )
                {
                    fCtrl->Bits.FOptsLen += MacCommandsBufferIndex;

                    // Update FCtrl field with new value of OptionsLength
                    LoRaMacBuffer[0x05] = fCtrl->Value;
                    for( i = 0; i < MacCommandsBufferIndex; i++ )
                    {
                        LoRaMacBuffer[pktHeaderLen++] = MacCommandsBuffer[i];
                    }
                }
            }
            else
            {
                if( ( MacCommandsBufferIndex > 0 ) && ( MacCommandsInNextTx ) )
                {
                    LoRaMacTxPayloadLen = MacCommandsBufferIndex;
                    payload = MacCommandsBuffer;
                    framePort = 0;
                }
            }
            MacCommandsInNextTx = false;
            // Store MAC commands which must be re-send in case the device does not receive a downlink anymore
            MacCommandsBufferToRepeatIndex = ParseMacCommandsToRepeat( MacCommandsBuffer, MacCommandsBufferIndex, MacCommandsBufferToRepeat );
            if( MacCommandsBufferToRepeatIndex > 0 )
            {
                MacCommandsInNextTx = true;
            }

            if( ( payload != NULL ) && ( LoRaMacTxPayloadLen > 0 ) )
            {
                LoRaMacBuffer[pktHeaderLen++] = framePort;

                if( framePort == 0 )
                {
                    LoRaMacPayloadEncrypt( (uint8_t* ) payload, LoRaMacTxPayloadLen, LoRaMacNwkSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, &LoRaMacBuffer[pktHeaderLen] );
                }
                else
                {
                    LoRaMacPayloadEncrypt( (uint8_t* ) payload, LoRaMacTxPayloadLen, LoRaMacAppSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, &LoRaMacBuffer[pktHeaderLen] );
                }
            }
            LoRaMacBufferPktLen = pktHeaderLen + LoRaMacTxPayloadLen;

            LoRaMacComputeMic( LoRaMacBuffer, LoRaMacBufferPktLen, LoRaMacNwkSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, &mic );

            LoRaMacBuffer[LoRaMacBufferPktLen + 0] = mic & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen + 1] = ( mic >> 8 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen + 2] = ( mic >> 16 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen + 3] = ( mic >> 24 ) & 0xFF;

            LoRaMacBufferPktLen += LORAMAC_MFR_LEN;

            break;
        case FRAME_TYPE_PROPRIETARY:
            if( ( fBuffer != NULL ) && ( LoRaMacTxPayloadLen > 0 ) )
            {
                memcpy1( LoRaMacBuffer + pktHeaderLen, ( uint8_t* ) fBuffer, LoRaMacTxPayloadLen );
                LoRaMacBufferPktLen = pktHeaderLen + LoRaMacTxPayloadLen;
            }
            break;
        default:
            return LORAMAC_STATUS_SERVICE_UNKNOWN;
    }

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t SendFrameOnChannel( ChannelParams_t channel )
{
    int8_t datarate = Datarates[LoRaMacParams.ChannelsDatarate];
    int8_t txPowerIndex = 0;
    int8_t txPower = 0;

    txPowerIndex = LimitTxPower( LoRaMacParams.ChannelsTxPower, Bands[channel.Band].TxMaxPower );
    txPower = TxPowers[txPowerIndex];

    MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
    McpsConfirm.Datarate = LoRaMacParams.ChannelsDatarate;
    McpsConfirm.TxPower = txPowerIndex;
    McpsConfirm.UpLinkFrequency = channel.Frequency;
   // lora_printf("UpLinkFrequency %d\n",channel.Frequency);
    Radio.SetChannel( channel.Frequency );

#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
    if( LoRaMacParams.ChannelsDatarate == DR_7 )
    { // High Speed FSK channel
        Radio.SetMaxPayloadLength( MODEM_FSK, LoRaMacBufferPktLen );
        Radio.SetTxConfig( MODEM_FSK, txPower, 25e3, 0, datarate * 1e3, 0, 5, false, true, 0, 0, false, 3e3 );
        TxTimeOnAir = Radio.TimeOnAir( MODEM_FSK, LoRaMacBufferPktLen );

    }
    else if( LoRaMacParams.ChannelsDatarate == DR_6 )
    { // High speed LoRa channel
        Radio.SetMaxPayloadLength( MODEM_LORA, LoRaMacBufferPktLen );
        Radio.SetTxConfig( MODEM_LORA, txPower, 0, 1, datarate, 1, 8, false, true, 0, 0, false, 3e3 );
        TxTimeOnAir = Radio.TimeOnAir( MODEM_LORA, LoRaMacBufferPktLen );
    }
    else
    { // Normal LoRa channel
        Radio.SetMaxPayloadLength( MODEM_LORA, LoRaMacBufferPktLen );
        Radio.SetTxConfig( MODEM_LORA, txPower, 0, 0, datarate, 1, 8, false, true, 0, 0, false, 3e3 );
        TxTimeOnAir = Radio.TimeOnAir( MODEM_LORA, LoRaMacBufferPktLen );
    }
#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    Radio.SetMaxPayloadLength( MODEM_LORA, LoRaMacBufferPktLen );
    if( LoRaMacParams.ChannelsDatarate >= DR_4 )
    { // High speed LoRa channel BW500 kHz
        Radio.SetTxConfig( MODEM_LORA, txPower, 0, 2, datarate, 1, 8, false, true, 0, 0, false, 3e3 );
        TxTimeOnAir = Radio.TimeOnAir( MODEM_LORA, LoRaMacBufferPktLen );
    }
    else
    { // Normal LoRa channel
        Radio.SetTxConfig( MODEM_LORA, txPower, 0, 0, datarate, 1, 8, false, true, 0, 0, false, 3e3 );
        TxTimeOnAir = Radio.TimeOnAir( MODEM_LORA, LoRaMacBufferPktLen );
    }
#elif defined( USE_BAND_470 ) || defined( USE_BAND_470PREQUEL )
    Radio.SetMaxPayloadLength( MODEM_LORA, LoRaMacBufferPktLen );
    Radio.SetTxConfig( MODEM_LORA, txPower, 0, 0, datarate, 1, 8, false, true, 0, 0, false, 3e3 );
    TxTimeOnAir = Radio.TimeOnAir( MODEM_LORA, LoRaMacBufferPktLen );
#else
    #error "Please define a frequency band in the compiler options."
#endif

    // Store the time on air
    McpsConfirm.TxTimeOnAir = TxTimeOnAir;
    MlmeConfirm.TxTimeOnAir = TxTimeOnAir;

    // Starts the MAC layer status check timer
    TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );
    TimerStart( &MacStateCheckTimer );

    if( IsLoRaMacNetworkJoined == false )
    {
        JoinRequestTrials++;
    }

    // Send now
    Radio.Send( LoRaMacBuffer, LoRaMacBufferPktLen );

    LoRaMacState |= LORAMAC_TX_RUNNING;

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t SetTxContinuousWave( uint16_t timeout )
{
    int8_t txPowerIndex = 0;
    int8_t txPower = 0;

    txPowerIndex = LimitTxPower( LoRaMacParams.ChannelsTxPower, Bands[Channels[Channel].Band].TxMaxPower );
    txPower = TxPowers[txPowerIndex];

    // Starts the MAC layer status check timer
    TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );
    TimerStart( &MacStateCheckTimer );

    Radio.SetTxContinuousWave( Channels[Channel].Frequency, txPower, timeout );

    LoRaMacState |= LORAMAC_TX_RUNNING;

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t SetTxContinuousWave1( uint16_t timeout, uint32_t frequency, uint8_t power )
{
    Radio.SetTxContinuousWave( frequency, power, timeout );

    // Starts the MAC layer status check timer
    TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );
    TimerStart( &MacStateCheckTimer );

    LoRaMacState |= LORAMAC_TX_RUNNING;

    return LORAMAC_STATUS_OK;
}


LoRaMacStatus_t LoRaMacInitialization( LoRaMacPrimitives_t *primitives, LoRaMacCallback_t *callbacks )
{


    if( primitives == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    if( ( primitives->MacMcpsConfirm == NULL ) ||
        ( primitives->MacMcpsIndication == NULL ) ||
        ( primitives->MacMlmeConfirm == NULL ) )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    LoRaMacPrimitives = primitives;
    LoRaMacCallbacks = callbacks;

    LoRaMacFlags.Value = 0;// LoRaMac tx/rx operation state

    LoRaMacDeviceClass = CLASS;//
    LoRaMacState = LORAMAC_IDLE;//缂傚倸鍊搁崐鎼佸磹妞嬪海鐭嗗〒姘炬嫹鐎殿喚鏁婚、妤呮晸娴犲锟芥棃宕橀钘夌檮婵犮垹鍘滈弲婊堟儎椤栫偛鏋侀柟鍓х帛閺呮悂鏌ㄩ悤鍌涘

    JoinRequestTrials = 0;// number of trials for the join request
    MaxJoinRequestTrials = 1; // Max number of trials for the join request 
    RepeaterSupport = false; // indicates if the node supports repeaters; 婵犵數濮烽弫鎼佸磻閻愬搫鍨傞柛顐ｆ礀缁犱即鏌涘┑鍕姢闁活厽鎹囬弻锝夊閵忊晝鍔搁梺鍝勵儎閻掞箓濡甸崟顖氬嵆妞ゅ繐鎳庨鑸典繆閵堝洤啸闁稿鍋ゅ畷婵嬪冀椤撶偟顔嗛梺鍛婄☉閻°劑宕戦鍛簻闁哄秲鍔忔竟妯肩磼閻樺啿鐏╃紒杈ㄦ崌瀹曟帒顫濋濠冨濡炲瀛╅浠嬫煏閸繍妲哥紒鐙�鍨堕弻銊╂偆閸屾稑顏�

    // Reset duty cycle times
    AggregatedLastTxDoneTime = 0;
    AggregatedTimeOff = 0;

    // Duty cycle   //enables/disables duty cycle management
#if defined( USE_BAND_433 )
    DutyCycleOn = true; 
#elif defined( USE_BAND_470 ) || defined( USE_BAND_470PREQUEL )
    DutyCycleOn = false;
#elif defined( USE_BAND_780 )
    DutyCycleOn = true;
#elif defined( USE_BAND_868 )
    DutyCycleOn = true;
#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    DutyCycleOn = false;
#else
    #error "Please define a frequency band in the compiler options."
#endif

    // Reset to defaults   //Global MAC layer parameters
    LoRaMacParamsDefaults.ChannelsTxPower = LORAMAC_DEFAULT_TX_POWER;  //tx power 10dBm
    LoRaMacParamsDefaults.ChannelsDatarate = LORAMAC_DEFAULT_DATARATE; //datarate DR_0    SF12  

    LoRaMacParamsDefaults.SystemMaxRxError = 10;//缂傚倸鍊搁崐鎼佸磹妞嬪孩顐介柨鐔哄Т閸ㄥ倿姊婚崼鐔恒�掗柡鍡畵閺岋綁濮�閵堝棙閿梺鎼炲妽缁诲啰鎹㈠☉姗嗗晠妞ゆ棁宕甸惄搴ｇ磽娴ｅ搫校闁搞劌娼￠獮鍐亹閹烘垹鍊炲銈呯箰鐎氼喖袙閵忋倖鈷戦柛婵嗗椤ユ瑩鏌涘Δ浣糕枙鐎殿喖顭烽弫鎾绘偐閼碱剦妲伴梻浣哥枃濡椼劎绮堟担瑙勫弿缂佸顑欏〒濠氭煏閸繃顥炵痪鐐倐閺屾稒绻濋崘鈺冾槹濡炪們鍨洪悷銊х不濞戞ǚ妲堟繛鍡樺姈椤忕喖姊绘担鑺ョ《闁革綇绠撻獮蹇涙晸閿燂拷
    LoRaMacParamsDefaults.MinRxSymbols = 6;     //Minimum required number of symbols to detect an RX frame
    LoRaMacParamsDefaults.MaxRxWindow = MAX_RX_WINDOW; //3s  maximum time a reception window stays open
    LoRaMacParamsDefaults.ReceiveDelay1 = RECEIVE_DELAY1;//1s
    LoRaMacParamsDefaults.ReceiveDelay2 = RECEIVE_DELAY2;//2s
    LoRaMacParamsDefaults.JoinAcceptDelay1 = JOIN_ACCEPT_DELAY1;//5s
    LoRaMacParamsDefaults.JoinAcceptDelay2 = JOIN_ACCEPT_DELAY2;//6s

    LoRaMacParamsDefaults.ChannelsNbRep = 1;// number of uplik messages repetition [1:15] (unconfirmed messages only)
    LoRaMacParamsDefaults.Rx1DrOffset = 0; //Datarate offset between uplink and downlink on first window

    LoRaMacParamsDefaults.Rx2Channel = ( Rx2ChannelParams_t )RX_WND_2_CHANNEL;//LoRaMAC 2nd reception window settings

    // Channel mask
#if defined( USE_BAND_433 )
    LoRaMacParamsDefaults.ChannelsMask[0] = LC( 1 ) + LC( 2 ) + LC( 3 );//0X0007
#elif defined( USE_BAND_470 ) || defined ( USE_BAND_470PREQUEL )
//婵犵數濮烽弫鎼佸磻閻樿绠垫い蹇撴缁躲倝鏌﹀Ο鐚寸礆婵炴垶菤閺嬪海锟界敻鍋婇崑鎾剁礊娓氾拷閻涱噣寮介妸锕�顎撻梺鍛婄缚閸庢壆妲愬鍫熲拻濞达絿鎳撻婊勬叏婵犲拋鍤欓柍璇茬Ч婵拷闁靛牆鎳愰鍡涙⒑缂佹ê鐏卞┑顔哄�濆鏌ュ箹娴ｅ湱鍙嗛梺缁樻礀閸婂湱锟芥熬鎷�1301/1308缂傚倸鍊搁崐鎼佸磹閹间礁纾归柟闂寸閻ゎ喗銇勯弽顐粶闁搞劌鍊块弻锝夊閵忊晝鍔搁梺娲诲幗椤ㄥ牓骞夐幖浣哥睄闁割偅绻勯悿鍥⒑缂佹ê濮冮悹锟介敃鍌氭瀬鐎广儱妫涚弧锟芥繝鐢靛Т閸婃悂寮搁妶澶嬬厸闁糕檧鏅涙晶瀛樻叏婵犲啯銇濈�规洏鍔嶇换婵嬪礋椤掞拷濞呇冣攽閻愯尙鎽犵紒顔肩Ф閹广垽宕掗悙瀛樻珖闂侀潧绻堥崐鏍疾閹间焦鐓ラ柣鏇炲�圭�氾拷8婵犵數濮烽弫鎼佸磻閻愬搫鍨傞柛顐ｆ礀缁犲綊鏌嶉崫鍕拷濠氥�呴崣澶岀闁糕剝锚閻忊晝绱掗悩铏棃闁哄本绋撴禒锕傚礈瑜忛悡渚�姊虹粙娆惧剱闁挎洏鍨介獮鍐潨閿熶粙骞冨▎鎾村�绘慨妤�鐗忛崥鍦磽閸屾瑦绁版い鏇嗗吘娑樷攽閸♀晜缍庡┑鐐叉▕娴滃爼寮崱妞曞綊鏁愰崼顐ｇ秷闂佺瀛╅幐鍓ф閹惧瓨濯村ù鐘差儏閹界數绱撴担绛嬪殭闁绘鎹囬幃浼搭敊绾板崬鎮戞繝銏ｆ硾椤戝棗鈻嶉姀銈嗏拺闁稿繘妫块懜顏堟煕鎼淬垹鈻曢柕鍡楁嚇閹粓鎳為妷褍甯惧┑鐘灱濞夋稑螞濞嗗繆鏋嶉柛鈩冾樅瑜版帗鍋愮紒娑氼攰閿熻棄娼￠弻鐔碱敊閹冨箣閻庢鍠楀娆擄綖濠靛鏁囬柣姗嗗枛娴犳帒鈹戦敍鍕杭闁稿﹥鐗曢～蹇涙偡閹冲﹤鎼…銊╁川椤曞懏缍楅梻浣告惈閸燁偊鎮ф繝鍥х；闁靛鍎弨浠嬫煟濞嗘瑦瀚瑰┑顕嗙稻閸旀瑥顫忕紒妯诲闁革富鍘介懣鍥⒑閸涘﹥鐓ラ柣顒�銈搁弫鎾绘嚍閵夈儱姣堟繛瀛樼矋缁诲嫮鍒掑顓熺秶闁靛ě鍛闂備焦鎮堕崕顖炲礉鎼淬劌绀夐柛顐ｆ礃閳锋垿鏌涢敂璇插箹濞存粎鍋撶换娑㈠礂閼测晜鍣梺鐟板槻缂嶅﹪骞冮姀銈呯闁兼祴鏅涢獮妤佷繆閻愵亜锟芥牕螞娴ｅ摜鏆﹂柣銏㈡暩妞规娊鐓崶銊︾缁炬儳銈稿鍫曞醇濞戞ê顬嬬紓浣哄Х閸嬬喖鏁撻崐鐕佹綈婵炴祴鏅濈槐鐐寸瑹閿熶粙濡存笟锟藉顕�宕煎┑鍫Ч婵＄偑鍊栭幐鐐垔鐎靛憡顫曢梺顒�绉甸埛鎺懨归敐鍛暈闁哥喓鍋ら弻娑㈠箣濠靛牏楔閻庤娲栧﹢閬嶅箲閸曨垰绠掗柟铏瑰仦鐎氬綊鏌熼悜姗嗘當缂侊拷閸緷褰掓晲閸ャ劌娈岄梺鍝勬－閸嬪懐鎹㈠☉娆愮秶闁告挆鍕还闂備胶顭堢花娲磹濠靛绠栧Δ锝呭暙绾拷闂佺粯鍔曢崯顐︽煀閿濆懐鏆﹂柤鍛婎問濡插綊骞栨潏鍓ф偧婵℃彃娲ら埞鎴︽倷閼碱剚鎲肩紓渚囧櫙缂嶄線鐛箛娑欐櫢闁跨噦鎷�
    LoRaMacParamsDefaults.ChannelsMask[0] = 0x00FF;//bit 15~0,闂傚倸鍊峰ù鍥х暦閻㈢纾婚柣鎰暩閻瑩鐓崶銊р槈缂侊拷婢舵劕绠规繛锝庡墮婵＄厧顩奸崨顓涙斀妞ゆ梻鐡旈悞鐐箾婢跺銆掗柨鐔诲Г閼归箖藝娴兼潙桅闁告洦鍨扮粻锝夋煥濠靛棙顥犻柕鍡楋攻缁绘盯寮堕幋婵囧�梺鍛婃⒐閻熲晠鐛崘銊庢棃鏁撴禒瀣叀濠㈣泛艌閺嬪酣鐓崶銊︽儎婵炲吋鎹囬弻锝夋偄閸濄儳鐓佹繝纰樺墲閹倸鐣烽悢纰辨晢闁跨喍绮欓、鎾诲箻缂佹鍘介梺缁樏崢鏍嚋椤忓牊鐓曢柡鍌濇硶鏁堥悗娈垮枟婵炲﹪寮幘缁樻櫢闁跨噦鎷�0~7婵犵數濮烽弫鎼佸磿閹寸姴绶ら柦妯侯棦瑜版帒绀嬫い鎾跺枎鎼村﹤鈹戦悩缁樻锭婵炲眰鍔戦妴鍛村蓟閵夛妇鍙嗛梺缁樻礀閸婂湱锟芥熬鎷�
    LoRaMacParamsDefaults.ChannelsMask[1] = 0x0000;
    LoRaMacParamsDefaults.ChannelsMask[2] = 0x0000;
    LoRaMacParamsDefaults.ChannelsMask[3] = 0x0000;
    LoRaMacParamsDefaults.ChannelsMask[4] = 0x0000;
    LoRaMacParamsDefaults.ChannelsMask[5] = 0x0000;//ast c
#elif defined( USE_BAND_780 )
    LoRaMacParamsDefaults.ChannelsMask[0] = LC( 1 ) + LC( 2 ) + LC( 3 );
#elif defined( USE_BAND_868 )
    LoRaMacParamsDefaults.ChannelsMask[0] = LC( 1 ) + LC( 2 ) + LC( 3 );
#elif defined( USE_BAND_915 )
    LoRaMacParamsDefaults.ChannelsMask[0] = 0xFFFF;
    LoRaMacParamsDefaults.ChannelsMask[1] = 0xFFFF;
    LoRaMacParamsDefaults.ChannelsMask[2] = 0xFFFF;
    LoRaMacParamsDefaults.ChannelsMask[3] = 0xFFFF;
    LoRaMacParamsDefaults.ChannelsMask[4] = 0x00FF;
    LoRaMacParamsDefaults.ChannelsMask[5] = 0x0000;
#elif defined( USE_BAND_915_HYBRID )
    LoRaMacParamsDefaults.ChannelsMask[0] = 0x00FF;
    LoRaMacParamsDefaults.ChannelsMask[1] = 0x0000;
    LoRaMacParamsDefaults.ChannelsMask[2] = 0x0000;
    LoRaMacParamsDefaults.ChannelsMask[3] = 0x0000;
    LoRaMacParamsDefaults.ChannelsMask[4] = 0x0001;
    LoRaMacParamsDefaults.ChannelsMask[5] = 0x0000;
#else
    #error "Please define a frequency band in the compiler options."
#endif

#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    // 125 kHz channels
    for( uint8_t i = 0; i < LORA_MAX_NB_CHANNELS - 8; i++ )
    {
        Channels[i].Frequency = 902.3e6 + i * 200e3;
        Channels[i].DrRange.Value = ( DR_3 << 4 ) | DR_0;
        Channels[i].Band = 0;
    }
    // 500 kHz channels
    for( uint8_t i = LORA_MAX_NB_CHANNELS - 8; i < LORA_MAX_NB_CHANNELS; i++ )
    {
        Channels[i].Frequency = 903.0e6 + ( i - ( LORA_MAX_NB_CHANNELS - 8 ) ) * 1.6e6;
        Channels[i].DrRange.Value = ( DR_4 << 4 ) | DR_4;
        Channels[i].Band = 0;
    }
#elif defined( USE_BAND_470 ) || defined( USE_BAND_470PREQUEL )
    // 125 kHz channels
    for( uint8_t i = 0; i < LORA_MAX_NB_CHANNELS; i++ )
    {
#if defined( USE_BAND_470PREQUEL )
      Channels[i].Frequency = 471.5e6 + i * 200e3; //ast c
#elif defined( USE_BAND_470 )
      Channels[i].Frequency = 470.3e6 + i * 200e3; //ast c
#else 
  #error "please define USE_BAND_470 OR USE_BAND_470PREQUEL."
#endif  
        Channels[i].DrRange.Value = ( DR_5 << 4 ) | DR_0;
        Channels[i].Band = 0;
    }
#endif

    // Init parameters which are not set in function ResetMacParameters
    LoRaMacParams.SystemMaxRxError = LoRaMacParamsDefaults.SystemMaxRxError;
    LoRaMacParams.MinRxSymbols = LoRaMacParamsDefaults.MinRxSymbols;
    LoRaMacParams.MaxRxWindow = LoRaMacParamsDefaults.MaxRxWindow;
    LoRaMacParams.ReceiveDelay1 = LoRaMacParamsDefaults.ReceiveDelay1;
    LoRaMacParams.ReceiveDelay2 = LoRaMacParamsDefaults.ReceiveDelay2;
    LoRaMacParams.JoinAcceptDelay1 = LoRaMacParamsDefaults.JoinAcceptDelay1;
    LoRaMacParams.JoinAcceptDelay2 = LoRaMacParamsDefaults.JoinAcceptDelay2;
    LoRaMacParams.ChannelsNbRep = LoRaMacParamsDefaults.ChannelsNbRep;

	if(IsLoRaMacNetworkJoined==false)
	{
		ResetMacParameters( );
//		lora_printf("ResetMacParameters\n");
	}


    // Initialize timers
    TimerInit( &MacStateCheckTimer, OnMacStateCheckTimerEvent );
    TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );

    TimerInit( &TxDelayedTimer, OnTxDelayedTimerEvent );
    TimerInit( &RxWindowTimer1, OnRxWindow1TimerEvent );
    TimerInit( &RxWindowTimer2, OnRxWindow2TimerEvent );
    TimerInit( &AckTimeoutTimer, OnAckTimeoutTimerEvent );

    // Store the current initialization time
    LoRaMacInitializationTime = TimerGetCurrentTime( );

    // Initialize Radio driver
    RadioEvents.TxDone = OnRadioTxDone;
    RadioEvents.RxDone = OnRadioRxDone;
    RadioEvents.RxError = OnRadioRxError;//CRC闂傚倸鍊搁崐鎼佸磹閻戣姤鍊块柨鏃�鎷濋幏椋庣箔濞戞ɑ澶勯柛瀣剁節閺屾洘绻涢悙顒佺彟闂佽桨绀侀澶愬蓟閵娾晜鍋嗛柛灞剧☉椤忥拷
    RadioEvents.TxTimeout = OnRadioTxTimeout;
    RadioEvents.RxTimeout = OnRadioRxTimeout;
    Radio.Init( &RadioEvents );

    // Random seed initialization
    srand1( Radio.Random( ) );

    PublicNetwork = true;
    Radio.SetPublicNetwork( PublicNetwork );
    Radio.Sleep( );

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacQueryTxPossible( uint8_t size, LoRaMacTxInfo_t* txInfo )
{
    int8_t datarate = LoRaMacParamsDefaults.ChannelsDatarate;
    uint8_t fOptLen = MacCommandsBufferIndex + MacCommandsBufferToRepeatIndex;

    if( txInfo == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    AdrNextDr( AdrCtrlOn, false, &datarate );

    if( RepeaterSupport == true )
    {
        txInfo->CurrentPayloadSize = MaxPayloadOfDatarateRepeater[datarate];
    }
    else
    {
        txInfo->CurrentPayloadSize = MaxPayloadOfDatarate[datarate];
    }

    if( txInfo->CurrentPayloadSize >= fOptLen )
    {
        txInfo->MaxPossiblePayload = txInfo->CurrentPayloadSize - fOptLen;
    }
    else
    {
        return LORAMAC_STATUS_MAC_CMD_LENGTH_ERROR;
    }

    if( ValidatePayloadLength( size, datarate, 0 ) == false )
    {
        return LORAMAC_STATUS_LENGTH_ERROR;
    }

    if( ValidatePayloadLength( size, datarate, fOptLen ) == false )
    {
        return LORAMAC_STATUS_MAC_CMD_LENGTH_ERROR;
    }

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacMibGetRequestConfirm( MibRequestConfirm_t *mibGet )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_OK;

    if( mibGet == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    switch( mibGet->Type )
    {
        case MIB_DEVICE_CLASS:
        {
            mibGet->Param.Class = LoRaMacDeviceClass;
            break;
        }
        case MIB_NETWORK_JOINED:
        {
            mibGet->Param.IsNetworkJoined = IsLoRaMacNetworkJoined;
            break;
        }
        case MIB_ADR:
        {
            mibGet->Param.AdrEnable = AdrCtrlOn;
            break;
        }
        case MIB_NET_ID:
        {
            mibGet->Param.NetID = LoRaMacNetID;
            break;
        }
        case MIB_DEV_ADDR:
        {
            mibGet->Param.DevAddr = LoRaMacDevAddr;
            break;
        }
        case MIB_NWK_SKEY:
        {
            mibGet->Param.NwkSKey = LoRaMacNwkSKey;
            break;
        }
        case MIB_APP_SKEY:
        {
            mibGet->Param.AppSKey = LoRaMacAppSKey;
            break;
        }
        case MIB_PUBLIC_NETWORK:
        {
            mibGet->Param.EnablePublicNetwork = PublicNetwork;
            break;
        }
        case MIB_REPEATER_SUPPORT:
        {
            mibGet->Param.EnableRepeaterSupport = RepeaterSupport;
            break;
        }
        case MIB_CHANNELS:
        {
            mibGet->Param.ChannelList = Channels;
            break;
        }
        case MIB_RX2_CHANNEL:
        {
            mibGet->Param.Rx2Channel = LoRaMacParams.Rx2Channel;
            break;
        }
        case MIB_RX2_DEFAULT_CHANNEL:
        {
            mibGet->Param.Rx2Channel = LoRaMacParamsDefaults.Rx2Channel;
            break;
        }
        case MIB_CHANNELS_DEFAULT_MASK:
        {
            mibGet->Param.ChannelsDefaultMask = LoRaMacParamsDefaults.ChannelsMask;
            break;
        }
        case MIB_CHANNELS_MASK:
        {
            mibGet->Param.ChannelsMask = LoRaMacParams.ChannelsMask;
            break;
        }
        case MIB_CHANNELS_NB_REP:
        {
            mibGet->Param.ChannelNbRep = LoRaMacParams.ChannelsNbRep;
            break;
        }
        case MIB_MAX_RX_WINDOW_DURATION:
        {
            mibGet->Param.MaxRxWindow = LoRaMacParams.MaxRxWindow;
            break;
        }
        case MIB_RECEIVE_DELAY_1:
        {
            mibGet->Param.ReceiveDelay1 = LoRaMacParams.ReceiveDelay1;
            break;
        }
        case MIB_RECEIVE_DELAY_2:
        {
            mibGet->Param.ReceiveDelay2 = LoRaMacParams.ReceiveDelay2;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_1:
        {
            mibGet->Param.JoinAcceptDelay1 = LoRaMacParams.JoinAcceptDelay1;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_2:
        {
            mibGet->Param.JoinAcceptDelay2 = LoRaMacParams.JoinAcceptDelay2;
            break;
        }
        case MIB_CHANNELS_DEFAULT_DATARATE:
        {
            mibGet->Param.ChannelsDefaultDatarate = LoRaMacParamsDefaults.ChannelsDatarate;
            break;
        }
        case MIB_CHANNELS_DATARATE:
        {
            mibGet->Param.ChannelsDatarate = LoRaMacParams.ChannelsDatarate;
            break;
        }
        case MIB_CHANNELS_DEFAULT_TX_POWER:
        {
            mibGet->Param.ChannelsDefaultTxPower = LoRaMacParamsDefaults.ChannelsTxPower;
            break;
        }
        case MIB_CHANNELS_TX_POWER:
        {
            mibGet->Param.ChannelsTxPower = LoRaMacParams.ChannelsTxPower;
            break;
        }
        case MIB_UPLINK_COUNTER:
        {
            mibGet->Param.UpLinkCounter = UpLinkCounter;
            break;
        }
        case MIB_DOWNLINK_COUNTER:
        {
            mibGet->Param.DownLinkCounter = DownLinkCounter;
            break;
        }
        case MIB_MULTICAST_CHANNEL:
        {
            mibGet->Param.MulticastList = MulticastChannels;
            break;
        }
        case MIB_SYSTEM_MAX_RX_ERROR:
        {
            mibGet->Param.SystemMaxRxError = LoRaMacParams.SystemMaxRxError;
            break;
        }
        case MIB_MIN_RX_SYMBOLS:
        {
            mibGet->Param.MinRxSymbols = LoRaMacParams.MinRxSymbols;
            break;
        }
        default:
            status = LORAMAC_STATUS_SERVICE_UNKNOWN;
            break;
    }

    return status;
}

LoRaMacStatus_t LoRaMacMibSetRequestConfirm( MibRequestConfirm_t *mibSet )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_OK;

    if( mibSet == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( LoRaMacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING )
    {
        return LORAMAC_STATUS_BUSY;
    }

    switch( mibSet->Type )
    {
        case MIB_DEVICE_CLASS:
        {
            LoRaMacDeviceClass = mibSet->Param.Class;
            switch( LoRaMacDeviceClass )
            {
                case CLASS_A:
                {
                    // Set the radio into sleep to setup a defined state
                    Radio.Sleep( );
                    break;
                }
                case CLASS_B:
                {
                    break;
                }
                case CLASS_C:
                {
                    // Set the NodeAckRequested indicator to default
                    NodeAckRequested = false;
                    OnRxWindow2TimerEvent( );
                    break;
                }
            }
            break;
        }
        case MIB_NETWORK_JOINED:
        {
            IsLoRaMacNetworkJoined = mibSet->Param.IsNetworkJoined;
            break;
        }
        case MIB_ADR:
        {
            AdrCtrlOn = mibSet->Param.AdrEnable;
            break;
        }
        case MIB_NET_ID:
        {
            LoRaMacNetID = mibSet->Param.NetID;
            break;
        }
        case MIB_DEV_ADDR:
        {
            LoRaMacDevAddr = mibSet->Param.DevAddr;
            break;
        }
        case MIB_NWK_SKEY:
        {
            if( mibSet->Param.NwkSKey != NULL )
            {
                memcpy1( LoRaMacNwkSKey, mibSet->Param.NwkSKey,
                               sizeof( LoRaMacNwkSKey ) );
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_APP_SKEY:
        {
            if( mibSet->Param.AppSKey != NULL )
            {
                memcpy1( LoRaMacAppSKey, mibSet->Param.AppSKey,
                               sizeof( LoRaMacAppSKey ) );
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_PUBLIC_NETWORK:
        {
            PublicNetwork = mibSet->Param.EnablePublicNetwork;
            Radio.SetPublicNetwork( PublicNetwork );
            break;
        }
        case MIB_REPEATER_SUPPORT:
        {
             RepeaterSupport = mibSet->Param.EnableRepeaterSupport;
            break;
        }
        case MIB_RX2_CHANNEL:
        {
            LoRaMacParams.Rx2Channel = mibSet->Param.Rx2Channel;
            break;
        }
        case MIB_RX2_DEFAULT_CHANNEL:
        {
            LoRaMacParamsDefaults.Rx2Channel = mibSet->Param.Rx2DefaultChannel;
            break;
        }
        case MIB_CHANNELS_DEFAULT_MASK:
        {
            if( mibSet->Param.ChannelsDefaultMask )
            {
#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
                bool chanMaskState = true;

#if defined( USE_BAND_915_HYBRID )
                chanMaskState = ValidateChannelMask( mibSet->Param.ChannelsDefaultMask );
#endif
                if( chanMaskState == true )
                {
                    if( ( CountNbEnabled125kHzChannels( mibSet->Param.ChannelsMask ) < 2 ) &&
                        ( CountNbEnabled125kHzChannels( mibSet->Param.ChannelsMask ) > 0 ) )
                    {
                        status = LORAMAC_STATUS_PARAMETER_INVALID;
                    }
                    else
                    {
                        memcpy1( ( uint8_t* ) LoRaMacParamsDefaults.ChannelsMask,
                                 ( uint8_t* ) mibSet->Param.ChannelsDefaultMask, sizeof( LoRaMacParamsDefaults.ChannelsMask ) );
                        for ( uint8_t i = 0; i < sizeof( LoRaMacParamsDefaults.ChannelsMask ) / 2; i++ )
                        {
                            // Disable channels which are no longer available
                            ChannelsMaskRemaining[i] &= LoRaMacParamsDefaults.ChannelsMask[i];
                        }
                    }
                }
                else
                {
                    status = LORAMAC_STATUS_PARAMETER_INVALID;
                }
#elif defined( USE_BAND_470 ) || defined( USE_BAND_470PREQUEL )
                memcpy1( ( uint8_t* ) LoRaMacParamsDefaults.ChannelsMask,
                         ( uint8_t* ) mibSet->Param.ChannelsDefaultMask, sizeof( LoRaMacParamsDefaults.ChannelsMask ) );
#else
                memcpy1( ( uint8_t* ) LoRaMacParamsDefaults.ChannelsMask,
                         ( uint8_t* ) mibSet->Param.ChannelsDefaultMask, 2 );
#endif
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_MASK:
        {
            if( mibSet->Param.ChannelsMask )
            {
#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
                bool chanMaskState = true;

#if defined( USE_BAND_915_HYBRID )
                chanMaskState = ValidateChannelMask( mibSet->Param.ChannelsMask );
#endif
                if( chanMaskState == true )
                {
                    if( ( CountNbEnabled125kHzChannels( mibSet->Param.ChannelsMask ) < 2 ) &&
                        ( CountNbEnabled125kHzChannels( mibSet->Param.ChannelsMask ) > 0 ) )
                    {
                        status = LORAMAC_STATUS_PARAMETER_INVALID;
                    }
                    else
                    {
                        memcpy1( ( uint8_t* ) LoRaMacParams.ChannelsMask,
                                 ( uint8_t* ) mibSet->Param.ChannelsMask, sizeof( LoRaMacParams.ChannelsMask ) );
                        for ( uint8_t i = 0; i < sizeof( LoRaMacParams.ChannelsMask ) / 2; i++ )
                        {
                            // Disable channels which are no longer available
                            ChannelsMaskRemaining[i] &= LoRaMacParams.ChannelsMask[i];
                        }
                    }
                }
                else
                {
                    status = LORAMAC_STATUS_PARAMETER_INVALID;
                }
#elif defined( USE_BAND_470 ) || defined( USE_BAND_470PREQUEL )
                memcpy1( ( uint8_t* ) LoRaMacParams.ChannelsMask,
                         ( uint8_t* ) mibSet->Param.ChannelsMask, sizeof( LoRaMacParams.ChannelsMask ) );
#else
                memcpy1( ( uint8_t* ) LoRaMacParams.ChannelsMask,
                         ( uint8_t* ) mibSet->Param.ChannelsMask, 2 );
#endif
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_NB_REP:
        {
            if( ( mibSet->Param.ChannelNbRep >= 1 ) &&
                ( mibSet->Param.ChannelNbRep <= 15 ) )
            {
                LoRaMacParams.ChannelsNbRep = mibSet->Param.ChannelNbRep;
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_MAX_RX_WINDOW_DURATION:
        {
            LoRaMacParams.MaxRxWindow = mibSet->Param.MaxRxWindow;
            break;
        }
        case MIB_RECEIVE_DELAY_1:
        {
            LoRaMacParams.ReceiveDelay1 = mibSet->Param.ReceiveDelay1;
            break;
        }
        case MIB_RECEIVE_DELAY_2:
        {
            LoRaMacParams.ReceiveDelay2 = mibSet->Param.ReceiveDelay2;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_1:
        {
            LoRaMacParams.JoinAcceptDelay1 = mibSet->Param.JoinAcceptDelay1;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_2:
        {
            LoRaMacParams.JoinAcceptDelay2 = mibSet->Param.JoinAcceptDelay2;
            break;
        }
        case MIB_CHANNELS_DEFAULT_DATARATE:
        {
#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
            if( ValueInRange( mibSet->Param.ChannelsDefaultDatarate,
                              DR_0, DR_5 ) )
            {
                LoRaMacParamsDefaults.ChannelsDatarate = mibSet->Param.ChannelsDefaultDatarate;
            }
#else
            if( ValueInRange( mibSet->Param.ChannelsDefaultDatarate,
                              LORAMAC_TX_MIN_DATARATE, LORAMAC_TX_MAX_DATARATE ) )
            {
                LoRaMacParamsDefaults.ChannelsDatarate = mibSet->Param.ChannelsDefaultDatarate;
            }
#endif
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_DATARATE:
        {
            if( ValueInRange( mibSet->Param.ChannelsDatarate,
                              LORAMAC_TX_MIN_DATARATE, LORAMAC_TX_MAX_DATARATE ) )
            {
                LoRaMacParams.ChannelsDatarate = mibSet->Param.ChannelsDatarate;
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_DEFAULT_TX_POWER:
        {
            if( ValueInRange( mibSet->Param.ChannelsDefaultTxPower,
                              LORAMAC_MAX_TX_POWER, LORAMAC_MIN_TX_POWER ) )
            {
                LoRaMacParamsDefaults.ChannelsTxPower = mibSet->Param.ChannelsDefaultTxPower;
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_TX_POWER:
        {
            if( ValueInRange( mibSet->Param.ChannelsTxPower,
                              LORAMAC_MAX_TX_POWER, LORAMAC_MIN_TX_POWER ) )
            {
                LoRaMacParams.ChannelsTxPower = mibSet->Param.ChannelsTxPower;
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_UPLINK_COUNTER:
        {
            UpLinkCounter = mibSet->Param.UpLinkCounter;
            break;
        }
        case MIB_DOWNLINK_COUNTER:
        {
            DownLinkCounter = mibSet->Param.DownLinkCounter;
            break;
        }
        case MIB_SYSTEM_MAX_RX_ERROR:
        {
            LoRaMacParams.SystemMaxRxError = LoRaMacParamsDefaults.SystemMaxRxError = mibSet->Param.SystemMaxRxError;
            break;
        }
        case MIB_MIN_RX_SYMBOLS:
        {
            LoRaMacParams.MinRxSymbols = LoRaMacParamsDefaults.MinRxSymbols = mibSet->Param.MinRxSymbols;
            break;
        }
        default:
            status = LORAMAC_STATUS_SERVICE_UNKNOWN;
            break;
    }

    return status;
}

LoRaMacStatus_t LoRaMacChannelAdd( uint8_t id, ChannelParams_t params )
{
#if defined( USE_BAND_470 ) || defined( USE_BAND_470PREQUEL ) || defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    return LORAMAC_STATUS_PARAMETER_INVALID;
#else
    bool datarateInvalid = false;
    bool frequencyInvalid = false;
    uint8_t band = 0;

    // The id must not exceed LORA_MAX_NB_CHANNELS
    if( id >= LORA_MAX_NB_CHANNELS )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    // Validate if the MAC is in a correct state
    if( ( LoRaMacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING )
    {
        if( ( LoRaMacState & LORAMAC_TX_CONFIG ) != LORAMAC_TX_CONFIG )
        {
            return LORAMAC_STATUS_BUSY;
        }
    }
    // Validate the datarate
    if( ( params.DrRange.Fields.Min > params.DrRange.Fields.Max ) ||
        ( ValueInRange( params.DrRange.Fields.Min, LORAMAC_TX_MIN_DATARATE,
                        LORAMAC_TX_MAX_DATARATE ) == false ) ||
        ( ValueInRange( params.DrRange.Fields.Max, LORAMAC_TX_MIN_DATARATE,
                        LORAMAC_TX_MAX_DATARATE ) == false ) )
    {
        datarateInvalid = true;
    }

#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
    if( id < 3 )
    {
        if( params.Frequency != Channels[id].Frequency )
        {
            frequencyInvalid = true;
        }

        if( params.DrRange.Fields.Min > DR_0 )
        {
            datarateInvalid = true;
        }
        if( ValueInRange( params.DrRange.Fields.Max, DR_5, LORAMAC_TX_MAX_DATARATE ) == false )
        {
            datarateInvalid = true;
        }
    }
#endif

    // Validate the frequency
    if( ( Radio.CheckRfFrequency( params.Frequency ) == true ) && ( params.Frequency > 0 ) && ( frequencyInvalid == false ) )
    {
#if defined( USE_BAND_868 )
        if( ( params.Frequency >= 863000000 ) && ( params.Frequency < 865000000 ) )
        {
            band = BAND_G1_2;
        }
        else if( ( params.Frequency >= 865000000 ) && ( params.Frequency <= 868000000 ) )
        {
            band = BAND_G1_0;
        }
        else if( ( params.Frequency > 868000000 ) && ( params.Frequency <= 868600000 ) )
        {
            band = BAND_G1_1;
        }
        else if( ( params.Frequency >= 868700000 ) && ( params.Frequency <= 869200000 ) )
        {
            band = BAND_G1_2;
        }
        else if( ( params.Frequency >= 869400000 ) && ( params.Frequency <= 869650000 ) )
        {
            band = BAND_G1_3;
        }
        else if( ( params.Frequency >= 869700000 ) && ( params.Frequency <= 870000000 ) )
        {
            band = BAND_G1_4;
        }
        else
        {
            frequencyInvalid = true;
        }
#endif
    }
    else
    {
        frequencyInvalid = true;
    }

    if( ( datarateInvalid == true ) && ( frequencyInvalid == true ) )
    {
        return LORAMAC_STATUS_FREQ_AND_DR_INVALID;
    }
    if( datarateInvalid == true )
    {
        return LORAMAC_STATUS_DATARATE_INVALID;
    }
    if( frequencyInvalid == true )
    {
        return LORAMAC_STATUS_FREQUENCY_INVALID;
    }

    // Every parameter is valid, activate the channel
    Channels[id] = params;
    Channels[id].Band = band;
    LoRaMacParams.ChannelsMask[0] |= ( 1 << id );

    return LORAMAC_STATUS_OK;
#endif
}

LoRaMacStatus_t LoRaMacChannelRemove( uint8_t id )
{
#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
    if( ( LoRaMacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING )
    {
        if( ( LoRaMacState & LORAMAC_TX_CONFIG ) != LORAMAC_TX_CONFIG )
        {
            return LORAMAC_STATUS_BUSY;
        }
    }

    if( ( id < 3 ) || ( id >= LORA_MAX_NB_CHANNELS ) )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    else
    {
        // Remove the channel from the list of channels
        Channels[id] = ( ChannelParams_t ){ 0, { 0 }, 0 };

        // Disable the channel as it doesn't exist anymore
        if( DisableChannelInMask( id, LoRaMacParams.ChannelsMask ) == false )
        {
            return LORAMAC_STATUS_PARAMETER_INVALID;
        }
    }
    return LORAMAC_STATUS_OK;
#elif ( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) || defined( USE_BAND_470 ) || defined( USE_BAND_470PREQUEL ) )
    return LORAMAC_STATUS_PARAMETER_INVALID;
#endif
}

LoRaMacStatus_t LoRaMacMulticastChannelLink( MulticastParams_t *channelParam )
{
    if( channelParam == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( LoRaMacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING )
    {
        return LORAMAC_STATUS_BUSY;
    }

    // Reset downlink counter
    channelParam->DownLinkCounter = 0;

    if( MulticastChannels == NULL )
    {
        // New node is the fist element
        MulticastChannels = channelParam;
    }
    else
    {
        MulticastParams_t *cur = MulticastChannels;

        // Search the last node in the list
        while( cur->Next != NULL )
        {
            cur = cur->Next;
        }
        // This function always finds the last node
        cur->Next = channelParam;
    }

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacMulticastChannelUnlink( MulticastParams_t *channelParam )
{
    if( channelParam == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( LoRaMacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING )
    {
        return LORAMAC_STATUS_BUSY;
    }

    if( MulticastChannels != NULL )
    {
        if( MulticastChannels == channelParam )
        {
          // First element
          MulticastChannels = channelParam->Next;
        }
        else
        {
            MulticastParams_t *cur = MulticastChannels;

            // Search the node in the list
            while( cur->Next && cur->Next != channelParam )
            {
                cur = cur->Next;
            }
            // If we found the node, remove it
            if( cur->Next )
            {
                cur->Next = channelParam->Next;
            }
        }
        channelParam->Next = NULL;
    }

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacMlmeRequest( MlmeReq_t *mlmeRequest )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_SERVICE_UNKNOWN;
    LoRaMacHeader_t macHdr;

    if( mlmeRequest == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( LoRaMacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING )
    {
        return LORAMAC_STATUS_BUSY;
    }

    memset1( ( uint8_t* ) &MlmeConfirm, 0, sizeof( MlmeConfirm ) );

    MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;

    switch( mlmeRequest->Type )
    {
        case MLME_JOIN:
        {
            if( ( LoRaMacState & LORAMAC_TX_DELAYED ) == LORAMAC_TX_DELAYED )
            {
                return LORAMAC_STATUS_BUSY;
            }

            if( ( mlmeRequest->Req.Join.DevEui == NULL ) ||
                ( mlmeRequest->Req.Join.AppEui == NULL ) ||
                ( mlmeRequest->Req.Join.AppKey == NULL ) ||
                ( mlmeRequest->Req.Join.NbTrials == 0 ) )
            {
                return LORAMAC_STATUS_PARAMETER_INVALID;
            }

#if ( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) )
            // Enables at least the usage of the 2 datarates.
            if( mlmeRequest->Req.Join.NbTrials < 2 )
            {
                mlmeRequest->Req.Join.NbTrials = 2;
            }
#else
            // Enables at least the usage of all datarates.
            if( mlmeRequest->Req.Join.NbTrials < 48 )
            {
                mlmeRequest->Req.Join.NbTrials = 48;
            }
#endif

            LoRaMacFlags.Bits.MlmeReq = 1;
            MlmeConfirm.MlmeRequest = mlmeRequest->Type;

            LoRaMacDevEui = mlmeRequest->Req.Join.DevEui;
            LoRaMacAppEui = mlmeRequest->Req.Join.AppEui;
            LoRaMacAppKey = mlmeRequest->Req.Join.AppKey;
            MaxJoinRequestTrials = mlmeRequest->Req.Join.NbTrials;

            // Reset variable JoinRequestTrials
            JoinRequestTrials = 0;

            // Setup header information
            macHdr.Value = 0;
            macHdr.Bits.MType  = FRAME_TYPE_JOIN_REQ;

            ResetMacParameters( );

            // Add a +1, since we start to count from 0
            LoRaMacParams.ChannelsDatarate = AlternateDatarate( JoinRequestTrials + 1 );
            //lora_printf("status = Send( &macHdr, 0, NULL, 0 )\n");
            status = Send( &macHdr, 0, NULL, 0 );

            break;
        }
        case MLME_LINK_CHECK:
        {
            LoRaMacFlags.Bits.MlmeReq = 1;
            // LoRaMac will send this command piggy-pack
            MlmeConfirm.MlmeRequest = mlmeRequest->Type;

            status = AddMacCommand( MOTE_MAC_LINK_CHECK_REQ, 0, 0 );
            break;
        }
        case MLME_TXCW:
        {
            MlmeConfirm.MlmeRequest = mlmeRequest->Type;
            LoRaMacFlags.Bits.MlmeReq = 1;
            status = SetTxContinuousWave( mlmeRequest->Req.TxCw.Timeout );
            break;
        }
        case MLME_TXCW_1:
        {
            MlmeConfirm.MlmeRequest = mlmeRequest->Type;
            LoRaMacFlags.Bits.MlmeReq = 1;
            status = SetTxContinuousWave1( mlmeRequest->Req.TxCw.Timeout, mlmeRequest->Req.TxCw.Frequency, mlmeRequest->Req.TxCw.Power );
            break;
        }
        default:
            break;
    }

    if( status != LORAMAC_STATUS_OK )
    {
        NodeAckRequested = false;
        LoRaMacFlags.Bits.MlmeReq = 0;
    }

    return status;
}

LoRaMacStatus_t LoRaMacMcpsRequest( McpsReq_t *mcpsRequest )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_SERVICE_UNKNOWN;
    LoRaMacHeader_t macHdr;
    uint8_t fPort = 0;
    void *fBuffer;
    uint16_t fBufferSize;
    int8_t datarate;
    bool readyToSend = false;

    if( mcpsRequest == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( ( LoRaMacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING ) ||
        ( ( LoRaMacState & LORAMAC_TX_DELAYED ) == LORAMAC_TX_DELAYED ) )
    {
        return LORAMAC_STATUS_BUSY;
    }

    macHdr.Value = 0;
    memset1 ( ( uint8_t* ) &McpsConfirm, 0, sizeof( McpsConfirm ) );
    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;

    switch( mcpsRequest->Type )
    {
        case MCPS_UNCONFIRMED:
        {
            readyToSend = true;
            AckTimeoutRetries = 1;

            macHdr.Bits.MType = FRAME_TYPE_DATA_UNCONFIRMED_UP;
            fPort = mcpsRequest->Req.Unconfirmed.fPort;
            fBuffer = mcpsRequest->Req.Unconfirmed.fBuffer;
            fBufferSize = mcpsRequest->Req.Unconfirmed.fBufferSize;
            datarate = mcpsRequest->Req.Unconfirmed.Datarate;
            break;
        }
        case MCPS_CONFIRMED:
        {
            readyToSend = true;
            AckTimeoutRetriesCounter = 1;
            AckTimeoutRetries = mcpsRequest->Req.Confirmed.NbTrials;

            macHdr.Bits.MType = FRAME_TYPE_DATA_CONFIRMED_UP;
            fPort = mcpsRequest->Req.Confirmed.fPort;
            fBuffer = mcpsRequest->Req.Confirmed.fBuffer;
            fBufferSize = mcpsRequest->Req.Confirmed.fBufferSize;
            datarate = mcpsRequest->Req.Confirmed.Datarate;
            break;
        }
        case MCPS_PROPRIETARY:
        {
            readyToSend = true;
            AckTimeoutRetries = 1;

            macHdr.Bits.MType = FRAME_TYPE_PROPRIETARY;
            fBuffer = mcpsRequest->Req.Proprietary.fBuffer;
            fBufferSize = mcpsRequest->Req.Proprietary.fBufferSize;
            datarate = mcpsRequest->Req.Proprietary.Datarate;
            break;
        }
        default:
            break;
    }

    if( readyToSend == true )
    {
        if( AdrCtrlOn == false )
        {
            if( ValueInRange( datarate, LORAMAC_TX_MIN_DATARATE, LORAMAC_TX_MAX_DATARATE ) == true )
            {
                LoRaMacParams.ChannelsDatarate = datarate;
            }
            else
            {
                return LORAMAC_STATUS_PARAMETER_INVALID;
            }
        }
//        lora_printf("status = Send( &macHdr, fPort, fBuffer, fBufferSize );\n");
        status = Send( &macHdr, fPort, fBuffer, fBufferSize );
        if( status == LORAMAC_STATUS_OK )
        {
            McpsConfirm.McpsRequest = mcpsRequest->Type;
            LoRaMacFlags.Bits.McpsReq = 1;
        }
        else
        {
            NodeAckRequested = false;
        }
    }

    return status;
}

void LoRaMacTestRxWindowsOn( bool enable )
{
    IsRxWindowsEnabled = enable;
}

void LoRaMacTestSetMic( uint16_t txPacketCounter )
{
    UpLinkCounter = txPacketCounter;
    IsUpLinkCounterFixed = true;
}

void LoRaMacTestSetDutyCycleOn( bool enable )
{
#if ( defined( USE_BAND_868 ) || defined( USE_BAND_433 ) || defined( USE_BAND_780 ) )
    DutyCycleOn = enable;
#else
    DutyCycleOn = false;
#endif
}

void LoRaMacTestSetChannel( uint8_t channel )
{
    Channel = channel;
}

static RxConfigParams_t ComputeRxWindowParameters( int8_t datarate, uint32_t rxError )
{
    RxConfigParams_t rxConfigParams = { 0, 0, 0, 0 };
    double tSymbol = 0.0;

    rxConfigParams.Datarate = datarate;
    switch( Bandwidths[datarate] )
    {
        default:
        case 125000:
            rxConfigParams.Bandwidth = 0;
            break;
        case 250000:
            rxConfigParams.Bandwidth = 1;
            break;
        case 500000:
            rxConfigParams.Bandwidth = 2;
            break;
    }

#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
    if( datarate == DR_7 )
    { // FSK
        tSymbol = ( 1.0 / ( double )Datarates[datarate] ) * 8.0; // 1 symbol equals 1 byte
    }
    else
#endif
    { // LoRa
        tSymbol = ( ( double )( 1 << Datarates[datarate] ) / ( double )Bandwidths[datarate] ) * 1e3;
    }

    rxConfigParams.RxWindowTimeout = MAX( ( uint32_t )ceil( ( ( 2 * LoRaMacParams.MinRxSymbols - 8 ) * tSymbol + 2 * rxError ) / tSymbol ), LoRaMacParams.MinRxSymbols ); // Computed number of symbols

    rxConfigParams.RxOffset = ( int32_t )ceil( ( 4.0 * tSymbol ) - ( ( rxConfigParams.RxWindowTimeout * tSymbol ) / 2.0 ) - RADIO_WAKEUP_TIME );

    return rxConfigParams;
}
