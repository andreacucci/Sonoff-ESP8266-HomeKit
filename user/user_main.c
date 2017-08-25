/*
 *  Copyright 2017 Andrea Cucci - andreacucci@outlook.com
 *
 *  Copyright 2016 HomeACcessoryKid - HacK - homeaccessorykid@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
 
#include "esp_common.h"
#include "hkc.h"
#include "gpio.h"
#include "queue.h"

xQueueHandle identifyQueue;

struct  gpio {
    int aid;
    int iid;
  cJSON *value;
} relay_info;


void button_pressed(){
    int             new;
    static uint32   oldtime;

    if ( (oldtime+200) < (oldtime=(system_get_time()/1000) ) ) {        //200ms debounce guard
        new = GPIO_INPUT(GPIO_Pin_12)^1; 		
        GPIO_OUTPUT(GPIO_Pin_12,new);			                        //Toggle relay
		GPIO_OUTPUT(GPIO_Pin_13,new^1);			                        //Toggle LED
        relay_info.value->type=new;
        change_value(relay_info.aid,relay_info.iid,relay_info.value);   //Save new value internally
        send_events(NULL,relay_info.aid,relay_info.iid);                //Propagate to HomeKit controller
    }
}

void relay(int aid, int iid, cJSON *value, int mode)
{
    GPIO_ConfigTypeDef gpio0_in_cfg;
    GPIO_ConfigTypeDef gpio12_in_cfg;
	GPIO_ConfigTypeDef gpio13_in_cfg;

    switch (mode) {
        case 1: { //changed by gui
            if (value) {
				GPIO_OUTPUT(GPIO_Pin_12, value->type); 		//GPIO12 high -> relay on
				GPIO_OUTPUT(GPIO_Pin_13, (value->type)^1); 	//GPIO13 low  -> LED   on
			}
        }break;
        case 0: { //init
            gpio0_in_cfg.GPIO_IntrType = GPIO_PIN_INTR_NEGEDGE;         //Falling edge trigger
            gpio0_in_cfg.GPIO_Mode     = GPIO_Mode_Input;               //Input mode
            gpio0_in_cfg.GPIO_Pin      = GPIO_Pin_0;                    //Enable GPIO0
            gpio_config(&gpio0_in_cfg);                                 //Init function
            gpio_intr_callbacks[0]=button_pressed;                      //GPIO0 callback
            
            //GPIO12 = relay
            gpio12_in_cfg.GPIO_IntrType = GPIO_PIN_INTR_DISABLE;        //No interrupt
            gpio12_in_cfg.GPIO_Mode     = GPIO_Mode_Output;             //Output mode
            gpio12_in_cfg.GPIO_Pullup	= GPIO_PullUp_EN;      		    //
            gpio12_in_cfg.GPIO_Pin      = GPIO_Pin_12;                  //Enable GPIO12
            gpio_config(&gpio12_in_cfg);                                //Init function
            
            //GPIO13 = LED
			gpio13_in_cfg.GPIO_IntrType = GPIO_PIN_INTR_DISABLE;        //No interrupt
            gpio13_in_cfg.GPIO_Mode     = GPIO_Mode_Output;             //Output mode
            gpio13_in_cfg.GPIO_Pullup	= GPIO_PullUp_EN;            	//
            gpio13_in_cfg.GPIO_Pin      = GPIO_Pin_13;                  //Enable GPIO13
            gpio_config(&gpio13_in_cfg);                                //Init function
            
            relay(aid,iid,value,1);                                     //Init the outputs

            relay_info.aid=aid;                                         //Init HomeKit related values 
			relay_info.iid=iid;
            relay_info.value=cJSON_CreateBool(0); 						
        }break;
        case 2: { //update
            //do nothing
        }break;
        default: {
            //print an error?
        }break;
    }
}

void identify_task(void *arg)
{
    int i,original;
    while(1) {
        while(!xQueueReceive(identifyQueue,NULL,10));	//wait for a queue item
        original=GPIO_INPUT(GPIO_Pin_13); 				//get original state
        for (i=0;i<4;i++) {
            GPIO_OUTPUT(GPIO_Pin_13,original^1); 		//toggle
            vTaskDelay(5); 								//0.05 sec
            GPIO_OUTPUT(GPIO_Pin_13,original^0);		//toggle
            vTaskDelay(5); 								//0.05 sec
        }
    }
}

void identify(int aid, int iid, cJSON *value, int mode)
{
    switch (mode) {
        case 1: { //changed by gui
            xQueueSend(identifyQueue,NULL,0);
        }break;
        case 0: { //init
        	identifyQueue = xQueueCreate( 1, 0 );
        	xTaskCreate(identify_task,"identify",256,NULL,2,NULL);
        }break;
        case 2: { //update
            //do nothing
        }break;
        default: {
            //print an error?
        }break;
    }
}

void    hkc_user_init(char *accname)
{
    //do your init thing beyond the bear minimum
    //avoid doing it in user_init else no heap left for pairing
    cJSON *accs,*sers,*chas;
    int aid=0,iid=0;

    accs=initAccessories();
    
    sers=addAccessory(accs,++aid);
    //service 0 describes the accessory
    chas=addService(      sers,++iid,APPLE,ACCESSORY_INFORMATION_S);
    addCharacteristic(chas,aid,++iid,APPLE,NAME_C,accname,NULL);
    addCharacteristic(chas,aid,++iid,APPLE,MANUFACTURER_C,"QC",NULL);
    addCharacteristic(chas,aid,++iid,APPLE,MODEL_C,"Rev-1",NULL);
    addCharacteristic(chas,aid,++iid,APPLE,SERIAL_NUMBER_C,"1",NULL);
    addCharacteristic(chas,aid,++iid,APPLE,IDENTIFY_C,NULL,identify);
    //service 1
    chas=addService(      sers,++iid,APPLE,SWITCH_S);
    addCharacteristic(chas,aid,++iid,APPLE,NAME_C,"Sonoff Switch",NULL);
    addCharacteristic(chas,aid,++iid,APPLE,POWER_STATE_C,"0",relay);        //Switched off by default, "1" if you want it on by default

    gpio_intr_handler_register(gpio_intr_handler,NULL);                     //Register the interrupt function
    GPIO_INTERRUPT_ENABLE;
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{   
	//use this block only once to set your favorite access point or put your own selection routine
    /*wifi_set_opmode(STATION_MODE); 
    struct station_config *sconfig = (struct station_config *)zalloc(sizeof(struct station_config));
    sprintf(sconfig->ssid, ""); 	//don't forget to set this if you use it
    sprintf(sconfig->password, ""); //don't forget to set this if you use it
    wifi_station_set_config(sconfig);
    free(sconfig);
    wifi_station_connect(); /**/
    
    // try to only do the bare minimum here and do the rest in hkc_user_init
    // if not you could easily run out of stack space during pairing-setup

    hkc_init("Sonoff");
}

/***********************************************************************************
 * FunctionName : user_rf_cal_sector_set forced upon us by espressif since RTOS1.4.2
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal    B : rf init data    C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
***********************************************************************************/
uint32 user_rf_cal_sector_set(void) {
    extern char flashchip;
    SpiFlashChip *flash = (SpiFlashChip*)(&flashchip + 4);
    // We know that sector size is 4096
    //uint32_t sec_num = flash->chip_size / flash->sector_size;
    uint32_t sec_num = flash->chip_size >> 12;
    return sec_num - 5;
}
