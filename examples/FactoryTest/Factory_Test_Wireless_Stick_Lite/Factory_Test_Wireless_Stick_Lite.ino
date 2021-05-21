#include <ESP32_LoRaWAN.h>
#include "Arduino.h"
#include "WiFi.h"
#include "images.h"

#define USER_KEY                                    0
#define YOUR_SSID                                   "your ssid"
#define YOUR_PASSWORD                               "your password"

#define RF_FREQUENCY                                868000000 // Hz

#define TX_OUTPUT_POWER                             14        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

typedef enum
{
    STATUS_LOWPOWER,
    STATUS_RX,
    STATUS_TX
}States_t;


int16_t txNumber;
States_t state;
bool sleepMode = false;
int16_t Rssi,rxSize;

uint32_t  license[4] = {0xD5397DF0, 0x8573F814, 0x7A38C73D, 0x48E68607};

void WIFISetUp(void)
{
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  WiFi.begin(YOUR_SSID,YOUR_PASSWORD);//fill in "Your WiFi SSID","Your Password"
  delay(100);

  byte count = 0;
  while(WiFi.status() != WL_CONNECTED && count < 10)
  {
    count ++;
    Serial.print(".");
    delay(500);
  }

  if(WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\r\nConnecting...OK.");
  }
  else
  {
    Serial.println("Connecting...Failed");
  }
  Serial.println("WIFI Setup done");
}

void WIFIScan(unsigned int value)
{
  unsigned int i;
  WiFi.mode(WIFI_STA);

  for(i=0;i<value;i++)
  {
    Serial.println("Scan start...");

    int n = WiFi.scanNetworks();
    Serial.println("Scan done");
    delay(500);

    if (n == 0)
    {
      Serial.println("no network found");
      //while(1);
    }
    else
    {
      Serial.print(n);
      Serial.println("networks found:");
      delay(500);

      for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
        Serial.print((i + 1));
        Serial.print(":");
        Serial.print((String)(WiFi.SSID(i)));
        Serial.print(" (");
        Serial.print((String)(WiFi.RSSI(i)));
        Serial.println(")");;
        delay(10);
      }
    }
    delay(800);
  }
}
 bool resendflag=false;
 bool deepsleepflag=false;
 void interrupt()
{
  delay(10);
  if(digitalRead(USER_KEY)==0)
  {
      if(digitalRead(LED)==LOW)
      {
        state=STATUS_TX;
      }
      else
      {
        deepsleepflag=true; 
      }  
  }
}

void setup()
{ 
  pinMode(LED,OUTPUT);
  for(int i = 0;i<5;i++)
  {
    digitalWrite(LED,1);
    delay(100);
    digitalWrite(LED,0);
    delay(100);
  }
  Serial.begin(115200);
  while (!Serial);
  factory_test=true;
  
  uint64_t chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf("ESP32ChipID=%04X",(uint16_t)(chipid>>32));//print High 2 bytes
  Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.
  delay(100);
  
  SPI.begin(SCK,MISO,MOSI,SS);
  Mcu.init(SS,RST_LoRa,DIO0,DIO1,license);

  WIFISetUp();
  WiFi.disconnect(); //重新初始化WIFI
  WiFi.mode(WIFI_STA);
  delay(100);

  WIFIScan(1);

  txNumber=0;
  Rssi=0;
  
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxDone = OnRxDone;
  
  Radio.Init( &RadioEvents );
  Radio.SetChannel( RF_FREQUENCY );
  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                               LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                               LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                               true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );
  
  Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                               LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                               LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                               0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
  state=STATUS_TX;
  pinMode(USER_KEY,INPUT_PULLUP);
  attachInterrupt(USER_KEY,interrupt,FALLING);
}


void loop()
{
  switch(state)
  {
    case STATUS_TX:
      delay(1000);
      txNumber++;
      sprintf(txpacket,"%s","hello");
      sprintf(txpacket+strlen(txpacket),"%d",txNumber);
      sprintf(txpacket+strlen(txpacket),"%s"," Rssi : ");
      sprintf(txpacket+strlen(txpacket),"%d",Rssi);

      Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));

      Radio.Send( (uint8_t *)txpacket, strlen(txpacket) );
      state=STATUS_LOWPOWER;
      break;
    case STATUS_RX:
      Serial.println("into RX mode");
      Radio.Rx( 0 );
      state=STATUS_LOWPOWER;
      break;
    case STATUS_LOWPOWER:
      LoRaWAN.sleep(CLASS_C,0);
      if(deepsleepflag)
      {
        delay(200);
        LoRaWAN.sleep(CLASS_C,0);
        Serial.println("lowpower");
        Radio.Sleep();
        pinMode(LED,INPUT);
        pinMode(4,INPUT);
        pinMode(5,INPUT);
        pinMode(14,INPUT);
        pinMode(15,INPUT);
        pinMode(16,INPUT);
        pinMode(17,INPUT);
        pinMode(18,INPUT);
        pinMode(19,INPUT);
        pinMode(26,INPUT);
        pinMode(27,INPUT);
        digitalWrite(Vext,HIGH);
        delay(2);
        esp_deep_sleep_start();
      }
      break;
    default:
      break;
  }
}

void OnTxDone( void )
{
  Serial.print("TX done......");
  state=STATUS_RX;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    Serial.print("TX Timeout......");
    state=STATUS_TX;
}
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    digitalWrite(LED,HIGH);
    Rssi=rssi;
    rxSize=size;
    memcpy(rxpacket, payload, size );
    rxpacket[size]='\0';
    Radio.Sleep( );

    Serial.printf("\r\nreceived packet \"%s\" with Rssi %d , length %d\r\n",rxpacket,Rssi,rxSize);
    Serial.println("wait to send next packet");
    state=STATUS_TX;
}
