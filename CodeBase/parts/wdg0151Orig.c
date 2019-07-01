#include "wdg0151.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sim_time.h>

#define DEBUG_PRINT printf
//#define DEBUG_PRINT if(0) printf

void wdg0151_print(struct wdg0151_t *wdg) {

    for(uint8_t y = 0; y < WDG0151_HEIGHT; y++) {

        for(uint8_t x = 0; x < WDG0151_WIDTH; x++) {
            DEBUG_PRINT("%u ", wdg->ctrl1.data[y][x]);
        }

        for(uint8_t x = 0; x < WDG0151_WIDTH; x++)
            DEBUG_PRINT("%u ", wdg->ctrl2.data[y][x]);

        DEBUG_PRINT("\n");
    }
}

static const char* irq_names[IRQ_WDG0151_COUNT] = {
    [IRQ_WDG0151_CS1] = "<wdg0151.cs1",
    [IRQ_WDG0151_CS2] = "<wdg0151.cs2",
    [IRQ_WDG0151_RS] = "<wdg0151.rs",
    [IRQ_WDG0151_RW] = "<wdg0151.rw",
    [IRQ_WDG0151_E] = "<wdg0151.e",
    [IRQ_WDG0151_D0] = "=wdg0151.d0",
    [IRQ_WDG0151_D1] = "=wdg0151.d1",
    [IRQ_WDG0151_D2] = "=wdg0151.d2",
    [IRQ_WDG0151_D3] = "=wdg0151.d3",
    [IRQ_WDG0151_D4] = "=wdg0151.d4",
    [IRQ_WDG0151_D5] = "=wdg0151.d5",
    [IRQ_WDG0151_D6] = "=wdg0151.d6",
    [IRQ_WDG0151_D7] = "=wdg0151.d7",
    [IRQ_WDG0151_RST] = "<wdg0151.rst",
};

static uint8_t wdg0151_get_cs(struct wdg0151_t *wdg) {
    printf("wdg0151_get_cs started.\n");
    uint8_t res = 0;
    if(!(wdg->pinstate & (1<<IRQ_WDG0151_CS2)))
        res |= (1<<2);
   
    if(!(wdg->pinstate & (1<<IRQ_WDG0151_CS1)))
        res |= (1<<1);
    
    printf("res=%d\nwdg0151_get_cs ended.\n",res);
    return res;
}

static avr_cycle_count_t wdg0151_busy_timer(struct avr_t *avr, avr_cycle_count_t when, void *param) {
    struct wdg0151_t *wdg = (struct wdg0151_t*) param;
    uint8_t res = wdg0151_get_cs(wdg);
    
    if(res & (1<<1))
        wdg->ctrl1.busy = false;
    
    if(res & (1<<2))
        wdg->ctrl2.busy = false;

    return 0;
}

static uint32_t wdg0151_write_data(struct wdg0151_t *wdg) {
    printf("wdg0151_write_data started.\n");
    uint32_t delay = 200;                       // TODO
    uint8_t cres = wdg0151_get_cs(wdg);

    if(cres & (1<<1)) {
        DEBUG_PRINT("glcd: write_data (1): data: %u, x: %u, y: %u\n", wdg->datapins, wdg->ctrl1.x_addr, wdg->ctrl1.y_addr);
        wdg->ctrl1.data[wdg->ctrl1.y_addr][wdg->ctrl1.x_addr] = wdg->datapins;
        wdg->ctrl1.x_addr++;

        if(wdg->ctrl1.x_addr >= 64)
            wdg->ctrl1.x_addr = 0;
    }

    if(cres & (1<<2)) {
        DEBUG_PRINT("glcd: write_data (2): data: %u, x: %u, y: %u\n", wdg->datapins, wdg->ctrl2.x_addr, wdg->ctrl2.y_addr);
        wdg->ctrl2.data[wdg->ctrl2.y_addr][wdg->ctrl2.x_addr] = wdg->datapins;
        wdg->ctrl2.x_addr++;

        if(wdg->ctrl2.x_addr >= 64)
            wdg->ctrl2.x_addr = 0;
    }


    if(wdg->cb)
        wdg->cb();

    return delay;
}

static uint32_t wdg0151_write_instruction(struct wdg0151_t *wdg) {
    
    printf("wdg0151_write_instruction started.\n");
    
    uint32_t delay = 200; // TODO
    
    uint8_t cres = wdg0151_get_cs(wdg); 

    printf("cres value=%d\n",cres);
    
    if((wdg->pinstate & (1<<IRQ_WDG0151_D6)) && !(wdg->pinstate & (1<<IRQ_WDG0151_D7))) {
        // Y address
        uint8_t y = wdg->datapins & 0x3F;
        if(cres & (1<<1)) {
            wdg->ctrl1.x_addr = y;
            printf("glcd: write_instruction (1): x_addr: %u\n", y);
        }

        if(cres & (1<<2)) {
            wdg->ctrl2.x_addr = y;
            printf("glcd: write_instruction (2): x_addr: %u\n", y);
        }

    } else if((wdg->pinstate & (1<<IRQ_WDG0151_D7)) && (wdg->pinstate & (1<<IRQ_WDG0151_D6))) {
        // start line
        uint8_t start = wdg->datapins & 0x3F;
    
        if(cres & (1<<1)) {
            wdg->ctrl1.start = start;
            printf("glcd: write_instruction (1): start: %u\n", start);
        }

        if(cres & (1<<2)) {
            printf("glcd: write_instruction (2): start: %u\n", start);
            wdg->ctrl2.start = start;;
        }

        DEBUG_PRINT("glcd: Using unsupported feature startline!\n");
    } else if((wdg->pinstate & (1<<IRQ_WDG0151_D7)) && !(wdg->pinstate & (1<<IRQ_WDG0151_D6))) {
        // X address
        uint8_t x = wdg->datapins & 0x7;        
        
        if(cres & (1<<1)) {
            printf("glcd: write_instruction (1): y_addr: %u\n", x);
            wdg->ctrl1.y_addr = x;
        }

        if(cres & (1<<2)) {
            printf("glcd: write_instruction (2): y_addr: %u\n", x);
            wdg->ctrl2.y_addr = x;
        }

    } else if(!(wdg->pinstate & (1<<IRQ_WDG0151_D7)) && !(wdg->pinstate & (1<<IRQ_WDG0151_D6))) {
        // on/off
        if(wdg->pinstate & (1<<IRQ_WDG0151_D0)) {
            if(cres & (1<<1)) {
                printf("glcd: write_instruction (1): on\n");
                wdg->ctrl1.enabled = true;
            }

            if(cres & (1<<2)) {
                printf("glcd: write_instruction (2): on\n");
                wdg->ctrl2.enabled = true;
            }

        } else {
            if(cres & (1<<1)) {
                printf("glcd: write_instruction (1): off\n");
                wdg->ctrl1.enabled = false;
            }

            if(cres & (1<<2)) {

                printf("glcd: write_instruction (2): off\n");
                wdg->ctrl2.enabled = false;
            }


        }
    }
    return delay;
}

static uint32_t wdg0151_process_write(struct wdg0151_t *wdg) {

    wdg->datapins = ((wdg->pinstate>>IRQ_WDG0151_D0) & 0xFF);
    uint8_t cres = wdg0151_get_cs(wdg);

    printf("wdg0151_process_write, Total LEDs ON= %d\n",cres);
    if(cres & (1<<1))
        wdg->ctrl1.dummy = 1;

    if(cres & (1<<2))
        wdg->ctrl2.dummy = 1;

    int delay = 0;
    if(wdg->pinstate & (1<<IRQ_WDG0151_RS))
    {
        delay = wdg0151_write_data(wdg);
    }
    else
    {
        delay = wdg0151_write_instruction(wdg);
    }
    printf("Delayprocesswrite= %d\n",delay);
    return delay;
}

static void wdg0151_write_datapins(struct wdg0151_t *wdg, uint8_t data) {
    for(int i = 0; i < 8; i++) {
        avr_raise_irq(wdg->irq + IRQ_WDG0151_D0 + i, (data<<i) & 1);
                printf("Here22 #write instruction!");

    }

    wdg->datapins = data;
}

static uint32_t wdg0151_process_read(struct wdg0151_t *wdg) {
    printf("wdg0151_process_read started.\n");
    uint32_t delay = 500;
    uint8_t cres = wdg0151_get_cs(wdg);

    if(wdg->pinstate & (1<<IRQ_WDG0151_RS)) {
        if(cres & (1<<1)) { 
            if(wdg->ctrl1.dummy) {
                DEBUG_PRINT("glcd: read_data (1): dummy read\n");
                wdg->ctrl1.dummy--;
            } else {
                DEBUG_PRINT("glcd: read_data (1): %u\n", wdg->ctrl1.data[wdg->ctrl1.y_addr][wdg->ctrl1.x_addr]);
                wdg0151_write_datapins(wdg, wdg->ctrl1.data[wdg->ctrl1.y_addr][wdg->ctrl1.x_addr]);
                wdg->ctrl1.x_addr++;

                if(wdg->ctrl1.x_addr >= 64)
                    wdg->ctrl1.x_addr = 0;
            }
        }

        if(cres & (1<<2)) { 
            if(wdg->ctrl2.dummy) {
                DEBUG_PRINT("glcd: read_data (2): dummy read\n");
                wdg->ctrl2.dummy--;
            } else {
                DEBUG_PRINT("glcd: read_data (2): %u\n", wdg->ctrl2.data[wdg->ctrl2.y_addr][wdg->ctrl2.x_addr]);
                wdg0151_write_datapins(wdg, wdg->ctrl2.data[wdg->ctrl2.y_addr][wdg->ctrl2.x_addr]);
                wdg->ctrl2.x_addr++;

                if(wdg->ctrl2.x_addr >= 64)
                    wdg->ctrl2.x_addr = 0;
            }
        }

       
    } else {
        uint8_t data;
        
        if(cres & (1<<1)) {
            DEBUG_PRINT("glcd: read_status (1): reset: %u, enabled: %u, busy:%u\n", wdg->ctrl1.reset, wdg->ctrl1.enabled, wdg->ctrl1.busy);
            data = 0 | (wdg->ctrl1.reset<<4) | (wdg->ctrl1.enabled<<5) | (wdg->ctrl1.busy<<7);
        }

        if(cres & (1<<2)) {
            DEBUG_PRINT("glcd: read_status (2): reset: %u, enabled: %u, busy:%u\n", wdg->ctrl2.reset, wdg->ctrl2.enabled, wdg->ctrl2.busy);
            data = 0 | (wdg->ctrl2.reset<<4) | (wdg->ctrl2.enabled<<5) | (wdg->ctrl2.busy<<7);
        }

        wdg0151_write_datapins(wdg, data);
    }

    return delay;
}

static avr_cycle_count_t wdg0151_process_e_pinchange(struct avr_t *avr, avr_cycle_count_t when, void *param) {
    printf("wdg0151_process_e_pinchange got called.\n");
    struct wdg0151_t *wdg = (struct wdg0151_t*) param;
    uint8_t cres = wdg0151_get_cs(wdg);

    if(cres & (1<<1))
        wdg->ctrl1.reentrant = true;

    if(cres & (1<<2))
        wdg->ctrl2.reentrant = true;

    int delay = 0;
    
    if(!(wdg->pinstate & (1<<IRQ_WDG0151_RW)))
        delay = wdg0151_process_write(wdg);
    else
        delay = wdg0151_process_read(wdg);

    if(delay) {
        if(cres & (1<<1))
            wdg->ctrl1.busy = true;

        if(cres & (1<<2))
            wdg->ctrl2.busy = true;

        avr_cycle_timer_register_usec(wdg->avr, delay, wdg0151_busy_timer, wdg);
    }

    if(cres & (1<<1))
        wdg->ctrl1.reentrant = false;

    if(cres & (1<<2))
        wdg->ctrl2.reentrant = false;

    printf("Delay_avrcyclecount=%d\n",delay);

    return delay;
}

static void wdg0151_pin_changed(struct avr_irq_t *irq, uint32_t value, void *param) {
    printf("wdg0151_pin_changed got called.");
    struct wdg0151_t *wdg = (struct wdg0151_t*) param;
    uint16_t old = wdg->pinstate;

    // check if currently reading
//    if(wdg->reentrant)
//        return;

    wdg->pinstate = (wdg->pinstate & ~(1<<irq->irq)) | (value<<irq->irq);

    int eo = old & (1<<IRQ_WDG0151_E);
    int e = wdg->pinstate & (1<<IRQ_WDG0151_E);

    if(!eo && e)
        wdg0151_process_e_pinchange(wdg->avr, 0, wdg);
        //avr_cycle_timer_register(wdg->avr, 1, wdg0151_process_e_pinchange, wdg);
}

void wdg0151_init(struct avr_t *avr, struct wdg0151_t *wdg) {
    printf("wdg0151_init got called\n");
    memset(wdg, 0, sizeof(*wdg));
    wdg->avr = avr;

    wdg->irq = avr_alloc_irq(&avr->irq_pool, 0, IRQ_WDG0151_COUNT, irq_names);

    for(int i = 0; i < IRQ_WDG0151_COUNT; i++)
       avr_irq_register_notify(wdg->irq + i, wdg0151_pin_changed, wdg);
}
