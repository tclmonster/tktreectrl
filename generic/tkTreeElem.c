/*
 * tkTreeElem.c --
 *
 *	This module implements elements for treectrl widgets.
 *
 * Copyright (c) 2002-2011 Tim Baker
 */

#include "tkTreeCtrl.h"
#include "tkTreeElem.h"

/* When a column header is in the pressed state, any bitmap, image, and text
 * elements are offset by 1,1. */
#define HEADER_OFFSET_HACK 1

/*
 *----------------------------------------------------------------------
 *
 * DO_BooleanForState --
 * DO_ColorForState --
 * DO_FontForState --
 *
 *	Returns the value of a per-state option for an element.
 *
 * Results:
 *	If the element has the dynamic option allocated, then the
 *	per-state info is checked for a match. If an exact match for
 *	the given state is not found, and if the element is an instance
 *	(not a master), then the master element is checked.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
DO_BooleanForState(
    TreeCtrl *tree,		/* Widget info. */
    TreeElement elem,		/* Element to examine. */
    int id,			/* Unique id of dynamic option. */
    int state			/* STATE_xxx flags. */
    )
{
    int result = -1;
    PerStateInfo *psi;
    int match = MATCH_NONE;

    psi = DynamicOption_FindData(elem->options, id);
    if (psi != NULL)
	result = PerStateBoolean_ForState(tree, psi, state, &match);
    if ((match != MATCH_EXACT) && (elem->master != NULL)) {
	PerStateInfo *psi = DynamicOption_FindData(elem->master->options, id);
	if (psi != NULL) {
	    int matchM;
	    int resultM = PerStateBoolean_ForState(tree, psi, state, &matchM);
	    if (matchM > match)
		result = resultM;
	}
    }
    return result;
}

static TreeColor *
DO_ColorForState(
    TreeCtrl *tree,
    TreeElement elem,
    int id,
    int state
    )
{
    TreeColor *result = NULL;
    PerStateInfo *psi;
    int match = MATCH_NONE;

    psi = DynamicOption_FindData(elem->options, id);
    if (psi != NULL)
	result = PerStateColor_ForState(tree, psi, state, &match);
    if ((match != MATCH_EXACT) && (elem->master != NULL)) {
	PerStateInfo *psi = DynamicOption_FindData(elem->master->options, id);
	if (psi != NULL) {
	    int matchM;
	    TreeColor *resultM = PerStateColor_ForState(tree, psi, state, &matchM);
	    if (matchM > match)
		result = resultM;
	}
    }
    return result;
}

static Tk_Font
DO_FontForState(
    TreeCtrl *tree,
    TreeElement elem,
    int id,
    int state
    )
{
    Tk_Font result = NULL;
    PerStateInfo *psi;
    int match = MATCH_NONE;

    psi = DynamicOption_FindData(elem->options, id);
    if (psi != NULL)
	result = PerStateFont_ForState(tree, psi, state, &match);
    if ((match != MATCH_EXACT) && (elem->master != NULL)) {
	PerStateInfo *psi = DynamicOption_FindData(elem->master->options, id);
	if (psi != NULL) {
	    int matchM;
	    Tk_Font resultM = PerStateFont_ForState(tree, psi, state, &matchM);
	    if (matchM > match)
		result = resultM;
	}
    }
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * DO_ObjectForState --
 *
 *	Returns the object representation of a per-state option
 *	for an element.
 *
 * Results:
 *	If the element has the dynamic option allocated, then the
 *	per-state info is checked for a match. If an exact match for
 *	the given state is not found, and if the element is an instance
 *	(not a master), then the master element is checked.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static Tcl_Obj *
DO_ObjectForState(
    TreeCtrl *tree,		/* Widget info. */
    PerStateType *typePtr,	/* Type-specific functions and values. */
    TreeElement elem,		/* Element to examine. */
    int id,			/* Unique id of dynamic option. */
    int state			/* STATE_xxx flags. */
    )
{
    Tcl_Obj *result = NULL;
    PerStateInfo *psi;
    int match = MATCH_NONE;

    psi = DynamicOption_FindData(elem->options, id);
    if (psi != NULL)
	result = PerStateInfo_ObjForState(tree, typePtr, psi, state, &match);
    if ((match != MATCH_EXACT) && (elem->master != NULL)) {
	PerStateInfo *psi = DynamicOption_FindData(elem->master->options, id);
	if (psi != NULL) {
	    int matchM;
	    Tcl_Obj *resultM = PerStateInfo_ObjForState(tree, typePtr, psi, state, &matchM);
	    if (matchM > match)
		result = resultM;
	}
    }
    return result;
}

/* BEGIN custom "boolean" option */

/* Just like TK_OPTION_BOOLEAN but supports TK_OPTION_NULL_OK */
/* Internal value is -1 for no-such-value */

static int BooleanSet(
    ClientData clientData,
    Tcl_Interp *interp,
    Tk_Window tkwin,
    Tcl_Obj **value,
    char *recordPtr,
    Tcl_Size internalOffset,
    char *saveInternalPtr,
    int flags)
{
    int objEmpty;
    int new, *internalPtr;

    if (internalOffset >= 0)
	internalPtr = (int *) (recordPtr + internalOffset);
    else
	internalPtr = NULL;

    objEmpty = ObjectIsEmpty((*value));

    if ((flags & TK_OPTION_NULL_OK) && objEmpty)
	(*value) = NULL;
    else {
	if (Tcl_GetBooleanFromObj(interp, (*value), &new) != TCL_OK)
	    return TCL_ERROR;
    }
    if (internalPtr != NULL) {
	if ((*value) == NULL)
	    new = -1;
	*((int *) saveInternalPtr) = *internalPtr;
	*internalPtr = new;
    }

    return TCL_OK;
}

static Tcl_Obj *BooleanGet(
    ClientData clientData,
    Tk_Window tkwin,
    char *recordPtr,
    Tcl_Size internalOffset)
{
    int value = *(int *) (recordPtr + internalOffset);
    if (value == -1)
	return NULL;
    return Tcl_NewBooleanObj(value);
}

static void BooleanRestore(
    ClientData clientData,
    Tk_Window tkwin,
    char *internalPtr,
    char *saveInternalPtr)
{
    *(int *) internalPtr = *(int *) saveInternalPtr;
}

static Tk_ObjCustomOption booleanCO =
{
    "boolean",
    BooleanSet,
    BooleanGet,
    BooleanRestore,
    NULL,
    (ClientData) NULL
};

static void
DynamicOptionInitBoolean(
    void *data
    )
{
    *((int *) data) = -1;
}

/* END custom "boolean" option */

/* BEGIN custom "integer" option */

/* Just like TK_OPTION_INT but supports TK_OPTION_NULL_OK and bounds checking */

typedef struct IntegerClientData
{
    int min;
    int max;
    int empty; /* internal form if empty */
    int flags; /* 0x01 - use min, 0x02 - use max */
} IntegerClientData;

static int IntegerSet(
    ClientData clientData,
    Tcl_Interp *interp,
    Tk_Window tkwin,
    Tcl_Obj **value,
    char *recordPtr,
    Tcl_Size internalOffset,
    char *saveInternalPtr,
    int flags)
{
    IntegerClientData *cd = clientData;
    int objEmpty;
    int new, *internalPtr;

    if (internalOffset >= 0)
	internalPtr = (int *) (recordPtr + internalOffset);
    else
	internalPtr = NULL;

    objEmpty = ObjectIsEmpty((*value));

    if ((flags & TK_OPTION_NULL_OK) && objEmpty)
	(*value) = NULL;
    else {
	if (Tcl_GetIntFromObj(interp, (*value), &new) != TCL_OK)
	    return TCL_ERROR;
	if ((cd->flags & 0x01) && (new < cd->min)) {
	    FormatResult(interp,
		    "bad integer value \"%d\": must be >= %d",
		    new, cd->min);
	    return TCL_ERROR;
	}
	if ((cd->flags & 0x02) && (new > cd->max)) {
	    FormatResult(interp,
		    "bad integer value \"%d\": must be <= %d",
		    new, cd->max);
	    return TCL_ERROR;
	}
    }
    if (internalPtr != NULL) {
	if ((*value) == NULL)
	    new = cd->empty;
	*((int *) saveInternalPtr) = *internalPtr;
	*internalPtr = new;
    }

    return TCL_OK;
}

static Tcl_Obj *IntegerGet(
    ClientData clientData,
    Tk_Window tkwin,
    char *recordPtr,
    Tcl_Size internalOffset)
{
    IntegerClientData *cd = clientData;
    int value = *(int *) (recordPtr + internalOffset);
    if (value == cd->empty)
	return NULL;
    return Tcl_NewIntObj(value);
}

static void IntegerRestore(
    ClientData clientData,
    Tk_Window tkwin,
    char *internalPtr,
    char *saveInternalPtr)
{
    *(int *) internalPtr = *(int *) saveInternalPtr;
}

/* END custom "integer" option */

/*****/

/* BEGIN custom "stringtable" option */

/* Just like TK_OPTION_STRING_TABLE but supports TK_OPTION_NULL_OK */
/* The integer rep is -1 if empty string specified */

typedef struct StringTableClientData
{
    const char **tablePtr; /* NULL-termintated list of strings */
    const char *msg; /* Tcl_GetIndexFromObj() message */
} StringTableClientData;

static int StringTableSet(
    ClientData clientData,
    Tcl_Interp *interp,
    Tk_Window tkwin,
    Tcl_Obj **value,
    char *recordPtr,
    Tcl_Size internalOffset,
    char *saveInternalPtr,
    int flags)
{
    StringTableClientData *cd = clientData;
    int objEmpty;
    int new, *internalPtr;

    if (internalOffset >= 0)
	internalPtr = (int *) (recordPtr + internalOffset);
    else
	internalPtr = NULL;

    objEmpty = ObjectIsEmpty((*value));

    if ((flags & TK_OPTION_NULL_OK) && objEmpty)
	(*value) = NULL;
    else {
	if (Tcl_GetIndexFromObj(interp, (*value), cd->tablePtr,
		    cd->msg, 0, &new) != TCL_OK)
	    return TCL_ERROR;
    }
    if (internalPtr != NULL) {
	if ((*value) == NULL)
	    new = -1;
	*((int *) saveInternalPtr) = *internalPtr;
	*internalPtr = new;
    }

    return TCL_OK;
}

static Tcl_Obj *StringTableGet(
    ClientData clientData,
    Tk_Window tkwin,
    char *recordPtr,
    Tcl_Size internalOffset)
{
    StringTableClientData *cd = clientData;
    int index = *(int *) (recordPtr + internalOffset);

    if (index == -1)
	return NULL;
    return Tcl_NewStringObj(cd->tablePtr[index], -1);
}

static void StringTableRestore(
    ClientData clientData,
    Tk_Window tkwin,
    char *internalPtr,
    char *saveInternalPtr)
{
    *(int *) internalPtr = *(int *) saveInternalPtr;
}

/* END custom "stringtable" option */

static int
BooleanCO_Init(
    Tk_OptionSpec *optionTable,
    const char *optionName)
{
    Tk_OptionSpec *specPtr;

    specPtr = Tree_FindOptionSpec(optionTable, optionName);
    specPtr->clientData = &booleanCO;
    return TCL_OK;
}

static Tk_ObjCustomOption *
IntegerCO_Alloc(
    const char *optionName,
    int min,
    int max,
    int empty,
    int flags
    )
{
    IntegerClientData *cd;
    Tk_ObjCustomOption *co;

    /* ClientData for the Tk custom option record */
    cd = (IntegerClientData *) ckalloc(sizeof(IntegerClientData));
    cd->min = min;
    cd->max = max;
    cd->empty = empty;
    cd->flags = flags;

    /* The Tk custom option record */
    co = (Tk_ObjCustomOption *) ckalloc(sizeof(Tk_ObjCustomOption));
    co->name = (char *) optionName + 1;
    co->setProc = IntegerSet;
    co->getProc = IntegerGet;
    co->restoreProc = IntegerRestore;
    co->freeProc = NULL;
    co->clientData = (ClientData) cd;

    return co;
}

#if 0 /* UNUSED, keep but shutup compiler warning */
static int
IntegerCO_Init(
    Tk_OptionSpec *optionTable,
    const char *optionName,
    int min,
    int max,
    int empty,
    int flags
    )
{
    Tk_OptionSpec *specPtr;

    specPtr = Tree_FindOptionSpec(optionTable, optionName);
    if (specPtr->type != TK_OPTION_CUSTOM)
	Tcl_Panic("IntegerCO_Init: %s is not TK_OPTION_CUSTOM", optionName);
    if (specPtr->clientData != NULL)
	return TCL_OK;

    specPtr->clientData = IntegerCO_Alloc(optionName, min, max, empty, flags);

    return TCL_OK;
}
#endif

static Tk_ObjCustomOption *
StringTableCO_Alloc(
    const char *optionName,
    const char **tablePtr
    )
{
    StringTableClientData *cd;
    Tk_ObjCustomOption *co;

    /* ClientData for the Tk custom option record */
    cd = (StringTableClientData *) ckalloc(sizeof(StringTableClientData));
    cd->tablePtr = tablePtr;
    cd->msg = optionName + 1;

    /* The Tk custom option record */
    co = (Tk_ObjCustomOption *) ckalloc(sizeof(Tk_ObjCustomOption));
    co->name = (char *) optionName + 1;
    co->setProc = StringTableSet;
    co->getProc = StringTableGet;
    co->restoreProc = StringTableRestore;
    co->freeProc = NULL;
    co->clientData = (ClientData) cd;

    return co;
}

int StringTableCO_Init(Tk_OptionSpec *optionTable, const char *optionName, const char **tablePtr)
{
    Tk_OptionSpec *specPtr;

    specPtr = Tree_FindOptionSpec(optionTable, optionName);
    if (specPtr->type != TK_OPTION_CUSTOM)
	Tcl_Panic("StringTableCO_Init: %s is not TK_OPTION_CUSTOM", optionName);
    if (specPtr->clientData != NULL)
	return TCL_OK;

    specPtr->clientData = StringTableCO_Alloc(optionName, tablePtr);

    return TCL_OK;
}

/*****/

int TreeStateFromObj(TreeCtrl *tree, int domain, Tcl_Obj *obj, int *stateOff, int *stateOn)
{
    int states[3];

    states[STATE_OP_ON] = states[STATE_OP_OFF] = states[STATE_OP_TOGGLE] = 0;
    if (Tree_StateFromObj(tree, domain, obj, states, NULL, SFO_NOT_TOGGLE) != TCL_OK)
	return TCL_ERROR;

    (*stateOn) |= states[STATE_OP_ON];
    (*stateOff) |= states[STATE_OP_OFF];
    return TCL_OK;
}

static void AdjustForSticky(int sticky, int cavityWidth, int cavityHeight,
    int expandX, int expandY,
    int *xPtr, int *yPtr, int *widthPtr, int *heightPtr)
{
    int dx = 0;
    int dy = 0;

    if (cavityWidth > *widthPtr) {
	dx = cavityWidth - *widthPtr;
    }

    if (cavityHeight > *heightPtr) {
	dy = cavityHeight - *heightPtr;
    }

    if ((sticky & STICKY_W) && (sticky & STICKY_E)) {
	if (expandX)
	    *widthPtr += dx;
	else
	    sticky &= ~(STICKY_W | STICKY_E);
    }
    if ((sticky & STICKY_N) && (sticky & STICKY_S)) {
	if (expandY)
	    *heightPtr += dy;
	else
	    sticky &= ~(STICKY_N | STICKY_S);
    }
    if (!(sticky & STICKY_W)) {
	*xPtr += (sticky & STICKY_E) ? dx : dx / 2;
    }
    if (!(sticky & STICKY_N)) {
	*yPtr += (sticky & STICKY_S) ? dy : dy / 2;
    }
}

#define DO_COLOR_FOR_STATE(xVAR,xID,xSTATE) \
    { \
	TreeColor *tc = DO_ColorForState(tree, elem, xID, xSTATE); \
	xVAR = (tc != NULL) ? tc->color : NULL; \
    }

/* This macro gets the value of a per-state option for an element, then
 * looks for a better match from the master element if it exists */
#define OPTION_FOR_STATE(xFUNC,xTYPE,xVAR,xFIELD,xSTATE) \
    xVAR = xFUNC(tree, &elemX->xFIELD, xSTATE, &match); \
    if ((match != MATCH_EXACT) && (masterX != NULL)) { \
	xTYPE varM = xFUNC(tree, &masterX->xFIELD, xSTATE, &match2); \
	if (match2 > match) \
	    xVAR = varM; \
    }
#define BITMAP_FOR_STATE(xVAR,xFIELD,xSTATE) \
    OPTION_FOR_STATE(PerStateBitmap_ForState,Pixmap,xVAR,xFIELD,xSTATE)
#define BOOLEAN_FOR_STATE(xVAR,xFIELD,xSTATE) \
    OPTION_FOR_STATE(PerStateBoolean_ForState,int,xVAR,xFIELD,xSTATE)
#define BORDER_FOR_STATE(xVAR,xFIELD,xSTATE) \
    OPTION_FOR_STATE(PerStateBorder_ForState,Tk_3DBorder,xVAR,xFIELD,xSTATE)
#define COLOR_FOR_STATE(xVAR,xFIELD,xSTATE) \
    { \
	TreeColor *tc; \
	OPTION_FOR_STATE(PerStateColor_ForState,TreeColor*,tc,xFIELD,xSTATE) \
	xVAR = (tc != NULL) ? tc->color : NULL; \
    }
#define TREECOLOR_FOR_STATE(xVAR,xFIELD,xSTATE) \
    OPTION_FOR_STATE(PerStateColor_ForState,TreeColor*,xVAR,xFIELD,xSTATE)
#define FLAGS_FOR_STATE(xVAR,xFIELD,xSTATE) \
    OPTION_FOR_STATE(PerStateFlags_ForState,int,xVAR,xFIELD,xSTATE)
#define FONT_FOR_STATE(xVAR,xFIELD,xSTATE) \
    OPTION_FOR_STATE(PerStateFont_ForState,Tk_Font,xVAR,xFIELD,xSTATE)
#define GRADIENT_FOR_STATE(xVAR,xFIELD,xSTATE) \
    OPTION_FOR_STATE(PerStateGradient_ForState,TreeGradient,xVAR,xFIELD,xSTATE)
#define IMAGE_FOR_STATE(xVAR,xFIELD,xSTATE) \
    OPTION_FOR_STATE(PerStateImage_ForState,Tk_Image,xVAR,xFIELD,xSTATE)
#define RELIEF_FOR_STATE(xVAR,xFIELD,xSTATE) \
    OPTION_FOR_STATE(PerStateRelief_ForState,int,xVAR,xFIELD,xSTATE)

/* This macro gets the object for a per-state option for an element, then
 * looks for a better match from the master element if it exists */
#define OBJECT_FOR_STATE(xVAR,xTYPE,xFIELD,xSTATE) \
    xVAR = PerStateInfo_ObjForState(tree, &xTYPE, &elemX->xFIELD, xSTATE, &match); \
    if ((match != MATCH_EXACT) && (masterX != NULL)) { \
	Tcl_Obj *objM = PerStateInfo_ObjForState(tree, &xTYPE, &masterX->xFIELD, xSTATE, &matchM); \
	if (matchM > match) \
	    xVAR = objM; \
    }

/*****/

typedef struct ElementBitmap ElementBitmap;

struct ElementBitmap
{
    TreeElement_ header;
#ifdef DEPRECATED
    PerStateInfo draw;
#endif
    PerStateInfo bitmap;
    PerStateInfo fg;
    PerStateInfo bg;
};

#define BITMAP_CONF_BITMAP 0x0001
#define BITMAP_CONF_FG 0x0002
#define BITMAP_CONF_BG 0x0004
#ifdef DEPRECATED
#define BITMAP_CONF_DRAW 0x0008
#endif

static Tk_OptionSpec bitmapOptionSpecs[] = {
    {TK_OPTION_CUSTOM, "-background", (char *) NULL, (char *) NULL,
     (char *) NULL,
     offsetof(ElementBitmap, bg.obj), offsetof(ElementBitmap, bg),
     TK_OPTION_NULL_OK, (ClientData) NULL, BITMAP_CONF_BG},
    {TK_OPTION_CUSTOM, "-bitmap", (char *) NULL, (char *) NULL,
     (char *) NULL,
     offsetof(ElementBitmap, bitmap.obj), offsetof(ElementBitmap, bitmap),
     TK_OPTION_NULL_OK, (ClientData) NULL, BITMAP_CONF_BITMAP},
#ifdef DEPRECATED
    {TK_OPTION_CUSTOM, "-draw", (char *) NULL, (char *) NULL,
     (char *) NULL,
     offsetof(ElementBitmap, draw.obj), offsetof(ElementBitmap, draw),
     TK_OPTION_NULL_OK, (ClientData) NULL, BITMAP_CONF_DRAW},
#endif
    {TK_OPTION_CUSTOM, "-foreground", (char *) NULL, (char *) NULL,
     (char *) NULL,
     offsetof(ElementBitmap, fg.obj), offsetof(ElementBitmap, fg),
     TK_OPTION_NULL_OK, (ClientData) NULL, BITMAP_CONF_FG},
    {TK_OPTION_END, (char *) NULL, (char *) NULL, (char *) NULL,
     (char *) NULL, 0, -1, 0, (ClientData) NULL, 0}
};

static void DeleteProcBitmap(TreeElementArgs *args)
{
/*    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementBitmap *elemX = (ElementBitmap *) elem;*/
}

static int WorldChangedProcBitmap(TreeElementArgs *args)
{
    int flagM = args->change.flagMaster;
    int flagS = args->change.flagSelf;
    int mask = 0;

    if ((flagS | flagM) & BITMAP_CONF_BITMAP)
	mask |= CS_DISPLAY | CS_LAYOUT;

    if ((flagS | flagM) & (
#ifdef DEPRECATED
	    BITMAP_CONF_DRAW |
#endif
	    BITMAP_CONF_FG | BITMAP_CONF_BG))
	mask |= CS_DISPLAY;

    return mask;
}

static int ConfigProcBitmap(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementBitmap *elemX = (ElementBitmap *) elem;
    Tk_SavedOptions savedOptions;
    int error;
    Tcl_Obj *errorResult = NULL;

    for (error = 0; error <= 1; error++) {
	if (error == 0) {
	    if (Tree_SetOptions(tree, elem->stateDomain, elemX,
			elem->typePtr->optionTable,
			args->config.objc, args->config.objv,
			&savedOptions, &args->config.flagSelf) != TCL_OK) {
		args->config.flagSelf = 0;
		continue;
	    }

	    Tk_FreeSavedOptions(&savedOptions);
	    break;
	} else {
	    errorResult = Tcl_GetObjResult(tree->interp);
	    Tcl_IncrRefCount(errorResult);
	    Tk_RestoreSavedOptions(&savedOptions);

	    Tcl_SetObjResult(tree->interp, errorResult);
	    Tcl_DecrRefCount(errorResult);
	    return TCL_ERROR;
	}
    }

    return TCL_OK;
}

static int CreateProcBitmap(TreeElementArgs *args)
{
    return TCL_OK;
}

static void DisplayProcBitmap(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementBitmap *elemX = (ElementBitmap *) elem;
    ElementBitmap *masterX = (ElementBitmap *) elem->master;
    int state = args->state;
    int x = args->display.x, y = args->display.y;
    int width, height;
    int match, match2;
#ifdef DEPRECATED
    int draw;
#endif
    Pixmap bitmap;
    XColor *fg, *bg;
    int imgW, imgH;
    int inHeader = elem->stateDomain == STATE_DOMAIN_HEADER;
    int columnState = COLUMN_STATE_NORMAL;

#ifdef DEPRECATED
    BOOLEAN_FOR_STATE(draw, draw, state)
    if (!draw)
	return;
#endif

    BITMAP_FOR_STATE(bitmap, bitmap, state)
    if (bitmap == None)
	return;

    COLOR_FOR_STATE(fg, fg, state)
    COLOR_FOR_STATE(bg, bg, state)

    Tk_SizeOfBitmap(tree->display, bitmap, &imgW, &imgH);
    width = imgW, height = imgH;
    AdjustForSticky(args->display.sticky,
	args->display.width, args->display.height,
	FALSE, FALSE,
	&x, &y, &width, &height);
#if HEADER_OFFSET_HACK == 1
    if (inHeader) {
	if (state & STATE_HEADER_ACTIVE)
	    columnState = COLUMN_STATE_ACTIVE;
	else if (state & STATE_HEADER_PRESSED)
	    columnState = COLUMN_STATE_PRESSED;
    }
    if (inHeader && columnState == COLUMN_STATE_PRESSED) {
	/* If this bitmap fills the whole header, don't offset it. */
	/* FIXME: should have a layout option to control this. */
	if (imgW < args->display.spanBbox.width ||
		imgH < args->display.spanBbox.height) {
	    x += 1, y += 1;
	}
    }
#endif
    if (imgW > args->display.width)
	imgW = args->display.width;
    if (imgH > args->display.height)
	imgH = args->display.height;
    Tree_DrawBitmap(tree, bitmap, args->display.drawable, fg, bg,
	0, 0, (unsigned int) imgW, (unsigned int) imgH,
	x, y);
}

static void NeededProcBitmap(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementBitmap *elemX = (ElementBitmap *) elem;
    ElementBitmap *masterX = (ElementBitmap *) elem->master;
    int state = args->state;
    int width = 0, height = 0;
    int match, match2;
    Pixmap bitmap;

    BITMAP_FOR_STATE(bitmap, bitmap, state)

    if (bitmap != None)
	Tk_SizeOfBitmap(tree->display, bitmap, &width, &height);

    args->needed.width = width;
    args->needed.height = height;
}

static int StateProcBitmap(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementBitmap *elemX = (ElementBitmap *) elem;
    ElementBitmap *masterX = (ElementBitmap *) elem->master;
    int match, match2;
#ifdef DEPRECATED
    int draw1, draw2;
#endif
    Pixmap bitmap1, bitmap2;
    XColor *fg1, *fg2;
    XColor *bg1, *bg2;

    if (!args->states.visible2)
	return 0;

    BITMAP_FOR_STATE(bitmap1, bitmap, args->states.state1)
    BITMAP_FOR_STATE(bitmap2, bitmap, args->states.state2)
    if (bitmap1 != bitmap2) {
	if ((bitmap1 != None) && (bitmap2 != None)) {
	    int w1, h1, w2, h2;
	    Tk_SizeOfBitmap(tree->display, bitmap1, &w1, &h1);
	    Tk_SizeOfBitmap(tree->display, bitmap2, &w2, &h2);
	    if ((w1 != w2) || (h1 != h2))
		return CS_DISPLAY | CS_LAYOUT;
	    return CS_DISPLAY;
	}
	return CS_DISPLAY | CS_LAYOUT;
    }

    /* Layout hasn't changed, and -draw layout option is false. */
    if (!args->states.draw2)
	return 0;
#ifdef DEPRECATED
    BOOLEAN_FOR_STATE(draw1, draw, args->states.state1)
    BOOLEAN_FOR_STATE(draw2, draw, args->states.state2)
    if ((draw1 != 0) != (draw2 != 0))
	return CS_DISPLAY;
    if (draw2 == 0)
	return 0;
#endif

    COLOR_FOR_STATE(fg1, fg, args->states.state1)
    COLOR_FOR_STATE(fg2, fg, args->states.state2)
    if (fg1 != fg2)
	return CS_DISPLAY;

    COLOR_FOR_STATE(bg1, bg, args->states.state1)
    COLOR_FOR_STATE(bg2, bg, args->states.state2)
    if (bg1 != bg2)
	return CS_DISPLAY;

    return 0;
}

static int UndefProcBitmap(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementBitmap *elemX = (ElementBitmap *) elem;
    int modified = 0;

#ifdef DEPRECATED
    modified |= PerStateInfo_Undefine(tree, &pstBoolean, &elemX->draw, elem->stateDomain, args->state);
#endif
    modified |= PerStateInfo_Undefine(tree, &pstColor, &elemX->fg, elem->stateDomain, args->state);
    modified |= PerStateInfo_Undefine(tree, &pstColor, &elemX->bg, elem->stateDomain, args->state);
    modified |= PerStateInfo_Undefine(tree, &pstBitmap, &elemX->bitmap, elem->stateDomain, args->state);
    return modified;
}

static int ActualProcBitmap(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    ElementBitmap *elemX = (ElementBitmap *) args->elem;
    ElementBitmap *masterX = (ElementBitmap *) args->elem->master;
    static const char *optionName[] = {
	"-background", "-bitmap",
#ifdef DEPRECATED
	"-draw",
#endif
	"-foreground",
	(char *) NULL };
    int index, match, matchM;
    Tcl_Obj *obj = NULL;

    if (Tcl_GetIndexFromObj(tree->interp, args->actual.obj, optionName,
		"option", 0, &index) != TCL_OK)
	return TCL_ERROR;

    switch (index) {
	case 0: {
	    OBJECT_FOR_STATE(obj, pstColor, bg, args->state)
	    break;
	}
	case 1: {
	    OBJECT_FOR_STATE(obj, pstBitmap, bitmap, args->state)
	    break;
	}
#ifdef DEPRECATED
	case 2: {
	    OBJECT_FOR_STATE(obj, pstBoolean, draw, args->state)
	    break;
	}
	case 3: {
	    OBJECT_FOR_STATE(obj, pstColor, fg, args->state)
	    break;
	}
#else
	case 2: {
	    OBJECT_FOR_STATE(obj, pstColor, fg, args->state)
	    break;
	}
#endif
    }
    if (obj != NULL)
	Tcl_SetObjResult(tree->interp, obj);
    return TCL_OK;
}

TreeElementType treeElemTypeBitmap = {
    "bitmap",
    sizeof(ElementBitmap),
    bitmapOptionSpecs,
    NULL,
    CreateProcBitmap,
    DeleteProcBitmap,
    ConfigProcBitmap,
    DisplayProcBitmap,
    NeededProcBitmap,
    NULL, /* heightProc */
    WorldChangedProcBitmap,
    StateProcBitmap,
    UndefProcBitmap,
    ActualProcBitmap,
    NULL /* onScreenProc */
};

/*****/

typedef struct ElementBorder ElementBorder;

struct ElementBorder
{
    TreeElement_ header; /* Must be first */
#ifdef DEPRECATED
    PerStateInfo draw;
#endif
    PerStateInfo border;
    PerStateInfo relief;
    int thickness;
    Tcl_Obj *thicknessObj;
    int width;
    Tcl_Obj *widthObj;
    int height;
    Tcl_Obj *heightObj;
    int filled;
};

#define BORDER_CONF_BG 0x0001
#define BORDER_CONF_RELIEF 0x0002
#define BORDER_CONF_SIZE 0x0004
#define BORDER_CONF_THICKNESS 0x0008
#define BORDER_CONF_FILLED 0x0010
#ifdef DEPRECATED
#define BORDER_CONF_DRAW 0x0020
#endif

static Tk_OptionSpec borderOptionSpecs[] = {
    {TK_OPTION_CUSTOM, "-background", (char *) NULL, (char *) NULL,
     (char *) NULL,
     offsetof(ElementBorder, border.obj), offsetof(ElementBorder, border),
     TK_OPTION_NULL_OK, (ClientData) NULL, BORDER_CONF_BG},
#ifdef DEPRECATED
    {TK_OPTION_CUSTOM, "-draw", (char *) NULL, (char *) NULL,
     (char *) NULL,
     offsetof(ElementBorder, draw.obj), offsetof(ElementBorder, draw),
     TK_OPTION_NULL_OK, (ClientData) NULL, BORDER_CONF_DRAW},
#endif
    {TK_OPTION_CUSTOM, "-filled", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(ElementBorder, filled),
     TK_OPTION_NULL_OK, (ClientData) &booleanCO, BORDER_CONF_FILLED},
    {TK_OPTION_PIXELS, "-height", (char *) NULL, (char *) NULL,
     (char *) NULL, offsetof(ElementBorder, heightObj),
     offsetof(ElementBorder, height),
     TK_OPTION_NULL_OK, (ClientData) NULL, BORDER_CONF_SIZE},
    {TK_OPTION_CUSTOM, "-relief", (char *) NULL, (char *) NULL,
     (char *) NULL,
     offsetof(ElementBorder, relief.obj), offsetof(ElementBorder, relief),
     TK_OPTION_NULL_OK, (ClientData) NULL, BORDER_CONF_RELIEF},
    {TK_OPTION_PIXELS, "-thickness", (char *) NULL, (char *) NULL,
     (char *) NULL, offsetof(ElementBorder, thicknessObj),
     offsetof(ElementBorder, thickness),
     TK_OPTION_NULL_OK, (ClientData) NULL, BORDER_CONF_THICKNESS},
    {TK_OPTION_PIXELS, "-width", (char *) NULL, (char *) NULL,
     (char *) NULL, offsetof(ElementBorder, widthObj),
     offsetof(ElementBorder, width),
     TK_OPTION_NULL_OK, (ClientData) NULL, BORDER_CONF_SIZE},
    {TK_OPTION_END, (char *) NULL, (char *) NULL, (char *) NULL,
     (char *) NULL, 0, -1, 0, (ClientData) NULL, 0}
};

static void DeleteProcBorder(TreeElementArgs *args)
{
/*    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementBorder *elemX = (ElementBorder *) elem;*/
}

static int WorldChangedProcBorder(TreeElementArgs *args)
{
    int flagM = args->change.flagMaster;
    int flagS = args->change.flagSelf;
    int mask = 0;

    if ((flagS | flagM) & BORDER_CONF_SIZE)
	mask |= CS_DISPLAY | CS_LAYOUT;

    if ((flagS | flagM) & (
#ifdef DEPRECATED
	    BORDER_CONF_DRAW |
#endif
	    BORDER_CONF_BG | BORDER_CONF_RELIEF | BORDER_CONF_THICKNESS |
	    BORDER_CONF_FILLED))
	mask |= CS_DISPLAY;

    return mask;
}

static int ConfigProcBorder(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementBorder *elemX = (ElementBorder *) elem;
    Tk_SavedOptions savedOptions;
    int error;
    Tcl_Obj *errorResult = NULL;

    for (error = 0; error <= 1; error++) {
	if (error == 0) {
	    if (Tree_SetOptions(tree, elem->stateDomain, elemX,
			elem->typePtr->optionTable,
			args->config.objc, args->config.objv,
			&savedOptions, &args->config.flagSelf) != TCL_OK) {
		args->config.flagSelf = 0;
		continue;
	    }

	    Tk_FreeSavedOptions(&savedOptions);
	    break;
	} else {
	    errorResult = Tcl_GetObjResult(tree->interp);
	    Tcl_IncrRefCount(errorResult);
	    Tk_RestoreSavedOptions(&savedOptions);

	    Tcl_SetObjResult(tree->interp, errorResult);
	    Tcl_DecrRefCount(errorResult);
	    return TCL_ERROR;
	}
    }

    return TCL_OK;
}

static int CreateProcBorder(TreeElementArgs *args)
{
    TreeElement elem = args->elem;
    ElementBorder *elemX = (ElementBorder *) elem;

    elemX->filled = -1;
    return TCL_OK;
}

static void DisplayProcBorder(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementBorder *elemX = (ElementBorder *) elem;
    ElementBorder *masterX = (ElementBorder *) elem->master;
    int state = args->state;
    int x = args->display.x, y = args->display.y;
    int width = args->display.width, height = args->display.height;
    int match, match2;
#ifdef DEPRECATED
    int draw;
#endif
    Tk_3DBorder border;
    int relief, filled = FALSE;
    int thickness = 0;
#if USE_ITEM_PIXMAP == 0
    TreeClip clip;
#endif
    TreeRectangle tr1, tr2;

#ifdef DEPRECATED
    BOOLEAN_FOR_STATE(draw, draw, state)
    if (!draw)
	return;
#endif

    BORDER_FOR_STATE(border, border, state)
    if (border == NULL)
	return;

    RELIEF_FOR_STATE(relief, relief, state)
    if (relief == TK_RELIEF_NULL)
	relief = TK_RELIEF_FLAT;

    if (elemX->thicknessObj)
	thickness = elemX->thickness;
    else if ((masterX != NULL) && (masterX->thicknessObj != NULL))
	thickness = masterX->thickness;

    if (elemX->filled != -1)
	filled = elemX->filled;
    else if ((masterX != NULL) && (masterX->filled != -1))
	filled = masterX->filled;

    if (elemX->widthObj != NULL)
	width = elemX->width;
    else if ((masterX != NULL) && (masterX->widthObj != NULL))
	width = masterX->width;

    if (elemX->heightObj != NULL)
	height = elemX->height;
    else if ((masterX != NULL) && (masterX->heightObj != NULL))
	height = masterX->height;

    AdjustForSticky(args->display.sticky,
	args->display.width, args->display.height,
	TRUE, TRUE,
	&x, &y, &width, &height);

    /* X11 coordinates are 16-bit. */
    TreeRect_SetXYWH(tr1, x, y, width, height);
    TreeRect_SetXYWH(tr2, -thickness, -thickness,
	args->display.td.width + thickness * 2,
	args->display.td.height + thickness * 2);
    TreeRect_Intersect(&tr1, &tr1, &tr2);

#if USE_ITEM_PIXMAP == 0
    /* Using a region instead of a rect because of how Tree_Fill3DRectangle
     * and Tree_Draw3DRectangle are implemented. */
    clip.type = TREE_CLIP_REGION;
    clip.region = Tree_GetRectRegion(tree, &args->display.bounds);

    if (filled) {
	Tree_Fill3DRectangle(tree, args->display.td, &clip, border,
		tr1.x, tr1.y, tr1.width, tr1.height, thickness, relief);
    } else if (thickness > 0) {
	Tree_Draw3DRectangle(tree, args->display.td, &clip, border,
		tr1.x, tr1.y, tr1.width, tr1.height, thickness, relief);
    }

    Tree_FreeRegion(tree, clip.region);
#else
    if (filled) {
	Tk_Fill3DRectangle(tree->tkwin, args->display.drawable, border,
		tr1.x, tr1.y, tr1.width, tr1.height, thickness, relief);
    } else if (thickness > 0) {
	Tk_Draw3DRectangle(tree->tkwin, args->display.drawable, border,
		tr1.x, tr1.y, tr1.width, tr1.height, thickness, relief);
    }
#endif
}

static void NeededProcBorder(TreeElementArgs *args)
{
    TreeElement elem = args->elem;
    ElementBorder *elemX = (ElementBorder *) elem;
    ElementBorder *masterX = (ElementBorder *) elem->master;
    int width = 0, height = 0;

    if (elemX->widthObj != NULL)
	width = elemX->width;
    else if ((masterX != NULL) && (masterX->widthObj != NULL))
	width = masterX->width;

    if (elemX->heightObj != NULL)
	height = elemX->height;
    else if ((masterX != NULL) && (masterX->heightObj != NULL))
	height = masterX->height;

    args->needed.width = width;
    args->needed.height = height;
}

static int StateProcBorder(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementBorder *elemX = (ElementBorder *) elem;
    ElementBorder *masterX = (ElementBorder *) elem->master;
    int match, match2;
#ifdef DEPRECATED
    int draw1, draw2;
#endif
    Tk_3DBorder border1, border2;
    int relief1, relief2;

    if (!args->states.visible2 || !args->states.draw2)
	return 0;

#ifdef DEPRECATED
    BOOLEAN_FOR_STATE(draw1, draw, args->states.state1)
    BOOLEAN_FOR_STATE(draw2, draw, args->states.state2)
    if ((draw1 != 0) != (draw2 != 0))
	return CS_DISPLAY;
    if (draw2 == 0)
	return 0;
#endif

    BORDER_FOR_STATE(border1, border, args->states.state1)
    BORDER_FOR_STATE(border2, border, args->states.state2)
    if (border1 != border2)
	return CS_DISPLAY;

    RELIEF_FOR_STATE(relief1, relief, args->states.state1)
    RELIEF_FOR_STATE(relief2, relief, args->states.state2)
    if (relief1 != relief2)
	return CS_DISPLAY;

    return 0;
}

static int UndefProcBorder(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementBorder *elemX = (ElementBorder *) elem;
    int modified = 0;

#ifdef DEPRECATED
    modified |= PerStateInfo_Undefine(tree, &pstBoolean, &elemX->draw, elem->stateDomain, args->state);
#endif
    modified |= PerStateInfo_Undefine(tree, &pstBorder, &elemX->border, elem->stateDomain, args->state);
    modified |= PerStateInfo_Undefine(tree, &pstRelief, &elemX->relief, elem->stateDomain, args->state);
    return modified;
}

static int ActualProcBorder(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    ElementBorder *elemX = (ElementBorder *) args->elem;
    ElementBorder *masterX = (ElementBorder *) args->elem->master;
    static const char *optionName[] = {
	"-background",
#ifdef DEPRECATED
	"-draw",
#endif
	"-relief",
	(char *) NULL };
    int index, match, matchM;
    Tcl_Obj *obj = NULL;

    if (Tcl_GetIndexFromObj(tree->interp, args->actual.obj, optionName,
		"option", 0, &index) != TCL_OK)
	return TCL_ERROR;

    switch (index) {
	case 0: {
	    OBJECT_FOR_STATE(obj, pstBorder, border, args->state)
	    break;
	}
#ifdef DEPRECATED
	case 1: {
	    OBJECT_FOR_STATE(obj, pstBoolean, draw, args->state)
	    break;
	}
	case 2: {
	    OBJECT_FOR_STATE(obj, pstRelief, relief, args->state)
	    break;
	}
#else
	case 1: {
	    OBJECT_FOR_STATE(obj, pstRelief, relief, args->state)
	    break;
	}
#endif
    }
    if (obj != NULL)
	Tcl_SetObjResult(tree->interp, obj);
    return TCL_OK;
}

TreeElementType treeElemTypeBorder = {
    "border",
    sizeof(ElementBorder),
    borderOptionSpecs,
    NULL,
    CreateProcBorder,
    DeleteProcBorder,
    ConfigProcBorder,
    DisplayProcBorder,
    NeededProcBorder,
    NULL, /* heightProc */
    WorldChangedProcBorder,
    StateProcBorder,
    UndefProcBorder,
    ActualProcBorder,
    NULL /* onScreenProc */
};

/*****/
#if 0

static const char *chkbutStateST[] = {
    "checked", "mixed", "normal", "active", "pressed", "disabled", (char *) NULL
};

typedef struct ElementCheckButton ElementCheckButton;

struct ElementCheckButton
{
    TreeElement_ header;
    PerStateInfo image;
    int state;
};

#define CHKBUT_CONF_IMAGE 0x0001
#define CHKBUT_CONF_STATE 0x0002

static Tk_OptionSpec chkbutOptionSpecs[] = {
    {TK_OPTION_STRING, "-image", (char *) NULL, (char *) NULL,
     (char *) NULL, offsetof(ElementCheckButton, image.obj), -1,
     TK_OPTION_NULL_OK, (ClientData) NULL, CHKBUT_CONF_IMAGE},
    {TK_OPTION_STRING_TABLE, "-state", (char *) NULL, (char *) NULL,
     "normal", -1, offsetof(ElementCheckButton, state),
     0, (ClientData) chkbutStateST, CHKBUT_CONF_STATE},
    {TK_OPTION_END, (char *) NULL, (char *) NULL, (char *) NULL,
     (char *) NULL, 0, -1, 0, (ClientData) NULL, 0}
};

static void DeleteProcCheckButton(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementCheckButton *elemX = (ElementCheckButton *) elem;

    PerStateInfo_Free(tree, &pstImage, &elemX->image);
}

static int WorldChangedProcCheckButton(TreeElementArgs *args)
{
    int flagM = args->change.flagMaster;
    int flagS = args->change.flagSelf;
    int mask = 0;

    if ((flagS | flagM) & (CHKBUT_CONF_IMAGE | CHKBUT_CONF_STATE))
	mask |= CS_DISPLAY | CS_LAYOUT;

    return mask;
}

static int ChkButStateFromObj(TreeCtrl *tree, Tcl_Obj *obj, int *stateOff, int *stateOn)
{
    Tcl_Interp *interp = tree->interp;
    int i, op = STATE_OP_ON, op2, op3, length, state = 0;
    char ch0, *string;
    int states[3];

    states[STATE_OP_ON] = 0;
    states[STATE_OP_OFF] = 0;
    states[STATE_OP_TOGGLE] = 0;

    string = Tcl_GetStringFromObj(obj, &length);
    if (length == 0)
	goto unknown;
    ch0 = string[0];
    if (ch0 == '!') {
	op = STATE_OP_OFF;
	++string;
	ch0 = string[0];
    } else if (ch0 == '~') {
	if (1) {
	    FormatResult(interp, "can't specify '~' for this command");
	    return TCL_ERROR;
	}
	op = STATE_OP_TOGGLE;
	++string;
	ch0 = string[0];
    }
    for (i = 0; chkbutStateST[i] != NULL; i++) {
	if ((ch0 == chkbutStateST[i][0]) && !strcmp(string, chkbutStateST[i])) {
	    state = 1L << i;
	    break;
	}
    }
    if (state == 0)
	goto unknown;

    if (op == STATE_OP_ON) {
	op2 = STATE_OP_OFF;
	op3 = STATE_OP_TOGGLE;
    }
    else if (op == STATE_OP_OFF) {
	op2 = STATE_OP_ON;
	op3 = STATE_OP_TOGGLE;
    } else {
	op2 = STATE_OP_ON;
	op3 = STATE_OP_OFF;
    }
    states[op2] &= ~state;
    states[op3] &= ~state;
    states[op] |= state;

    *stateOn |= states[STATE_OP_ON];
    *stateOff |= states[STATE_OP_OFF];

    return TCL_OK;

unknown:
    FormatResult(interp, "unknown state \"%s\"", string);
    return TCL_ERROR;
}

static int ConfigProcCheckButton(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementCheckButton *elemX = (ElementCheckButton *) elem;
    ElementCheckButton savedX;
    Tk_SavedOptions savedOptions;
    int error;
    Tcl_Obj *errorResult = NULL;

    for (error = 0; error <= 1; error++) {
	if (error == 0) {
	    if (Tree_SetOptions(tree, tree, elem->stateDomain,
			elem->typePtr->optionTable,
			args->config.objc, args->config.objv,
			&savedOptions, &args->config.flagSelf) != TCL_OK) {
		args->config.flagSelf = 0;
		continue;
	    }

	    if (args->config.flagSelf & CHKBUT_CONF_IMAGE)
		PSTSave(&elemX->image, &savedX.image);

	    if (args->config.flagSelf & CHKBUT_CONF_IMAGE) {
		if (PerStateInfo_FromObj(tree, ChkButStateFromObj, &pstImage, &elemX->image) != TCL_OK)
		    continue;
	    }

	    if (args->config.flagSelf & CHKBUT_CONF_IMAGE)
		PerStateInfo_Free(tree, &pstImage, &savedX.image);
	    Tk_FreeSavedOptions(&savedOptions);
	    break;
	} else {
	    errorResult = Tcl_GetObjResult(tree->interp);
	    Tcl_IncrRefCount(errorResult);
	    Tk_RestoreSavedOptions(&savedOptions);

	    if (args->config.flagSelf & CHKBUT_CONF_IMAGE)
		PSTRestore(tree, &pstImage, &elemX->image, &savedX.image);

	    Tcl_SetObjResult(tree->interp, errorResult);
	    Tcl_DecrRefCount(errorResult);
	    return TCL_ERROR;
	}
    }

    return TCL_OK;
}

static int CreateProcCheckButton(TreeElementArgs *args)
{
    return TCL_OK;
}

static void DisplayProcCheckButton(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementCheckButton *elemX = (ElementCheckButton *) elem;
    ElementCheckButton *masterX = (ElementCheckButton *) elem->master;
    int state = args->state;
    int match, matchM;
    Tk_Image image;
    int imgW, imgH;
    int dx = 0, dy = 0;

    image = PerStateImage_ForState(tree, &elemX->image, state, &match);
    if ((match != MATCH_EXACT) && (masterX != NULL)) {
	Tk_Image imageM = PerStateImage_ForState(tree, &masterX->image,
		state, &matchM);
	if (matchM > match)
	    image = imageM;
    }

    if (image != NULL) {
	Tk_SizeOfImage(image, &imgW, &imgH);
	if (imgW < args->display.width)
	    dx = (args->display.width - imgW) / 2;
	else if (imgW > args->display.width)
	    imgW = args->display.width;
	if (imgH < args->display.height)
	    dy = (args->display.height - imgH) / 2;
	else if (imgH > args->display.height)
	    imgH = args->display.height;
	Tk_RedrawImage(image, 0, 0, imgW, imgH, args->display.drawable,
		args->display.x + dx,
		args->display.y + dy);
    }
}

static void NeededProcCheckButton(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementCheckButton *elemX = (ElementCheckButton *) elem;
    ElementCheckButton *masterX = (ElementCheckButton *) elem->master;
    int state = args->state;
    int match, match2;
    Tk_Image image;
    int width = 0, height = 0;

    image = PerStateImage_ForState(tree, &elemX->image, state, &match);
    if ((match != MATCH_EXACT) && (masterX != NULL)) {
	Tk_Image image2 = PerStateImage_ForState(tree, &masterX->image,
		state, &match2);
	if (match2 > match)
	    image = image2;
    }

    if (image != NULL)
	Tk_SizeOfImage(image, &width, &height);

    args->layout.width = width;
    args->layout.height = height;
}

static int StateProcCheckButton(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementCheckButton *elemX = (ElementCheckButton *) elem;
    ElementCheckButton *masterX = (ElementCheckButton *) elem->master;
    int match, match2;
    Tk_Image image1, image2;
    int mask = 0;

    image1 = PerStateImage_ForState(tree, &elemX->image,
	    args->states.state1, &match);
    if ((match != MATCH_EXACT) && (masterX != NULL)) {
	Tk_Image image = PerStateImage_ForState(tree, &masterX->image, args->states.state1, &match2);
	if (match2 > match)
	    image1 = image;
    }

    image2 = PerStateImage_ForState(tree, &elemX->image,
	    args->states.state2, &match);
    if ((match != MATCH_EXACT) && (masterX != NULL)) {
	Tk_Image image = PerStateImage_ForState(tree, &masterX->image,
		args->states.state2, &match2);
	if (match2 > match)
	    image2 = image;
    }

    if (image1 != image2) {
	mask |= CS_DISPLAY;
	if ((image1 != NULL) && (image2 != NULL)) {
	    int w1, h1, w2, h2;
	    Tk_SizeOfImage(image1, &w1, &h1);
	    Tk_SizeOfImage(image2, &w2, &h2);
	    if ((w1 != w2) || (h1 != h2))
		mask |= CS_LAYOUT;
	} else
	    mask |= CS_LAYOUT;
    }

    return mask;
}

static int UndefProcCheckButton(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    ElementCheckButton *elemX = (ElementCheckButton *) args->elem;

    return PerStateInfo_Undefine(tree, &pstImage, &elemX->image, args->state);
}

static int ActualProcCheckButton(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    ElementCheckButton *elemX = (ElementCheckButton *) args->elem;
    ElementCheckButton *masterX = (ElementCheckButton *) args->elem->master;
    static const char *optionName[] = {
	"-image",
	(char *) NULL };
    int index, match, matchM;
    Tcl_Obj *obj = NULL;

    if (Tcl_GetIndexFromObj(tree->interp, args->actual.obj, optionName,
		"option", 0, &index) != TCL_OK)
	return TCL_ERROR;

    switch (index) {
	case 0: {
	    obj = PerStateInfo_ObjForState(tree, &pstImage,
		    &elemX->image, args->state, &match);
	    if ((match != MATCH_EXACT) && (masterX != NULL)) {
		objM = PerStateInfo_ObjForState(tree, &pstImage,
			&masterX->image, args->state, &matchM);
		if (matchM > match)
		    obj = objM;
	    }
	    break;
	}
    }
    if (obj != NULL)
	Tcl_SetObjResult(tree->interp, obj);
    return TCL_OK;
}

TreeElementType treeElemTypeCheckButton = {
    "checkbutton",
    sizeof(ElementCheckButton),
    chkbutOptionSpecs,
    NULL,
    CreateProcCheckButton,
    DeleteProcCheckButton,
    ConfigProcCheckButton,
    DisplayProcCheckButton,
    NeededProcCheckButton,
    NULL, /* heightProc */
    WorldChangedProcCheckButton,
    StateProcCheckButton,
    UndefProcCheckButton,
    ActualProcCheckButton,
    NULL /* onScreenProc */
};

#endif

/*****/

typedef struct ElementHeader ElementHeader;

struct ElementHeader
{
    TreeElement_ header; /* Must be first */
    PerStateInfo border;	/* -background */
    int borderWidth;		/* -borderwidth */
    Tcl_Obj *borderWidthObj;	/* -borderwidth */
    PerStateInfo arrowBitmap;	/* -arrowbitmap */
    PerStateInfo arrowImage;	/* -arrowimage */
    Tcl_Obj *arrowPadXObj;	/* -arrowpadx */
    int *arrowPadX;		/* -arrowpadx */
    Tcl_Obj *arrowPadYObj;	/* -arrowpady */
    int *arrowPadY;		/* -arrowpady */
    int arrow;			/* -arrow */
#define SIDE_LEFT 0
#define SIDE_RIGHT 1
    int arrowSide;		/* -arrowside */
    int arrowGravity;		/* -arrowgravity */
    int state;			/* -state */
};

static const char *headerStateST[] = { "normal", "active", "pressed", (char *) NULL };
static const char *headerArrowST[] = { "none", "up", "down", (char *) NULL };
static const char *headerArrowSideST[] = { "left", "right", (char *) NULL };

#define HEADER_CONF_SIZE 0x0001
#define HEADER_CONF_DISPLAY 0x0002

static Tk_OptionSpec headerOptionSpecs[] = {
    {TK_OPTION_CUSTOM, "-arrow", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(ElementHeader, arrow), TK_OPTION_NULL_OK,
     (ClientData) NULL, HEADER_CONF_DISPLAY | HEADER_CONF_SIZE},
    {TK_OPTION_CUSTOM, "-arrowbitmap", (char *) NULL, (char *) NULL,
     (char *) NULL,
     offsetof(ElementHeader, arrowBitmap.obj), offsetof(ElementHeader, arrowBitmap),
     TK_OPTION_NULL_OK, (ClientData) NULL, HEADER_CONF_DISPLAY | HEADER_CONF_SIZE},
    {TK_OPTION_CUSTOM, "-arrowgravity", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(ElementHeader, arrowGravity), TK_OPTION_NULL_OK,
     (ClientData) NULL, HEADER_CONF_DISPLAY | HEADER_CONF_SIZE},
    {TK_OPTION_CUSTOM, "-arrowimage", (char *) NULL, (char *) NULL,
     (char *) NULL,
     offsetof(ElementHeader, arrowImage.obj), offsetof(ElementHeader, arrowImage),
     TK_OPTION_NULL_OK, (ClientData) NULL, HEADER_CONF_DISPLAY | HEADER_CONF_SIZE},
    {TK_OPTION_CUSTOM, "-arrowpadx", (char *) NULL, (char *) NULL,
     (char *) NULL, offsetof(ElementHeader, arrowPadXObj), offsetof(ElementHeader, arrowPadX),
     TK_OPTION_NULL_OK, (ClientData) &TreeCtrlCO_pad, HEADER_CONF_DISPLAY | HEADER_CONF_SIZE},
    {TK_OPTION_CUSTOM, "-arrowpady", (char *) NULL, (char *) NULL,
     (char *) NULL, offsetof(ElementHeader, arrowPadYObj), offsetof(ElementHeader, arrowPadY),
     TK_OPTION_NULL_OK, (ClientData) &TreeCtrlCO_pad, HEADER_CONF_DISPLAY | HEADER_CONF_SIZE},
    {TK_OPTION_CUSTOM, "-arrowside", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(ElementHeader, arrowSide), TK_OPTION_NULL_OK,
     (ClientData) NULL, HEADER_CONF_DISPLAY | HEADER_CONF_SIZE},
    {TK_OPTION_CUSTOM, "-background", (char *) NULL, (char *) NULL,
     (char *) NULL,
     offsetof(ElementHeader, border.obj), offsetof(ElementHeader, border),
     TK_OPTION_NULL_OK, (ClientData) NULL, HEADER_CONF_DISPLAY},
    {TK_OPTION_PIXELS, "-borderwidth", (char *) NULL, (char *) NULL,
     (char *) NULL, offsetof(ElementHeader, borderWidthObj),
     offsetof(ElementHeader, borderWidth),
     TK_OPTION_NULL_OK, (ClientData) NULL, HEADER_CONF_SIZE | HEADER_CONF_DISPLAY},
    {TK_OPTION_CUSTOM, "-state", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(ElementHeader, state), TK_OPTION_NULL_OK,
     (ClientData) NULL, HEADER_CONF_DISPLAY},
    {TK_OPTION_END, (char *) NULL, (char *) NULL, (char *) NULL,
     (char *) NULL, 0, -1, 0, (ClientData) NULL, 0}
};

static void DeleteProcHeader(TreeElementArgs *args)
{
/*    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementHeader *elemX = (ElementHeader *) elem;*/
}

static int WorldChangedProcHeader(TreeElementArgs *args)
{
    int flagT = args->change.flagTree;
    int flagM = args->change.flagMaster;
    int flagS = args->change.flagSelf;
    int mask = 0;

    if (flagT & TREE_CONF_THEME)
	mask |= CS_DISPLAY | CS_LAYOUT;
    else if ((flagS | flagM) & HEADER_CONF_SIZE)
	mask |= CS_DISPLAY | CS_LAYOUT;
    else if ((flagS | flagM) & HEADER_CONF_DISPLAY)
	mask |= CS_DISPLAY;

    return mask;
}

static int ConfigProcHeader(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementHeader *elemX = (ElementHeader *) elem;
    Tk_SavedOptions savedOptions;
    int error;
    Tcl_Obj *errorResult = NULL;

    for (error = 0; error <= 1; error++) {
	if (error == 0) {
	    if (Tree_SetOptions(tree, elem->stateDomain, elemX,
			elem->typePtr->optionTable,
			args->config.objc, args->config.objv,
			&savedOptions, &args->config.flagSelf) != TCL_OK) {
		args->config.flagSelf = 0;
		continue;
	    }

	    Tk_FreeSavedOptions(&savedOptions);
	    break;
	} else {
	    errorResult = Tcl_GetObjResult(tree->interp);
	    Tcl_IncrRefCount(errorResult);
	    Tk_RestoreSavedOptions(&savedOptions);

	    Tcl_SetObjResult(tree->interp, errorResult);
	    Tcl_DecrRefCount(errorResult);
	    return TCL_ERROR;
	}
    }

    return TCL_OK;
}

static int CreateProcHeader(TreeElementArgs *args)
{
    TreeElement elem = args->elem;
    ElementHeader *elemX = (ElementHeader *) elem;

    elemX->arrow = -1;
    elemX->arrowGravity = -1;
    elemX->arrowSide = -1;
    elemX->state = -1;

    return TCL_OK;
}

struct HeaderParams
{
    int state;		/* COLUMN_STATE_XXX */
    int arrow;		/* COLUMN_ARROW_XXX */
    int borderWidth;
    int margins[4];	/* content margins, not including arrow */
    int elemState;	/* STATE_XXX */
    int eUnionBbox[4];
    int iUnionBbox[4];
};

static void
HeaderGetParams(
    TreeCtrl *tree,
    ElementHeader *elemX,
    int state,
    struct HeaderParams *params
    )
{
    ElementHeader *masterX = (ElementHeader *) elemX->header.master;
    int i;

    params->elemState = state;

    params->state = COLUMN_STATE_NORMAL;
    if (elemX->state != -1)
	params->state = elemX->state;
    else if (masterX != NULL && masterX->state != -1)
	params->state = masterX->state;
    else if (elemX->header.stateDomain == STATE_DOMAIN_HEADER) {
	if (state & STATE_HEADER_ACTIVE) params->state = COLUMN_STATE_ACTIVE;
	if (state & STATE_HEADER_PRESSED) params->state = COLUMN_STATE_PRESSED;
    }

    params->arrow = COLUMN_ARROW_NONE;
    if (elemX->arrow != -1)
	params->arrow = elemX->arrow;
    else if (masterX != NULL && masterX->arrow != -1)
	params->arrow = masterX->arrow;
    else if (elemX->header.stateDomain == STATE_DOMAIN_HEADER) {
	if (state & STATE_HEADER_SORT_UP) params->arrow = COLUMN_ARROW_UP;
	if (state & STATE_HEADER_SORT_DOWN) params->arrow = COLUMN_ARROW_DOWN;
    }

    if (elemX->borderWidthObj)
	params->borderWidth = elemX->borderWidth;
    else if ((masterX != NULL) && (masterX->borderWidthObj != NULL))
	params->borderWidth = masterX->borderWidth;
    else
	params->borderWidth = 2; /* Column header -borderwidth defaults to 2. */
    if (params->borderWidth < 0)
	params->borderWidth = 2;

    if (tree->useTheme &&
	    (TreeTheme_GetHeaderContentMargins(tree, params->state,
	    params->arrow, params->margins) == TCL_OK)) {
#ifdef WIN32
	/* I'm hacking these margins since the default XP theme does not give
	 * reasonable ContentMargins for HP_HEADERITEM */
	int bw = MAX(params->borderWidth, 3);
	params->margins[1] = MAX(params->margins[1], bw);
	params->margins[3] = MAX(params->margins[3], bw);
#endif /* WIN32 */
    } else {
	params->margins[0] = params->margins[2] = 0;
	params->margins[1] = params->margins[3] = params->borderWidth;
    }

    for (i = 0; i < 4; i++)
	params->eUnionBbox[i] = params->iUnionBbox[i] = -1;
}

struct ArrowLayout {
    int arrow;
    int arrowSide;
    int x;
    int y;
    int width;
    int height;
    int padX[2];
    int padY[2];
};

static void
HeaderLayoutArrow(
    TreeCtrl *tree,
    ElementHeader *elemX,
    struct HeaderParams *params,
    int x, int y, int width, int height, /* bounds of whole element */
    int indent,
    struct ArrowLayout *layout
    )
{
    ElementHeader *masterX = (ElementHeader *) elemX->header.master;
    int state = params->elemState;
    int arrowSide, arrowWidth = -1, arrowGravity, arrowHeight;
    Tk_Image image;
    Pixmap bitmap;
    int defPadX[2] = {6, 6}, defPadY[2] = {0, 0}, *arrowPadX, *arrowPadY;
    int match, match2;
    int minX, maxX, padX[2];
#ifdef MAC_OSX_TK
    int margins[4];
#endif

    layout->arrow = params->arrow;
    if (layout->arrow == COLUMN_ARROW_NONE) {
	return;
    }

    arrowSide = elemX->arrowSide;
    if (arrowSide == -1 && masterX != NULL)
	arrowSide = masterX->arrowSide;
    if (arrowSide == -1)
	arrowSide = SIDE_RIGHT;

    arrowGravity = elemX->arrowGravity;
    if (arrowGravity == -1 && masterX != NULL)
	arrowGravity = masterX->arrowGravity;
    if (arrowGravity == -1)
	arrowGravity = SIDE_LEFT;

    arrowPadX = elemX->arrowPadX;
    if (arrowPadX == NULL && masterX != NULL) {
	arrowPadX = masterX->arrowPadX;
    }
    if (arrowPadX == NULL) {
	arrowPadX = defPadX;
    }

    arrowPadY = elemX->arrowPadY;
    if (arrowPadY == NULL && masterX != NULL) {
	arrowPadY = masterX->arrowPadY;
    }
    if (arrowPadY == NULL) {
	arrowPadY = defPadY;
    }

#ifdef MAC_OSX_TK
    /* Under Aqua, the Appearance Manager draws the sort arrow as part of
    * the header background. */
    if (tree->useTheme && TreeTheme_GetHeaderContentMargins(tree,
	    params->state, params->arrow, margins) == TCL_OK) {
	layout->arrowSide = SIDE_RIGHT;
	layout->width = margins[2];
	layout->x = width - layout->width;
	layout->y = 0;
	layout->height = 1; /* bogus value */
	/* The content margins do not include padding on the left of the
	 * sort arrow. */
	layout->padX[PAD_TOP_LEFT] = arrowPadX[PAD_TOP_LEFT];
	layout->padX[PAD_BOTTOM_RIGHT] = 0;
	layout->padY[PAD_TOP_LEFT] = layout->padY[PAD_BOTTOM_RIGHT] = 0;
	return;
    }
#endif

    if (arrowWidth == -1) {
	IMAGE_FOR_STATE(image, arrowImage, state);
	if (image != NULL) {
	    Tk_SizeOfImage(image, &arrowWidth, &arrowHeight);
	}
    }

    if (arrowWidth == -1) {
	BITMAP_FOR_STATE(bitmap, arrowBitmap, state);
	if (bitmap != None) {
	    Tk_SizeOfBitmap(tree->display, bitmap, &arrowWidth, &arrowHeight);
	}
    }

    if ((arrowWidth == -1) && tree->useTheme &&
	    TreeTheme_GetArrowSize(tree, Tk_WindowId(tree->tkwin),
	    params->arrow == COLUMN_ARROW_UP, &arrowWidth, &arrowHeight) == TCL_OK) {
	/* nothing */
    }

    if (arrowWidth == -1) {
#if 0
	/* This is the original calculation of arrow size. */
	Tk_Font tkfont = column->tkfont ? column->tkfont : tree->tkfont;
	Tk_FontMetrics fm;
	Tk_GetFontMetrics(tkfont, &fm);
	arrowWidth = (fm.linespace + column->textPadY[PAD_TOP_LEFT] +
	    column->textPadY[PAD_BOTTOM_RIGHT] + column->borderWidth * 2) / 2;
	if (!(arrowWidth & 1))
	    arrowWidth--;
	arrowHeight = arrowWidth;
#endif
	arrowWidth = 9; /* FIXME: -sortarrowwidth */
	arrowHeight = arrowWidth;
    }

    minX = x + indent;
    maxX = x + width;
    padX[PAD_TOP_LEFT] = padX[PAD_BOTTOM_RIGHT] = 0;
    if (arrowSide == SIDE_LEFT) {
	if (params->iUnionBbox[0] != -1) {
	    maxX = x + params->iUnionBbox[0];
	    padX[PAD_TOP_LEFT] = params->iUnionBbox[0] - params->eUnionBbox[0];
	}
    } else {
	if (params->iUnionBbox[2] != -1) {
	    minX = x + params->iUnionBbox[2];
	    padX[PAD_BOTTOM_RIGHT] = params->eUnionBbox[2] - params->iUnionBbox[2];
	}
    }

    if (arrowGravity == SIDE_LEFT) {
	layout->x = minX + MAX(padX[PAD_BOTTOM_RIGHT], arrowPadX[PAD_TOP_LEFT]);
	layout->x = MIN(layout->x, (x + width) - arrowPadX[PAD_BOTTOM_RIGHT] - arrowWidth);
    } else {
	layout->x = maxX - MAX(padX[PAD_TOP_LEFT], arrowPadX[PAD_BOTTOM_RIGHT]) - arrowWidth;
    }

    /* Don't let the arrow go too far left when the column is very narrow. */
    layout->x = MAX(layout->x, x + indent + arrowPadX[PAD_TOP_LEFT]);

    layout->width = arrowWidth;

    layout->y = y + (height - (arrowHeight + arrowPadY[PAD_TOP_LEFT] +
	arrowPadY[PAD_BOTTOM_RIGHT])) / 2 + arrowPadY[PAD_TOP_LEFT];
    layout->height = arrowHeight;

    layout->arrowSide = arrowSide;
    layout->padX[0] = arrowPadX[0];
    layout->padX[1] = arrowPadX[1];
    layout->padY[0] = arrowPadY[0];
    layout->padY[1] = arrowPadY[1];
}

static void
HeaderDrawArrow(
    TreeElementArgs *args,
    struct HeaderParams *params,
    int x, int y, int width, int height, /* bounds of whole element */
    int indent
    )
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementHeader *elemX = (ElementHeader *) elem;
    ElementHeader *masterX = (ElementHeader *) elem->master;
    Tk_Image image;
    Pixmap bitmap;
    Tk_3DBorder border;
    int state = params->elemState;
    int sunken = params->state == COLUMN_STATE_PRESSED;
    struct ArrowLayout layout;
    int match, match2;

    if (params->arrow == COLUMN_ARROW_NONE)
	return;

    HeaderLayoutArrow(tree, elemX, params, x, y, width, height, indent, &layout);

    IMAGE_FOR_STATE(image, arrowImage, state);
    if (image != NULL) {
	Tree_RedrawImage(image, 0, 0, layout.width, layout.height,
	    args->display.td,
	    layout.x + sunken,
	    layout.y + sunken);
	return;
    }

    BITMAP_FOR_STATE(bitmap, arrowBitmap, state);
    if (bitmap != None) {
	int bx, by;
	bx = layout.x + sunken;
	by = layout.y + sunken;
	Tree_DrawBitmap(tree, bitmap, args->display.drawable, NULL, NULL,
		0, 0,
		(unsigned int) layout.width, (unsigned int) layout.height,
		bx, by);
	return;
    }

    if (tree->useTheme) {
	if (TreeTheme_DrawHeaderArrow(tree, args->display.td, params->state,
	    layout.arrow == COLUMN_ARROW_UP, layout.x + sunken,
	    layout.y + sunken,
	    layout.width, layout.height) == TCL_OK)
	    return;
    }

    if (1) {
	int arrowWidth = layout.width;
	int arrowHeight = layout.height;
	int arrowBottom = layout.y + arrowHeight;
	XPoint points[5];
	int color1 = 0, color2 = 0;
	int i;

	switch (layout.arrow) {
	    case COLUMN_ARROW_UP:
		points[0].x = layout.x;
		points[0].y = arrowBottom - 1;
		points[1].x = layout.x + arrowWidth / 2;
		points[1].y = layout.y - 1;
		color1 = TK_3D_DARK_GC;
		points[4].x = layout.x + arrowWidth / 2;
		points[4].y = layout.y - 1;
		points[3].x = layout.x + arrowWidth - 1;
		points[3].y = arrowBottom - 1;
		points[2].x = layout.x;
		points[2].y = arrowBottom - 1;
		color2 = TK_3D_LIGHT_GC;
		break;
	    case COLUMN_ARROW_DOWN:
		points[0].x = layout.x + arrowWidth - 1;
		points[0].y = layout.y;
		points[1].x = layout.x + arrowWidth / 2;
		points[1].y = arrowBottom;
		color1 = TK_3D_LIGHT_GC;
		points[2].x = layout.x + arrowWidth - 1;
		points[2].y = layout.y;
		points[3].x = layout.x;
		points[3].y = layout.y;
		points[4].x = layout.x + arrowWidth / 2;
		points[4].y = arrowBottom;
		color2 = TK_3D_DARK_GC;
		break;
	}
	for (i = 0; i < 5; i++) {
	    points[i].x += sunken;
	    points[i].y += sunken;
	}

	BORDER_FOR_STATE(border, border, state)
	if (border == NULL) {
	    Tk_Uid colorName = Tk_GetUid(DEF_BUTTON_BG_COLOR);
	    if (params->state == COLUMN_STATE_ACTIVE)
		colorName = Tk_GetUid(DEF_BUTTON_ACTIVE_BG_COLOR);
	    border = Tk_Get3DBorder(tree->interp, tree->tkwin,
		colorName); /* FIXME: cache it! */
	    if (border == NULL)
		border = tree->border;
	}
	XDrawLines(tree->display, args->display.drawable,
		Tk_3DBorderGC(tree->tkwin, border, color2),
		points + 2, 3, CoordModeOrigin);
	XDrawLines(tree->display, args->display.drawable,
		Tk_3DBorderGC(tree->tkwin, border, color1),
		points, 2, CoordModeOrigin);
    }
}

static void DisplayProcHeader(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementHeader *elemX = (ElementHeader *) elem;
    ElementHeader *masterX = (ElementHeader *) elem->master;
/*    int state = args->state;*/
    int x = args->display.x, y = args->display.y;
    int width = args->display.width, height = args->display.height;
    int arrowIndent = 0;
    Tk_3DBorder border, borderDefault = NULL;
    int i, relief;
    int match, match2;
    struct HeaderParams params;
    TreeRectangle tr1, tr2;

    if (tree->useTheme && (tree->themeHeaderHeight > 0)) {
	height = tree->themeHeaderHeight;
    }

    AdjustForSticky(args->display.sticky,
	args->display.width, args->display.height,
	TRUE, TRUE,
	&x, &y, &width, &height);

    /* We want the right side of the header to match the right edge of the
     * column and not be clipped when the column width is less than the needed
     * width of the style. */
    width = MIN(width, TreeRect_Right(args->display.spanBbox) - x);

    /* Don't draw the sort arrow in the canvasPadX space. */
    if (x < args->display.spanBbox.x + args->display.indent) {
	arrowIndent = args->display.indent;
    }

    HeaderGetParams(tree, elemX, args->state, &params);

    for (i = 0; i < 4; i++) {
	params.eUnionBbox[i] = args->display.eUnionBbox[i] - (x - args->display.spanBbox.x);
	params.iUnionBbox[i] = args->display.iUnionBbox[i] - (x - args->display.spanBbox.x);
    }

    if (tree->useTheme && TreeTheme_DrawHeaderItem(tree, args->display.td,
	    params.state, params.arrow, args->display.spanIndex, x, y, width, height)
	    == TCL_OK) {

#if !defined(MAC_OSX_TK)
	/* Under Aqua, the Appearance Manager draws the sort arrow as part of
	 * the header background. */
	HeaderDrawArrow(args, &params, x, y, width, height, arrowIndent);
#endif
	return;
    }

    BORDER_FOR_STATE(border, border, params.elemState)
    if (border == NULL) {
	Tk_Uid colorName = Tk_GetUid(DEF_BUTTON_BG_COLOR);
	if (params.state != COLUMN_STATE_NORMAL)
	    colorName = Tk_GetUid(DEF_BUTTON_ACTIVE_BG_COLOR);
	borderDefault = Tk_Get3DBorder(tree->interp, tree->tkwin,
	    colorName); /* FIXME: cache it! */
	if (borderDefault == NULL)
	    return;
	border = borderDefault;
    }

    /* X11 coordinates are 16-bit. */
    TreeRect_SetXYWH(tr1, x, y, width, height);
    TreeRect_SetXYWH(tr2, -params.borderWidth, -params.borderWidth,
	args->display.td.width + params.borderWidth * 2,
	args->display.td.height + params.borderWidth * 2);
    TreeRect_Intersect(&tr1, &tr1, &tr2);

    Tk_Fill3DRectangle(tree->tkwin, args->display.drawable, border,
	    tr1.x, tr1.y, tr1.width, tr1.height, params.borderWidth, TK_RELIEF_FLAT);

    HeaderDrawArrow(args, &params, x, y, width, height, arrowIndent);

    relief = (params.state == COLUMN_STATE_PRESSED) ?
	TK_RELIEF_SUNKEN : TK_RELIEF_RAISED;
    Tk_Draw3DRectangle(tree->tkwin, args->display.drawable, border,
	   tr1. x, tr1.y, tr1.width, tr1.height, params.borderWidth, relief);

    if (borderDefault != NULL)
	Tk_Free3DBorder(borderDefault);
}

static void NeededProcHeader(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementHeader *elemX = (ElementHeader *) elem;
/*    ElementHeader *masterX = (ElementHeader *) elem->master;*/
    struct HeaderParams params;
    struct ArrowLayout layout;
    TreeRectangle bounds = {0, 0, 100, 24};
    int width = 0, height = 0, fixedHeight = -1;

    if (args->tree->useTheme && (tree->themeHeaderHeight > 0)) {
	fixedHeight = tree->themeHeaderHeight;
    }

    HeaderGetParams(tree, elemX, args->state, &params);

    HeaderLayoutArrow(tree, elemX, &params, TreeRect_Left(bounds),
	TreeRect_Top(bounds), TreeRect_Width(bounds),
	TreeRect_Height(bounds), 0, &layout);

    if (layout.arrow != COLUMN_ARROW_NONE) {
	width = layout.padX[0] + layout.width + layout.padX[1];
	height = layout.padY[0] + layout.height + layout.padY[1];
    }

#if 0
     /* Original header code never considered borderWidth or
      * TreeTheme_GetHeaderContentMargins when calculating needed width. */
    width += params.margins[0] + params.margins[1];
#endif
    height += params.margins[1] + params.margins[3];

    args->needed.width = width;
    args->needed.height = (fixedHeight > 0) ? fixedHeight : height;
}

static int StateProcHeader(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementHeader *elemX = (ElementHeader *) elem;
    ElementHeader *masterX = (ElementHeader *) elem->master;
    int match, match2;
    Tk_Image image1 = NULL, image2 = NULL;
    Pixmap bitmap1 = None, bitmap2 = None;
    Tk_3DBorder border1, border2;
    struct HeaderParams params1, params2;

    if (!args->states.visible2)
	return 0;

    HeaderGetParams(tree, elemX, args->states.state1, &params1);
    HeaderGetParams(tree, elemX, args->states.state2, &params2);

    /* Check for -arrow changing to/from "none" */
    if ((params1.arrow != COLUMN_ARROW_NONE) ^
	    (params2.arrow != COLUMN_ARROW_NONE))
	return CS_DISPLAY | CS_LAYOUT;

    if (params1.arrow != COLUMN_ARROW_NONE) {
	IMAGE_FOR_STATE(image1, arrowImage, args->states.state1)
    }
    if (params2.arrow != COLUMN_ARROW_NONE) {
	IMAGE_FOR_STATE(image2, arrowImage, args->states.state2)
    }
    if (image1 != image2) {
	if ((image1 != NULL) && (image2 != NULL)) {
	    int w1, h1, w2, h2;
	    Tk_SizeOfImage(image1, &w1, &h1);
	    Tk_SizeOfImage(image2, &w2, &h2);
	    if ((w1 != w2) || (h1 != h2))
		return CS_DISPLAY | CS_LAYOUT;
	    return CS_DISPLAY;
	}
	return CS_DISPLAY | CS_LAYOUT;
    }

    if (params1.arrow != COLUMN_ARROW_NONE) {
	BITMAP_FOR_STATE(bitmap1, arrowBitmap, args->states.state1)
    }
    if (params2.arrow != COLUMN_ARROW_NONE) {
	BITMAP_FOR_STATE(bitmap2, arrowBitmap, args->states.state2)
    }
    if (bitmap1 != bitmap2) {
	if ((bitmap1 != None) && (bitmap2 != None)) {
	    int w1, h1, w2, h2;
	    Tk_SizeOfBitmap(tree->display, bitmap1, &w1, &h1);
	    Tk_SizeOfBitmap(tree->display, bitmap2, &w2, &h2);
	    if ((w1 != w2) || (h1 != h2))
		return CS_DISPLAY | CS_LAYOUT;
	    return CS_DISPLAY;
	}
	return CS_DISPLAY | CS_LAYOUT;
    }

    if (!args->states.draw2)
	return 0;

    /* Headers look different on MacOSX in the background. */
    if ((args->states.state1 & STATE_HEADER_BG) ^
	    (args->states.state2 & STATE_HEADER_BG))
	return CS_DISPLAY;

    if (params1.state != params2.state)
	return CS_DISPLAY;

    if (params1.arrow != params2.arrow)
	return CS_DISPLAY;

    BORDER_FOR_STATE(border1, border, args->states.state1)
    BORDER_FOR_STATE(border2, border, args->states.state2)
    if (border1 != border2)
	return CS_DISPLAY;

    return 0;
}

static int UndefProcHeader(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementHeader *elemX = (ElementHeader *) elem;
    int modified = 0;

    modified |= PerStateInfo_Undefine(tree, &pstBitmap, &elemX->arrowBitmap, elem->stateDomain, args->state);
    modified |= PerStateInfo_Undefine(tree, &pstImage, &elemX->arrowImage, elem->stateDomain, args->state);
    modified |= PerStateInfo_Undefine(tree, &pstBorder, &elemX->border, elem->stateDomain, args->state);
    return modified;
}

static int ActualProcHeader(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    ElementHeader *elemX = (ElementHeader *) args->elem;
    ElementHeader *masterX = (ElementHeader *) args->elem->master;
    static const char *optionName[] = {
	"-arrowbitmap",
	"-arrowimage",
	"-background",
	(char *) NULL };
    int index, match, matchM;
    Tcl_Obj *obj = NULL;

    if (Tcl_GetIndexFromObj(tree->interp, args->actual.obj, optionName,
		"option", 0, &index) != TCL_OK)
	return TCL_ERROR;

    switch (index) {
	case 0: {
	    OBJECT_FOR_STATE(obj, pstBitmap, arrowBitmap, args->state)
	    break;
	}
	case 1: {
	    OBJECT_FOR_STATE(obj, pstImage, arrowImage, args->state)
	    break;
	}
	case 2: {
	    OBJECT_FOR_STATE(obj, pstBorder, border, args->state)
	    break;
	}
    }
    if (obj != NULL)
	Tcl_SetObjResult(tree->interp, obj);
    return TCL_OK;
}

TreeElementType treeElemTypeHeader = {
    "header",
    sizeof(ElementHeader),
    headerOptionSpecs,
    NULL,
    CreateProcHeader,
    DeleteProcHeader,
    ConfigProcHeader,
    DisplayProcHeader,
    NeededProcHeader,
    NULL, /* heightProc */
    WorldChangedProcHeader,
    StateProcHeader,
    UndefProcHeader,
    ActualProcHeader,
    NULL /* onScreenProc */
};

void
TreeElement_GetContentMargins(
    TreeCtrl *tree,
    TreeElement elem,
    int state,
    int eMargins[4],
    int uMargins[4],
    int *arrowHeight
    )
{
    eMargins[0] = eMargins[1] = eMargins[2] = eMargins[3] = 0;
    uMargins[0] = uMargins[1] = uMargins[2] = uMargins[3] = 0;
    *arrowHeight = 0;

    if (ELEMENT_TYPE_MATCHES(elem->typePtr, &treeElemTypeHeader)) {
	ElementHeader *elemX = (ElementHeader *) elem;
	struct HeaderParams params;
	struct ArrowLayout layout;
	TreeRectangle bounds = {0, 0, 100, 24};

	HeaderGetParams(tree, elemX, state, &params);

	/* Added to ePadY of elements in the -union */
	eMargins[1] = params.margins[1];
	eMargins[3] = params.margins[3];

	/* Added to iPadY of this element */
	uMargins[1] = params.margins[1];
	uMargins[3] = params.margins[3];

	if (params.arrow == COLUMN_ARROW_NONE)
	    return;

	HeaderLayoutArrow(tree, elemX, &params, TreeRect_Left(bounds),
	    TreeRect_Top(bounds), TreeRect_Width(bounds),
	    TreeRect_Height(bounds), 0, &layout);

	if (layout.arrowSide == SIDE_LEFT) {
	    uMargins[0] = layout.padX[PAD_TOP_LEFT] + layout.width + layout.padX[PAD_BOTTOM_RIGHT];
	    eMargins[0] = layout.padX[PAD_TOP_LEFT] + layout.width;
	} else {
	    uMargins[2] = layout.padX[PAD_TOP_LEFT] + layout.width + layout.padX[PAD_BOTTOM_RIGHT];
	    eMargins[2] =                             layout.width + layout.padX[PAD_BOTTOM_RIGHT];
	}

	*arrowHeight = layout.padY[0] + layout.height + layout.padY[1];
    }
}

/*****/

typedef struct ElementImage ElementImage;

struct ElementImage
{
    TreeElement_ header;
    PerStateInfo image;
};

typedef struct ElementImageSize
{
    int width;
    Tcl_Obj *widthObj;
    int height;
    Tcl_Obj *heightObj;
} ElementImageSize;

#define IMAGE_CONF_IMAGE 0x0001
#define IMAGE_CONF_SIZE 0x0002
#define IMAGE_CONF_DISPLAY 0x0004
#ifdef DEPRECATED
#define IMAGE_CONF_DRAW 0x0008
#endif

static Tk_OptionSpec imageOptionSpecs[] = {
#ifdef DEPRECATED
    {TK_OPTION_CUSTOM, "-draw", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(TreeElement_, options),
     TK_OPTION_NULL_OK, (ClientData) NULL, IMAGE_CONF_DRAW},
#endif
    {TK_OPTION_CUSTOM, "-height", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(TreeElement_, options),
     TK_OPTION_NULL_OK, (ClientData) NULL, IMAGE_CONF_SIZE},
    {TK_OPTION_CUSTOM, "-image", (char *) NULL, (char *) NULL,
     (char *) NULL,
     offsetof(ElementImage, image.obj), offsetof(ElementImage, image),
     TK_OPTION_NULL_OK, (ClientData) NULL, IMAGE_CONF_IMAGE},
    {TK_OPTION_CUSTOM, "-tiled", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(TreeElement_, options),
     TK_OPTION_NULL_OK, (ClientData) NULL, IMAGE_CONF_DISPLAY},
    {TK_OPTION_CUSTOM, "-width", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(TreeElement_, options),
     TK_OPTION_NULL_OK, (ClientData) NULL, IMAGE_CONF_SIZE},
    {TK_OPTION_END, (char *) NULL, (char *) NULL, (char *) NULL,
     (char *) NULL, 0, -1, 0, (ClientData) NULL, 0}
};

static void DeleteProcImage(TreeElementArgs *args)
{
/*    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementImage *elemX = (ElementImage *) elem;*/
}

static int WorldChangedProcImage(TreeElementArgs *args)
{
    int flagM = args->change.flagMaster;
    int flagS = args->change.flagSelf;
    int mask = 0;

    if ((flagS | flagM) & (
#ifdef DEPRECATED
	    IMAGE_CONF_DRAW |
#endif
	    IMAGE_CONF_IMAGE | IMAGE_CONF_SIZE))
	mask |= CS_DISPLAY | CS_LAYOUT;
    if ((flagS | flagM) & IMAGE_CONF_DISPLAY)
	mask |= CS_DISPLAY;

    return mask;
}

static int ConfigProcImage(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementImage *elemX = (ElementImage *) elem;
    Tk_SavedOptions savedOptions;
    int error;
    Tcl_Obj *errorResult = NULL;

    for (error = 0; error <= 1; error++) {
	if (error == 0) {
	    if (Tree_SetOptions(tree, elem->stateDomain, elemX,
			elem->typePtr->optionTable,
			args->config.objc, args->config.objv,
			&savedOptions, &args->config.flagSelf) != TCL_OK) {
		args->config.flagSelf = 0;
		continue;
	    }

	    Tk_FreeSavedOptions(&savedOptions);
	    break;
	} else {
	    errorResult = Tcl_GetObjResult(tree->interp);
	    Tcl_IncrRefCount(errorResult);
	    Tk_RestoreSavedOptions(&savedOptions);

	    Tcl_SetObjResult(tree->interp, errorResult);
	    Tcl_DecrRefCount(errorResult);
	    return TCL_ERROR;
	}
    }

    return TCL_OK;
}

static int CreateProcImage(TreeElementArgs *args)
{
/*    ElementImage *elemX = (ElementImage *) args->elem;*/

    return TCL_OK;
}

static void DisplayProcImage(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementImage *elemX = (ElementImage *) elem;
    ElementImage *masterX = (ElementImage *) elem->master;
    int state = args->state;
    int x = args->display.x, y = args->display.y;
    int width, height;
    int match, match2;
#ifdef DEPRECATED
    int draw;
#endif
    Tk_Image image;
    int imgW, imgH;
    int tiled = 0, *eit, *eitM = NULL;
    int inHeader = elem->stateDomain == STATE_DOMAIN_HEADER;
    int columnState = COLUMN_STATE_NORMAL;

#ifdef DEPRECATED
    draw = DO_BooleanForState(tree, elem, 1002, state);
    if (!draw)
	return;
#endif

    IMAGE_FOR_STATE(image, image, state)
    if (image == NULL)
	return;

    eit = DynamicOption_FindData(elem->options, 1003);
    if (masterX != NULL)
	eitM = DynamicOption_FindData(elem->master->options, 1003);

    if (eit != NULL && *eit != -1)
	tiled = *eit;
    else if ((eitM != NULL) && (*eitM != -1))
	tiled = *eitM;
    if (tiled) {
	TreeRectangle tr;
	tr.x = x, tr.y = y;
	tr.width = args->display.width, tr.height = args->display.height;
	Tree_DrawTiledImage(tree, args->display.td, image, tr,
		x, y, TRUE, TRUE);
	return;
    }

    Tk_SizeOfImage(image, &imgW, &imgH);
    width = imgW, height = imgH;
    AdjustForSticky(args->display.sticky,
	args->display.width, args->display.height,
	FALSE, FALSE,
	&x, &y, &width, &height);
#if HEADER_OFFSET_HACK == 1
    if (inHeader) {
	if (state & STATE_HEADER_ACTIVE)
	    columnState = COLUMN_STATE_ACTIVE;
	else if (state & STATE_HEADER_PRESSED)
	    columnState = COLUMN_STATE_PRESSED;
    }
    if (inHeader && columnState == COLUMN_STATE_PRESSED) {
	/* If this image fills the whole header, don't offset it. */
	/* FIXME: should have a layout option to control this. */
	if (imgW < args->display.spanBbox.width ||
		imgH < args->display.spanBbox.height) {
	    x += 1, y += 1;
	}
    }
#endif
    if (imgW > args->display.width)
	imgW = args->display.width;
    if (imgH > args->display.height)
	imgH = args->display.height;
    Tree_RedrawImage(image, 0, 0, imgW, imgH, args->display.td, x, y);
}

static void NeededProcImage(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementImage *elemX = (ElementImage *) elem;
    ElementImage *masterX = (ElementImage *) elem->master;
    int state = args->state;
    int width = 0, height = 0;
    int match, match2;
    Tk_Image image;
    ElementImageSize *eis, *eisM = NULL;

    IMAGE_FOR_STATE(image, image, state)

    if (image != NULL)
	Tk_SizeOfImage(image, &width, &height);

    eis = DynamicOption_FindData(elem->options, 1001);
    if (masterX != NULL)
	eisM = DynamicOption_FindData(elem->master->options, 1001);

    if (eis != NULL && eis->widthObj != NULL)
	width = eis->width;
    else if ((eisM != NULL) && (eisM->widthObj != NULL))
	width = eisM->width;

    if (eis != NULL && eis->heightObj != NULL)
	height = eis->height;
    else if ((eisM != NULL) && (eisM->heightObj != NULL))
	height = eisM->height;

    args->needed.width = width;
    args->needed.height = height;
}

static int StateProcImage(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementImage *elemX = (ElementImage *) elem;
    ElementImage *masterX = (ElementImage *) elem->master;
    int match, match2;
#ifdef DEPRECATED
    int draw1, draw2;
#endif
    Tk_Image image1, image2;

    if (!args->states.visible2)
	return 0;

    IMAGE_FOR_STATE(image1, image, args->states.state1)
    IMAGE_FOR_STATE(image2, image, args->states.state2)

    if (image1 != image2) {
	if ((image1 != NULL) && (image2 != NULL)) {
	    int w1, h1, w2, h2;
	    Tk_SizeOfImage(image1, &w1, &h1);
	    Tk_SizeOfImage(image2, &w2, &h2);
	    if ((w1 != w2) || (h1 != h2))
		return CS_DISPLAY | CS_LAYOUT;
	    return CS_DISPLAY;
	}
	return CS_DISPLAY | CS_LAYOUT;
    }

    if (!args->states.draw2)
	return 0;
#ifdef DEPRECATED
    draw1 = DO_BooleanForState(tree, elem, 1002, args->states.state1);
    draw2 = DO_BooleanForState(tree, elem, 1002, args->states.state2);
    if ((draw1 != 0) != (draw2 != 0))
	return CS_DISPLAY;
#endif

    return 0;
}

static int UndefProcImage(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementImage *elemX = (ElementImage *) elem;
    int modified = 0;
#ifdef DEPRECATED
    PerStateInfo *psi;
#endif

#ifdef DEPRECATED
    if ((psi = DynamicOption_FindData(elem->options, 1002)) != NULL)
	modified |= PerStateInfo_Undefine(tree, &pstBoolean, psi, elem->stateDomain, args->state);
#endif
    modified |= PerStateInfo_Undefine(tree, &pstImage, &elemX->image, elem->stateDomain, args->state);
    return modified;
}

static int ActualProcImage(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    ElementImage *elemX = (ElementImage *) args->elem;
    ElementImage *masterX = (ElementImage *) args->elem->master;
    static const char *optionName[] = {
#ifdef DEPRECATED
	"-draw",
#endif
	"-image",
	(char *) NULL };
    int index, match, matchM;
    Tcl_Obj *obj = NULL;

    if (Tcl_GetIndexFromObj(tree->interp, args->actual.obj, optionName,
		"option", 0, &index) != TCL_OK)
	return TCL_ERROR;

    switch (index) {
#ifdef DEPRECATED
	case 0: {
	    obj = DO_ObjectForState(tree, &pstBoolean, args->elem, 1002, args->state);
	    break;
	}
	case 1: {
	    OBJECT_FOR_STATE(obj, pstImage, image, args->state)
	    break;
	}
#else
	case 0: {
	    OBJECT_FOR_STATE(obj, pstImage, image, args->state)
	    break;
	}
#endif
    }
    if (obj != NULL)
	Tcl_SetObjResult(tree->interp, obj);
    return TCL_OK;
}

TreeElementType treeElemTypeImage = {
    "image",
    sizeof(ElementImage),
    imageOptionSpecs,
    NULL,
    CreateProcImage,
    DeleteProcImage,
    ConfigProcImage,
    DisplayProcImage,
    NeededProcImage,
    NULL, /* heightProc */
    WorldChangedProcImage,
    StateProcImage,
    UndefProcImage,
    ActualProcImage,
    NULL /* onScreenProc */
};

/*****/

typedef struct ElementRect ElementRect;

struct ElementRect
{
    TreeElement_ header;
#ifdef DEPRECATED
    PerStateInfo draw;
#endif
    int width;
    Tcl_Obj *widthObj;
    int height;
    Tcl_Obj *heightObj;
    PerStateInfo fill;
    PerStateInfo outline;
    int outlineWidth;
    Tcl_Obj *outlineWidthObj;
    PerStateInfo open;
    int showFocus;
    int rx;
    Tcl_Obj *rxObj;
    int ry;
    Tcl_Obj *ryObj;
};

#define RECT_CONF_FILL 0x0001
#define RECT_CONF_OUTLINE 0x0002
#define RECT_CONF_OUTWIDTH 0x0004
#define RECT_CONF_OPEN 0x0008
#define RECT_CONF_SIZE 0x0010
#define RECT_CONF_FOCUS 0x0020
#define RECT_CONF_RADIUS 0x0040
#ifdef DEPRECATED
#define RECT_CONF_DRAW 0x0080
#endif

static Tk_OptionSpec rectOptionSpecs[] = {
#ifdef DEPRECATED
    {TK_OPTION_CUSTOM, "-draw", (char *) NULL, (char *) NULL,
     (char *) NULL,
     offsetof(ElementRect, draw.obj), offsetof(ElementRect, draw),
     TK_OPTION_NULL_OK, (ClientData) NULL, RECT_CONF_DRAW},
#endif
    {TK_OPTION_CUSTOM, "-fill", (char *) NULL, (char *) NULL,
     (char *) NULL,
     offsetof(ElementRect, fill.obj), offsetof(ElementRect, fill),
     TK_OPTION_NULL_OK, (ClientData) NULL, RECT_CONF_FILL},
    {TK_OPTION_PIXELS, "-height", (char *) NULL, (char *) NULL,
     (char *) NULL, offsetof(ElementRect, heightObj),
     offsetof(ElementRect, height),
     TK_OPTION_NULL_OK, (ClientData) NULL, RECT_CONF_SIZE},
    {TK_OPTION_CUSTOM, "-open", (char *) NULL, (char *) NULL, (char *) NULL,
     offsetof(ElementRect, open.obj), offsetof(ElementRect, open),
     TK_OPTION_NULL_OK, (ClientData) NULL, RECT_CONF_OPEN},
    {TK_OPTION_CUSTOM, "-outline", (char *) NULL, (char *) NULL,
     (char *) NULL,
     offsetof(ElementRect, outline.obj), offsetof(ElementRect, outline),
     TK_OPTION_NULL_OK, (ClientData) NULL, RECT_CONF_OUTLINE},
    {TK_OPTION_PIXELS, "-outlinewidth", (char *) NULL, (char *) NULL,
     (char *) NULL, offsetof(ElementRect, outlineWidthObj),
     offsetof(ElementRect, outlineWidth),
     TK_OPTION_NULL_OK, (ClientData) NULL, RECT_CONF_OUTWIDTH},
    {TK_OPTION_PIXELS, "-rx", (char *) NULL, (char *) NULL,
     (char *) NULL, offsetof(ElementRect, rxObj),
     offsetof(ElementRect, rx),
     TK_OPTION_NULL_OK, (ClientData) NULL, RECT_CONF_RADIUS},
    {TK_OPTION_PIXELS, "-ry", (char *) NULL, (char *) NULL,
     (char *) NULL, offsetof(ElementRect, ryObj),
     offsetof(ElementRect, ry),
     TK_OPTION_NULL_OK, (ClientData) NULL, RECT_CONF_RADIUS},
    {TK_OPTION_CUSTOM, "-showfocus", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(ElementRect, showFocus),
     TK_OPTION_NULL_OK, (ClientData) &booleanCO, RECT_CONF_FOCUS},
    {TK_OPTION_PIXELS, "-width", (char *) NULL, (char *) NULL,
     (char *) NULL, offsetof(ElementRect, widthObj),
     offsetof(ElementRect, width),
     TK_OPTION_NULL_OK, (ClientData) NULL, RECT_CONF_SIZE},
    {TK_OPTION_END, (char *) NULL, (char *) NULL, (char *) NULL,
     (char *) NULL, 0, -1, 0, (ClientData) NULL, 0}
};

static void DeleteProcRect(TreeElementArgs *args)
{
/*    TreeCtrl *tree = args->tree;
    ElementRect *elemX = (ElementRect *) args->elem;*/
}

static int WorldChangedProcRect(TreeElementArgs *args)
{
    int flagM = args->change.flagMaster;
    int flagS = args->change.flagSelf;
    int mask = 0;

    if ((flagS | flagM) & (RECT_CONF_SIZE | RECT_CONF_OUTWIDTH))
	mask |= CS_DISPLAY | CS_LAYOUT;

    if ((flagS | flagM) & (
#ifdef DEPRECATED
	    RECT_CONF_DRAW |
#endif
	    RECT_CONF_FILL | RECT_CONF_OUTLINE | RECT_CONF_OPEN |
	    RECT_CONF_FOCUS | RECT_CONF_RADIUS))
	mask |= CS_DISPLAY;

    return mask;
}

static int ConfigProcRect(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementRect *elemX = (ElementRect *) elem;
    Tk_SavedOptions savedOptions;
    int error;
    Tcl_Obj *errorResult = NULL;

    for (error = 0; error <= 1; error++) {
	if (error == 0) {
	    if (Tree_SetOptions(tree, elem->stateDomain, elemX,
			elem->typePtr->optionTable,
			args->config.objc, args->config.objv,
			&savedOptions, &args->config.flagSelf) != TCL_OK) {
		args->config.flagSelf = 0;
		continue;
	    }

	    Tk_FreeSavedOptions(&savedOptions);
	    break;
	} else {
	    errorResult = Tcl_GetObjResult(tree->interp);
	    Tcl_IncrRefCount(errorResult);
	    Tk_RestoreSavedOptions(&savedOptions);

	    Tcl_SetObjResult(tree->interp, errorResult);
	    Tcl_DecrRefCount(errorResult);
	    return TCL_ERROR;
	}
    }

    return TCL_OK;
}

static int CreateProcRect(TreeElementArgs *args)
{
    ElementRect *elemX = (ElementRect *) args->elem;

    elemX->showFocus = -1;
    return TCL_OK;
}

static void DisplayProcRect(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementRect *elemX = (ElementRect *) elem;
    ElementRect *masterX = (ElementRect *) elem->master;
    int state = args->state;
    int x = args->display.x, y = args->display.y;
    int width = args->display.width, height = args->display.height;
    int match, match2;
#ifdef DEPRECATED
    int draw;
#endif
    TreeColor *tc;
    TreeRectangle tr;
    TreeRectangle trBrush;
    TreeClip clip, *clipPtr = &clip;
    int open = 0;
    int outlineWidth = 0;
    int showFocus = 0;
    int rx = 0, ry = 0;

#ifdef DEPRECATED
    BOOLEAN_FOR_STATE(draw, draw, state)
    if (!draw)
	return;
#endif

    if (elemX->outlineWidthObj != NULL)
	outlineWidth = elemX->outlineWidth;
    else if ((masterX != NULL) && (masterX->outlineWidthObj != NULL))
	outlineWidth = masterX->outlineWidth;

    FLAGS_FOR_STATE(open, open, state)
    if (open == 0xFFFFFFFF) /* unspecified */
	open = 0;

    if (elemX->showFocus != -1)
	showFocus = elemX->showFocus;
    else if ((masterX != NULL) && (masterX->showFocus != -1))
	showFocus = masterX->showFocus;

    if (elemX->widthObj != NULL)
	width = elemX->width;
    else if ((masterX != NULL) && (masterX->widthObj != NULL))
	width = masterX->width;

    if (elemX->heightObj != NULL)
	height = elemX->height;
    else if ((masterX != NULL) && (masterX->heightObj != NULL))
	height = masterX->height;

    if (elemX->rxObj != NULL)
	rx = elemX->rx;
    else if ((masterX != NULL) && (masterX->rxObj != NULL))
	rx = masterX->rx;

    if (elemX->ryObj != NULL)
	ry = elemX->ry;
    else if ((masterX != NULL) && (masterX->ryObj != NULL))
	ry = masterX->ry;

    AdjustForSticky(args->display.sticky,
	args->display.width, args->display.height,
	TRUE, TRUE,
	&x, &y, &width, &height);

    TreeRect_SetXYWH(tr, x, y, width, height);

#if USE_ITEM_PIXMAP == 0
    clip.type = TREE_CLIP_RECT;
    clip.tr = args->display.bounds;
#else
    clipPtr = NULL;
#endif

    if (rx < 1 && ry < 1)
	rx = ry = 0;
    else if (ry < 1)
	ry = rx;
    else if (rx < 1)
	rx = ry;
    rx = MIN(rx, width/2);
    ry = MIN(ry, height/2);
    if (rx >= 1 && ry >= 1) {
	TREECOLOR_FOR_STATE(tc, fill, state)
	if (tc != NULL) {
	    TreeColor_GetBrushBounds(tree, tc, tr,
		    tree->drawableXOrigin, tree->drawableYOrigin,
		    args->display.column, args->display.item, &trBrush);
	    TreeColor_FillRoundRect(tree, args->display.td, clipPtr, tc, trBrush, tr, rx, ry, open);
	}
	TREECOLOR_FOR_STATE(tc, outline, state)
	if ((tc != NULL) && (outlineWidth > 0) && (open != RECT_OPEN_WNES)) {
	    TreeColor_GetBrushBounds(tree, tc, tr,
		    tree->drawableXOrigin, tree->drawableYOrigin,
		    args->display.column, args->display.item, &trBrush);
	    TreeColor_DrawRoundRect(tree, args->display.td, clipPtr, tc,
		trBrush, tr, outlineWidth, rx, ry, open);
	}
	/* FIXME: its not round! */
	if (showFocus && (state & STATE_ITEM_FOCUS) && (state & STATE_ITEM_ACTIVE)) {
	    Tree_DrawActiveOutline(tree, args->display.drawable,
		    args->display.x, args->display.y,
		    args->display.width, args->display.height,
		    open);
	}
	return;
    }

    TREECOLOR_FOR_STATE(tc, fill, state)
    if (tc != NULL) {
	TreeColor_GetBrushBounds(tree, tc, tr,
		tree->drawableXOrigin, tree->drawableYOrigin,
		args->display.column, args->display.item, &trBrush);
	TreeColor_FillRect(tree, args->display.td, clipPtr, tc, trBrush, tr);
    }

    TREECOLOR_FOR_STATE(tc, outline, state)
    if ((tc != NULL) && (outlineWidth > 0) && (open != RECT_OPEN_WNES)) {
	TreeColor_GetBrushBounds(tree, tc, tr,
		tree->drawableXOrigin, tree->drawableYOrigin,
		args->display.column, args->display.item, &trBrush);
	TreeColor_DrawRect(tree, args->display.td, clipPtr, tc, trBrush, tr,
	    outlineWidth, open);
    }

    if (showFocus && (state & STATE_ITEM_FOCUS) && (state & STATE_ITEM_ACTIVE)) {
	Tree_DrawActiveOutline(tree, args->display.drawable,
		args->display.x, args->display.y,
		args->display.width, args->display.height,
		open);
    }
}

static void NeededProcRect(TreeElementArgs *args)
{
    TreeElement elem = args->elem;
    ElementRect *elemX = (ElementRect *) elem;
    ElementRect *masterX = (ElementRect *) elem->master;
    int width = 0, height = 0;
    int outlineWidth = 0;

    if (elemX->outlineWidthObj != NULL)
	outlineWidth = elemX->outlineWidth;
    else if ((masterX != NULL) && (masterX->outlineWidthObj != NULL))
	outlineWidth = masterX->outlineWidth;

    if (elemX->widthObj != NULL)
	width = elemX->width;
    else if ((masterX != NULL) && (masterX->widthObj != NULL))
	width = masterX->width;

    if (elemX->heightObj != NULL)
	height = elemX->height;
    else if ((masterX != NULL) && (masterX->heightObj != NULL))
	height = masterX->height;

    args->needed.width = MAX(width, outlineWidth * 2);
    args->needed.height = MAX(height, outlineWidth * 2);
}

static int StateProcRect(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementRect *elemX = (ElementRect *) elem;
    ElementRect *masterX = (ElementRect *) elem->master;
    int match, match2;
#ifdef DEPRECATED
    int draw1, draw2;
#endif
    int open1, open2;
    XColor *c1, *c2;
    TreeColor *tc1, *tc2;
    int s1, s2;
    int showFocus = 0;

    /* If either the -draw or -visible layout option is false for the
     * current state, then changes to colors etc don't warrant a redisplay. */
    if (!args->states.visible2 || !args->states.draw2)
	return 0;

#ifdef DEPRECATED
    BOOLEAN_FOR_STATE(draw1, draw, args->states.state1)
    BOOLEAN_FOR_STATE(draw2, draw, args->states.state2)
    if ((draw1 != 0) != (draw2 != 0))
	return CS_DISPLAY;
    /* If the element isn't drawn, then changes to colors etc don't
     * warrant a redisplay. */
    if (draw2 == 0)
	return 0;
#endif

    if (elemX->showFocus != -1)
	showFocus = elemX->showFocus;
    else if ((masterX != NULL) && (masterX->showFocus != -1))
	showFocus = masterX->showFocus;

    s1 = showFocus &&
	(args->states.state1 & STATE_ITEM_FOCUS) &&
	(args->states.state1 & STATE_ITEM_ACTIVE);
    s2 = showFocus &&
	(args->states.state2 & STATE_ITEM_FOCUS) &&
	(args->states.state2 & STATE_ITEM_ACTIVE);
    if (s1 != s2)
	return CS_DISPLAY;

    TREECOLOR_FOR_STATE(tc1, fill, args->states.state1)
    TREECOLOR_FOR_STATE(tc2, fill, args->states.state2)
    if (TREECOLOR_CMP(tc1, tc2))
	return CS_DISPLAY;

    FLAGS_FOR_STATE(open1, open, args->states.state1)
    FLAGS_FOR_STATE(open2, open, args->states.state2)
    if (open1 != open2)
	return CS_DISPLAY;

    COLOR_FOR_STATE(c1, outline, args->states.state1)
    COLOR_FOR_STATE(c2, outline, args->states.state2)
    if (c1 != c2)
	return CS_DISPLAY;

    return 0;
}

static int UndefProcRect(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementRect *elemX = (ElementRect *) elem;
    int modified = 0;

#ifdef DEPRECATED
    modified |= PerStateInfo_Undefine(tree, &pstBoolean, &elemX->draw, elem->stateDomain, args->state);
#endif
    modified |= PerStateInfo_Undefine(tree, &pstColor, &elemX->fill, elem->stateDomain, args->state);
    modified |= PerStateInfo_Undefine(tree, &pstFlags, &elemX->open, elem->stateDomain, args->state);
    modified |= PerStateInfo_Undefine(tree, &pstColor, &elemX->outline, elem->stateDomain, args->state);
    return modified;
}

static int ActualProcRect(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    ElementRect *elemX = (ElementRect *) args->elem;
    ElementRect *masterX = (ElementRect *) args->elem->master;
    static const char *optionName[] = {
#ifdef DEPRECATED
	"-draw",
#endif
	"-fill",
	 "-open",
	 "-outline",
	(char *) NULL };
    enum optionEnum {
#ifdef DEPRECATED
	OPT_DRAW,
#endif
	OPT_FILL,
	OPT_OPEN,
	OPT_OUTLINE
    };
    int index, match, matchM;
    Tcl_Obj *obj = NULL;

    if (Tcl_GetIndexFromObj(tree->interp, args->actual.obj, optionName,
		"option", 0, &index) != TCL_OK)
	return TCL_ERROR;

    switch ((enum optionEnum) index) {
#ifdef DEPRECATED
	case OPT_DRAW: {
	    OBJECT_FOR_STATE(obj, pstBoolean, draw, args->state)
	    break;
	}
#endif
	case OPT_FILL: {
	    OBJECT_FOR_STATE(obj, pstColor, fill, args->state)
	    break;
	}
	case OPT_OPEN: {
	    OBJECT_FOR_STATE(obj, pstFlags, open, args->state)
	    break;
	}
	case OPT_OUTLINE: {
	    OBJECT_FOR_STATE(obj, pstColor, outline, args->state)
	    break;
	}
    }
    if (obj != NULL)
	Tcl_SetObjResult(tree->interp, obj);
    return TCL_OK;
}

TreeElementType treeElemTypeRect = {
    "rect",
    sizeof(ElementRect),
    rectOptionSpecs,
    NULL,
    CreateProcRect,
    DeleteProcRect,
    ConfigProcRect,
    DisplayProcRect,
    NeededProcRect,
    NULL, /* heightProc */
    WorldChangedProcRect,
    StateProcRect,
    UndefProcRect,
    ActualProcRect,
    NULL /* onScreenProc */
};

/*****/

typedef struct ElementText ElementText;

struct ElementText
{
    TreeElement_ header;
    char *textCfg;		/* -text */
    char *text;			/* This will be the same as textCfg, or it
				 * will be a dynamically allocated string
				 * from any -data or -textvariable. */
#define STRINGREP_INVALID -1
    Tcl_Size textLen;		/* Number of bytes (not characters) in the
				 * UTF-8 string. If -1, it means the string
				 * representation is invalid. */
};
#define TEXTVAR

/* for Tk_SetOptions() */
#define TEXT_CONF_LAYOUT 0x0001
#define TEXT_CONF_DISPLAY 0x0002
#define TEXT_CONF_STRINGREP 0x0040
#ifdef TEXTVAR
#define TEXT_CONF_TEXTVAR 0x0080
#endif

/*
 * Dynamic option ids for the text element.
 */
#define DOID_TEXT_VAR 1001
#define DOID_TEXT_DRAW 1002
#define DOID_TEXT_FILL 1003
#define DOID_TEXT_FONT 1004
#define DOID_TEXT_LAYOUT 1005
#define DOID_TEXT_DATA 1006
#define DOID_TEXT_LAYOUT2 1007
#define DOID_TEXT_STYLE 1008
#define DOID_TEXT_LAYOUT3 1009

typedef struct ElementTextData {
    Tcl_Obj *dataObj;			/* -data */
#define TDT_NULL -1
#define TDT_DOUBLE 0
#define TDT_INTEGER 1
#define TDT_LONG 2
#define TDT_STRING 3
#define TDT_TIME 4
    int dataType;			/* -datatype */
    Tcl_Obj *formatObj;			/* -format */
} ElementTextData;

typedef struct ElementTextLayout {
#define TK_JUSTIFY_NULL -1
    int justify;			/* -justify */
    int lines;				/* -lines */
    Tcl_Obj *widthObj;			/* -width */
    int width;				/* -width */
#define TEXT_WRAP_NULL -1
#define TEXT_WRAP_CHAR 0
#define TEXT_WRAP_NONE 1
#define TEXT_WRAP_WORD 2
    int wrap;				/* -wrap */
    int elidePos;			/* -elidepos */
} ElementTextLayout;

/* This structure doesn't hold any option values, but it is managed by
 * the dynamic-option code. */
typedef struct ElementTextLayout2 {
    TextLayout layout;
    int layoutWidth;
    int neededWidth;
    int totalWidth;
} ElementTextLayout2;

typedef struct ElementTextLayout3 {
    Tcl_Obj *lMargin1Obj;		/* -lmargin1 */
    int lMargin1;			/* -lmargin2 */
    Tcl_Obj *lMargin2Obj;		/* -lmargin1 */
    int lMargin2;			/* -lmargin2 */
} ElementTextLayout3;

#define TEXT_STYLE
#ifdef TEXT_STYLE
typedef struct ElementTextStyle {
    int underline;			/* -underline */
} ElementTextStyle;

/* Called by the dynamic-option code when an ElementTextData is allocated. */
static void
ElementTextStyleInit(
    void *data
    )
{
    ElementTextStyle *ets = data;

#define TEXT_UNDERLINE_EMPTYVAL -100000
    ets->underline = TEXT_UNDERLINE_EMPTYVAL;
}
#endif

#ifdef TEXTVAR
typedef struct ElementTextVar {
    Tcl_Obj *varNameObj;		/* -textvariable */
    TreeCtrl *tree;			/* needed to redisplay */
    TreeItem item;			/* needed to redisplay */
    TreeItemColumn column;		/* needed to redisplay */
} ElementTextVar;
#endif

/* Called by the dynamic-option code when an ElementTextData is allocated. */
static void
ElementTextDataInit(
    void *data
    )
{
    ElementTextData *etd = data;

    etd->dataType = TDT_NULL;
}

/* Called by the dynamic-option code when an ElementTextLayout is allocated. */
static void
ElementTextLayoutInit(
    void *data
    )
{
    ElementTextLayout *etl = data;

    etl->justify = TK_JUSTIFY_NULL;
    etl->lines = -1;
    etl->wrap = TEXT_WRAP_NULL;
    etl->elidePos = -1;
}

static const char *textDataTypeST[] = { "double", "integer", "long", "string",
					"time", (char *) NULL };
static const char *textElideST[] = { "start", "middle", "end", (char *) NULL };
static const char *textJustifyST[] = { "left", "right", "center", (char *) NULL };
static const char *textWrapST[] = { "char", "none", "word", (char *) NULL };

static Tk_OptionSpec textOptionSpecs[] = {
    {TK_OPTION_CUSTOM, "-data", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(TreeElement_, options),
     TK_OPTION_NULL_OK, (ClientData) NULL, TEXT_CONF_STRINGREP},
    {TK_OPTION_CUSTOM, "-datatype", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(TreeElement_, options),
     TK_OPTION_NULL_OK, (ClientData) NULL, TEXT_CONF_STRINGREP},
#ifdef DEPRECATED
    {TK_OPTION_CUSTOM, "-draw", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(TreeElement_, options),
     TK_OPTION_NULL_OK, (ClientData) NULL, TEXT_CONF_DISPLAY},
#endif
    {TK_OPTION_CUSTOM, "-elidepos", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(TreeElement_, options),
     TK_OPTION_NULL_OK, (ClientData) NULL, TEXT_CONF_LAYOUT},
    {TK_OPTION_CUSTOM, "-fill", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(TreeElement_, options),
     TK_OPTION_NULL_OK, (ClientData) NULL,  TEXT_CONF_DISPLAY},
    {TK_OPTION_CUSTOM, "-font", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(TreeElement_, options),
     TK_OPTION_NULL_OK, (ClientData) NULL, TEXT_CONF_LAYOUT},
    {TK_OPTION_CUSTOM, "-format", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(TreeElement_, options),
     TK_OPTION_NULL_OK, (ClientData) NULL, TEXT_CONF_STRINGREP},
    {TK_OPTION_CUSTOM, "-justify", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(TreeElement_, options),
     TK_OPTION_NULL_OK, (ClientData) NULL, TEXT_CONF_LAYOUT},
    {TK_OPTION_CUSTOM, "-lines", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(TreeElement_, options),
     TK_OPTION_NULL_OK, (ClientData) NULL, TEXT_CONF_LAYOUT},
    {TK_OPTION_CUSTOM, "-lmargin1", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(TreeElement_, options),
     TK_OPTION_NULL_OK, (ClientData) NULL, TEXT_CONF_LAYOUT},
    {TK_OPTION_CUSTOM, "-lmargin2", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(TreeElement_, options),
     TK_OPTION_NULL_OK, (ClientData) NULL, TEXT_CONF_LAYOUT},
    {TK_OPTION_STRING, "-text", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(ElementText, textCfg),
     TK_OPTION_NULL_OK, (ClientData) NULL, TEXT_CONF_STRINGREP},
#ifdef TEXTVAR
    {TK_OPTION_CUSTOM, "-textvariable", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(TreeElement_, options),
     TK_OPTION_NULL_OK, (ClientData) NULL, TEXT_CONF_STRINGREP | TEXT_CONF_TEXTVAR},
#endif
#ifdef TEXT_STYLE
    {TK_OPTION_CUSTOM, "-underline", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(TreeElement_, options),
     TK_OPTION_NULL_OK, (ClientData) NULL, TEXT_CONF_DISPLAY},
#endif
    {TK_OPTION_CUSTOM, "-width", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(TreeElement_, options),
     TK_OPTION_NULL_OK, (ClientData) NULL, TEXT_CONF_LAYOUT},
    {TK_OPTION_CUSTOM, "-wrap", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(TreeElement_, options),
     TK_OPTION_NULL_OK, (ClientData) NULL, TEXT_CONF_LAYOUT},
    {TK_OPTION_END, (char *) NULL, (char *) NULL, (char *) NULL,
     (char *) NULL, 0, -1, 0, 0, 0}
};

static int WorldChangedProcText(TreeElementArgs *args)
{
/*	TreeCtrl *tree = args->tree;*/
    TreeElement elem = args->elem;
    ElementText *elemX = (ElementText *) elem;
/*	ElementText *masterX = (ElementText *) elem->master;*/
    int flagT = args->change.flagTree;
    int flagM = args->change.flagMaster;
    int flagS = args->change.flagSelf;
    int mask = 0;

    if ((flagS | flagM) & TEXT_CONF_STRINGREP) {
	elemX->textLen = STRINGREP_INVALID;
    }

    if ((elemX->textLen == STRINGREP_INVALID) ||
	    ((flagS | flagM) & TEXT_CONF_LAYOUT) ||
	    /* Not needed if this element has its own font. */
	    (flagT & TREE_CONF_FONT)) {
	mask |= CS_DISPLAY | CS_LAYOUT;
    }

    if ((flagS | flagM) & TEXT_CONF_DISPLAY)
	mask |= CS_DISPLAY;

    return mask;
}

static void TextUpdateStringRep(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementText *elemX = (ElementText *) elem;
    ElementText *masterX = (ElementText *) elem->master;
    Tcl_Obj *dataObj, *formatObj;
    char *text;
    ElementTextData *etd, *etdM = NULL;
#ifdef TEXTVAR
    ElementTextVar *etv;
    Tcl_Obj *varNameObj;
#endif
    int dataType;

    /* Free any string allocated as a result of -data or -textvariable. */
    if ((elemX->text != NULL) && (elemX->text != elemX->textCfg)) {
	ckfree(elemX->text);
    }

    /* Forget any string, and mark the string rep as no-longer invalid. */
    elemX->text = NULL;
    elemX->textLen = 0;

    /* If -text is specified, then -data and -textvariable are ignored. */
    if (elemX->textCfg != NULL) {
	elemX->text = elemX->textCfg;
	elemX->textLen = (int) strlen(elemX->textCfg);
	return;
    }

#ifdef TEXTVAR
    etv = DynamicOption_FindData(elem->options, DOID_TEXT_VAR);
    varNameObj = etv ? etv->varNameObj : NULL;

    if (varNameObj != NULL) {
	Tcl_Obj *valueObj = Tcl_ObjGetVar2(tree->interp, varNameObj, NULL,
		TCL_GLOBAL_ONLY);

	if (valueObj == NULL) {
	    /* not possible I think */
	} else {
	    /* FIXME: do I need to allocate a copy, or can I just point
	     * to the internal rep of the string object? */
	    text = Tcl_GetStringFromObj(valueObj, &elemX->textLen);
	    if (elemX->textLen > 0) {
		elemX->text = ckalloc(elemX->textLen);
		memcpy(elemX->text, text, elemX->textLen);
	    }
	}
	return;
    }
#endif

    etd = DynamicOption_FindData(elem->options, DOID_TEXT_DATA);
    if (masterX != NULL)
	etdM = DynamicOption_FindData(elem->master->options, DOID_TEXT_DATA);

    dataObj = etd ? etd->dataObj : NULL;
    if ((dataObj == NULL) && (etdM != NULL))
	dataObj = etdM->dataObj;

    dataType = etd ? etd->dataType : TDT_NULL;
    if ((dataType == TDT_NULL) && (etdM != NULL))
	dataType = etdM->dataType;

    formatObj = etd ? etd->formatObj : NULL;
    if ((formatObj == NULL) && (etdM != NULL))
	formatObj = etdM->formatObj;

    /* Only create a string rep if elemX (not masterX) has dataObj,
       dataType or formatObj. */
    if ((dataObj != NULL) && (dataType != TDT_NULL) &&
	    ((etd != NULL) && ((etd->dataObj != NULL) ||
		    (etd->dataType != TDT_NULL) ||
		    (etd->formatObj != NULL)))) {
	int objc = 0;
	Tcl_Obj *objv[5], *resultObj = NULL;
	Tcl_ObjCmdProc *clockObjCmd = NULL, *formatObjCmd = NULL;
	ClientData clockClientData = NULL, formatClientData = NULL;
	Tcl_CmdInfo cmdInfo;

	if (Tcl_GetCommandInfo(tree->interp, "::clock", &cmdInfo) == 1) {
	    clockObjCmd = cmdInfo.objProc;
	    clockClientData = cmdInfo.objClientData;
	}
	if (Tcl_GetCommandInfo(tree->interp, "::format", &cmdInfo) == 1) {
	    formatObjCmd = cmdInfo.objProc;
	    formatClientData = cmdInfo.objClientData;
	}

	/* Important to remove any shared result object, otherwise
	 * calls like Tcl_SetStringObj(Tcl_GetObjResult()) fail. */
	Tcl_ResetResult(tree->interp);

	switch (dataType) {
	    case TDT_DOUBLE:
		if (formatObjCmd == NULL)
		    break;
		if (formatObj == NULL) formatObj = tree->formatFloatObj;
		objv[objc++] = tree->stringFormatObj;
		objv[objc++] = formatObj;
		objv[objc++] = dataObj;
		if (formatObjCmd(formatClientData, tree->interp, objc, objv)
			    == TCL_OK)
		    resultObj = Tcl_GetObjResult(tree->interp);
		break;
	    case TDT_INTEGER:
		if (formatObjCmd == NULL)
		    break;
		if (formatObj == NULL) formatObj = tree->formatIntObj;
		objv[objc++] = tree->stringFormatObj;
		objv[objc++] = formatObj;
		objv[objc++] = dataObj;
		if (formatObjCmd(formatClientData, tree->interp, objc, objv)
			    == TCL_OK)
		    resultObj = Tcl_GetObjResult(tree->interp);
		break;
	    case TDT_LONG:
		if (formatObjCmd == NULL)
		    break;
		if (formatObj == NULL) formatObj = tree->formatLongObj;;
		objv[objc++] = tree->stringFormatObj;
		objv[objc++] = formatObj;
		objv[objc++] = dataObj;
		if (formatObjCmd(formatClientData, tree->interp, objc, objv)
			    == TCL_OK)
		    resultObj = Tcl_GetObjResult(tree->interp);
		break;
	    case TDT_STRING:
		if (formatObjCmd == NULL)
		    break;
		if (formatObj == NULL) formatObj = tree->formatStringObj;;
		objv[objc++] = tree->stringFormatObj;
		objv[objc++] = formatObj;
		objv[objc++] = dataObj;
		if (formatObjCmd(formatClientData, tree->interp, objc, objv)
			    == TCL_OK)
		    resultObj = Tcl_GetObjResult(tree->interp);
		break;
	    case TDT_TIME:
		if (clockObjCmd == NULL)
		    break;
		objv[objc++] = tree->stringClockObj;
		objv[objc++] = tree->stringFormatObj;
		objv[objc++] = dataObj;
		if (formatObj != NULL) {
		    objv[objc++] = tree->optionFormatObj;
		    objv[objc++] = formatObj;
		}
		if (clockObjCmd(clockClientData, tree->interp, objc, objv)
			    == TCL_OK)
		    resultObj = Tcl_GetObjResult(tree->interp);
		break;
	    default:
		Tcl_Panic("unknown ElementText dataType");
		break;
	}

	if (resultObj != NULL) {
	    text = Tcl_GetStringFromObj(resultObj, &elemX->textLen);
	    if (elemX->textLen > 0) {
		elemX->text = ckalloc(elemX->textLen);
		memcpy(elemX->text, text, elemX->textLen);
	    }
	}
    }
}

static ElementTextLayout2 *
TextUpdateLayout(
    char *func,
    TreeElementArgs *args,
    int fixedWidth,
    int maxWidth
    )
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementText *elemX = (ElementText *) elem;
    ElementText *masterX = (ElementText *) elem->master;
    int state = args->state;
    int inHeader = elem->stateDomain == STATE_DOMAIN_HEADER;
    Tk_Font tkfont;
    char *text = NULL;
    int textLen = 0;
    int justify = TK_JUSTIFY_LEFT;
    int lines = 0;
    int wrap = TEXT_WRAP_WORD;
    int width = 0;
    int flags = 0;
    int i, multiLine = FALSE;
    int textWidth;
    ElementTextLayout *etl, *etlM = NULL;
    ElementTextLayout2 *etl2;
    ElementTextLayout3 *etl3, *etl3M = NULL;
    DynamicOption *opt;
    int lMargin1 = 0, lMargin2 = 0;

    if (tree->debug.enable && tree->debug.textLayout)
	dbwin("TextUpdateLayout: %s %p (%s) %s\n    fixedWidth %d maxWidth %d\n",
	    Tk_PathName(tree->tkwin), elemX, masterX ? "instance" : "master",
	    func, fixedWidth, maxWidth);

    etl2 = DynamicOption_FindData(elem->options, DOID_TEXT_LAYOUT2);
    if (etl2 != NULL && etl2->layout != NULL) {
	if (tree->debug.enable && tree->debug.textLayout)
	    dbwin("    FREE\n");
	TextLayout_Free(etl2->layout);
	etl2->layout = NULL;
    }

    if (elemX->text != NULL) {
	text = elemX->text;
	textLen = elemX->textLen;
    } else if ((masterX != NULL) && (masterX->text != NULL)) {
	text = masterX->text;
	textLen = masterX->textLen;
    }
    if ((text == NULL) || (textLen == 0))
	return etl2;

    etl = DynamicOption_FindData(elem->options, DOID_TEXT_LAYOUT);
    if (masterX != NULL)
	etlM = DynamicOption_FindData(elem->master->options, DOID_TEXT_LAYOUT);

    if (etl != NULL && etl->lines != -1)
	lines = etl->lines;
    else if (etlM != NULL && etlM->lines != -1)
	lines = etlM->lines;
    if (lines == 1)
	return etl2;

    tkfont = DO_FontForState(tree, elem, DOID_TEXT_FONT, state);
    if (tkfont == NULL)
	tkfont = inHeader ? tree->tkfontHeader : tree->tkfont;

    if (etl != NULL && etl->wrap != TEXT_WRAP_NULL)
	wrap = etl->wrap;
    else if (etlM != NULL && etlM->wrap != TEXT_WRAP_NULL)
	wrap = etlM->wrap;

    if (wrap != TEXT_WRAP_NONE) {
	if (fixedWidth >= 0)
	    width = fixedWidth;
	else if (maxWidth >= 0)
	    width = maxWidth;
	if (etl != NULL && etl->widthObj != NULL) {
	    if (!width || (etl->width < width))
		width = etl->width;
	} else if ((etlM != NULL) && (etlM->widthObj != NULL)) {
	    if (!width || (etlM->width < width))
		width = etlM->width;
	}
    }

    for (i = 0; i < textLen; i++) {
	if ((text[i] == '\n') || (text[i] == '\r')) {
	    multiLine = TRUE;
	    break;
	}
    }
    if (tree->debug.enable && tree->debug.textLayout)
	dbwin("    lines %d multiLine %d width %d wrap %s\n",
		lines, multiLine, width, textWrapST[wrap]);
    if (!multiLine) {
	if (width == 0)
	    return etl2;
	textWidth = Tk_TextWidth(tkfont, text, textLen);
if (tree->debug.enable && tree->debug.textLayout) dbwin("    available width %d textWidth %d\n", width, textWidth);
	if (width >= textWidth)
	    return etl2;
    }

    if (etl != NULL && etl->justify != TK_JUSTIFY_NULL)
	justify = etl->justify;
    else if (etlM != NULL && etlM->justify != TK_JUSTIFY_NULL)
	justify = etlM->justify;

    if (wrap == TEXT_WRAP_WORD)
	flags |= TK_WHOLE_WORDS;

    if (etl2 == NULL) {
	opt = (DynamicOption *) DynamicOption_AllocIfNeeded(tree,
	    &elem->options, DOID_TEXT_LAYOUT2, sizeof(ElementTextLayout2), NULL);
	etl2 = (ElementTextLayout2 *) opt->data;
	/* It is possible that the needed size of this element does not
	 * require a TextLayout, in which case neededWidth never gets
	 * set. */
	etl2->neededWidth = -1;
    }

    etl3 = DynamicOption_FindData(elem->options, DOID_TEXT_LAYOUT3);
    if (masterX != NULL)
	etl3M = DynamicOption_FindData(elem->master->options, DOID_TEXT_LAYOUT3);
    if (etl3 != NULL && etl3->lMargin1Obj != NULL)
	lMargin1 = etl3->lMargin1;
    else if (etl3M != NULL && etl3M->lMargin1Obj != NULL)
	lMargin1 = etl3M->lMargin1;
    if (etl3 != NULL && etl3->lMargin2Obj != NULL)
	lMargin2 = etl3->lMargin2;
    else if (etl3M != NULL && etl3M->lMargin2Obj != NULL)
	lMargin2 = etl3M->lMargin2;

    etl2->layout = TextLayout_Compute(tkfont, text,
	    Tcl_NumUtfChars(text, textLen), width, justify, lines,
	    lMargin1, lMargin2, flags);

    if (tree->debug.enable && tree->debug.textLayout)
	dbwin("    ALLOC\n");
    return etl2;
}

#ifdef TEXTVAR
static Tcl_VarTraceProc VarTraceProc_Text;

static void TextTraceSet(Tcl_Interp *interp, ElementText *elemX)
{
    ElementTextVar *etv = DynamicOption_FindData(elemX->header.options,
	DOID_TEXT_VAR);
    Tcl_Obj *varNameObj = etv ? etv->varNameObj : NULL;

    if (varNameObj != NULL) {
	Tcl_TraceVar2(interp, Tcl_GetString(varNameObj),
	    NULL,
	    TCL_GLOBAL_ONLY | TCL_TRACE_WRITES | TCL_TRACE_UNSETS,
	    VarTraceProc_Text, (ClientData) elemX);
    }
}

static void TextTraceUnset(Tcl_Interp *interp, ElementText *elemX)
{
    ElementTextVar *etv = DynamicOption_FindData(elemX->header.options,
	DOID_TEXT_VAR);
    Tcl_Obj *varNameObj = etv ? etv->varNameObj : NULL;

    if (varNameObj != NULL) {
	Tcl_UntraceVar2(interp, Tcl_GetString(varNameObj),
	    NULL,
	    TCL_GLOBAL_ONLY | TCL_TRACE_WRITES | TCL_TRACE_UNSETS,
	    VarTraceProc_Text, (ClientData) elemX);
    }
}

static char *VarTraceProc_Text(ClientData clientData, Tcl_Interp *interp,
    const char *name1, const char *name2, int flags)
{
    ElementText *elemX = (ElementText *) clientData;
    ElementTextVar *etv = DynamicOption_FindData(elemX->header.options,
	DOID_TEXT_VAR);
    Tcl_Obj *varNameObj = etv ? etv->varNameObj : NULL;
    Tcl_Obj *valueObj;

    /*
     * If the variable is unset, then immediately recreate it unless
     * the whole interpreter is going away.
     */

    if (flags & TCL_TRACE_UNSETS) {
	if ((flags & TCL_TRACE_DESTROYED) && !Tcl_InterpDeleted(interp)) {
	    /* Use the current string if it is valid. */
	    if (elemX->textLen > 0) {
		valueObj = Tcl_NewStringObj(elemX->text, elemX->textLen);
	    } else {
		valueObj = Tcl_NewStringObj("", 0);
	    }
	    Tcl_IncrRefCount(valueObj);
	    Tcl_ObjSetVar2(interp, varNameObj, NULL, valueObj,
		TCL_GLOBAL_ONLY);
	    Tcl_DecrRefCount(valueObj);
	    TextTraceSet(interp, elemX);
	}
	return (char *) NULL;
    }

    elemX->textLen = STRINGREP_INVALID;
    Tree_ElementChangedItself(etv->tree, etv->item, etv->column,
	(TreeElement) elemX, TEXT_CONF_LAYOUT | TEXT_CONF_TEXTVAR,
	CS_LAYOUT | CS_DISPLAY);
    return (char *) NULL;
}
#endif /* TEXTVAR */

static void DeleteProcText(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementText *elemX = (ElementText *) elem;
    ElementTextLayout2 *etl2;

    if ((elemX->textCfg == NULL) && (elemX->text != NULL)) {
	ckfree(elemX->text);
	elemX->text = NULL;
    }
    etl2 = DynamicOption_FindData(elem->options, DOID_TEXT_LAYOUT2);
    if (etl2 != NULL && etl2->layout != NULL)
	TextLayout_Free(etl2->layout);
    DynamicOption_Free1(tree, &elem->options, DOID_TEXT_LAYOUT2,
	sizeof(ElementTextLayout2));
#ifdef TEXTVAR
    TextTraceUnset(tree->interp, elemX);
#endif
}

static int ConfigProcText(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    Tcl_Interp *interp = tree->interp;
    TreeElement elem = args->elem;
    ElementText *elemX = (ElementText *) elem;
    Tk_SavedOptions savedOptions;
    int error;
    Tcl_Obj *errorResult = NULL;
    char *textCfg = elemX->textCfg;
#ifdef TEXTVAR
    ElementTextVar *etv;
    Tcl_Obj *varNameObj;
#endif

#ifdef TEXTVAR
    TextTraceUnset(interp, elemX);
#endif

    for (error = 0; error <= 1; error++) {
	if (error == 0) {
	    if (Tree_SetOptions(tree, elem->stateDomain, elemX,
			elem->typePtr->optionTable,
			args->config.objc, args->config.objv,
			&savedOptions, &args->config.flagSelf) != TCL_OK) {
		args->config.flagSelf = 0;
		continue;
	    }

#ifdef TEXTVAR
	    etv = DynamicOption_FindData(elem->options, DOID_TEXT_VAR);
	    if (etv != NULL) {
		etv->tree = tree;
		etv->item = args->config.item;
		etv->column = args->config.column;
		varNameObj = etv->varNameObj;
	    } else
		varNameObj = NULL;
	    if (varNameObj != NULL) {
		Tcl_Obj *valueObj;
		valueObj = Tcl_ObjGetVar2(interp, varNameObj,
			NULL, TCL_GLOBAL_ONLY);
		if (valueObj == NULL) {
		    valueObj = Tcl_NewStringObj("", 0);
		    Tcl_IncrRefCount(valueObj);
		    /* This validates the variable name. We get an error
		     * if it is the name of an array */
		    if (Tcl_ObjSetVar2(interp, varNameObj, NULL,
			    valueObj, TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG)
			    == NULL) {
			Tcl_DecrRefCount(valueObj);
			continue;
		    }
		    Tcl_DecrRefCount(valueObj);
		}
	    }
#endif

	    Tk_FreeSavedOptions(&savedOptions);
	    break;
	} else {
	    errorResult = Tcl_GetObjResult(interp);
	    Tcl_IncrRefCount(errorResult);
	    Tk_RestoreSavedOptions(&savedOptions);
	}
    }

    /* If -text was specified, then do not try to free the old value in
     * TextUpdateStringRep. */
    if (textCfg != elemX->textCfg && elemX->text == textCfg)
	elemX->text = NULL;

#ifdef TEXTVAR
    TextTraceSet(interp, elemX);
#endif

    if (error) {
	Tcl_SetObjResult(interp, errorResult);
	Tcl_DecrRefCount(errorResult);
	return TCL_ERROR;
    }

    return TCL_OK;
}

static int CreateProcText(TreeElementArgs *args)
{
/*    ElementText *elemX = (ElementText *) args->elem;*/

    return TCL_OK;
}

static ElementTextLayout2 *
TextRedoLayoutIfNeeded(
    char *func,
    TreeElementArgs *args,
    int fixedWidth
    )
{
    TreeElement elem = args->elem;
/*    ElementText *elemX = (ElementText *) elem;*/
    ElementText *masterX = (ElementText *) elem->master;
    int doLayout = 0;
    int wrap = TEXT_WRAP_WORD;
    ElementTextLayout *etl, *etlM = NULL;
    ElementTextLayout2 *etl2;

    etl = DynamicOption_FindData(elem->options, DOID_TEXT_LAYOUT);
    if (masterX != NULL)
	etlM = DynamicOption_FindData(elem->master->options, DOID_TEXT_LAYOUT);

    etl2 = DynamicOption_FindData(elem->options, DOID_TEXT_LAYOUT2);

    /* If text wrapping is disabled, the layout doesn't change */
    if (etl != NULL && etl->wrap != TEXT_WRAP_NULL)
	wrap = etl->wrap;
    else if ((etlM != NULL) && (etlM->wrap != TEXT_WRAP_NULL))
	wrap = etlM->wrap;
    if (wrap == TEXT_WRAP_NONE)
	return etl2;

    if (etl2 != NULL && etl2->layout != NULL) {

	/* See comment in NeededProc about totalWidth */
	if ((etl2->neededWidth != -1) && (fixedWidth >= etl2->neededWidth))
	    fixedWidth = etl2->totalWidth;

	/* Already layed out at this width */
	if (fixedWidth == etl2->layoutWidth)
	    return etl2;
    }

    /* May switch from layout -> no layout or vice versa */
    if (etl2 == NULL || etl2->layout == NULL)
	doLayout = 1;

    /* Width was constrained and we have more space now */
    else if ((etl2->layoutWidth != -1) && (fixedWidth > etl2->layoutWidth))
	doLayout = 1;

    /* Width was unconstrained or we have less space now */
    else {
	int width;
	TextLayout_Size(etl2->layout, &width, NULL);
	/* Redo if we are narrower than the layout */
	if (fixedWidth < width)
	    doLayout = 1;
    }
    if (doLayout)
	etl2 = TextUpdateLayout(func, args, fixedWidth, -1);
    if (etl2 != NULL)
	etl2->layoutWidth = (etl2->layout != NULL) ? fixedWidth : -1;
    return etl2;
}

static void DisplayProcText(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementText *elemX = (ElementText *) elem;
    ElementText *masterX = (ElementText *) elem->master;
    int state = args->state;
    int x = args->display.x, y = args->display.y;
    int width, height;
#ifdef DEPRECATED
    int draw;
#endif
    XColor *color;
    char *text = elemX->text;
    int textLen = elemX->textLen;
    Tk_Font tkfont;
    TextLayout layout = NULL;
    Tk_FontMetrics fm;
    GC gc;
    int bytesThatFit, pixelsForText;
    char *ellipsis = "...";
    TkRegion clipRgn = NULL;
    ElementTextLayout2 *etl2;
#ifdef TEXT_STYLE
    ElementTextStyle *ets, *etsM = NULL;
    int underline = TEXT_UNDERLINE_EMPTYVAL;
#endif
#if USE_ITEM_PIXMAP == 0
    TreeRectangle trClip, trElem;
#endif
    int inHeader = elem->stateDomain == STATE_DOMAIN_HEADER;
    int columnState = COLUMN_STATE_NORMAL;
    ElementTextLayout *etl, *etlM = NULL;
    Tcl_DString dString;
    enum ElidePosition elidePos = ElideEnd;
    int elideStart, elideEnd;

#ifdef DEPRECATED
    draw = DO_BooleanForState(tree, elem, DOID_TEXT_DRAW, state);
    if (!draw)
	return;
#endif

    if ((text == NULL) && (masterX != NULL)) {
	text = masterX->text;
	textLen = masterX->textLen;
    }

    if (text == NULL) /* always false (or layout sets height/width to zero) */
	return;

    if (inHeader) {
	if (state & STATE_HEADER_ACTIVE)
	    columnState = COLUMN_STATE_ACTIVE;
	else if (state & STATE_HEADER_PRESSED)
	    columnState = COLUMN_STATE_PRESSED;
    }

    DO_COLOR_FOR_STATE(color, DOID_TEXT_FILL, state);

    /* If no color is specified and this element is in a header, get the
     * system theme color.  If no theme color is provided then use the default
     * header text color. */
    if ((color == NULL) && inHeader) {
	if (!tree->useTheme || TreeTheme_GetHeaderTextColor(tree, columnState,
		&color) != TCL_OK) {
	    color = tree->defHeaderTextColor;
	}
	if (color->pixel == tree->defHeaderTextColor->pixel)
	    color = NULL;
    }

    tkfont = DO_FontForState(tree, elem, DOID_TEXT_FONT, state);

    /* FIXME: -font {"" {state...}}*/
    if ((color != NULL) || (tkfont != NULL)) {
	XGCValues gcValues;
	unsigned long gcMask = 0;
	if (color == NULL)
	    color = inHeader ? tree->defHeaderTextColor : tree->fgColorPtr;
	gcValues.foreground = color->pixel;
	gcMask |= GCForeground;
	if (tkfont == NULL)
	    tkfont = inHeader ? tree->tkfontHeader : tree->tkfont;
	gcValues.font = Tk_FontId(tkfont);
	gcMask |= GCFont;
	gcValues.graphics_exposures = False;
	gcMask |= GCGraphicsExposures;
	gc = Tree_GetGC(tree, gcMask, &gcValues);
    } else if (inHeader) {
	tkfont = tree->tkfontHeader;
	gc = tree->headerTextGC;
    } else {
	tkfont = tree->tkfont;
	gc = tree->textGC;
    }

#ifdef TEXT_STYLE
    ets = DynamicOption_FindData(elem->options, DOID_TEXT_STYLE);
    if (ets != NULL && ets->underline != TEXT_UNDERLINE_EMPTYVAL)
	underline = ets->underline;
    else if (masterX != NULL) {
	etsM = DynamicOption_FindData(elem->master->options, DOID_TEXT_STYLE);
	if (etsM != NULL && etsM->underline != TEXT_UNDERLINE_EMPTYVAL)
	    underline = etsM->underline;
    }
#endif

    etl2 = TextRedoLayoutIfNeeded("DisplayProcText", args, args->display.width);
    if (etl2 != NULL && etl2->layout != NULL)
	layout = etl2->layout;

    if (layout != NULL) {
	TextLayout_Size(layout, &width, &height);
	pixelsForText = width;
	/* Hack -- The actual size of the text may be slightly smaller than
	 * the available space when squeezed. If so we don't want to center
	 * the text horizontally */
	if ((etl2->neededWidth == -1) || (etl2->neededWidth > width))
	    width = args->display.width;
	AdjustForSticky(args->display.sticky,
	    args->display.width, args->display.height,
	    FALSE, FALSE,
	    &x, &y, &width, &height);
#if HEADER_OFFSET_HACK == 1
	/* FIXME: should have a layout option to control this. */
	if (inHeader && columnState == COLUMN_STATE_PRESSED)
	    x += 1, y += 1;
#endif
#if USE_ITEM_PIXMAP == 0
	/* Use clipping if text is larger than the display area. */
	trElem.x = x, trElem.y = y, trElem.width = args->display.width,
	    trElem.height = args->display.height;
	if (TreeRect_Intersect(&trClip, &trElem, &args->display.bounds)) {
	    clipRgn = Tree_GetRectRegion(tree, &trClip);
	    TkSetRegion(tree->display, gc, clipRgn);
	} else {
	    return;
	}
#else
	/* Use clipping if text is larger than the display area. */
	if (pixelsForText > args->display.width || height > args->display.height) {
	    XRectangle rect;
	    clipRgn = Tree_GetRegion(tree);
	    rect.x = x;
	    rect.y = y;
	    rect.width = args->display.width;
	    rect.height = args->display.height;
	    TkUnionRectWithRegion(&rect, clipRgn, clipRgn);
	    TkSetRegion(tree->display, gc, clipRgn);
	}
#endif
	TextLayout_Draw(tree->display, args->display.drawable, gc,
		layout, x, y, 0, -1, underline);
	if (clipRgn != NULL) {
	    Tree_UnsetClipMask(tree, args->display.drawable, gc);
	    Tree_FreeRegion(tree, clipRgn);
	}
	return;
    }

    Tk_GetFontMetrics(tkfont, &fm);

    etl = DynamicOption_FindData(elem->options, DOID_TEXT_LAYOUT);
    if (etl != NULL && etl->elidePos != -1)
	elidePos = etl->elidePos;
    else if (masterX != NULL) {
	etlM = DynamicOption_FindData(elem->master->options, DOID_TEXT_LAYOUT);
	if (etlM != NULL && etlM->elidePos != -1)
	    elidePos = etlM->elidePos;
    }

    pixelsForText = args->display.width;
    bytesThatFit = Tree_Ellipsis(tkfont, text, textLen, &pixelsForText,
	    ellipsis, FALSE, elidePos, &elideStart, &elideEnd, &dString);
    width = pixelsForText, height = fm.linespace;
    /* Hack -- The actual size of the text may be slightly smaller than
    * the available space when squeezed. If so we don't want to center
    * the text horizontally */
    if (bytesThatFit != textLen)
	width = args->display.width;
    AdjustForSticky(args->display.sticky,
	args->display.width, args->display.height,
	FALSE, FALSE,
	&x, &y, &width, &height);
#if HEADER_OFFSET_HACK == 1
    /* FIXME: should have a layout option to control this. */
    if (inHeader && columnState == COLUMN_STATE_PRESSED)
	x += 1, y += 1;
#endif
#if USE_ITEM_PIXMAP == 0
    /* Use clipping if text is larger than the display area. */
    trElem.x = x, trElem.y = y, trElem.width = args->display.width,
	trElem.height = args->display.height;
    if (TreeRect_Intersect(&trClip, &trElem, &args->display.bounds)) {
	clipRgn = Tree_GetRectRegion(tree, &trClip);
	TkSetRegion(tree->display, gc, clipRgn);
    } else {
	Tcl_DStringFree(&dString);
	return;
    }
#else
    /* Use clipping if text is larger than the display area. */
    if (pixelsForText > args->display.width || height > args->display.height) {
	XRectangle rect;
	clipRgn = Tree_GetRegion(tree);
	rect.x = x;
	rect.y = y;
	rect.width = args->display.width;
	rect.height = args->display.height;
	TkUnionRectWithRegion(&rect, clipRgn, clipRgn);
	TkSetRegion(tree->display, gc, clipRgn);
    }
#endif
    {
	char *buf = Tcl_DStringValue(&dString);
	int bufLen = Tcl_DStringLength(&dString);
	Tk_DrawChars(tree->display, args->display.drawable, gc,
		tkfont, buf, bufLen, x, y + fm.ascent);
#ifdef TEXT_STYLE
	if (underline >= 0 && underline <= elideStart) {
	    const char *fstBytePtr = Tcl_UtfAtIndex(buf, underline);
	    const char *sndBytePtr = Tcl_UtfNext(fstBytePtr);
	    Tk_UnderlineChars(tree->display, args->display.drawable, gc,
		    tkfont, buf, x, y + fm.ascent,
		    (int) (fstBytePtr - buf), (int) (sndBytePtr - buf));
	} else if (underline >= 0 && elideEnd >= 0 && underline >= elideEnd && underline < Tcl_NumUtfChars(text, textLen)) {
	    if (elideEnd > 0)
		underline += strlen(ellipsis); /* chars added before elideEnd */
	    underline -= elideEnd - MAX(-1, elideStart) - 1; /* chars removed before elideEnd*/
	    {
		const char *fstBytePtr = Tcl_UtfAtIndex(buf, underline);
		const char *sndBytePtr = Tcl_UtfNext(fstBytePtr);
		Tk_UnderlineChars(tree->display, args->display.drawable, gc,
			tkfont, buf, x, y + fm.ascent,
			(int) (fstBytePtr - buf), (int) (sndBytePtr - buf));
	    }
	}
#endif
    }
    if (clipRgn != NULL) {
	Tree_UnsetClipMask(tree, args->display.drawable, gc);
	Tree_FreeRegion(tree, clipRgn);
    }
    Tcl_DStringFree(&dString);
}

static void NeededProcText(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementText *elemX = (ElementText *) elem;
    ElementText *masterX = (ElementText *) elem->master;
    int state = args->state;
    char *text = NULL;
    int textLen = 0;
    Tk_Font tkfont;
    Tk_FontMetrics fm;
    int width = 0, height = 0;
    ElementTextLayout *etl, *etlM = NULL;
    ElementTextLayout2 *etl2;
    int inHeader = elem->stateDomain == STATE_DOMAIN_HEADER;

    etl = DynamicOption_FindData(args->elem->options, DOID_TEXT_LAYOUT);
    if (masterX != NULL)
	etlM = DynamicOption_FindData(args->elem->master->options, DOID_TEXT_LAYOUT);

    if ((masterX != NULL) && (masterX->textLen == STRINGREP_INVALID)) {
	args->elem = (TreeElement) masterX;
	TextUpdateStringRep(args);
	args->elem = elem;
    }
    if (elemX->textLen == STRINGREP_INVALID) {
	TextUpdateStringRep(args);
    }

    etl2 = TextUpdateLayout("NeededProcText", args, args->needed.fixedWidth,
	    args->needed.maxWidth);
    if (etl2 != NULL) {
	etl2->layoutWidth = -1;
	etl2->neededWidth = -1;
    }

    if (etl2 != NULL && etl2->layout != NULL) {
	TextLayout_Size(etl2->layout, &width, &height);
	if (args->needed.fixedWidth >= 0)
	    etl2->layoutWidth = args->needed.fixedWidth;
	else if (args->needed.maxWidth >= 0)
	    etl2->layoutWidth = args->needed.maxWidth;
	etl2->neededWidth = width;

	/*
	 * Hack -- If we call TextLayout_Compute() with the same width
	 * returned by TextLayout_Size(), we may get a different layout.
	 * I think this has to do with whitespace at the end of lines.
	 * So if HeightProc or DisplayProc is given neededWidth, I do the
	 * layout at totalWidth, not neededWidth.
	 */
	etl2->totalWidth = TextLayout_TotalWidth(etl2->layout);
    } else {
	if (elemX->text != NULL) {
	    text = elemX->text;
	    textLen = elemX->textLen;
	} else if ((masterX != NULL) && (masterX->text != NULL)) {
	    text = masterX->text;
	    textLen = masterX->textLen;
	}
	if (textLen > 0) {
	    int maxWidth = -1;

	    tkfont = DO_FontForState(tree, elem, DOID_TEXT_FONT, state);
	    if (tkfont == NULL)
		tkfont = inHeader ? tree->tkfontHeader : tree->tkfont;

	    width = Tk_TextWidth(tkfont, text, textLen);
	    if (etl != NULL && etl->widthObj != NULL)
		maxWidth = etl->width;
	    else if ((etlM != NULL) && (etlM->widthObj != NULL))
		maxWidth = etlM->width;
	    if ((maxWidth >= 0) && (maxWidth < width))
		width = maxWidth;

	    Tk_GetFontMetrics(tkfont, &fm);
	    height = fm.linespace;
	}
    }

    args->needed.width = width;
    args->needed.height = height;
}

static void HeightProcText(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementText *elemX = (ElementText *) elem;
    ElementText *masterX = (ElementText *) elem->master;
    int inHeader = elem->stateDomain == STATE_DOMAIN_HEADER;
    int state = args->state;
    int height = 0;
    int textLen = 0;
    Tk_Font tkfont;
    Tk_FontMetrics fm;
    ElementTextLayout2 *etl2;

    etl2 = TextRedoLayoutIfNeeded("HeightProcText", args, args->height.fixedWidth);

    if (etl2 != NULL && etl2->layout != NULL) {
	TextLayout_Size(etl2->layout, NULL, &height);
    } else {
	if (elemX->text != NULL) {
	    textLen = elemX->textLen;
	} else if ((masterX != NULL) && (masterX->text != NULL)) {
	    textLen = masterX->textLen;
	}
	if (textLen > 0) {
	    tkfont = DO_FontForState(tree, elem, DOID_TEXT_FONT, state);
	    if (tkfont == NULL)
		tkfont = inHeader ? tree->tkfontHeader : tree->tkfont;
	    Tk_GetFontMetrics(tkfont, &fm);
	    height = fm.linespace;
	}
    }

    args->height.height = height;
}

int
TreeElement_GetSortData(
    TreeCtrl *tree,
    TreeElement elem,
    int type,
    long *lv,
    double *dv,
    char **sv)
{
    ElementText *elemX = (ElementText *) elem;
    ElementText *masterX = (ElementText *) elem->master;
    ElementTextData *etd, *etdM = NULL;
    Tcl_Obj *dataObj = NULL;
    int dataType = TDT_NULL;

    etd = DynamicOption_FindData(elem->options, DOID_TEXT_DATA);
    if (etd != NULL) {
	dataObj = etd->dataObj;
	dataType = etd->dataType;
    }
    if (dataType == TDT_NULL && masterX != NULL) {
	etdM = DynamicOption_FindData(elem->master->options, DOID_TEXT_DATA);
	/* FIXME: get dataObj from master? */
	if (etdM != NULL)
	    dataType = etdM->dataType;
    }

    switch (type) {
	case SORT_ASCII:
	case SORT_DICT:
	    if (dataObj != NULL && dataType != TDT_NULL)
		(*sv) = Tcl_GetString(dataObj);
	    else
		(*sv) = elemX->textCfg;
	    break;
	case SORT_DOUBLE:
	    if (dataObj != NULL && dataType == TDT_DOUBLE) {
		if (Tcl_GetDoubleFromObj(tree->interp, dataObj, dv) != TCL_OK)
		    return TCL_ERROR;
		break;
	    }
	    if (elemX->textCfg != NULL) {
		if (Tcl_GetDouble(tree->interp, elemX->textCfg, dv) != TCL_OK)
		    return TCL_ERROR;
		break;
	    }
	    FormatResult(tree->interp, "can't get a double from an empty -text value");
	    return TCL_ERROR;
	case SORT_LONG:
	    if (dataObj != NULL && dataType != TDT_NULL) {
		if (dataType == TDT_LONG || dataType == TDT_TIME) {
		    if (Tcl_GetLongFromObj(tree->interp, dataObj, lv) != TCL_OK)
			return TCL_ERROR;
		    break;
		}
		if (dataType == TDT_INTEGER) {
		    int iv;
		    if (Tcl_GetIntFromObj(tree->interp, dataObj, &iv) != TCL_OK)
			return TCL_ERROR;
		    (*lv) = iv;
		    break;
		}
	    }
	    if (elemX->textCfg != NULL) {
		Tcl_Obj obj;

		obj.refCount = 1;
		obj.bytes = (char *) elemX->textCfg;
		obj.length = (int) strlen(elemX->textCfg);
		obj.typePtr = NULL;

		if (Tcl_GetLongFromObj(tree->interp, &obj, lv) != TCL_OK)
		    return TCL_ERROR;
		break;
	    }
	    FormatResult(tree->interp, "can't get a long from an empty -text value");
	    return TCL_ERROR;
    }
    return TCL_OK;
}

static int StateProcText(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
/*    ElementText *elemX = (ElementText *) elem;
    ElementText *masterX = (ElementText *) elem->master;*/
#ifdef DEPRECATED
    int draw1, draw2;
#endif
    XColor *f1, *f2;
    Tk_Font tkfont1, tkfont2;

    if (!args->states.visible2)
	return 0;

    tkfont1 = DO_FontForState(tree, elem, DOID_TEXT_FONT, args->states.state1);
    tkfont2 = DO_FontForState(tree, elem, DOID_TEXT_FONT, args->states.state2);
    if (tkfont1 != tkfont2)
	return CS_DISPLAY | CS_LAYOUT;

    if (!args->states.draw2)
	return 0;
#ifdef DEPRECATED
    draw1 = DO_BooleanForState(tree, elem, DOID_TEXT_DRAW, args->states.state1);
    draw2 = DO_BooleanForState(tree, elem, DOID_TEXT_DRAW, args->states.state2);
    if ((draw1 != 0) != (draw2 != 0))
	return CS_DISPLAY;
    if (draw2 == 0)
	return 0;
#endif

    DO_COLOR_FOR_STATE(f1, DOID_TEXT_FILL, args->states.state1);
    DO_COLOR_FOR_STATE(f2, DOID_TEXT_FILL, args->states.state2);
    if (f1 != f2)
	return CS_DISPLAY;

    return 0;
}

static int UndefProcText(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
/*    ElementText *elemX = (ElementText *) elem;*/
    int modified = 0;
    PerStateInfo *psi;

    if ((psi = DynamicOption_FindData(args->elem->options, DOID_TEXT_DRAW)) != NULL)
	modified |= PerStateInfo_Undefine(tree, &pstBoolean, psi, elem->stateDomain, args->state);
    if ((psi = DynamicOption_FindData(args->elem->options, DOID_TEXT_FILL)) != NULL)
	modified |= PerStateInfo_Undefine(tree, &pstColor, psi, elem->stateDomain, args->state);
    if ((psi = DynamicOption_FindData(args->elem->options, DOID_TEXT_FONT)) != NULL)
	modified |= PerStateInfo_Undefine(tree, &pstFont, psi, elem->stateDomain, args->state);

    return modified;
}

static int ActualProcText(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
/*    ElementText *elemX = (ElementText *) args->elem;
    ElementText *masterX = (ElementText *) args->elem->master;*/
    static const char *optionName[] = {
#ifdef DEPRECATED
	"-draw",
#endif
	"-fill", "-font",
	(char *) NULL };
    int index;
    Tcl_Obj *obj = NULL;

    if (Tcl_GetIndexFromObj(tree->interp, args->actual.obj, optionName,
		"option", 0, &index) != TCL_OK)
	return TCL_ERROR;

    switch (index) {
#ifdef DEPRECATED
	case 0: {
	    obj = DO_ObjectForState(tree, &pstBoolean, args->elem, DOID_TEXT_DRAW, args->state);
	    break;
	}
	case 1: {
	    obj = DO_ObjectForState(tree, &pstColor, args->elem, DOID_TEXT_FILL, args->state);
	    break;
	}
	case 2: {
	    obj = DO_ObjectForState(tree, &pstFont, args->elem, DOID_TEXT_FONT, args->state);
	    break;
	}
#else
	case 0: {
	    obj = DO_ObjectForState(tree, &pstColor, args->elem, DOID_TEXT_FILL, args->state);
	    break;
	}
	case 1: {
	    obj = DO_ObjectForState(tree, &pstFont, args->elem, DOID_TEXT_FONT, args->state);
	    break;
	}
#endif
    }
    if (obj != NULL)
	Tcl_SetObjResult(tree->interp, obj);
    return TCL_OK;
}

TreeElementType treeElemTypeText = {
    "text",
    sizeof(ElementText),
    textOptionSpecs,
    NULL,
    CreateProcText,
    DeleteProcText,
    ConfigProcText,
    DisplayProcText,
    NeededProcText,
    HeightProcText,
    WorldChangedProcText,
    StateProcText,
    UndefProcText,
    ActualProcText,
    NULL /* onScreenProc */
};

/*****/

typedef struct ElementWindow ElementWindow;

struct ElementWindow
{
    TreeElement_ header;
#ifdef DEPRECATED
    PerStateInfo draw;		/* -draw */
#endif
    TreeCtrl *tree;
    TreeItem item; 		/* Needed if window changes size */
    TreeItemColumn column; 	/* Needed if window changes size */
    Tk_Window tkwin;		/* Window associated with item.  NULL means
				 * window has been destroyed. */
    int destroy;		/* Destroy window when element
				 * is deleted */
#define CLIP_WINDOW 1
#ifdef CLIP_WINDOW
    int clip;			/* TRUE if tkwin is a borderless frame
				 * widget whose first child is the actual
				 * window we want displayed. */
    Tk_Window child;		/* The first child of tkwin. tkwin is resized
				 * so that it is never out-of-bounds, and
				 * the child is positioned within tkwin to
				 * provide clipping of the child. */
#endif
};

#define EWIN_CONF_WINDOW 0x0001
#ifdef DEPRECATED
#define EWIN_CONF_DRAW 0x0002
#endif

static Tk_OptionSpec windowOptionSpecs[] = {
#ifdef CLIP_WINDOW
    {TK_OPTION_CUSTOM, "-clip", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(ElementWindow, clip),
     TK_OPTION_NULL_OK, (ClientData) &booleanCO, 0},
#endif
    {TK_OPTION_CUSTOM, "-destroy", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(ElementWindow, destroy),
     TK_OPTION_NULL_OK, (ClientData) &booleanCO, 0},
#ifdef DEPRECATED
    {TK_OPTION_CUSTOM, "-draw", (char *) NULL, (char *) NULL,
     (char *) NULL,
     offsetof(ElementWindow, draw.obj), offsetof(ElementWindow, draw),
     TK_OPTION_NULL_OK, (ClientData) NULL, EWIN_CONF_DRAW},
#endif
    {TK_OPTION_WINDOW, "-window", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, offsetof(ElementWindow, tkwin),
     TK_OPTION_NULL_OK, (ClientData) NULL, EWIN_CONF_WINDOW},
    {TK_OPTION_END, (char *) NULL, (char *) NULL, (char *) NULL,
     (char *) NULL, 0, -1, 0, (ClientData) NULL, 0}
};

static void
WinItemStructureProc(
    ClientData clientData,	/* Pointer to record describing window elem. */
    XEvent *eventPtr)		/* Describes what just happened. */
{
    ElementWindow *elemX = clientData;

    if (eventPtr->type == DestroyNotify) {
	elemX->tkwin = elemX->child = NULL;
	Tree_ElementChangedItself(elemX->tree, elemX->item, elemX->column,
	    (TreeElement) elemX, EWIN_CONF_WINDOW, CS_LAYOUT | CS_DISPLAY);
    }
}

static void
WinItemRequestProc(
    ClientData clientData,		/* Pointer to record for window item. */
    Tk_Window tkwin)			/* Window that changed its desired
					 * size. */
{
    ElementWindow *elemX = clientData;

#ifdef CLIP_WINDOW
    /* We don't care about size changes for the clip window. */
    if (elemX->child != NULL && tkwin != elemX->child)
	return;
#endif
    Tree_ElementChangedItself(elemX->tree, elemX->item, elemX->column,
	(TreeElement) elemX, EWIN_CONF_WINDOW, CS_LAYOUT | CS_DISPLAY);
}

static void
WinItemLostSlaveProc(
    ClientData clientData,	/* WindowItem structure for slave window that
				 * was stolen away. */
    Tk_Window tkwin)		/* Tk's handle for the slave window. */
{
    ElementWindow *elemX = clientData;
    TreeCtrl *tree = elemX->tree;

#ifdef CLIP_WINDOW
    /* If either window is lost to another geometry manager, forget
     * about both windows. */
    if (elemX->child != NULL) {
	Tk_DeleteEventHandler(elemX->child, StructureNotifyMask,
		WinItemStructureProc, (ClientData) elemX);
	if (tkwin != elemX->child) {
	    Tk_ManageGeometry(elemX->child, (Tk_GeomMgr *) NULL,
		    (ClientData) NULL);
	    Tk_UnmapWindow(elemX->child);
	}
	elemX->child = NULL;
    }
    if (elemX->tkwin != NULL) {
	Tk_DeleteEventHandler(elemX->tkwin, StructureNotifyMask,
		WinItemStructureProc, (ClientData) elemX);
	if (tkwin != elemX->tkwin) {
	    Tk_ManageGeometry(elemX->tkwin, (Tk_GeomMgr *) NULL,
		    (ClientData) NULL);
	    if (tree->tkwin != Tk_Parent(elemX->tkwin)) {
		Tk_UnmaintainGeometry(elemX->tkwin, tree->tkwin);
	    }
	    Tk_UnmapWindow(elemX->tkwin);
	}
	elemX->tkwin = NULL;
    }
#else
    Tk_DeleteEventHandler(elemX->tkwin, StructureNotifyMask,
	    WinItemStructureProc, (ClientData) elemX);
    if (tree->tkwin != Tk_Parent(elemX->tkwin)) {
	Tk_UnmaintainGeometry(elemX->tkwin, tree->tkwin);
    }
    Tk_UnmapWindow(elemX->tkwin);
    elemX->tkwin = NULL;
#endif
    Tree_ElementChangedItself(elemX->tree, elemX->item, elemX->column,
	(TreeElement) elemX, EWIN_CONF_WINDOW, CS_LAYOUT | CS_DISPLAY);
}

static Tk_GeomMgr winElemGeomType = {
    "treectrl",				/* name */
    WinItemRequestProc,			/* requestProc */
    WinItemLostSlaveProc,		/* lostSlaveProc */
};

static void DeleteProcWindow(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementWindow *elemX = (ElementWindow *) elem;
    ElementWindow *masterX = (ElementWindow *) elem->master;

    if (elemX->tkwin != NULL) {
#ifdef CLIP_WINDOW
	if (elemX->child != NULL) {
	    Tk_DeleteEventHandler(elemX->child, StructureNotifyMask,
		    WinItemStructureProc, (ClientData) elemX);
	    Tk_ManageGeometry(elemX->child, (Tk_GeomMgr *) NULL,
		    (ClientData) NULL);
	    Tk_UnmapWindow(elemX->child);
	    elemX->child = NULL;
	}
#endif
	Tk_DeleteEventHandler(elemX->tkwin, StructureNotifyMask,
		WinItemStructureProc, (ClientData) elemX);
	Tk_ManageGeometry(elemX->tkwin, (Tk_GeomMgr *) NULL,
		(ClientData) NULL);
	if (tree->tkwin != Tk_Parent(elemX->tkwin)) {
	    Tk_UnmaintainGeometry(elemX->tkwin, tree->tkwin);
	}
	Tk_UnmapWindow(elemX->tkwin);

	if ((elemX->destroy == 1) ||
		((masterX != NULL) && (masterX->destroy == 1))) {
	    Tk_DestroyWindow(elemX->tkwin);
	}

	elemX->tkwin = NULL;
    }
}

static int WorldChangedProcWindow(TreeElementArgs *args)
{
    int flagM = args->change.flagMaster;
    int flagS = args->change.flagSelf;
    int mask = 0;

#ifdef DEPRECATED
    if ((flagS | flagM) & (EWIN_CONF_DRAW))
	mask |= CS_DISPLAY;
#endif

    if ((flagS | flagM) & (EWIN_CONF_WINDOW))
	mask |= CS_DISPLAY | CS_LAYOUT;

    return mask;
}

static int ConfigProcWindow(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementWindow *elemX = (ElementWindow *) elem;
    ElementWindow *masterX = (ElementWindow *) elem->master;
    ElementWindow savedX;
    Tk_SavedOptions savedOptions;
    int error;
    Tcl_Obj *errorResult = NULL;

    savedX.tkwin = elemX->tkwin;

    for (error = 0; error <= 1; error++) {
	if (error == 0) {
	    if (Tree_SetOptions(tree, elem->stateDomain, elemX,
			elem->typePtr->optionTable,
			args->config.objc, args->config.objv,
			&savedOptions, &args->config.flagSelf) != TCL_OK) {
		args->config.flagSelf = 0;
		continue;
	    }

	    if (args->config.flagSelf & EWIN_CONF_WINDOW) {
		if ((elem->master == NULL) && (elemX->tkwin != NULL)){
		    FormatResult(tree->interp, "can't specify -window for a master element");
		    continue;
		}
	    }

	    Tk_FreeSavedOptions(&savedOptions);
	    break;
	} else {
	    errorResult = Tcl_GetObjResult(tree->interp);
	    Tcl_IncrRefCount(errorResult);
	    Tk_RestoreSavedOptions(&savedOptions);

	    Tcl_SetObjResult(tree->interp, errorResult);
	    Tcl_DecrRefCount(errorResult);
	    return TCL_ERROR;
	}
    }

    if (savedX.tkwin != elemX->tkwin) {
	if (savedX.tkwin != NULL) {
#ifdef CLIP_WINDOW
	    if (elemX->child != NULL) {
		Tk_DeleteEventHandler(elemX->child, StructureNotifyMask,
			WinItemStructureProc, (ClientData) elemX);
		Tk_ManageGeometry(elemX->child, (Tk_GeomMgr *) NULL,
			(ClientData) NULL);
		Tk_UnmapWindow(elemX->child);
		elemX->child = NULL;
	    }
#endif
	    Tk_DeleteEventHandler(savedX.tkwin, StructureNotifyMask,
		    WinItemStructureProc, (ClientData) elemX);
	    Tk_ManageGeometry(savedX.tkwin, (Tk_GeomMgr *) NULL,
		    (ClientData) NULL);
	    Tk_UnmaintainGeometry(savedX.tkwin, tree->tkwin);
	    Tk_UnmapWindow(savedX.tkwin);
	}
	if (elemX->tkwin != NULL) {
	    Tk_Window ancestor, parent;

	    /*
	     * Make sure that the treectrl is either the parent of the
	     * window associated with the element or a descendant of that
	     * parent.  Also, don't allow a top-of-hierarchy window to be
	     * managed inside a treectrl.
	     */

	    parent = Tk_Parent(elemX->tkwin);
	    for (ancestor = tree->tkwin; ;
		    ancestor = Tk_Parent(ancestor)) {
		if (ancestor == parent) {
		    break;
		}
		if (((Tk_FakeWin *) (ancestor))->flags & TK_TOP_HIERARCHY) {
		    badWindow:
		    FormatResult(tree->interp,
			    "can't use %s in a window element of %s",
			    Tk_PathName(elemX->tkwin),
			    Tk_PathName(tree->tkwin));
		    elemX->tkwin = NULL;
		    return TCL_ERROR;
		}
	    }
	    if (((Tk_FakeWin *) (elemX->tkwin))->flags & TK_TOP_HIERARCHY) {
		goto badWindow;
	    }
	    if (elemX->tkwin == tree->tkwin) {
		goto badWindow;
	    }
#ifdef CLIP_WINDOW
	    if ((elemX->clip == 1) || ((masterX != NULL) && (masterX->clip == 1))) {
		elemX->child = (Tk_Window) ((TkWindow *) elemX->tkwin)->childList;
		if (elemX->child != NULL) {
		    Tk_CreateEventHandler(elemX->child, StructureNotifyMask,
			    WinItemStructureProc, (ClientData) elemX);
		    Tk_ManageGeometry(elemX->child, &winElemGeomType,
			    (ClientData) elemX);
		}
	    }
#endif
	    Tk_CreateEventHandler(elemX->tkwin, StructureNotifyMask,
		    WinItemStructureProc, (ClientData) elemX);
	    Tk_ManageGeometry(elemX->tkwin, &winElemGeomType,
		    (ClientData) elemX);
	}
    }
#if 0
    if ((elemX->tkwin != NULL)
	    && (itemPtr->state == TK_STATE_HIDDEN)) {
	if (tree->tkwin == Tk_Parent(elemX->tkwin)) {
	    Tk_UnmapWindow(elemX->tkwin);
	} else {
	    Tk_UnmaintainGeometry(elemX->tkwin, tree->tkwin);
	}
    }
#endif
    return TCL_OK;
}

static int CreateProcWindow(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementWindow *elemX = (ElementWindow *) elem;

    elemX->tree = tree;
    elemX->item = args->create.item;
    elemX->column = args->create.column;
    elemX->destroy = -1;
#ifdef CLIP_WINDOW
    elemX->clip = -1;
#endif
    return TCL_OK;
}

static void DisplayProcWindow(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementWindow *elemX = (ElementWindow *) elem;
    ElementWindow *masterX = (ElementWindow *) elem->master;
    int state = args->state;
    int x = args->display.x, y = args->display.y;
    int minX, maxX, minY, maxY;
    int width, height;
    int match, match2;
#ifdef DEPRECATED
    int draw;
#endif
    int requests;

#ifdef DEPRECATED
    BOOLEAN_FOR_STATE(draw, draw, state);
    if (!draw)
	goto hideIt;
#endif

    if (elemX->tkwin == NULL)
	return;

#ifdef CLIP_WINDOW
    if (elemX->child != NULL) {
	width = Tk_ReqWidth(elemX->child);
	height = Tk_ReqHeight(elemX->child);
    } else {
	width = Tk_ReqWidth(elemX->tkwin);
	height = Tk_ReqHeight(elemX->tkwin);
    }
    if (width < 1 || height < 1)
	goto hideIt;
#else
    width = Tk_ReqWidth(elemX->tkwin);
    height = Tk_ReqHeight(elemX->tkwin);
#endif
    AdjustForSticky(args->display.sticky,
	args->display.width, args->display.height,
	TRUE, TRUE,
	&x, &y, &width, &height);

    x += tree->drawableXOrigin - tree->xOrigin;
    y += tree->drawableYOrigin - tree->yOrigin;

    /* -squeeze layout may give the element less space than requested. */
    if (width > args->display.width)
	width = args->display.width;
    if (height > args->display.height)
	height = args->display.height;

    TreeRect_XYXY(args->display.bounds, &minX, &minY, &maxX, &maxY);

    /*
     * If the window is completely out of the visible area of the treectrl
     * then unmap it.  The window could suddenly reappear if the treectrl
     * window gets resized.
     */

    if (((x + width) <= minX) || ((y + height) <= minY)
	    || (x >= maxX) || (y >= maxY)) {
hideIt:
	if (tree->tkwin == Tk_Parent(elemX->tkwin)) {
	    Tk_UnmapWindow(elemX->tkwin);
	} else {
	    Tk_UnmaintainGeometry(elemX->tkwin, tree->tkwin);
	}
	return;
    }

    TreeDisplay_GetReadyForTrouble(tree, &requests);

#ifdef CLIP_WINDOW
    if (elemX->child != NULL) {
	int cx = x, cy = y, cw = width, ch = height; /* clip win coords */

	if (cx < minX) {
	    cw -= minX - cx;
	    cx = minX;
	}
	if (cy < minY) {
	    ch -= minY - cy;
	    cy = minY;
	}
	if (cx + cw > maxX)
	    cw = maxX - cx;
	if (cy + ch > maxY)
	    ch = maxY - cy;

	/*
	* Reposition and map the window (but in different ways depending
	* on whether the treectrl is the window's parent).
	*/

	if (tree->tkwin == Tk_Parent(elemX->tkwin)) {
	    if ((cx != Tk_X(elemX->tkwin)) || (cy != Tk_Y(elemX->tkwin))
		    || (cw != Tk_Width(elemX->tkwin))
		    || (ch != Tk_Height(elemX->tkwin))) {
		Tk_MoveResizeWindow(elemX->tkwin, cx, cy, cw, ch);
		if (TreeDisplay_WasThereTrouble(tree, requests))
		    return;
	    }
	    Tk_MapWindow(elemX->tkwin);
	} else {
	    Tk_MaintainGeometry(elemX->tkwin, tree->tkwin, cx, cy, cw, ch);
	}
	if (TreeDisplay_WasThereTrouble(tree, requests))
	    return;

	/*
	 * Position the child window within the clip window.
	 */
	x -= cx;
	y -= cy;
	if ((x != Tk_X(elemX->child)) || (y != Tk_Y(elemX->child))
		|| (width != Tk_Width(elemX->child))
		|| (height != Tk_Height(elemX->child))) {
	    Tk_MoveResizeWindow(elemX->child, x, y, width, height);
	    if (TreeDisplay_WasThereTrouble(tree, requests))
		return;
	}
	Tk_MapWindow(elemX->child);
	return;
    }
#endif /* CLIP_WINDOW */

    /*
     * Reposition and map the window (but in different ways depending
     * on whether the treectrl is the window's parent).
     */

    if (tree->tkwin == Tk_Parent(elemX->tkwin)) {
	if ((x != Tk_X(elemX->tkwin)) || (y != Tk_Y(elemX->tkwin))
		|| (width != Tk_Width(elemX->tkwin))
		|| (height != Tk_Height(elemX->tkwin))) {
	    Tk_MoveResizeWindow(elemX->tkwin, x, y, width, height);
	    if (TreeDisplay_WasThereTrouble(tree, requests))
		return;
	}
	Tk_MapWindow(elemX->tkwin);
    } else {
	Tk_MaintainGeometry(elemX->tkwin, tree->tkwin, x, y,
		width, height);
    }
}

static void NeededProcWindow(TreeElementArgs *args)
{
/*    TreeCtrl *tree = args->tree;*/
    TreeElement elem = args->elem;
    ElementWindow *elemX = (ElementWindow *) elem;
/*    ElementWindow *masterX = (ElementWindow *) elem->master;
    int state = args->state;*/
    int width = 0, height = 0;

#ifdef CLIP_WINDOW
    if (elemX->child != NULL) {
	width = Tk_ReqWidth(elemX->child);
	if (width <= 0) {
	    width = 1;
	}
	height = Tk_ReqHeight(elemX->child);
	if (height <= 0) {
	    height = 1;
	}
    } else
#endif
    if (elemX->tkwin) {
	width = Tk_ReqWidth(elemX->tkwin);
	if (width <= 0) {
	    width = 1;
	}
	height = Tk_ReqHeight(elemX->tkwin);
	if (height <= 0) {
	    height = 1;
	}
    }
    args->needed.width = width;
    args->needed.height = height;
}

static int StateProcWindow(TreeElementArgs *args)
{
#ifdef DEPRECATED
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementBitmap *elemX = (ElementBitmap *) elem;
    ElementBitmap *masterX = (ElementBitmap *) elem->master;
    int match, match2;
    int draw1, draw2;

    if (!args->states.visible2 || !args->states.draw2)
	return 0;

    BOOLEAN_FOR_STATE(draw1, draw, args->states.state1)
    BOOLEAN_FOR_STATE(draw2, draw, args->states.state2)
    if ((draw1 != 0) != (draw2 != 0))
	return CS_DISPLAY;
#endif

    return 0;
}

static int UndefProcWindow(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementWindow *elemX = (ElementWindow *) elem;
    int modified = 0;

#ifdef DEPRECATED
    modified |= PerStateInfo_Undefine(tree, &pstBoolean, &elemX->draw, elem->stateDomain, args->state);
#endif
    return modified;
}

static int ActualProcWindow(TreeElementArgs *args)
{
#ifdef DEPRECATED
    TreeCtrl *tree = args->tree;
    ElementWindow *elemX = (ElementWindow *) args->elem;
    ElementWindow *masterX = (ElementWindow *) args->elem->master;
    static const char *optionName[] = {
	"-draw",
	(char *) NULL };
    int index, match, matchM;
    Tcl_Obj *obj = NULL;

    if (Tcl_GetIndexFromObj(tree->interp, args->actual.obj, optionName,
		"option", 0, &index) != TCL_OK)
	return TCL_ERROR;

    switch (index) {
	case 0: {
	    OBJECT_FOR_STATE(obj, pstBoolean, draw, args->state)
	    break;
	}
    }
    if (obj != NULL)
	Tcl_SetObjResult(tree->interp, obj);
#endif
    return TCL_OK;
}

static void OnScreenProcWindow(TreeElementArgs *args)
{
    TreeCtrl *tree = args->tree;
    TreeElement elem = args->elem;
    ElementWindow *elemX = (ElementWindow *) elem;

    if (!args->screen.visible && (elemX->tkwin != NULL)) {
	if (tree->tkwin == Tk_Parent(elemX->tkwin)) {
	    Tk_UnmapWindow(elemX->tkwin);
	} else {
	    Tk_UnmaintainGeometry(elemX->tkwin, tree->tkwin);
	}
    }
}

TreeElementType treeElemTypeWindow = {
    "window",
    sizeof(ElementWindow),
    windowOptionSpecs,
    NULL,
    CreateProcWindow,
    DeleteProcWindow,
    ConfigProcWindow,
    DisplayProcWindow,
    NeededProcWindow,
    NULL, /* heightProc */
    WorldChangedProcWindow,
    StateProcWindow,
    UndefProcWindow,
    ActualProcWindow,
    OnScreenProcWindow
};

/*****/

/* This structure holds the list of element types for an interpreter.
 * Each element type has a Tk_OptionTable that is per-interp. */
typedef struct ElementAssocData ElementAssocData;
struct ElementAssocData
{
    TreeElementType *typeList;
};

int TreeElement_TypeFromObj(TreeCtrl *tree, Tcl_Obj *objPtr, TreeElementType **typePtrPtr)
{
    Tcl_Interp *interp = tree->interp;
    ElementAssocData *assocData;
    char *typeStr;
    Tcl_Size length;
    TreeElementType *typeList;
    TreeElementType *typePtr, *matchPtr = NULL;

    assocData = Tcl_GetAssocData(interp, "TreeCtrlElementTypes", NULL);
    typeList = assocData->typeList;

    typeStr = Tcl_GetStringFromObj(objPtr, &length);
    if (!length) {
	FormatResult(interp, "invalid element type \"\"");
	return TCL_ERROR;
    }
    for (typePtr = typeList;
	typePtr != NULL;
	typePtr = typePtr->next) {
	if ((typeStr[0] == typePtr->name[0]) &&
		!strncmp(typeStr, typePtr->name, length)) {
	    if (matchPtr != NULL) {
		FormatResult(interp,
			"ambiguous element type \"%s\"",
			typeStr);
		return TCL_ERROR;
	    }
	    matchPtr = typePtr;
	}
    }
    if (matchPtr == NULL) {
	FormatResult(interp, "unknown element type \"%s\"", typeStr);
	return TCL_ERROR;
    }
    *typePtrPtr = matchPtr;

    return TCL_OK;
}

int TreeCtrl_RegisterElementType(Tcl_Interp *interp, TreeElementType *newTypePtr)
{
    ElementAssocData *assocData;
    TreeElementType *typeList;
    TreeElementType *prevPtr, *typePtr, *nextPtr;

    assocData = Tcl_GetAssocData(interp, "TreeCtrlElementTypes", NULL);
    typeList = assocData->typeList;

    for (typePtr = typeList, prevPtr = NULL;
	 typePtr != NULL;
	 prevPtr = typePtr, typePtr = nextPtr) {
	nextPtr = typePtr->next;
	/* Remove duplicate type */
	if (!strcmp(typePtr->name, newTypePtr->name)) {
	    if (prevPtr == NULL)
		typeList = typePtr->next;
	    else
		prevPtr->next = typePtr->next;
	    ckfree((char *) typePtr);
	}
    }
    typePtr = (TreeElementType *) ckalloc(sizeof(TreeElementType));
    memcpy(typePtr, newTypePtr, sizeof(TreeElementType));

    typePtr->next = typeList;
    typeList = typePtr;

    typePtr->optionTable = Tk_CreateOptionTable(interp,
	    newTypePtr->optionSpecs);

    assocData->typeList = typeList;

    return TCL_OK;
}

static TreeCtrlStubs stubs = {
#ifdef TREECTRL_DEBUG
    sizeof(TreeCtrl),
    sizeof(TreeCtrlStubs),
    sizeof(TreeElement),
    sizeof(TreeElementArgs),
#endif
    TreeCtrl_RegisterElementType,
    Tree_RedrawElement,
    Tree_ElementIterateBegin,
    Tree_ElementIterateNext,
    Tree_ElementIterateGet,
    Tree_ElementIterateChanged,
    PerStateInfo_Free,
    PerStateInfo_FromObj,
    PerStateInfo_ForState,
    PerStateInfo_ObjForState,
    PerStateInfo_Undefine,
    &pstBoolean,
    PerStateBoolean_ForState,
    PSTSave,
    PSTRestore,
    TreeStateFromObj,
    BooleanCO_Init,
    StringTableCO_Init,
    PerStateCO_Init
};

/*
 *----------------------------------------------------------------------
 *
 * TreeElement_InitWidget --
 *
 *	Element-related initialization when a treectrl is created.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
TreeElement_InitWidget(
    TreeCtrl *tree
    )
{
    /* These options are used by the text element. Originally these were
     * static globals but that isn't thread safe. */
    tree->formatFloatObj = Tcl_NewStringObj("%g", -1);
    tree->formatIntObj = Tcl_NewStringObj("%d", -1);
    tree->formatLongObj = Tcl_NewStringObj("%ld", -1);
    tree->formatStringObj = Tcl_NewStringObj("%s", -1);
    Tcl_IncrRefCount(tree->formatFloatObj);
    Tcl_IncrRefCount(tree->formatIntObj);
    Tcl_IncrRefCount(tree->formatLongObj);
    Tcl_IncrRefCount(tree->formatStringObj);

    tree->stringClockObj = Tcl_NewStringObj("clock", -1);
    tree->stringFormatObj = Tcl_NewStringObj("format", -1);
    tree->optionFormatObj = Tcl_NewStringObj("-format", -1);
    Tcl_IncrRefCount(tree->stringClockObj);
    Tcl_IncrRefCount(tree->stringFormatObj);
    Tcl_IncrRefCount(tree->optionFormatObj);

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * TreeElement_FreeWidget --
 *
 *	Free element-related resources for a deleted TreeCtrl.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void
TreeElement_FreeWidget(
    TreeCtrl *tree
    )
{
    Tcl_DecrRefCount(tree->formatFloatObj);
    Tcl_DecrRefCount(tree->formatIntObj);
    Tcl_DecrRefCount(tree->formatLongObj);
    Tcl_DecrRefCount(tree->formatStringObj);

    Tcl_DecrRefCount(tree->stringClockObj);
    Tcl_DecrRefCount(tree->stringFormatObj);
    Tcl_DecrRefCount(tree->optionFormatObj);
}

static void FreeAssocData(ClientData clientData, Tcl_Interp *interp)
{
    ElementAssocData *assocData = clientData;
    TreeElementType *typeList = assocData->typeList;
    TreeElementType *next;

    while (typeList != NULL) {
	next = typeList->next;
	/* The ElementType.optionTables are freed when the interp is deleted */
	ckfree((char *) typeList);
	typeList = next;
    }
    ckfree((char *) assocData);
}

/*
 *----------------------------------------------------------------------
 *
 * TreeElement_InitInterp --
 *
 *	Element-related initialization when the package is loaded.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
TreeElement_InitInterp(
    Tcl_Interp *interp
    )
{
    ElementAssocData *assocData;

    /* FIXME: memory leak with dynamically-allocated ClientData. */

    /*
     * bitmap
     */
    PerStateCO_Init(treeElemTypeBitmap.optionSpecs, "-background",
	&pstColor, TreeStateFromObj);
    PerStateCO_Init(treeElemTypeBitmap.optionSpecs, "-bitmap",
	&pstBitmap, TreeStateFromObj);
#ifdef DEPRECATED
    PerStateCO_Init(treeElemTypeBitmap.optionSpecs, "-draw",
	&pstBoolean, TreeStateFromObj);
#endif
    PerStateCO_Init(treeElemTypeBitmap.optionSpecs, "-foreground",
	&pstColor, TreeStateFromObj);

    /*
     * border
     */
#ifdef DEPRECATED
    PerStateCO_Init(treeElemTypeBorder.optionSpecs, "-draw",
	&pstBoolean, TreeStateFromObj);
#endif
    PerStateCO_Init(treeElemTypeBorder.optionSpecs, "-background",
	&pstBorder, TreeStateFromObj);
    PerStateCO_Init(treeElemTypeBorder.optionSpecs, "-relief",
	&pstRelief, TreeStateFromObj);

    /*
     * header
     */
    StringTableCO_Init(treeElemTypeHeader.optionSpecs, "-arrow",
	headerArrowST);
    PerStateCO_Init(treeElemTypeHeader.optionSpecs, "-arrowbitmap",
	&pstBitmap, TreeStateFromObj);
    StringTableCO_Init(treeElemTypeHeader.optionSpecs, "-arrowgravity",
	headerArrowSideST);
    PerStateCO_Init(treeElemTypeHeader.optionSpecs, "-arrowimage",
	&pstImage, TreeStateFromObj);
    StringTableCO_Init(treeElemTypeHeader.optionSpecs, "-arrowside",
	headerArrowSideST);
    PerStateCO_Init(treeElemTypeHeader.optionSpecs, "-background",
	&pstBorder, TreeStateFromObj);
    StringTableCO_Init(treeElemTypeHeader.optionSpecs, "-state", headerStateST);

    /*
     * image
     */
#ifdef DEPRECATED
    DynamicCO_Init(treeElemTypeImage.optionSpecs, "-draw",
	1002, sizeof(PerStateInfo),
	offsetof(PerStateInfo, obj),
	0, PerStateCO_Alloc("-draw", &pstBoolean, TreeStateFromObj),
	(DynamicOptionInitProc *) NULL);
#endif
    PerStateCO_Init(treeElemTypeImage.optionSpecs, "-image",
	&pstImage, TreeStateFromObj);

    /* 2 options in the same structure. */
    DynamicCO_Init(treeElemTypeImage.optionSpecs, "-height",
	1001, sizeof(ElementImageSize),
	offsetof(ElementImageSize, heightObj),
	offsetof(ElementImageSize, height), &TreeCtrlCO_pixels,
	(DynamicOptionInitProc *) NULL);
    DynamicCO_Init(treeElemTypeImage.optionSpecs, "-width",
	1001, sizeof(ElementImageSize),
	offsetof(ElementImageSize, widthObj),
	offsetof(ElementImageSize, width), &TreeCtrlCO_pixels,
	(DynamicOptionInitProc *) NULL);

    DynamicCO_Init(treeElemTypeImage.optionSpecs, "-tiled",
	1003, sizeof(int),
	-1,
	0, &booleanCO,
	DynamicOptionInitBoolean);

    /*
     * rect
     */
#ifdef DEPRECATED
    PerStateCO_Init(treeElemTypeRect.optionSpecs, "-draw",
	&pstBoolean, TreeStateFromObj);
#endif
    PerStateCO_Init(treeElemTypeRect.optionSpecs, "-fill",
	&pstColor, TreeStateFromObj);
    PerStateCO_Init(treeElemTypeRect.optionSpecs, "-open",
	&pstFlags, TreeStateFromObj);
    PerStateCO_Init(treeElemTypeRect.optionSpecs, "-outline",
	&pstColor, TreeStateFromObj);

    /*
     * text
     */
    /* 3 options in the same structure. */
    DynamicCO_Init(treeElemTypeText.optionSpecs, "-data",
	DOID_TEXT_DATA, sizeof(ElementTextData),
	offsetof(ElementTextData, dataObj),
	-1, &TreeCtrlCO_string,
	ElementTextDataInit);
    DynamicCO_Init(treeElemTypeText.optionSpecs, "-datatype",
	DOID_TEXT_DATA, sizeof(ElementTextData),
	-1,
	offsetof(ElementTextData, dataType),
	StringTableCO_Alloc("-datatype", textDataTypeST),
	ElementTextDataInit);
    DynamicCO_Init(treeElemTypeText.optionSpecs, "-format",
	DOID_TEXT_DATA, sizeof(ElementTextData),
	offsetof(ElementTextData, formatObj),
	-1, &TreeCtrlCO_string,
	ElementTextDataInit);

    /* 5 options in the same structure. */
    DynamicCO_Init(treeElemTypeText.optionSpecs, "-justify",
	DOID_TEXT_LAYOUT, sizeof(ElementTextLayout),
	-1,
	offsetof(ElementTextLayout, justify),
	StringTableCO_Alloc("-justify", textJustifyST),
	ElementTextLayoutInit);
    DynamicCO_Init(treeElemTypeText.optionSpecs, "-lines",
	DOID_TEXT_LAYOUT, sizeof(ElementTextLayout),
	-1,
	offsetof(ElementTextLayout, lines),
	IntegerCO_Alloc("-lines",
	    0,		/* min */
	    0,		/* max (ignored) */
	    -1,		/* empty */
	    0x01),	/* flags: min */
	ElementTextLayoutInit);
    DynamicCO_Init(treeElemTypeText.optionSpecs, "-width",
	DOID_TEXT_LAYOUT, sizeof(ElementTextLayout),
	offsetof(ElementTextLayout, widthObj),
	offsetof(ElementTextLayout, width), &TreeCtrlCO_pixels,
	ElementTextLayoutInit);
    DynamicCO_Init(treeElemTypeText.optionSpecs, "-wrap",
	DOID_TEXT_LAYOUT, sizeof(ElementTextLayout),
	-1,
	offsetof(ElementTextLayout, wrap),
	StringTableCO_Alloc("-wrap", textWrapST),
	ElementTextLayoutInit);
    DynamicCO_Init(treeElemTypeText.optionSpecs, "-elidepos",
	DOID_TEXT_LAYOUT, sizeof(ElementTextLayout),
	-1,
	offsetof(ElementTextLayout, elidePos),
	StringTableCO_Alloc("-elidepos", textElideST),
	ElementTextLayoutInit);

#ifdef DEPRECATED
    DynamicCO_Init(treeElemTypeText.optionSpecs, "-draw",
	DOID_TEXT_DRAW, sizeof(PerStateInfo),
	offsetof(PerStateInfo, obj),
	0, PerStateCO_Alloc("-draw", &pstBoolean, TreeStateFromObj),
	(DynamicOptionInitProc *) NULL);
#endif
    DynamicCO_Init(treeElemTypeText.optionSpecs, "-fill",
	DOID_TEXT_FILL, sizeof(PerStateInfo),
	offsetof(PerStateInfo, obj),
	0, PerStateCO_Alloc("-fill", &pstColor, TreeStateFromObj),
	(DynamicOptionInitProc *) NULL);
    DynamicCO_Init(treeElemTypeText.optionSpecs, "-font",
	DOID_TEXT_FONT, sizeof(PerStateInfo),
	offsetof(PerStateInfo, obj),
	0, PerStateCO_Alloc("-font", &pstFont, TreeStateFromObj),
	(DynamicOptionInitProc *) NULL);
#ifdef TEXTVAR
    DynamicCO_Init(treeElemTypeText.optionSpecs, "-textvariable",
	DOID_TEXT_VAR, sizeof(ElementTextVar),
	offsetof(struct ElementTextVar, varNameObj),
	-1, &TreeCtrlCO_string,
	(DynamicOptionInitProc *) NULL);
#endif

#ifdef TEXT_STYLE
    DynamicCO_Init(treeElemTypeText.optionSpecs, "-underline",
	DOID_TEXT_STYLE, sizeof(ElementTextStyle),
	-1,
	offsetof(ElementTextStyle, underline),
	IntegerCO_Alloc("-underline",
	    0,		/* min (ignored) */
	    0,		/* max (ignored) */
	    TEXT_UNDERLINE_EMPTYVAL,	/* empty */
	    0x00),	/* flags */
	ElementTextStyleInit);
#endif

    /* 2 options in the same structure */
    DynamicCO_Init(treeElemTypeText.optionSpecs, "-lmargin1",
	DOID_TEXT_LAYOUT3, sizeof(ElementTextLayout3),
	offsetof(ElementTextLayout3, lMargin1Obj),
	offsetof(ElementTextLayout3, lMargin1), &TreeCtrlCO_pixels,
	(DynamicOptionInitProc *) NULL);
    DynamicCO_Init(treeElemTypeText.optionSpecs, "-lmargin2",
	DOID_TEXT_LAYOUT3, sizeof(ElementTextLayout3),
	offsetof(ElementTextLayout3, lMargin2Obj),
	offsetof(ElementTextLayout3, lMargin2), &TreeCtrlCO_pixels,
	(DynamicOptionInitProc *) NULL);

    /*
     * window
     */
#ifdef DEPRECATED
    PerStateCO_Init(treeElemTypeWindow.optionSpecs, "-draw",
	&pstBoolean, TreeStateFromObj);
#endif

    assocData = (ElementAssocData *) ckalloc(sizeof(ElementAssocData));
    assocData->typeList = NULL;
    Tcl_SetAssocData(interp, "TreeCtrlElementTypes", FreeAssocData, assocData);

    TreeCtrl_RegisterElementType(interp, &treeElemTypeBitmap);
    TreeCtrl_RegisterElementType(interp, &treeElemTypeBorder);
/*    TreeCtrl_RegisterElementType(interp, &treeElemTypeCheckButton);*/
    TreeCtrl_RegisterElementType(interp, &treeElemTypeHeader);
    TreeCtrl_RegisterElementType(interp, &treeElemTypeImage);
    TreeCtrl_RegisterElementType(interp, &treeElemTypeRect);
    TreeCtrl_RegisterElementType(interp, &treeElemTypeText);
    TreeCtrl_RegisterElementType(interp, &treeElemTypeWindow);

    Tcl_SetAssocData(interp, "TreeCtrlStubs", NULL, &stubs);

    return TCL_OK;
}
