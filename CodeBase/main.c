#include <stdlib.h>
#include <stdio.h>

//#include <sim_avr.h>
#include <sim_hex.h>
#include <avr_ioport.h>
#include <sim_elf.h>
#include <memory.h>
#include "simulator.h"

#include "parts/wdg0151.h"
#include "parts/hd44780.h"

avr_t *avr = NULL;
wdg0151_t glcd;
hd44780_t hlcd;

//Mine(sim_elf.c)

//#include <sys/stat.h>
//#include <fcntl.h>
//#include <unistd.h>
//#include <stdlib.h>
//#include <stdio.h>
#include <string.h>
//#include <libelf.h>
//#include <gelf.h>

//#include "sim_elf.h"
#include "sim_vcd_file.h"
#include "avr_eeprom.h"
//#include "avr_ioport.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif


#define AVR_FREQ 16000000L

#ifdef EMSCRIPTEN
#include <emscripten.h>
#else
void print_bits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = b[i] & (1<<j);
            byte >>= j;
            printf("%u", byte);
        }
    }
    puts("");
}
#endif

void avr_load_firmware(avr_t * avr, elf_firmware_t * firmware)
{
   
    if (firmware->frequency)
        avr->frequency = firmware->frequency;
    if (firmware->vcc)
        avr->vcc = firmware->vcc;
    if (firmware->avcc)
        avr->avcc = firmware->avcc;
    if (firmware->aref)
        avr->aref = firmware->aref;

#if CONFIG_SIMAVR_TRACE && ELF_SYMBOLS
    int scount = firmware->flashsize >> 1;

    avr->trace_data->codeline = malloc(scount * sizeof(avr_symbol_t*));
    memset(avr->trace_data->codeline, 0, scount * sizeof(avr_symbol_t*));


    for (int i = 0; i < firmware->symbolcount; i++)
        if (firmware->symbol[i]->addr < firmware->flashsize)    // code address
            avr->trace_data->codeline[firmware->symbol[i]->addr >> 1] =
                firmware->symbol[i];
    // "spread" the pointers for known symbols forward
    avr_symbol_t * last = NULL;
    for (int i = 0; i < scount; i++) {
        if (!avr->trace_data->codeline[i])
            avr->trace_data->codeline[i] = last;
        else
            last = avr->trace_data->codeline[i];
    }
#endif

    avr_loadcode(avr, firmware->flash, firmware->flashsize, firmware->flashbase);
    avr->codeend = firmware->flashsize + firmware->flashbase - firmware->datasize;
    if (firmware->eeprom && firmware->eesize) {
        avr_eeprom_desc_t d = { .ee = firmware->eeprom, .offset = 0, .size = firmware->eesize };
        avr_ioctl(avr, AVR_IOCTL_EEPROM_SET, &d);
    }

    // load the default pull up/down values for ports
    for (int i = 0; i < 8; i++)
        if (firmware->external_state[i].port == 0)
            break;
        else {
            avr_ioport_external_t e = {
                .name = firmware->external_state[i].port,
                .mask = firmware->external_state[i].mask,
                .value = firmware->external_state[i].value,
            };
            avr_ioctl(avr, AVR_IOCTL_IOPORT_SET_EXTERNAL(e.name), &e);
        }
        
     avr_set_command_register(avr, firmware->command_register_addr);
     avr_set_console_register(avr, firmware->console_register_addr);

    // rest is initialization of the VCD file

    if (firmware->tracecount == 0)
        return;
    avr->vcd = malloc(sizeof(*avr->vcd));
    memset(avr->vcd, 0, sizeof(*avr->vcd));
    avr_vcd_init(avr,
        firmware->tracename[0] ? firmware->tracename: "gtkwave_trace.vcd",
        avr->vcd,
        firmware->traceperiod >= 1000 ? firmware->traceperiod : 1000);

    AVR_LOG(avr, LOG_TRACE, "Creating VCD trace file '%s'\n", avr->vcd->filename);
    for (int ti = 0; ti < firmware->tracecount; ti++) {
        if (firmware->trace[ti].mask == 0xff || firmware->trace[ti].mask == 0) {
            // easy one
            avr_irq_t * all = avr_iomem_getirq(avr,
                    firmware->trace[ti].addr,
                    firmware->trace[ti].name,
                    AVR_IOMEM_IRQ_ALL);
            if (!all) {
                AVR_LOG(avr, LOG_ERROR, "ELF: %s: unable to attach trace to address %04x\n",
                    __FUNCTION__, firmware->trace[ti].addr);
            } else {
                avr_vcd_add_signal(avr->vcd, all, 8, firmware->trace[ti].name);
            }
        } else {
            int count = 0;
            for (int bi = 0; bi < 8; bi++)
                if (firmware->trace[ti].mask & (1 << bi))
                    count++;
            for (int bi = 0; bi < 8; bi++)
                if (firmware->trace[ti].mask & (1 << bi)) {
                    avr_irq_t * bit = avr_iomem_getirq(avr,
                            firmware->trace[ti].addr,
                            firmware->trace[ti].name,
                            bi);
                    if (!bit) {
                        AVR_LOG(avr, LOG_ERROR, "ELF: %s: unable to attach trace to address %04x\n",
                            __FUNCTION__, firmware->trace[ti].addr);
                        break;
                    }

                    if (count == 1) {
                        avr_vcd_add_signal(avr->vcd, bit, 1, firmware->trace[ti].name);
                        break;
                    }
                    char comp[128];
                    sprintf(comp, "%s.%d", firmware->trace[ti].name, bi);
                    avr_vcd_add_signal(avr->vcd, bit, 1, firmware->trace[ti].name);
                }
        }
    }
    // if the firmware has specified a command register, do NOT start the trace here
    // the firmware probably knows best when to start/stop it
    if (!firmware->command_register_addr)
        avr_vcd_start(avr->vcd);
}


static void pin_changed_hook(struct avr_irq_t *irq, uint32_t value, void *param) {
#ifdef EMSCRIPTEN
    EM_ASM_({
            port_changed($0, $1);
            }, *((char*)param), value);
#else
    printf("pin_changed: %c, ", *((char*)param));
    print_bits(sizeof(uint8_t), &value);
#endif
}

#ifdef EMSCRIPTEN
EMSCRIPTEN_KEEPALIVE
void set_pin(char port, uint16_t pin, uint8_t value) {
    printf("set_pin: Port %c, pin: %u, value: %u\n", port, pin, value);
    avr_raise_irq(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ(port), pin), value);
}

EMSCRIPTEN_KEEPALIVE
char *get_ports() {
    return ports;
}

EMSCRIPTEN_KEEPALIVE
int run() {
    int state;
    for(int i = 0; i < 5000; i++) {
        state = avr_run(avr);

        if(state == cpu_Done || state == cpu_Crashed)
            return state;
    }

    return -1;
}
#endif

#ifndef EMSCRIPTEN
void loop() {
    for (; ;) {
        int state = avr_run(avr);
        if (state == cpu_Done || state == cpu_Crashed)
            break;
    }
    printf("i'll be back\n");
}

#endif

static void glcd_callback() {
#ifdef EMSCRIPTEN
    char data[WDG0151_WIDTH*WDG0151_HEIGHT+1];
    data[WDG0151_WIDTH*WDG0151_HEIGHT] = 0;
    
    int i = 0;
    for(int y = 0; y < WDG0151_HEIGHT; y++) {
        for(int x = 0; x < WDG0151_WIDTH; x++) {
            data[i++] = (char)glcd.ctrl1.data[y][x];
        }
    }    

    for(int y = 0; y < WDG0151_HEIGHT; y++) {
        for(int x = 0; x < WDG0151_WIDTH; x++) {
            data[i++] = (char)glcd.ctrl2.data[y][x];
        }
    }    

    EM_ASM_({
        glcd_data($0);
    }, data);
#else
    wdg0151_print(&glcd);
#endif
}

static void load_file(char *filename) {
    elf_firmware_t f;
    ihex_chunk_p chunk = NULL;
   
    int cnt = read_ihex_chunks(filename, &chunk);
    uint32_t loadBase = AVR_SEGMENT_OFFSET_FLASH;

    if (cnt <= 0) {
        fprintf(stderr, "Unable to load IHEX file %s\n",
                filename);
        exit(1);
    }

    printf("Loaded %d section of ihex\n", cnt);
    for (int ci = 0; ci < cnt; ci++) {
        if (chunk[ci].baseaddr < (1 * 1024 * 1024)) {
            f.flash = chunk[ci].data;
            f.flashsize = chunk[ci].size;
            f.flashbase = chunk[ci].baseaddr;
            printf("Load HEX flash %08x, %d\n", f.flashbase, f.flashsize);
        } else if (chunk[ci].baseaddr >= AVR_SEGMENT_OFFSET_EEPROM ||
                chunk[ci].baseaddr + loadBase >= AVR_SEGMENT_OFFSET_EEPROM){
            // TODO: implement eeprom loading
            // eeprom!
            f.eeprom = chunk[ci].data;
            f.eesize = chunk[ci].size;
            printf("Load HEX eeprom %08x, %d\n", chunk[ci].baseaddr, f.eesize);
        }
    }

    f.frequency = AVR_FREQ;
    strcpy(f.mmcu, "atmega1280");
    printf("firmware %s f=%d mmcu=%s\n", filename, (int) f.frequency, f.mmcu);

    avr = avr_make_mcu_by_name(f.mmcu);
    if (!avr) {
        fprintf(stderr, "AVR '%s' not known\n", f.mmcu);
        exit(1);
    }

    avr_init(avr);

    avr->frequency = AVR_FREQ;

    avr_load_firmware(avr, &f);
    
    avr->log = LOG_TRACE;
#ifndef EMSCRIPTEN
    avr->trace = 1;
#endif

    printf("Registering Port Callbacks:");
    for (unsigned pi = 0; pi < (sizeof(ports) / sizeof(ports[0])) -1; pi++) {
        printf(" %c", ports[pi]);
        avr_irq_t *irq = avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ(ports[pi]), IOPORT_IRQ_PIN_ALL);
        avr_irq_register_notify(irq, pin_changed_hook, &(ports[pi]));
    }

    printf("\n");

    wdg0151_init(avr, &glcd);
    hd44780_init(avr, &hlcd,256,128);

    glcd.cb = &glcd_callback;

    for(int i = 0; i < 8; i++) {
        avr_irq_t *iavr = avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('A'), i);

        avr_irq_t *ilcd = glcd.irq + IRQ_WDG0151_D0 + i;
        //avr_irq_t *ihlcd= hlcd.irq + IRQ_HD44780_D0 + i;
          avr_connect_irq(iavr, ilcd);
          avr_connect_irq(ilcd, iavr);
    }

    avr_connect_irq(
            avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('E'), 2),
            glcd.irq + IRQ_WDG0151_CS1);

    avr_connect_irq(
            avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('E'), 3),
            glcd.irq + IRQ_WDG0151_CS2);

    avr_connect_irq(
            avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('E'), 4),
            glcd.irq + IRQ_WDG0151_RS);

    avr_connect_irq(
            avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('E'), 5),
            glcd.irq + IRQ_WDG0151_RW);

    avr_connect_irq(
            avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('E'), 6),
            glcd.irq + IRQ_WDG0151_E);

    avr_connect_irq(
            avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('E'), 7),
            glcd.irq + IRQ_WDG0151_RST);
}

#ifdef EMSCRIPTEN

EMSCRIPTEN_KEEPALIVE
void init(char* filename)
{
    load_file(filename);
}
#else
void init(char* filename) {
    load_file(filename);
}
#endif

#ifndef EMSCRIPTEN
void usage(char* cmd) {
    printf("Usage: %s [hex-file]\nsimavr.js test tool\n", cmd);
    exit(1);
}

int main(int argc, char **argv) {
    if(argc < 2)
        usage(argv[0]);

    init(argv[1]);
    loop();
    return 0;
}
#endif
