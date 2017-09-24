
#include "asf.h"
#include "stdio_serial.h"
#include "conf_uart_serial.h"
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif
static struct usart_module lora_uart_module;
static struct usart_module bt_uart_module;
static struct tc_module tc_valve;
static struct tc_module tc_valve_stop;
static struct tc_module tc_lora_send;

#define TC_COUNT_VALUE 55535
#define Valve_pin_1   PIN_PA14
#define Valve_pin_2   PIN_PA15
#define Status_pin_1  PIN_PA05
#define Status_pin_2  PIN_PA06

#define MAX_RX_BUFFER_LENGTH   4

#define max_humidity           4000
#define min_humidity           1000
#define hysteresis_humidity    500

#define bautrate_bt 9600
int lora_counter;
int humidity_lora; 
int lora_data; 
bool valve_status;
volatile uint8_t rx_buffer[MAX_RX_BUFFER_LENGTH];

int humidity ; 
void valve_stop ()
{
	port_pin_set_output_level(Valve_pin_1,HIGH);
	port_pin_set_output_level(Valve_pin_2,HIGH);
}
void valve_close ()
{
	port_pin_set_output_level(Valve_pin_1,LOW);
	port_pin_set_output_level(Valve_pin_2,HIGH);
	valve_status=false;

}
void valve_open ()
{
	port_pin_set_output_level(Valve_pin_1,HIGH);
	port_pin_set_output_level(Valve_pin_2,LOW);
	valve_status=true;
}
void valve_start_open ()
{   
	valve_open();
	tc_start_counter(&tc_valve);
	printf("valve open\r\n");
}
void valve_start_close()
{
	valve_close();
    tc_start_counter(&tc_valve);
    printf("valve close\r\n");

}
void status_bt ()
{
	bool Status_1 = port_pin_get_input_level(Status_pin_1);
	bool Status_2 = port_pin_get_input_level(Status_pin_2);
	int status;
    if (Status_1)
    {
		if (Status_2)
		{
			printf("Power on");
		}
    }
	if (Status_1)
	{
		if (!Status_2)
		{
			printf("Stand by");
		}
	}
	if (!Status_1)
	{
		if (Status_2)
		{
			printf("transparent uart ");
		}
	}
	if (!Status_1)
	{
		if (!Status_2)
		{
			printf("conection");
		}
	}
}
void connect_bt ()
{
char start[]   ="$$$";
char echo[]    ="+\r\n";
char setup[]   ="SS,C0\r\n";
char scan[]    ="f\r\n";
char connect[]  ="C,0,D88039F3F2B7\r\n"; //bt adresa

usart_write_buffer_job(&bt_uart_module,start,sizeof(start));
delay_ms (100);
usart_write_buffer_job(&bt_uart_module,echo, sizeof(echo));
delay_ms (100);
usart_write_buffer_job(&bt_uart_module,setup,sizeof(setup));
delay_ms (100);
usart_write_buffer_wait(&bt_uart_module,connect,sizeof(connect));
delay_ms (1000);	
}
void disconet_bt ()
{
char start[]   ="$$$";
char bt_end[]   ="K";
usart_write_buffer_job(&bt_uart_module,start,sizeof(start));
delay_ms(100);
usart_write_buffer_job(&bt_uart_module,bt_end,sizeof(bt_end));

}
void lora_send()
{
char port[]="17";
		char *data[21] = {"7B","65","C9","12D","191","1F5","259","2BD","321","385","3E9","64","C8","12C","190","1F4","258","2BC","320","384","3E8"};
		char lsend[]="mac tx uncnf ";
		
		strcat(lsend, port);
		strcat(lsend, " ");
		strcat(lsend, data[lora_data]);
		strcat(lsend, "\r\n");
		usart_write_buffer_wait(&lora_uart_module,lsend ,strlen(lsend));
			
}
void humidity_lora_transfer ()
{  
	humidity_lora=humidity/409;
	if (valve_status)
	{
		if (humidity_lora<100)
		{
			lora_data=10;
		}
		if (humidity_lora<90)
		{
			lora_data=9;
		}
		if (humidity_lora<80)
		{
			lora_data=8;
		}
		if (humidity_lora<70)
		{
			lora_data=7;
		}
		if (humidity_lora<60)
		{
			lora_data=6;
		}
		if (humidity_lora<50)
		{
		   lora_data=5;
		}
		if (humidity_lora<40)
		{
			lora_data=4;
		}
		if (humidity_lora<30)
		{
			lora_data=3;
		}
		if (humidity_lora<20)
		{
			lora_data=2;
		}
		 if (humidity_lora<10)
		 {
			lora_data=1;
		 } 
	}
	if (!valve_status)
	{
		if (humidity_lora<100)
		{
			lora_data=20;
		}
		if (humidity_lora<90)
		{
			lora_data=19;
		}
		if (humidity_lora<80)
		{
			lora_data=18;
		}
		if (humidity_lora<70)
		{
			lora_data=17;
		}
		if (humidity_lora<60)
		{
			lora_data=16;
		}
		if (humidity_lora<50)
		{
			lora_data=15;
		}
		if (humidity_lora<40)
		{
			lora_data=14;
		}
		if (humidity_lora<30)
		{
			lora_data=13;
		}
		if (humidity_lora<20)
		{
			lora_data=12;
		}
		if (humidity_lora<10)
		{
			lora_data=11;
		}
	}
}
void cyklus ()
{   
	connect_bt();
	delay_ms(200);  
	usart_read_buffer_job(&bt_uart_module,(uint8_t *)rx_buffer, MAX_RX_BUFFER_LENGTH);
	humidity=atoi(rx_buffer);
	if (humidity>max_humidity)
	{
		valve_start_close();
		humidity_lora_transfer();
		lora_send();
	}
	if (humidity<min_humidity)
	{
		valve_start_open();
		humidity_lora_transfer();
		lora_send();
	}
	if (lora_counter == 5)
	{   
		humidity_lora_transfer();
		lora_send();
		lora_counter=0;
	}
	lora_counter ++; 
}
void usart_read_callback(struct usart_module *const usart_module)
{
}
void usart_write_callback(struct usart_module *const usart_module)
{
	port_pin_toggle_output_level(LED_0_PIN);
}
static void configure_console(void)
{
	struct usart_config usart_conf;

	usart_get_config_defaults(&usart_conf);
	usart_conf.mux_setting = USART_RX_3_TX_2_XCK_3;
	usart_conf.pinmux_pad0 = PINMUX_UNUSED;
	usart_conf.pinmux_pad1 = PINMUX_UNUSED;
	usart_conf.pinmux_pad2 = PINMUX_PA10D_SERCOM2_PAD2;
	usart_conf.pinmux_pad3 = PINMUX_PA11D_SERCOM2_PAD3;
	usart_conf.baudrate    = bautrate_bt;

	stdio_serial_init(&bt_uart_module, SERCOM2, &usart_conf);
	usart_enable(&bt_uart_module);
}
void configure_usart_callbacks(void)
{
	usart_register_callback(&bt_uart_module,
	usart_write_callback, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_register_callback(&bt_uart_module,
	usart_read_callback, USART_CALLBACK_BUFFER_RECEIVED);
	usart_enable_callback(&bt_uart_module, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_enable_callback(&bt_uart_module, USART_CALLBACK_BUFFER_RECEIVED);
}
static void configure_lora_console(void)
{
	struct usart_config usart_conf;

	usart_get_config_defaults(&usart_conf);
	usart_conf.mux_setting = USART_RX_3_TX_2_XCK_3;
	usart_conf.pinmux_pad0 = PINMUX_UNUSED;
	usart_conf.pinmux_pad1 = PINMUX_UNUSED;
	usart_conf.pinmux_pad2 = PINMUX_PA16C_SERCOM1_PAD2;
	usart_conf.pinmux_pad3 = PINMUX_PA17C_SERCOM1_PAD3;
	usart_conf.baudrate    = bautrate_bt;

	stdio_serial_init(&lora_uart_module, SERCOM1, &usart_conf);
	usart_enable(&lora_uart_module);
}
static void tc_lora_to_counter(
struct tc_module *const module_inst)
{
	static uint32_t count = 0;
	count ++;
	if(count == 10000)
	{   cyklus();
		count = 0;
	}

	tc_set_count_value(module_inst,TC_COUNT_VALUE);
}

static void configure_lora_tc(void)
{
	struct tc_config config_tc;

	tc_get_config_defaults(&config_tc);
	config_tc.counter_size    = TC_COUNTER_SIZE_16BIT;
	config_tc.counter_16_bit.value = TC_COUNT_VALUE;
	config_tc.clock_prescaler =TC_CLOCK_PRESCALER_DIV2;
	config_tc.run_in_standby =true; 
	tc_init(&tc_lora_send, TC1, &config_tc);
	tc_enable(&tc_lora_send);
}

static void configure_tc_lora_callbacks(void)
{
	tc_register_callback(
	&tc_lora_send,
	tc_lora_to_counter,
	TC_CALLBACK_OVERFLOW);
	tc_enable_callback(&tc_lora_send, TC_CALLBACK_OVERFLOW);
}


static void tc_valve_to_counter(struct tc_module *const module_inst)
{
	static uint32_t count = 0;
	count ++;
	if(count == 1000){
			valve_stop();
		tc_stop_counter(&tc_valve);
		count = 0;
	}

	tc_set_count_value(module_inst,TC_COUNT_VALUE);
}

static void configure_tc(void)
{
	struct tc_config config_tc;

	tc_get_config_defaults(&config_tc);
	config_tc.counter_size    = TC_COUNTER_SIZE_16BIT;
	config_tc.counter_16_bit.value = TC_COUNT_VALUE;
    config_tc.clock_prescaler =TC_CLOCK_PRESCALER_DIV2;
	tc_init(&tc_valve, TC2, &config_tc);
	tc_enable(&tc_valve);
}

/** Registers TC callback function with the  driver.
 */
static void configure_tc_callbacks(void)
{
	tc_register_callback(
			&tc_valve,
			tc_valve_to_counter,
			TC_CALLBACK_OVERFLOW);
	tc_enable_callback(&tc_valve, TC_CALLBACK_OVERFLOW);
}

void configure_port_pins(void)
{
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	/*
	Nastavení vstupu
	*/
	config_port_pin.direction = PORT_PIN_DIR_INPUT;
	port_pin_set_config(Status_pin_1, &config_port_pin);  //Bt status 1
	port_pin_set_config(Status_pin_2, &config_port_pin);  //Bt status 2

	/*
	Nastavení výstupu
	*/
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(LED_0_PIN ,  &config_port_pin);   //led test
    port_pin_set_config(Valve_pin_1 ,  &config_port_pin);   //led test
    port_pin_set_config(Valve_pin_2 ,  &config_port_pin);   //led test

}

int main(void)
{
	struct port_config pin;

	system_init();

    
    /*Configures  TC driver*/
    configure_tc();

    /*Configures TC callback*/
    configure_tc_callbacks();
	
	configure_lora_tc();
	
	configure_tc_lora_callbacks();

	/*Configure UART console.*/
	configure_console();
  
   	configure_usart_callbacks();
     
	configure_lora_console();
	
    configure_port_pins();
	/*Initialize the delay driver*/
	delay_init();
	
	/*Enable system interrupt*/
	system_interrupt_enable_global();
   
    tc_stop_counter(&tc_valve);
		/*main loop*/
	while(true)
	{
	}
	
}

#ifdef __cplusplus
}
#endif
