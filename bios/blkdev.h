/*
 * blkdev.h - bios block devices
 *
 * Copyright (c) 2001 EmuTOS development team
 *
 * Authors:
 *  MAD   Martin Doering
 *  joy   Petr Stehlik
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#ifndef _BLKDEV_H
#define _BLKDEV_H

#include "portab.h"
#include "bios.h"


/*
 * defines
 */

#define RWABS_RETRIES   1   /* on real machine might want to increase this */

#define BLKDEVNUM   16  /* A: .. P: */
#define UNITSNUM    23  /* 2xFDC, 8xACSI, 8xSCSI, 4xIDE, 1xARAnyM */

struct bs {
  /*   0 */  UBYTE bra[2];
  /*   2 */  UBYTE loader[6];
  /*   8 */  UBYTE serial[3];
  /*   b */  UBYTE bps[2];    /* bytes per sector */
  /*   d */  UBYTE spc;       /* sectors per cluster */
  /*   e */  UBYTE res[2];    /* number of reserved sectors */
  /*  10 */  UBYTE fat;       /* number of FATs */
  /*  11 */  UBYTE dir[2];    /* number of DIR root entries */
  /*  13 */  UBYTE sec[2];    /* total number of sectors */
  /*  15 */  UBYTE media;     /* media descriptor */
  /*  16 */  UBYTE spf[2];    /* sectors per FAT */
  /*  18 */  UBYTE spt[2];    /* sectors per track */
  /*  1a */  UBYTE sides[2];  /* number of sides */
  /*  1c */  UBYTE hid[2];    /* number of hidden sectors */
  /*  1e */  UBYTE data[0x1e0];
  /* 1fe */  UBYTE cksum[2];
};

/*
 *  BPB - Bios Parameter Block
 */

struct _bpb /* bios parameter block */
{
    int     recsiz;         /* sector size in bytes */
    int     clsiz;          /* cluster size in sectors */
    int     clsizb;         /* cluster size in bytes */
    int     rdlen;          /* root directory length in records */
    int     fsiz;           /* fat size in records */
    int     fatrec;         /* first fat record (of last fat) */
    int     datrec;         /* first data record */
    int     numcl;          /* number of data clusters available */
    int     b_flags;
};

typedef struct _bpb BPB;


struct _geometry        /* disk parameter block */
{
    WORD spt;          /* number of sectors per track */
    WORD sides;        /* number of sides */
};

typedef struct _geometry GEOMETRY;


#define RW_READ             0
#define RW_WRITE            1
/* bit masks */
#define RW_RW               1
#define RW_NOMEDIACH        2
#define RW_NORETRIES        4
#define RW_NOTRANSLATE          8

/*
 *  return codes
 */

#define DEVREADY        -1L             /*  device ready                */
#define DEVNOTREADY     0L              /*  device not ready            */
#define MEDIANOCHANGE   0L              /*  media def has not changed   */
#define MEDIAMAYCHANGE  1L              /*  media may have changed      */
#define MEDIACHANGE     2L              /*  media def has changed       */





/*
 * Prototypes
 */

/* Init block device vectors */
void blkdev_init(void);

/* general block device functions */
void blkdev_hdv_init(void);
LONG blkdev_hdv_boot(void);
LONG blkdev_getbpb(WORD dev);
LONG blkdev_rwabs(WORD r_w, LONG adr, WORD numb, WORD first, WORD dev, LONG lfirst);
LONG blkdev_mediach(WORD dev);
LONG blkdev_drvmap(void);
LONG blkdev_avail(WORD dev);
UWORD compute_cksum(LONG buf);



/*
 *  flags for BPB
 */

#define B_16    1                       /* device has 16-bit FATs       */
#define B_FIX   2                       /* device has fixed media       */

/*
 * Modes of block devices
 */

#define BDM_WP_MODE            0x01    /* write-protect bit (soft) */
#define BDM_WB_MODE            0x02    /* write-back bit (soft) */
#define BDM_REMOVABLE          0x04    /* removable media */
#define BDM_LOCKABLE           0x08    /* lockable media */
#define BDM_LRECNO             0x10    /* lrecno supported */
#define BDM_WP_HARD            0x20    /* write-protected partition */


/* an idea how to assign partitions to a physical unit */
typedef struct _partition       PARTITION;
struct _partition
{
    BLKDEV          *blkdev;
    PARTITION       *next;
};

/* physical unit (floppy/harddisk) identificator */
struct _unit
{
    int     valid;      /* unit valid */
    ULONG       size;           /* number of physical sectors */
    ULONG       pssize;         /* physical sector size */
    LONG    last_access;/* used in mediach only */
    PARTITION *partitions;

};
typedef struct _unit UNIT;

#endif /* _BLKDEV_H */

