/*
 * ZZScanlinesGUI - Graphical scanline control for ZZ9000
 * AmigaOS Intuition native, OS 3.1 look
 *
 * Build: m68k-amigaos-gcc -O2 -noixemul -o ZZScanlinesGUI ZZScanlinesGUI.c -lamiga
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <libraries/expansion.h>
#include <libraries/expansionbase.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/expansion.h>
#include <proto/graphics.h>
#include <graphics/gfxbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MNT_MANUFACTURER  0x6d6e
#define ZZ9000_PRODUCT_Z3 4
#define ZZ9000_PRODUCT_Z2 3
#define REG_MODE          0x100C
#define REG_PARITY        0x100E

#define GAD_MODE0   0
#define GAD_MODE1   1
#define GAD_MODE2   2
#define GAD_MODE3   3
#define GAD_PAR0    4
#define GAD_PAR1    5
#define GAD_APPLY   6
#define NUM_GADGETS 7

#define WIN_W    210
#define WIN_H    130
#define BTN_W    88
#define BTN_H    13
#define APPLY_W  70
#define APPLY_H  16
#define COL_1    8
#define COL_2    112

struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase       *GfxBase       = NULL;

/* 3D borders — raised */
static WORD hi_pts[]  = {0,BTN_H-1, 0,0, BTN_W-1,0};
static WORD lo_pts[]  = {BTN_W-1,0, BTN_W-1,BTN_H-1, 0,BTN_H-1};
static struct Border hi_brd = {0,0, 2,0, JAM1, 3, hi_pts, NULL};
static struct Border lo_brd = {0,0, 1,0, JAM1, 3, lo_pts, NULL};

/* 3D borders — pressed */
static WORD phi_pts[] = {0,BTN_H-1, 0,0, BTN_W-1,0};
static WORD plo_pts[] = {BTN_W-1,0, BTN_W-1,BTN_H-1, 0,BTN_H-1};
static struct Border phi_brd = {0,0, 1,0, JAM1, 3, phi_pts, NULL};
static struct Border plo_brd = {0,0, 2,0, JAM1, 3, plo_pts, NULL};

/* Apply button borders */
static WORD ahi_pts[] = {0,APPLY_H-1, 0,0, APPLY_W-1,0};
static WORD alo_pts[] = {APPLY_W-1,0, APPLY_W-1,APPLY_H-1, 0,APPLY_H-1};
static struct Border ahi_brd = {0,0, 2,0, JAM1, 3, ahi_pts, NULL};
static struct Border alo_brd = {0,0, 1,0, JAM1, 3, alo_pts, NULL};

static struct IntuiText mode_texts[4] = {
    {1,0,JAM1, 6,2, NULL, "Off",      NULL},
    {1,0,JAM1, 6,2, NULL, "Classic",  NULL},
    {1,0,JAM1, 6,2, NULL, "Soft",     NULL},
    {1,0,JAM1, 6,2, NULL, "Gradient", NULL},
};

static struct IntuiText par_texts[2] = {
    {1,0,JAM1, 6,2, NULL, "Odd",  NULL},
    {1,0,JAM1, 6,2, NULL, "Even", NULL},
};

static struct IntuiText apply_text = {1,0,JAM1, 15,3, NULL, "Apply", NULL};

static struct Gadget gads[NUM_GADGETS];

static ULONG find_zz9000(void) {
    struct ExpansionBase *ExpBase;
    struct ConfigDev *cd = NULL;
    ULONG addr = 0;
    ExpBase = (struct ExpansionBase *)OpenLibrary("expansion.library", 0);
    if (!ExpBase) return 0;
    cd = NULL;
    while ((cd = FindConfigDev(cd, MNT_MANUFACTURER, ZZ9000_PRODUCT_Z3)))
        { addr = (ULONG)cd->cd_BoardAddr; break; }
    if (!addr) {
        cd = NULL;
        while ((cd = FindConfigDev(cd, MNT_MANUFACTURER, ZZ9000_PRODUCT_Z2)))
            { addr = (ULONG)cd->cd_BoardAddr; break; }
    }
    CloseLibrary((struct Library *)ExpBase);
    return addr;
}

static void set_borders(struct Gadget *g, int selected, int is_apply) {
    if (is_apply) {
        ahi_brd.NextBorder = &alo_brd;
        g->GadgetRender    = (APTR)&ahi_brd;
    } else if (selected) {
        phi_brd.NextBorder = &plo_brd;
        g->GadgetRender    = (APTR)&phi_brd;
    } else {
        hi_brd.NextBorder  = &lo_brd;
        g->GadgetRender    = (APTR)&hi_brd;
    }
}

static void init_gadgets(int cur_mode, int cur_parity) {
    int i;
    memset(gads, 0, sizeof(gads));

    for (i = 0; i < 4; i++) {
        gads[i].LeftEdge   = COL_1;
        gads[i].TopEdge    = 32 + i * (BTN_H + 4);
        gads[i].Width      = BTN_W;
        gads[i].Height     = BTN_H;
        gads[i].Flags      = GADGHCOMP;
        gads[i].Activation = RELVERIFY;
        gads[i].GadgetType = BOOLGADGET;
        gads[i].GadgetText = &mode_texts[i];
        gads[i].GadgetID   = i;
        set_borders(&gads[i], (i == cur_mode), 0);
    }

    for (i = 0; i < 2; i++) {
        gads[GAD_PAR0+i].LeftEdge   = COL_2;
        gads[GAD_PAR0+i].TopEdge    = 32 + i * (BTN_H + 4);
        gads[GAD_PAR0+i].Width      = BTN_W;
        gads[GAD_PAR0+i].Height     = BTN_H;
        gads[GAD_PAR0+i].Flags      = GADGHCOMP;
        gads[GAD_PAR0+i].Activation = RELVERIFY;
        gads[GAD_PAR0+i].GadgetType = BOOLGADGET;
        gads[GAD_PAR0+i].GadgetText = &par_texts[i];
        gads[GAD_PAR0+i].GadgetID   = GAD_PAR0+i;
        set_borders(&gads[GAD_PAR0+i], (i == cur_parity), 0);
    }

    gads[GAD_APPLY].LeftEdge   = (WIN_W - APPLY_W) / 2;
    gads[GAD_APPLY].TopEdge    = WIN_H - APPLY_H - 8;
    gads[GAD_APPLY].Width      = APPLY_W;
    gads[GAD_APPLY].Height     = APPLY_H;
    gads[GAD_APPLY].Flags      = GADGHCOMP;
    gads[GAD_APPLY].Activation = RELVERIFY;
    gads[GAD_APPLY].GadgetType = BOOLGADGET;
    gads[GAD_APPLY].GadgetText = &apply_text;
    gads[GAD_APPLY].GadgetID   = GAD_APPLY;
    set_borders(&gads[GAD_APPLY], 0, 1);

    for (i = 0; i < NUM_GADGETS - 1; i++)
        gads[i].NextGadget = &gads[i+1];
    gads[NUM_GADGETS-1].NextGadget = NULL;
}

static void refresh_btn(struct Window *win, int idx, int selected) {
    RemoveGadget(win, &gads[idx]);
    set_borders(&gads[idx], selected, (idx == GAD_APPLY));
    AddGadget(win, &gads[idx], -1);
    RefreshGList(&gads[idx], win, NULL, 1);
}

static void select_mode(struct Window *win, int *cur, int nw) {
    refresh_btn(win, *cur, 0);
    refresh_btn(win, nw,   1);
    *cur = nw;
}

static void select_parity(struct Window *win, int *cur, int nw) {
    refresh_btn(win, GAD_PAR0 + *cur, 0);
    refresh_btn(win, GAD_PAR0 + nw,   1);
    *cur = nw;
}

static void draw_ui(struct Window *win) {
    struct RastPort *rp = win->RPort;
    SetAPen(rp, 1);
    SetBPen(rp, 0);
    SetDrMd(rp, JAM2);

    Move(rp, COL_1, 24); Text(rp, "Mode",   4);
    Move(rp, COL_2, 24); Text(rp, "Parity", 6);

    /* Séparateur 3D au-dessus d'Apply */
    SetAPen(rp, 1);
    Move(rp, 8,          WIN_H - 28);
    Draw(rp, WIN_W - 10, WIN_H - 28);
    SetAPen(rp, 2);
    Move(rp, 8,          WIN_H - 27);
    Draw(rp, WIN_W - 10, WIN_H - 27);
}

int main(void) {
    struct Window       *win = NULL;
    struct IntuiMessage *msg;
    struct NewWindow     nw;
    ULONG board_addr;
    int cur_mode = 0, cur_parity = 0, running = 1;

    IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 37);
    if (!IntuitionBase) { puts("No intuition.library v37+"); return 1; }
    GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 37);
    if (!GfxBase) {
        CloseLibrary((struct Library *)IntuitionBase);
        return 1;
    }

    board_addr = find_zz9000();
    if (!board_addr) {
        CloseLibrary((struct Library *)GfxBase);
        CloseLibrary((struct Library *)IntuitionBase);
        puts("ZZ9000 not found");
        return 1;
    }

    init_gadgets(cur_mode, cur_parity);

    memset(&nw, 0, sizeof(nw));
    nw.LeftEdge    = 120;
    nw.TopEdge     = 60;
    nw.Width       = WIN_W;
    nw.Height      = WIN_H;
    nw.DetailPen   = 0;
    nw.BlockPen    = 1;
    nw.Title       = (UBYTE *)"ZZ9000 Scanlines V2.2";
    nw.Flags       = WINDOWCLOSE | WINDOWDRAG | WINDOWDEPTH | ACTIVATE | NOCAREREFRESH;
    nw.IDCMPFlags  = CLOSEWINDOW | GADGETUP | REFRESHWINDOW;
    nw.Type        = WBENCHSCREEN;
    nw.FirstGadget = &gads[0];

    win = OpenWindow(&nw);
    if (!win) {
        CloseLibrary((struct Library *)GfxBase);
        CloseLibrary((struct Library *)IntuitionBase);
        puts("Cannot open window");
        return 1;
    }

    draw_ui(win);

    while (running) {
        WaitPort(win->UserPort);
        while ((msg = (struct IntuiMessage *)GetMsg(win->UserPort))) {
            ULONG cls          = msg->Class;
            struct Gadget *gad = (struct Gadget *)msg->IAddress;
            ReplyMsg((struct Message *)msg);

            if (cls == CLOSEWINDOW) {
                running = 0;
            } else if (cls == REFRESHWINDOW) {
                BeginRefresh(win);
                draw_ui(win);
                EndRefresh(win, TRUE);
            } else if (cls == GADGETUP) {
                UWORD id = gad->GadgetID;
                if (id <= GAD_MODE3) {
                    if ((int)id != cur_mode)
                        select_mode(win, &cur_mode, (int)id);
                } else if (id == GAD_PAR0 || id == GAD_PAR1) {
                    int np = id - GAD_PAR0;
                    if (np != cur_parity)
                        select_parity(win, &cur_parity, np);
                } else if (id == GAD_APPLY) {
                    volatile UWORD *rm = (volatile UWORD *)(board_addr + REG_MODE);
                    volatile UWORD *rp = (volatile UWORD *)(board_addr + REG_PARITY);
                    *rp = (UWORD)cur_parity;
                    *rm = (UWORD)cur_mode;
                }
            }
        }
    }

    CloseWindow(win);
    CloseLibrary((struct Library *)GfxBase);
    CloseLibrary((struct Library *)IntuitionBase);
    return 0;
}
