/**
 *******************************************************************************
 * @file 	main.c
 * @author 	jjo
 * @date 	Mar 29, 2024
 * @brief	Fichier principal de votre projet sur carte Nucléo STM32G431KB
 *******************************************************************************
 */

#include "config.h"
#include "stm32g4_sys.h"

#include "stm32g4_systick.h"
#include "stm32g4_gpio.h"
#include "stm32g4_uart.h"
#include "stm32g4_adc.h"
#include "stm32g4_utils.h"
#include "HC-SR04/stm32g4_hcsr04.h"

#include "stm32g4_rtc.h"
#include "button.h"
#include "led.h"

#include <stdio.h>
#define channel ADC_1

static void state_machine(void);
void process_test_button(GPIO_TypeDef* gpio, uint32_t pin);
void process_test_photoresistor(uint16_t adc_id);
void process_test_telemeter(GPIO_TypeDef * TRIG_GPIO, uint16_t TRIG_PIN, GPIO_TypeDef * ECHO_GPIO, uint16_t ECHO_PIN);

static volatile uint32_t t = 0;

void process_ms(void)
{
	if(t)
		t--;
}

//static uint16_t count = 0;
typedef struct
{
    uint16_t valu;
    bool     updated;
    uint32_t times;
} count_t;

static count_t hikers_count = {0, false,0};



/**
  * @brief  Point d'entrée de votre application
  */
int main(void)

{
	/* Cette ligne doit rester la première de votre main !
	 * Elle permet d'initialiser toutes les couches basses des drivers (Hardware Abstraction Layer),
	 * condition préalable indispensable à l'exécution des lignes suivantes.
	 */
	HAL_Init();

	/* Initialisation des périphériques utilisés dans votre programme */
	BSP_GPIO_enable();
	BSP_UART_init(UART2_ID,115200);

	/* Indique que les printf sont dirigés vers l'UART2 */
	BSP_SYS_set_std_usart(UART2_ID, UART2_ID, UART2_ID);

	//process_test_button(GPIOA, GPIO_PIN_5);
	//process_test_photoresistor(ADC_1);
	//process_test_telemeter(GPIOA, GPIO_PIN_8, GPIOB, GPIO_PIN_4);

	/* Tâche de fond, boucle infinie, Infinite loop,... quelque soit son nom vous n'en sortirez jamais */
	while (1)
	{
		state_machine();
	}
}

void state_machine(void)
{
	typedef enum
		{
		INIT =0,
		INSTALLATION,
		WAITING_HIKERS,
		DETECTION,
		BIP,
		OFF
		}state_e;
	static state_e state = INIT;
	static state_e previous_state = INIT;
	bool entry = (state!=previous_state)?true:false;	//ce booléen sera vrai seulement 1 fois après chaque changement d'état.
	previous_state = state;

	static uint8_t telemeter_id = 0;
	static uint16_t distance = 0;
	static uint16_t detection_threshold = 0;
	static uint32_t detection_timer = 0;
	int16_t value;
	value = BSP_ADC_getValue(channel);
if (state == WAITING_HIKERS || state==INSTALLATION || state== DETECTION || INIT){
	BSP_HCSR04_process_main();
		if(!t){
			BSP_HCSR04_run_measure(telemeter_id);
			t = HCSR04_TIMEOUT;
		};
};


	// Récupération des évènements
	button_event_t button_event = BUTTON_state_machine();
	bool new_measure_event = (BSP_HCSR04_get_value(telemeter_id, &distance) == HAL_OK)?true:false;


	if (hikers_count.updated)
	{
		uint32_t ms =hikers_count.times;
		uint16_t min =(ms/60000);
		uint16_t sec =(ms % 60000) /1000 ;
	    printf("Hicker %u a %lu:%i\r\n", hikers_count.valu,min,sec);
	    hikers_count.updated = false;
	};

	switch(state)
		{
		case INIT:
			LED_init();
			BUTTON_init(GPIOA,GPIO_PIN_5);
			BSP_HCSR04_add(&telemeter_id,GPIOA,GPIO_PIN_8,GPIOB,GPIO_PIN_4);
			BSP_systick_add_callback_function(&process_ms);
			BSP_ADC_init();

			state = INSTALLATION;
			break;
		case INSTALLATION :

			if (entry) LED_set(LED_BLINK);
			if( value<800) state=OFF;
			else if (button_event == BUTTON_EVENT_SHORT_PRESS)
			{
				detection_threshold = distance*0.9;
				state = WAITING_HIKERS;
			};
			 if (button_event == BUTTON_EVENT_LONG_PRESS)
			{
				hikers_count.valu   = 0;
				hikers_count.updated = true;
			};

			break;
		case WAITING_HIKERS:
			if (entry) LED_set(LED_FLASH);
			if(button_event == BUTTON_EVENT_LONG_PRESS) state = INSTALLATION ;

			if (distance<detection_threshold) state = DETECTION ;
			else if (value <800) state=OFF;
			break;
		case DETECTION:
			if (entry){
				LED_set(LED_ON);
				detection_timer = HAL_GetTick();
			}
			if (distance>detection_threshold )
				{
				//count++ ;
				 hikers_count.valu++;
				 hikers_count.times = HAL_GetTick();
				 hikers_count.updated = true;
				state= WAITING_HIKERS ;

				};
			if(button_event == BUTTON_EVENT_SHORT_PRESS) state=INSTALLATION;
			if (distance <150 && HAL_GetTick() - detection_timer > 15000)state=BIP;

			break;
		case BIP:
			if (distance<150){
				printf(" Attention potentiel hiker en approche a %u mm\r\n",distance);
				state=DETECTION;
			};
			if (distance >150) state = WAITING_HIKERS;
			break;

		case OFF:
			if (entry)
				{LED_set(LED_OFF);
				 hikers_count.valu   = 0;
				 hikers_count.updated = true;
				 t=0;
				}
			if (value >800)state=INSTALLATION;
			break;
		default:
			break;
		}
	};



/**
 * @brief Fonction de test du bouton, la LED clignote avec un appui court et reste allumée avec un appui long
 */
void process_test_button(GPIO_TypeDef* gpio, uint32_t pin)
{
	LED_init();
	BUTTON_init(gpio, pin);
	button_event_t button_event;
	while(1)
	{
		button_event = BUTTON_state_machine();
		if (button_event == BUTTON_EVENT_LONG_PRESS) LED_set(LED_ON);
		if (button_event == BUTTON_EVENT_SHORT_PRESS) LED_set(LED_BLINK);
	}
}


/**
 * @brief La tension de la broche PA0 est envoyée sur l'UART
 */
void process_test_photoresistor(uint16_t channel)
{
	int16_t value;
	BSP_ADC_init();
	LED_init();
	while(1)
	{
		value = BSP_ADC_getValue(channel);
		printf("Raw ADC value: %d\n",value);
		HAL_Delay(500);
	}
}


/**
 * @brief	Fonction de test pour valider le télémètre, la distance est affichée en mm via l'UART
 */
void process_test_telemeter(GPIO_TypeDef * TRIG_GPIO, uint16_t TRIG_PIN, GPIO_TypeDef * ECHO_GPIO, uint16_t ECHO_PIN)
{
	uint8_t telemeter_id = 0;
	uint16_t distance = 0;
	BSP_systick_add_callback_function(&process_ms);
	BSP_HCSR04_add(&telemeter_id, TRIG_GPIO, TRIG_PIN, ECHO_GPIO, ECHO_PIN);

	while(1)
	{
		BSP_HCSR04_process_main();
		if(!t)
		{
			printf("Distance: %d mm \n",  distance);

			BSP_HCSR04_run_measure(telemeter_id);
			t = HCSR04_TIMEOUT;
		}
		BSP_HCSR04_get_value(telemeter_id, &distance);
	}
}



