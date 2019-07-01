#ifndef WDG0151_H
#define WDG0151_H

#include <sim_irq.h>
#include <stdbool.h>

#define WDG0151_WIDTH  64
#define WDG0151_HEIGHT (64/8)

enum {
    IRQ_WDG0151_CS1 = 0,
    IRQ_WDG0151_CS2,
    IRQ_WDG0151_RS,
    IRQ_WDG0151_RW,
    IRQ_WDG0151_E,
    IRQ_WDG0151_D0,   //5
    IRQ_WDG0151_D1,
    IRQ_WDG0151_D2,
    IRQ_WDG0151_D3,
    IRQ_WDG0151_D4,
    IRQ_WDG0151_D5,
    IRQ_WDG0151_D6,
    IRQ_WDG0151_D7,   //12
    IRQ_WDG0151_RST,
    IRQ_WDG0151_COUNT
};

typedef struct wdg0151_ctrl_t {
    bool enabled;
    bool busy;
    bool reset;
    uint8_t dummy;

    bool reentrant;

    uint8_t y_addr;
    uint8_t x_addr;
    uint8_t start;
   
    uint8_t data[WDG0151_HEIGHT][WDG0151_WIDTH];
} wdg0151_ctrl_t;

typedef struct wdg0151_t {
    avr_irq_t *irq;
    struct avr_t *avr;
    
    wdg0151_ctrl_t ctrl1;
    wdg0151_ctrl_t ctrl2;
 
    uint8_t datapins;
    uint16_t pinstate;
   
    void (*cb)(void);
} wdg0151_t;

void wdg0151_init(struct avr_t *avr, struct wdg0151_t *wdg);
void wdg0151_print(struct wdg0151_t *wdg);

#endif
