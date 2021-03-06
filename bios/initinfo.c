/*
 *  initinfo.c - Info screen at startup
 *
 * Copyright (C) 2001-2017 by Authors:
 *
 * Authors:
 *  MAD     Martin Doering
 *  PES     Petr Stehlik
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



/*
 * Well, this can be made nicer later, if we have much time... :-)
 */

/* #define ENABLE_KDEBUG */

#include "config.h"
#include "portab.h"
#include "kprint.h"
#include "nls.h"
#include "ikbd.h"
#include "asm.h"
#include "string.h"
#include "blkdev.h"     /* for BLKDEVNUM */
#include "font.h"
#include "tosvars.h"
#include "machine.h"
#include "processor.h"
#include "xbiosbind.h"
#include "biosext.h"
#include "version.h"

#include "initinfo.h"
#include "conout.h"

#ifdef ENABLE_KDEBUG
#include "lineavars.h"
#endif

#if FULL_INITINFO


/*==== Defines ============================================================*/

/* allowed values for Mxalloc mode: (defined in mem.h) */
#define MX_STRAM 0
#define MX_TTRAM 1

/*==== External declarations ==============================================*/

#if CONF_WITH_ALT_RAM
extern long total_alt_ram(void); /* in bdos/umem.c */
#endif


/*
 * set_margin - Set
 */

static void set_margin(void)
{
    WORD marl;
    WORD celx;

    marl=(v_cel_mx-34) / 2;     /* 36 = length of Logo */

    cprintf("\r");              /* goto left side */

    /* count for columns */
    for (celx = 0; celx<=marl; celx++)
        cprintf(" ");
}

/* print a line in which each char stands for the background color */
static void print_art(const char *s)
{
    int old = -1;
    int color;

    set_margin();
    while(*s) {
        color = (*s++) & 15;
        if(color != old) {
            cprintf("\033c%c", color + 32);
            old = color;
        }
        cprintf(" ");
    }
    if(old != 0) {
        cprintf("\033c ");
    }
    cprintf("\r\n");
}


static void set_line(void)
{
    WORD llen;
    WORD celx;

    llen=v_cel_mx - 6 ;     /* line length */

    cprintf("\r   ");          /* goto left side */

    /* count for columns */
    for (celx = 0; celx<=llen; celx++)
        cprintf("_");

    cprintf("\r\n");          /* goto left side */
}


static void pair_start(const char *left)
{
    int n;
    set_margin();
    n = cprintf(left);
    cprintf(": ");
    while(n++ < 14)
        cprintf(" ");
    cprintf("\033b!");
}

static void pair_end(void)
{
    cprintf("\033b/ \r\n");
}

/*
 * cprint_asctime shows boot date and time in YYYY/MM/DD HH:MM:SS format
 */

static void cprint_asctime(void)
{
    int years, months, days;
    int hours, minutes, seconds;
    ULONG system_time = Gettime(); /* Use the trap interface as a workaround
                                      to wrong Steem IKBD date after 16:00 */
    seconds = (system_time & 0x1F) * 2;
    system_time >>= 5;
    minutes = system_time & 0x3F;
    system_time >>= 6;
    hours = system_time & 0x1F;
    system_time >>= 5;
    days = system_time & 0x1F;
    system_time >>= 5;
    months = (system_time & 0x0F);
    system_time >>= 4;
    years = (system_time & 0x7F) + 1980;
    cprintf("%04d/%02d/%02d %02d:%02d:%02d", years, months, days, hours, minutes, seconds);
}

/*
 * display the available devices, with the 'dev' device highlighted
 */
static void cprint_devices(WORD dev)
{
    int i;
    LONG mask;

    cprintf("\033k\033j");  /* restore, then resave, current cursor posn */

    pair_start(_("GEMDOS drives"));

    for (i = 0, mask = 1L; i < BLKDEVNUM; i++, mask <<= 1) {
        if (drvbits & mask) {
            if (i == dev)
                cprintf("\033p");
            cprintf("%c",'A'+i);
            if (i == dev)
                cprintf("\033q");
        }
    }

    pair_end();
}

/*
 * display a size in bytes with best multiple unit
 */
static void cprintf_bytesize(ULONG bytes)
{
    const char *unit;
    ULONG value;

    if (bytes % (1024UL * 1024UL) == 0) {
        unit = _("MB");
        value = bytes / (1024UL * 1024UL);
    }
    else {
        unit = _("KB");
        value = bytes / 1024UL;
    }

    cprintf("%lu %s", value, unit);
}

/*
 * initinfo - Show initial configuration at startup
 *
 * returns the selected boot device
 */
WORD initinfo(void)
{
    int screen_height = v_cel_my + 1;
    int initinfo_height = 19; /* Define ENABLE_KDEBUG to guess correct value */
    int top_margin;
#ifdef ENABLE_KDEBUG
    int actual_initinfo_height;
#endif
    int i;
    WORD olddev = -1, dev = bootdev;
    long stramsize = (long)phystop;
#if CONF_WITH_ALT_RAM
    long altramsize = total_alt_ram();
#endif
    LONG hdd_available = blkdev_avail(HARDDISK_BOOTDEV);
    LONG shiftbits;

    /*
     * If additional info lines are going to be printed in specific cases,
     * then initinfo_height must be adjusted in the same way here.
     */
#if WITH_CLI
    initinfo_height += 1;
#endif
#if CONF_WITH_AROS
    initinfo_height += 3;
#endif
#if CONF_WITH_ALT_RAM
    if (altramsize > 0)
        initinfo_height += 1;
#endif
    if (hdd_available)
        initinfo_height += 1;

    /* Center the initinfo screen vertically */
    top_margin = (screen_height - initinfo_height) / 2;
    KDEBUG(("screen_height = %d, initinfo_height = %d, top_margin = %d\n",
        screen_height, initinfo_height, top_margin));
    for (i = 0; i < top_margin; i++)
        cprintf("\r\n");

    /* Now print the EmuTOS Logo */
    print_art("11111111111 7777777777  777   7777");
    print_art("1                  7   7   7 7    ");
    print_art("1111   1 1  1   1  7   7   7  777 ");
    print_art("1     1 1 1 1   1  7   7   7     7");
    print_art("11111 1   1  111   7    777  7777 ");

    /* Just a separator */
    set_line();
    cprintf("\n\r");

    pair_start(_("EmuTOS Version")); cprintf("%s", version); pair_end();

    pair_start(_("CPU type"));
#ifdef __mcoldfire__
    cprintf("ColdFire V4e");
#else
# if CONF_WITH_APOLLO_68080
    if (is_apollo_68080)
        cprintf("Apollo 68080");
    else
# endif
        cprintf("M680%02ld", mcpu);
#endif
    pair_end();

    pair_start(_("Machine")); cprintf(machine_name()); pair_end();
/*  pair_start(_("MMU available")); cprintf(_("No")); pair_end(); */
    pair_start("ST-RAM"); cprintf_bytesize(stramsize); pair_end();

#if CONF_WITH_ALT_RAM
    if (altramsize > 0) {
        pair_start("Alt-RAM"); cprintf_bytesize(altramsize); pair_end();
    }
#endif

    cprintf("\033j");       /* save current cursor position */
    cprint_devices(dev);

    pair_start(_("Boot time")); cprint_asctime(); pair_end();

    /* Just a separator */
    set_line();
    cprintf("\n\r");

    set_margin(); cprintf(_("Hold <Control> to skip AUTO/ACC"));
    cprintf("\r\n");
    if (hdd_available) {
        set_margin(); cprintf(_("Hold <Alternate> to skip HDD boot"));
        cprintf("\r\n");
    }
    set_margin(); cprintf(_("Press key 'X' to boot from X:"));
    cprintf("\r\n");
#if WITH_CLI
    set_margin(); cprintf(_("Press <Esc> to run an early console"));
    cprintf("\r\n");
#endif
#if CONF_WITH_AROS
    cprintf("\r\n");
    set_margin(); cprintf("\033pThis binary mixes GPL and AROS APL\033q\r\n");
    set_margin(); cprintf("\033pcode, redistribution is forbidden.\033q\r\n");
#endif
    cprintf("\r\n");
    set_margin();
    cprintf("\033p");
    cprintf(_(" Hold <Shift> to pause this screen "));
    cprintf("\033q");
#ifdef ENABLE_KDEBUG
    /* We need +1 because the previous line is not ended with CRLF */
    actual_initinfo_height = v_cur_cy + 1 - top_margin;
    if (actual_initinfo_height == initinfo_height)
        KDEBUG(("initinfo_height is correct\n"));
    else
        KDEBUG(("Warning: initinfo_height = %d, should be %d.\n",
            initinfo_height, actual_initinfo_height));
#endif

    /*
     * pause for a short while, or longer if:
     *  . a Shift key is held down, or
     *  . the user selects an alternate boot drive
     */
    while (1)
    {
        /* Wait until timeout or keypress */
        long end = hz_200 + INITINFO_DURATION * 200UL;

        olddev = dev;

        do
        {
            shiftbits = kbshift(-1);

            /* If Shift, Control, Alt or normal key is pressed, stop waiting */
            if ((shiftbits & MODE_SCA) || bconstat2())
                break;

#if USE_STOP_INSN_TO_FREE_HOST_CPU
            stop_until_interrupt();
#endif
        }
        while (hz_200 < end);

        /* Wait while Shift is pressed */
        while (shiftbits & MODE_SHIFT)
        {
#if USE_STOP_INSN_TO_FREE_HOST_CPU
            stop_until_interrupt();
#endif
            shiftbits = kbshift(-1);
        }

        /* if a non-modifier key was pressed, examine it */
        if (bconstat2()) {
            int c = LOBYTE(bconin2());

            c = toupper(c);
#if WITH_CLI
            if (c == 0x1b) {
                bootflags |= BOOTFLAG_EARLY_CLI;
            } else
#endif
            {
                c -= 'A';
                if ((c >= 0) && (c < BLKDEVNUM))
                    if (blkdev_avail(c))
                        dev = c;
            }
        }
        if (dev == olddev)
            break;
        /*
         * user has changed boot device, so we display it and go round again
         */
        cprint_devices(dev);
    }

    if (shiftbits & MODE_ALT)
        bootflags |= BOOTFLAG_SKIP_HDD_BOOT;

    if (shiftbits & MODE_CTRL)
        bootflags |= BOOTFLAG_SKIP_AUTO_ACC;

    KDEBUG(("bootflags = 0x%02x\n", bootflags));

    /*
     * on exit, restore (pop) cursor position (neatness), then
     * clear screen so that subsequent text displays are clean
     */
#if CONF_SERIAL_CONSOLE_ANSI
    /* FIXME: Quick fix until save/restore cursor works with CONF_SERIAL_CONSOLE_ANSI */
    cprintf("\r\n\r\n");
#else
    cprintf("\033k\033E");
#endif

    return dev;
}


#else    /* FULL_INITINFO */


WORD initinfo(void)
{
    cprintf("EmuTOS Version %s\r\n", version);

    return bootdev;
}


#endif   /* FULL_INITINFO */
