/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/uart.h>


#define LED1_NODE DT_NODELABEL(led1)

static const struct gpio_dt_spec gpio_led1 = GPIO_DT_SPEC_GET( LED1_NODE , gpios);


#define AGT_TIMER0_NODE DT_CHILD(DT_NODELABEL(agt0), counter) 

static const struct device *agt_timer_dev = DEVICE_DT_GET(AGT_TIMER0_NODE); 

#define S1_NODE DT_NODELABEL(button0)

static const struct gpio_dt_spec gpio_s1 = GPIO_DT_SPEC_GET(S1_NODE, gpios);

static struct gpio_callback s1_callback_struct;


#define ADC_CH0_NODE DT_PATH(zephyr_user)

static const struct adc_dt_spec adc_chan0 = ADC_DT_SPEC_GET_BY_IDX(ADC_CH0_NODE, 0);


#define UART0_NODE DT_NODELABEL(uart0)

static const struct device *uart0_dev = DEVICE_DT_GET(UART0_NODE);




/*  Callback called when counter overflows*/
static void agt0_callback(const struct device *dev, void *user_data)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(user_data);

    gpio_pin_toggle_dt(&gpio_led1);
}

/* Initialize a struct adc_sequence */
int16_t buf;
static int32_t adc_result_mV;
static struct adc_sequence sequence = 
{
    .buffer = &buf,
    /* buffer size in bytes, not number of samples */
    .buffer_size = sizeof(buf),
};

/*  Callback called when switch S1 is pressed */
void s1_callback_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    int err = 0;

    /* Set a read request */
    err = adc_read_dt(&adc_chan0, &sequence);
    if (err < 0) {
        printk("Failed to read ADC channel 0: %d", err);
    }

    adc_result_mV = buf;

    err = adc_raw_to_millivolts_dt(&adc_chan0, &adc_result_mV);
    if (err < 0) {
        printk("Failed to convert ADC channel 0 raw value to mV: %d", err);
    }


	static char tx_buf[48];
    int len = snprintk(tx_buf, sizeof(tx_buf),
                       "Raw value: %d | Voltage: %d mV \r\n",
                       buf,
                       adc_result_mV);

    err = uart_tx(uart0_dev, tx_buf, len, SYS_FOREVER_US);
    if (err < 0) 
    {
        printk("Failed to transmit UART0 data: %d", err);
    }

}




int main(void)
{
    int err = 0;
    
    /* Validate that GPIO port is ready */
    if (!gpio_is_ready_dt(&gpio_led1)) {
        printk("GPIO LED1 device is not ready");
    }
    
    /* Configure the pin connected to LED1 as output high*/
    err = gpio_pin_configure_dt(&gpio_led1, GPIO_OUTPUT_ACTIVE);
    if (err < 0) {
        printk("Failed to configure GPIO LED1: %d", err);
    }

    if (!device_is_ready(agt_timer_dev)) {
    	printk("AGT timer device is not ready");
    }

	/* Get the AGT timer frequency */
    uint32_t freq = counter_get_frequency(agt_timer_dev);

    /* Configure a Counter top value structure  */
    struct counter_top_cfg  top_cfg = 
    {
        .flags = 0,
        .ticks = freq,   /* expire after 1s */
        .callback = agt0_callback,
        .user_data = NULL,
    };



	/* Set the top value of the Counter and call the callback when the top value is reached */
    err = counter_set_top_value(agt_timer_dev, &top_cfg);
    if (err < 0) {
        printk("Failed to set counter top value: %d", err);
        return err;
    }

    /* Start the Counter */
    err = counter_start(agt_timer_dev);
    if (err < 0) {
        printk("Failed to start counter: %d", err);
        return err;
    }


    /* Validate that GPIO port is ready */

    if (!gpio_is_ready_dt(&gpio_s1)) {
        printk("GPIO S1 device is not ready");
    }

    /* Configure the pin connected to S1 as input */

    err = gpio_pin_configure_dt(&gpio_s1, GPIO_INPUT);
    if (err < 0) {
        printk("Failed to configure GPIO S1: %d", err);
        return err;
    }

     /* Configure a pin interrupt */
    err = gpio_pin_interrupt_configure_dt(&gpio_s1, GPIO_INT_EDGE_FALLING);
    if (err < 0) {
        printk("Failed to configure GPIO S1 interrupt: %d", err);
        return err;
    }

    /* Initialize a struct gpio_callback */
    gpio_init_callback(&s1_callback_struct, s1_callback_handler, BIT(gpio_s1.pin));

    /* Add an application callback */
    err = gpio_add_callback(gpio_s1.port, &s1_callback_struct);
    if (err < 0) {
        printk("Failed to add GPIO S1 callback: %d", err);
        return err;
    }



	   /* Validate that the ADC device is ready */
    if (!adc_is_ready_dt(&adc_chan0)) {
        printk("ADC channel 0 device is not ready");
    }

    /* Configure an ADC channel */
    err = adc_channel_setup_dt(&adc_chan0);
    if (err < 0) {
        printk("Failed to configure ADC channel 0: %d", err);
        return err;
    }

    /* Initialize an ADC sequence */
    err = adc_sequence_init_dt(&adc_chan0, &sequence);
    if (err < 0) {
        printk("Failed to initialize ADC sequence: %d", err);
        return err;
    }


	/* Validate that UART9 is ready */
    if (!device_is_ready(uart0_dev)) {
        printk("UART0 device is not ready");
    }


	return 0;
}
