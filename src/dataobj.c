#include "wrassp.h"
#include <math.h>               /* ceil, floor */
#include <dataobj.h>
#include <asspfio.h>
#include <asspmess.h>
#include <headers.h>            /* KDTAB */

#include <R_ext/PrtUtil.h>

/*
 * This was the original reading function that did not allow for
 * preselecting time. Should be save to remove now. 
 */
SEXP
getDObj(SEXP fname)
{
    SEXP            res;
    DOBJ           *data = NULL;
    long            numRecs;
    /*
     * read the data
     */
    data =
        asspFOpen(strdup(CHAR(STRING_ELT(fname, 0))), AFO_READ,
                  (DOBJ *) NULL);
    if (data == NULL)
        error(getAsspMsg(asspMsgNum));
    /*
     * error(CHAR(STRING_ELT(fname,0)));
     */
    allocDataBuf(data, data->numRecords);
    data->bufStartRec = data->startRecord;
    if ((numRecs = asspFFill(data)) < 0)
        error(getAsspMsg(asspMsgNum));
    asspFClose(data, AFC_KEEP);
    res = PROTECT(dobj2AsspDataObj(data));
    asspFClose(data, AFC_FREE);
    UNPROTECT(1);
    return res;
}


/*
 * This function loads a DOBJ from a file and return its contents as a
 * SEXP. Arguments include the name of the input file, start and end point 
 * for reading and whether these points are sample values (and not times) 
 */
SEXP
getDObj2(SEXP args)
{

    SEXP            el,
                    ans;
    DOBJ           *data = NULL;
    long            numRecs;
    int             i;
    char           *fName = NULL;
    const char     *name;
    double          begin = 0,
        end = 0;
    int             isSample = 0;

    /*
     * parse args
     */
    args = CDR(args);           /* skip name of function */
    el = CAR(args);
    fName = strdup(CHAR(STRING_ELT(el, 0)));

    args = CDR(args);
    for (i = 0; args != R_NilValue; i++, args = CDR(args)) {
        name = isNull(TAG(args)) ? "" : CHAR(PRINTNAME(TAG(args)));
        el = CAR(args);
        if (strcmp(name, "begin") == 0) {
            begin = REAL(el)[0];
            if (begin < 0)
                begin = 0;
        } else if (strcmp(name, "end") == 0) {
            end = REAL(el)[0];
            if (end < 0)
                end = 0;
        } else if (strcmp(name, "samples") == 0) {
            isSample = INTEGER(el)[0];
        } else {
            error("Bad option '%s'.", name);
        }
    }

    if (end < begin && end > 0)
        error("End before begin. That's not clever, dude!");

    /*
     * open the file
     */
    data = asspFOpen(fName, AFO_READ, (DOBJ *) NULL);
    if (data == NULL)
        error("%s (%s)", getAsspMsg(asspMsgNum), fName);
    /*
     * figure out timing
     */
    if (isSample) {
        if (end == 0)
            end = data->startRecord + data->numRecords - 1;
    } else {
        begin = ceil(begin * data->dataRate) + data->startRecord;
        if (end == 0)
            end = data->startRecord + data->numRecords - 1;
        else
            end = floor(end * data->dataRate) + data->startRecord;
    }
    if (end > (data->startRecord + data->numRecords))
        end = data->startRecord + data->numRecords - 1;
    if (begin > (data->startRecord + data->numRecords)) {
        asspFClose(data, AFC_FREE);
        error("Begin after end of data. That's not clever, dude!");
    }

    numRecs = (long) (end - begin) + 1;
    /*
     * read the data
     */
    allocDataBuf(data, numRecs);
    data->bufStartRec = (long) begin;
    if ((numRecs = asspFFill(data)) < 0) {
        asspFClose(data, AFC_FREE);
        error(getAsspMsg(asspMsgNum));
    }
    asspFClose(data, AFC_KEEP);
    ans = PROTECT(dobj2AsspDataObj(data));
    asspFClose(data, AFC_FREE);
    UNPROTECT(1);
    return ans;
}

/*
 * Originally, we retained the DOBJ and stored a pointer to it in the
 * SEXP. For that reason, garbage collection was an issue and this
 * function was used to clean up the data object when the SEXP was
 * deleted. No longer needed, should be save to remove. 
 */
static void
DObjFinalizer(SEXP dPtr)
{
    DOBJ           *data = R_ExternalPtrAddr(dPtr);
    asspFClose(data, AFC_FREE);
    R_ClearExternalPtr(dPtr);   /* not really needed */
}

/*
 * This function turns a DOBJ and places the contents in a SEXP of class
 * AsspDataObject. (Hopefully) all information is retained in order to
 * safely rewrite without data loss. 
 */
SEXP
dobj2AsspDataObj(DOBJ * data)
{
    SEXP            ans,        /* dPtr, */
                    class,
                    rate,
                    tracks,
                    startTime,
                    origRate,
                    filePath,
                    startRec,
                    endRec,
                    trackFormats,
                    finfo,
                    genericVars;
    DDESC          *desc = NULL;
    TSSFF_Generic  *genVar = NULL;
    int             i,
                    n;

    /*
     * count tracks
     */
    for (n = 0, desc = &(data->ddl); desc != NULL; desc = desc->next) {
        n++;
        /*
         * Rprintf("Cur n=%d\n", n);
         */
    }

    /*
     * create result, a list with a matrix for each track
     */
    PROTECT(ans = allocVector(VECSXP, n));
    /*
     * create list of tracks and formats
     */
    PROTECT(tracks = allocVector(STRSXP, n));
    PROTECT(trackFormats = allocVector(STRSXP, n));
    for (i = 0, desc = &(data->ddl); desc != NULL; desc = desc->next, i++) {
        SET_STRING_ELT(tracks, i, mkChar(desc->ident));
        SET_STRING_ELT(trackFormats, i,
                       mkChar(asspDF2ssffString(desc->format)));
        /*
         * fill tracks with data
         */
        /*
         * Rprintf ("Loading track %s.\n", desc->ident);
         */
        SET_VECTOR_ELT(ans, i, getDObjTrackData(data, desc));
    }
    /*
     * set the names
     */
    setAttrib(ans, R_NamesSymbol, tracks);
    setAttrib(ans, install("trackFormats"), trackFormats);

    /*
     * PROTECT (dPtr = R_MakeExternalPtr (data, install ("DOBJ"), 
     * install ("something"))); 
     * R_RegisterCFinalizerEx (dPtr, DObjFinalizer, TRUE); 
     * setAttrib (ans, install ("data pointer"), dPtr); 
     */

    PROTECT(rate = allocVector(REALSXP, 1));
    REAL(rate)[0] = data->dataRate;
    setAttrib(ans, install("sampleRate"), rate);
    if (data->filePath == NULL || strlen(data->filePath) == 0){
        // Rprintf("at non caps protect call\n");
        PROTECT(filePath = R_NilValue);
    } else {
        PROTECT(filePath = allocVector(STRSXP, 1));
        SET_STRING_ELT(filePath, 0, mkCharCE(data->filePath, CE_UTF8));
    }
    setAttrib(ans, install("filePath"), filePath);
    PROTECT(origRate = allocVector(REALSXP, 1));
    if (data->fileFormat == FF_SSFF) {
        REAL(origRate)[0] = data->sampFreq;
    } else {
        REAL(origRate)[0] = 0;
    }
    setAttrib(ans, install("origFreq"), origRate);
    PROTECT(startTime = allocVector(REALSXP, 1));
    /*
     * REAL (startTime)[0] = data->Start_Time + 
     * (data->bufStartRec / data->dataRate); 
     */
    REAL(startTime)[0] = data->Start_Time;
    setAttrib(ans, install("startTime"), startTime);

    PROTECT(startRec = allocVector(INTSXP, 1));
    INTEGER(startRec)[0] = (int) (data->bufStartRec + 1);
    setAttrib(ans, install("startRecord"), startRec);
    PROTECT(endRec = allocVector(INTSXP, 1));
    INTEGER(endRec)[0] = (int) (data->bufStartRec + data->bufNumRecs);
    setAttrib(ans, install("endRecord"), endRec);

    PROTECT(class = allocVector(STRSXP, 1));
    SET_STRING_ELT(class, 0, mkChar(WRASSP_CLASS));
    classgets(ans, class);

    PROTECT(finfo = allocVector(INTSXP, 2));
    INTEGER(finfo)[0] = (int) data->fileFormat;
    INTEGER(finfo)[1] = (int) data->fileData;
    setAttrib(ans, install("fileInfo"), finfo);


    PROTECT(genericVars = getGenericVars(data));
    setAttrib(ans, install("genericVars"), genericVars);

    UNPROTECT(12);
    return ans;

}

/*
 * This function parses generic variables from a DOBJ (SFFF only) and
 * returns them in a useful format for R 
 */
SEXP
getGenericVars(DOBJ * dop)
{
    SEXP            ans,
                    var,
                    names,
                    varNames,
                    value;
    TSSFF_Generic  *genVar;
    SSFFST         *ssff_types;
    int             i;
    PROTECT(names = allocVector(STRSXP, 2));
    SET_STRING_ELT(names, 1, mkChar("Type"));
    SET_STRING_ELT(names, 0, mkChar("Value"));
    /*
     * count generic variables 
     */
    for (i = 0, genVar = &(dop->meta); genVar != NULL;
         genVar = genVar->next, i++) {
    }
    if (i == 0) {
        UNPROTECT(1);
        return (R_NilValue);
    }

    PROTECT(ans = allocVector(VECSXP, i));
    PROTECT(varNames = allocVector(STRSXP, i));
    for (i = 0, genVar = &(dop->meta); genVar != NULL;
         i++, genVar = genVar->next) {
        if (genVar->ident == NULL) {
            UNPROTECT(3);
            return (R_NilValue);
        }
        PROTECT(var = allocVector(VECSXP, 2));
        for (ssff_types = SSFF_TYPES; ssff_types->type != SSFF_UNDEF;
             ssff_types++) {
            if (ssff_types->type == genVar->type)
                break;
        }
        if (ssff_types->type == SSFF_UNDEF)
            error("Invalid type for SSFF generic variable.");
        PROTECT(value = allocVector(STRSXP, 1));
        SET_STRING_ELT(value, 0, mkChar(ssff_types->ident));
        SET_VECTOR_ELT(var, 1, value);
        switch (genVar->type) {
        case SSFF_CHAR:
        case SSFF_BYTE:
            PROTECT(value = allocVector(STRSXP, 1));
            SET_STRING_ELT(value, 0, mkChar(genVar->data));
            SET_VECTOR_ELT(var, 0, value);
            UNPROTECT(1);
            break;
        case SSFF_SHORT:
        case SSFF_LONG:
        case SSFF_FLOAT:
        case SSFF_DOUBLE:
            PROTECT(value = allocVector(REALSXP, 1));
            REAL(value)[0] = strtod(genVar->data, NULL);
            SET_VECTOR_ELT(var, 0, value);
            UNPROTECT(1);
        case SSFF_UNDEF:
            break;
        }
        setAttrib(var, R_NamesSymbol, names);
        SET_VECTOR_ELT(ans, i, var);
        SET_STRING_ELT(varNames, i, mkChar(genVar->ident));
        UNPROTECT(2);
    }
    setAttrib(ans, R_NamesSymbol, varNames);
    UNPROTECT(3);
    return (ans);
}

/*
 * This function generates a string vector of track names from the data
 * descriptors in a DOBJ and returns it. 
 */
SEXP
getDObjTracks(SEXP dobj)
{
    SEXP            ans,
                    ptr;
    ptr = getAttrib(dobj, install("data pointer"));
    DOBJ           *data = R_ExternalPtrAddr(ptr);
    DDESC          *desc;
    int             i = 0,
        n = 0;
    /*
     * count tracks
     */
    for (desc = &(data->ddl); desc != NULL; desc = desc->next) {
        n++;
    }
    /*
     * Rprintf("Number of descs = %i.", n);
     */
    /*
     * create result
     */
    PROTECT(ans = allocVector(STRSXP, n));
    for (desc = &(data->ddl); desc != NULL; desc = desc->next) {
        SET_STRING_ELT(ans, i, mkChar(desc->ident));
        i++;
    }
    /*
     * for (; i<n; i++) 
     */
    /*
     * SET_STRING_ELT(ans, i, mkChar("")); 
     */
    UNPROTECT(1);
    return (ans);
}

/*
 * This function extracts data corresponding to one data descriptor in a
 * DOBJ and returns it as an R Matrix 
 */
SEXP
getDObjTrackData(DOBJ * data, DDESC * desc)
{
    SEXP            ans;
    void           *tempBuffer,
                   *bufPtr;
    int             i,
                    m,
                    n;
    tempBuffer = malloc((size_t) data->recordSize);
    /*
     * various pointers for variuos data sizes
     */
    uint8_t        *u8Ptr;
    int8_t         *i8Ptr;
    uint16_t       *u16Ptr;
    int16_t        *i16Ptr;
    uint32_t       *u32Ptr;
    int32_t        *i32Ptr;
    float          *f32Ptr;
    double         *f64Ptr;

    double         *Rans;
    int            *Ians;
    uint8_t        *bPtr;
    bPtr = (uint8_t *) tempBuffer;
    i = 0;                      /* initial index in buffer */

    switch (desc->format) {
    case DF_UINT8:
    case DF_INT8:
    case DF_UINT16:
    case DF_INT16:
    case DF_UINT32:
    case DF_INT32:
        {
            PROTECT(ans =
                    allocMatrix(INTSXP, data->bufNumRecs,
                                desc->numFields));
            Ians = INTEGER(ans);
        }
        break;
    case DF_REAL32:
    case DF_REAL64:
        {
            PROTECT(ans =
                    allocMatrix(REALSXP, data->bufNumRecs,
                                desc->numFields));
            Rans = REAL(ans);
        }
        break;
    default:
        {
            error("Unsupported data format.");
            free(tempBuffer);
        }
        break;
    }

    for (m = 0; m < data->bufNumRecs; m++) {
        bufPtr = (void *)((char *)data->dataBuffer + m * data->recordSize);
        memcpy(tempBuffer, bufPtr, (size_t) data->recordSize);
        switch (desc->format) {
        case DF_UINT8:
            {
                u8Ptr = &bPtr[desc->offset];
                for (n = 0; n < desc->numFields; n++) {
                    Ians[m + n * data->bufNumRecs] =
                        (unsigned int) u8Ptr[n];
                }
            }
            break;
        case DF_INT8:
            {
                i8Ptr = (int8_t *) & bPtr[desc->offset];
                for (n = 0; n < desc->numFields; n++) {
                    Ians[m + n * data->bufNumRecs] = (int) u8Ptr[n];
                }
            }
            break;
        case DF_UINT16:
            {
                u16Ptr = (uint16_t *) & bPtr[desc->offset];
                for (n = 0; n < desc->numFields; n++) {
                    Ians[m + n * data->bufNumRecs] =
                        (unsigned int) u16Ptr[n];
                }
            }
            break;
        case DF_INT16:
            {
                i16Ptr = (int16_t *) & bPtr[desc->offset];
                for (n = 0; n < desc->numFields; n++) {
                    Ians[m + n * data->bufNumRecs] = (int) i16Ptr[n];
                }
            }
            break;
        case DF_UINT32:
            {
                u32Ptr = (uint32_t *) & bPtr[desc->offset];
                for (n = 0; n < desc->numFields; n++) {
                    Ians[m + n * data->bufNumRecs] =
                        (unsigned long) u32Ptr[n];
                }
            }
            break;
        case DF_INT32:
            {
                i32Ptr = (int32_t *) & bPtr[desc->offset];
                for (n = 0; n < desc->numFields; n++) {
                    Ians[m + n * data->bufNumRecs] = (long) i32Ptr[n];
                }
            }
            break;
        case DF_REAL32:
            {
                f32Ptr = (float *) &bPtr[desc->offset];
                for (n = 0; n < desc->numFields; n++) {
                    Rans[m + n * data->bufNumRecs] = (double) f32Ptr[n];
                }
            }
            break;
        case DF_REAL64:
            {
                f64Ptr = (double *) &bPtr[desc->offset];
                for (n = 0; n < desc->numFields; n++) {
                    Rans[m + n * data->bufNumRecs] = (double) f64Ptr[n];
                }
            }
            break;
        default:
            error
                ("Hi, I just landed in the default of a switch in dataobj.c."
                 "I am sorry, I should not be here and I don't know what to do.");
            break;
        }
    }
    free(tempBuffer);
    UNPROTECT(1);
    return (ans);
}

/*
 * switch trough assp data formats and return corresponding string 
 */
char           *
asspDF2ssffString(int df)
{
    switch ((dform_e) df) {
    case DF_BIT:
        return "BIT";
        break;
    case DF_STR:
        return "STR";
        break;
    case DF_CHAR:
        return "CHAR";
        break;
    case DF_UINT8:
        return "UINT8";
        break;
    case DF_INT8:
        return "INT8";
        break;
    case DF_UINT16:
        return "UINT16";
        break;
    case DF_INT16:
        return "INT16";
        break;
    case DF_UINT24:
        return "UINT24";
        break;
    case DF_INT24:
        return "INT24";
        break;
    case DF_UINT32:
        return "UINT32";
        break;
    case DF_INT32:
        return "INT32";
        break;
    case DF_UINT64:
        return "UINT64";
        break;
    case DF_INT64:
        return "INT64";
        break;
    case DF_REAL32:
        return "REAL32";
        break;
    case DF_REAL64:
        return "REAL64";
        break;
    default:
        return NULL;
    }
}

/*
 * This function is the inverse of dobj2AsspDataObj. It takes a SEXP of
 * class AsspDataObj and turns it into a DOBJ 
 */
DOBJ           *
sexp2dobj(SEXP rdobj)
{
    DOBJ           *dop = NULL;
    DDESC          *desc = NULL;
    int             FIRST = 1,
        i = 0,
        myBool = 0;
    size_t          numFields = -1;
    SEXP            attr,
                    tracks,
                    formats,
                    track,
                    var,
                    varNames;
    SSFFST         *ssff_types;
    TSSFF_Generic  *genVar;
    KDTAB          *entry;
    char           *format;

    /*
     * check for right class
     */
    attr = getAttrib(rdobj, R_ClassSymbol);
    for (i = 0; i < LENGTH(attr); i++) {
        if (strcmp(CHAR(STRING_ELT(attr, i)), WRASSP_CLASS) == 0) {
            myBool = 1;
            break;
        }
    }
    if (!myBool) {              /* classname does not match */
        error("Argument must be of class %s", WRASSP_CLASS);
    }
    /*
     * create DObj
     */
    dop = allocDObj();
    desc = &(dop->ddl);
    if (dop == NULL) {
        error(getAsspMsg(asspMsgNum));
    }
    /*
     * assign attributes
     */
    attr = getAttrib(rdobj, install("sampleRate"));
    if (isNull(attr)) {
        freeDObj(dop);
        error("Invalid argument: no 'sampleRate' attribute.");
    }
    dop->dataRate = REAL(attr)[0];

    attr = getAttrib(rdobj, install("origFreq"));
    if (!isNull(attr))
        dop->sampFreq = REAL(attr)[0];

    attr = getAttrib(rdobj, install("startTime"));
    if (!isNull(attr))
        dop->Start_Time = REAL(attr)[0];

    attr = getAttrib(rdobj, install("startRecord"));
    if (!isNull(attr))
        dop->startRecord = INTEGER(attr)[0];

    attr = getAttrib(rdobj, install("fileInfo"));
    if (LENGTH(attr) != 2) {
        dop->fileFormat = FF_SSFF;
        dop->fileData = FDF_BIN;
        warning("Incomplete 'fileInfo' attribute. Writing to binary"
                "SSFF format (dafault).");
    }
    dop->fileFormat = (fform_e) INTEGER(attr)[0];
    dop->fileData = (fdata_e) INTEGER(attr)[1];

    /*
     * Install generic variables
     */
    attr = getAttrib(rdobj, install("genericVars"));
    tracks = getAttrib(attr, R_NamesSymbol);
    if (!isNull(attr)) {
        for (i = 0; i < LENGTH(attr); i++) {
            var = VECTOR_ELT(attr, i);
            /*
             * determine ssff type
             */
            for (ssff_types = SSFF_TYPES; ssff_types->type != SSFF_UNDEF;
                 ssff_types++) {
                format = strdup(CHAR(STRING_ELT(VECTOR_ELT(var, 1), 0)));
                if (strncmp
                    (format, ssff_types->ident,
                     strlen(ssff_types->ident)) == 0)
                    break;
            }
            if (ssff_types->type == SSFF_UNDEF)
                error("Invalid type for SSFF generic variable.");
            if (FIRST) {
                genVar = &(dop->meta);
            } else {
                genVar = addTSSFF_Generic(dop);
                if (genVar == NULL)
                    error("Unable to add Generic Variable (%s).",
                          getAsspMsg(asspMsgNum));
            }
            FIRST = 0;

            genVar->type = ssff_types->type;
            genVar->ident = strdup(CHAR(STRING_ELT(tracks, i)));
            switch (genVar->type) {
            case SSFF_CHAR:
            case SSFF_BYTE:
                genVar->data =
                    strdup(CHAR(STRING_ELT(VECTOR_ELT(var, 0), 0)));
                break;
            case SSFF_SHORT:
            case SSFF_LONG:
            case SSFF_FLOAT:
            case SSFF_DOUBLE:
                sprintf(format, "%f", REAL(VECTOR_ELT(var, 0))[0]);
                genVar->data = strdup(format);
                break;
            case SSFF_UNDEF:
                break;
            }
            free((void *) format);
        }
    }
    /*
     * prepare ddescs
     * check tracks and formats and are there enough data parts?
     */
    FIRST = 1;
    if (isNull(tracks = getAttrib(rdobj, R_NamesSymbol)) ||
        LENGTH(tracks) == 0 || TYPEOF(tracks) != STRSXP) {
        freeDObj(dop);
        error("There are no data tracks!");
    }

    if (isNull(formats = getAttrib(rdobj, install("trackFormats"))) ||
        TYPEOF(tracks) != STRSXP) {
        freeDObj(dop);
        error("There are no track format specifiers!");
    }
    if (LENGTH(tracks) > LENGTH(formats)) {
        freeDObj(dop);
        error("Not enough format specifiers for the data tracks.");
    }

    for (i = 0; i < LENGTH(tracks); i++) {
        /*
         * get dimensions
         */
        track = VECTOR_ELT(rdobj, i);
        attr = getAttrib(track, R_DimSymbol);
        /*
         * if there is more than one track, add descriptor
         */
        if (FIRST) {
            dop->numRecords = INTEGER(attr)[0];
            FIRST = 0;
        } else {
            desc = addDDesc(dop);
            if (desc == NULL) {
                freeDObj(dop);
                error(getAsspMsg(asspMsgNum));
            }
            if (dop->numRecords != INTEGER(attr)[0]) {
                freeDObj(dop);
                error("Dimensions of tracks do not match."
                      "(%d rows in first track, but %d rows in track %d).",
                      dop->numRecords, INTEGER(attr)[0], i);
            }
        }
        desc->ident = strdup(CHAR(STRING_ELT(tracks, i)));
        format = strdup(CHAR(STRING_ELT(formats, i)));
        entry = keyword2entry(desc->ident, KDT_SSFF);   /* search SSFF
                                                         * info */
        if (entry != NULL) {
            desc->type = entry->dataType;
            if (entry->factor != NULL)
                strcpy(desc->factor, entry->factor);
            if (entry->unit != NULL)
                strcpy(desc->unit, entry->unit);
        }
        if (strcmp(format, "BIT") == 0) {
            desc->format = DF_BIT;
            desc->numBits = 1;
        } else if (strcmp(format, "STR") == 0) {
            desc->format = DF_STR;
            desc->numBits = 1;
        } else if (strcmp(format, "CHAR") == 0) {
            desc->format = DF_CHAR;
            desc->numBits = 8;
        } else if (strcmp(format, "UINT8") == 0) {
            desc->format = DF_UINT8;
            desc->numBits = 8;
        } else if (strcmp(format, "INT8") == 0) {
            desc->format = DF_INT8;
            desc->numBits = 8;
        } else if (strcmp(format, "UINT16") == 0) {
            desc->format = DF_UINT16;
            desc->numBits = 16;
        } else if (strcmp(format, "INT16") == 0) {
            desc->format = DF_INT16;
            desc->numBits = 16;
        } else if (strcmp(format, "UINT32") == 0) {
            desc->format = DF_UINT32;
            desc->numBits = 32;
        } else if (strcmp(format, "INT32") == 0) {
            desc->format = DF_INT32;
            desc->numBits = 32;
        } else if (strcmp(format, "UINT64") == 0) {
            desc->format = DF_UINT64;
            desc->numBits = 64;
        } else if (strcmp(format, "INT64") == 0) {
            desc->format = DF_INT64;
            desc->numBits = 64;
        } else if (strcmp(format, "REAL32") == 0) {
            desc->format = DF_REAL32;
            desc->numBits = 32;
        } else if (strcmp(format, "REAL64") == 0) {
            desc->format = DF_REAL64;
            desc->numBits = 64;
        } else {
            freeDObj(dop);
            error("Cannot handle data format %s.", format);
        }

        desc->coding = DC_LIN;
        desc->numFields = (size_t) INTEGER(attr)[1];
        free((void *) format);
    }
    setRecordSize(dop);
    dop->frameDur = -1;
    checkRates(dop);
    allocDataBuf(dop, dop->numRecords);
    if (dop->dataBuffer == NULL) {
        freeDObj(dop);
        error(getAsspMsg(asspMsgNum));
    }

    for (i = 0, desc = &(dop->ddl); i < LENGTH(tracks);
         i++, desc = desc->next) {
        track = VECTOR_ELT(rdobj, i);
        if (!addTrackData(dop, desc, track)) {
            freeDObj(dop);
            error("Adding Trackdata did not work...");
        }
    }
    dop->bufNumRecs = dop->numRecords;
    return dop;
}


/*
 * This function takes a SEXP of class AsspDataFormat, turns it into a
 * DOBJ and writes it to file. The DOBJ is deleted after wards. 
 */
SEXP writeDObj_(SEXP data, SEXP fname)
{
    DOBJ           *dop = sexp2dobj(data);
    dop = asspFOpen(strdup(CHAR(STRING_ELT(fname, 0))), AFO_WRITE, dop);
    if (dop == NULL) {
        freeDObj(dop);
        error(getAsspMsg(asspMsgNum));
    }
    asspFWrite(dop->dataBuffer, dop->bufNumRecs, dop);
    asspFClose(dop, AFC_FREE);
    return R_NilValue;
}



/*
 * this function takes trackdata in the form of an R Matrix (rdobj) and
 * adds its contents to a DOBJ in correspondence with a given data
 * descriptor. 
 */
int
addTrackData(DOBJ * dop, DDESC * ddl, SEXP rdobj)
{
    void           *bufPtr;
    int             i,
                    m,
                    n,
                    unp = 0;
    /*
     * various pointers for variuos data sizes
     */
    uint8_t        *u8Ptr;
    int8_t         *i8Ptr;
    uint16_t       *u16Ptr;
    int16_t        *i16Ptr;
    uint32_t       *u32Ptr;
    int32_t        *i32Ptr;
    float          *f32Ptr;
    double         *f64Ptr;

    SEXP            numMat;
    double         *numPtr;
    uint8_t        *bPtr;

    if (isReal(rdobj))
        numMat = rdobj;
    else if (isInteger(rdobj)) {
        PROTECT(numMat = coerceVector(rdobj, REALSXP));
        unp++;
    } else
        error("Bad data type, must be INTEGER or REAL.");
    numPtr = REAL(numMat);

    i = 0;                      /* initial index in buffer */

    for (m = 0; m < dop->numRecords; m++) {
        bufPtr = (void *)((char *)dop->dataBuffer + m * dop->recordSize);
        bPtr = (uint8_t *) bufPtr;
        switch (ddl->format) {
        case DF_UINT8:
            {
                u8Ptr = &bPtr[ddl->offset];
                for (n = 0; n < ddl->numFields; n++) {
                    u8Ptr[n] = (uint8_t) numPtr[m + n * dop->numRecords];
                }
            }
            break;
        case DF_INT8:
            {
                i8Ptr = (int8_t *) & bPtr[ddl->offset];
                for (n = 0; n < ddl->numFields; n++) {
                    u8Ptr[n] = (int8_t) numPtr[m + n * dop->numRecords];
                }
            }
            break;
        case DF_UINT16:
            {
                u16Ptr = (uint16_t *) & bPtr[ddl->offset];
                for (n = 0; n < ddl->numFields; n++) {
                    u16Ptr[n] = (uint16_t) numPtr[m + n * dop->numRecords];
                }
            }
            break;
        case DF_INT16:
            {
                i16Ptr = (int16_t *) & bPtr[ddl->offset];
                for (n = 0; n < ddl->numFields; n++) {
                    i16Ptr[n] = (int16_t) numPtr[m + n * dop->numRecords];
                }
            }
            break;
        case DF_UINT32:
            {
                u32Ptr = (uint32_t *) & bPtr[ddl->offset];
                for (n = 0; n < ddl->numFields; n++) {
                    u32Ptr[n] = (uint32_t) numPtr[m + n * dop->numRecords];
                }
            }
            break;
        case DF_INT32:
            {
                i32Ptr = (int32_t *) & bPtr[ddl->offset];
                for (n = 0; n < ddl->numFields; n++) {
                    i32Ptr[n] = (int32_t) numPtr[m + n * dop->numRecords];
                }
            }
            break;
        case DF_REAL32:
            {
                f32Ptr = (float *) &bPtr[ddl->offset];
                for (n = 0; n < ddl->numFields; n++) {
                    f32Ptr[n] = (float) numPtr[m + n * dop->numRecords];
                }
            }
            break;
        case DF_REAL64:
            {
                f64Ptr = (double *) &bPtr[ddl->offset];
                for (n = 0; n < ddl->numFields; n++) {
                    f64Ptr[n] = (double) numPtr[m + n * dop->numRecords];
                }
            }
            break;
        default:
            error
                ("Hi, I just landed in the default of a switch in dataobj.c."
                 "I am sorry, I should not be here and I don't know what to do.");
            break;
        }
    }

    UNPROTECT(unp);
    return 1;
}
