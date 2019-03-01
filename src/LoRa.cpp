#include <LoRa.h>

void LoRaClass::DeviceStateInit()
{
    LoRaMacPrimitives.MacMcpsConfirm = McpsConfirm;//
    LoRaMacPrimitives.MacMcpsIndication = McpsIndication;//
    LoRaMacPrimitives.MacMlmeConfirm = MlmeConfirm;//
    LoRaMacCallbacks.GetBatteryLevel = BoardGetBatteryLevel;
    LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks );

    TimerInit( &TxNextPacketTimer, OnTxNextPacketTimerEvent );//
    mibReq.Type = MIB_ADR;
    mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
    LoRaMacMibSetRequestConfirm( &mibReq );//global variable AdrCtrlon=1

    mibReq.Type = MIB_PUBLIC_NETWORK;
    mibReq.Param.EnablePublicNetwork = LORAWAN_PUBLIC_NETWORK;
    LoRaMacMibSetRequestConfirm( &mibReq );//PublicNetwork=true; //SX1276SetModem(MODEM_LORA);//1276.settings.lora.publicnetwork=true;//  public SYNCWORD;

#if defined( USE_BAND_868 )
    LoRaMacTestSetDutyCycleOn( LORAWAN_DUTYCYCLE_ON );
    if(IsLoRaMacNetworkJoined==false)
    {
#if( USE_SEMTECH_DEFAULT_CHANNEL_LINEUP == 1 )
    LoRaMacChannelAdd( 3, ( ChannelParams_t )LC4 );
    LoRaMacChannelAdd( 4, ( ChannelParams_t )LC5 );
    LoRaMacChannelAdd( 5, ( ChannelParams_t )LC6 );
    LoRaMacChannelAdd( 6, ( ChannelParams_t )LC7 );
    LoRaMacChannelAdd( 7, ( ChannelParams_t )LC8 );
    LoRaMacChannelAdd( 8, ( ChannelParams_t )LC9 );
    LoRaMacChannelAdd( 9, ( ChannelParams_t )LC10 );

    mibReq.Type = MIB_RX2_DEFAULT_CHANNEL;
    mibReq.Param.Rx2DefaultChannel = ( Rx2ChannelParams_t ){ 869525000, DR_3 };
    LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_RX2_CHANNEL;
    mibReq.Param.Rx2Channel = ( Rx2ChannelParams_t ){ 869525000, DR_3 };
    LoRaMacMibSetRequestConfirm( &mibReq );
#endif
    }
#endif

}

void LoRaClass::DeviceStateJion()
{
#if( OVER_THE_AIR_ACTIVATION != 0 )
                MlmeReq_t mlmeReq;

                mlmeReq.Type = MLME_JOIN;

                mlmeReq.Req.Join.DevEui = DevEui;
                mlmeReq.Req.Join.AppEui = AppEui;
                mlmeReq.Req.Join.AppKey = AppKey;
                mlmeReq.Req.Join.NbTrials = 3;

                if( NextTx == true )
                {
                	lora_printf("+JOIN:STARTING...\n");
                    LoRaMacMlmeRequest( &mlmeReq );
                }
                DeviceState = DEVICE_STATE_SLEEP;
#else
                // Choose a random device address if not already defined in Commissioning.h
//                if( DevAddr == 0 )
//                {
//                    // Random seed initialization
//                    srand1( BoardGetRandomSeed( ) );
//
//                    // Choose a random device address
//                    DevAddr = randr( 0, 0x01FFFFFF );
//                }

                mibReq.Type = MIB_NET_ID;
                mibReq.Param.NetID = LORAWAN_NETWORK_ID;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_DEV_ADDR;
                mibReq.Param.DevAddr = DevAddr;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_NWK_SKEY;
                mibReq.Param.NwkSKey = NwkSKey;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_APP_SKEY;
                mibReq.Param.AppSKey = AppSKey;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_NETWORK_JOINED;
                mibReq.Param.IsNetworkJoined = true;
                LoRaMacMibSetRequestConfirm( &mibReq );

                DeviceState = DEVICE_STATE_SEND;
#endif
}
void LoRaClass::DeviceStateSend()
{
	if( NextTx == true )
	{
		lora_printf("In sending...\r\n");
		DelayMs(100);
		NextTx = SendFrame( );
	}
	if( ComplianceTest.Running == true )
	{
			// Schedule next packet transmission
			TxDutyCycleTime = 5000; // 5000 ms
	}
	else
	{
			// Schedule next packet transmission
			TxDutyCycleTime = APP_TX_DUTYCYCLE + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
	}
}
void LoRaClass::DeviceSleep(uint8_t isLowPowerOn,uint8_t debuglevel)
{
	Mcu.sleep(isLowPowerOn,debuglevel);
}
LoRaClass LoRa;

