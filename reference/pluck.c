/* pluck.c - elementary waveguide simulation of plucked strings - JOS 6/6/92 */

/* Note: The word "inline" below can be deleted if your compiler 
   does not support it.  It is a nice GNU feature not in the ANSII C spec. */

#import <libc.h>

#define SRATE 44100
#define DOUBLE_TO_SHORT(x) ((int)((x)*32768.0))

typedef struct _DelayLine {
    short *data;
    int length;
    short *pointer;
    short *end;
} DelayLine;

static DelayLine *initDelayLine(int len) {
    DelayLine *dl = (DelayLine *)calloc(len, sizeof(DelayLine));
    dl->length = len;
    if (len > 0)
    	dl->data = (short *)calloc(len, len * sizeof(short));
    else
    	dl->data = 0;
    dl->pointer = dl->data;
    dl->end = dl->data + len - 1;
    return dl;
}

static void freeDelayLine(DelayLine *dl) {
    if (dl && dl->data)
		free(dl->data);
    dl->data = 0;
    free(dl);
}

inline static void setDelayLine(DelayLine *dl, double *values, double scale) {
    int i;
    for (i=0; i<dl->length; i++)
    	dl->data[i] = DOUBLE_TO_SHORT(scale * values[i]);
}

/* lg_dl_update(dl, insamp);
 * Places "nut-reflected" sample from upper delay-line into
 * current lower delay-line pointer location (which represents
 * x = 0 position).  The pointer is then incremented (i.e. the
 * wave travels one sample to the left), turning the previous
 * position into an "effective" x = L position for the next
 * iteration.
 */
static inline void lg_dl_update(DelayLine *dl, short insamp) {
    register short *ptr = dl->pointer;
    *ptr = insamp;
	ptr++;
    if (ptr > dl->end)
    	ptr = dl->data;
    dl->pointer = ptr;
}

/* rg_dl_update(dl, insamp);
 * Decrements current upper delay-line pointer position (i.e.
 * the wave travels one sample to the right), moving it to the
 * "effective" x = 0 position for the next iteration.  The
 * "bridge-reflected" sample from lower delay-line is then placed
 * into this position.
 */
static inline void rg_dl_update(DelayLine *dl, short insamp) {
    register short *ptr = dl->pointer;    
	ptr--;
    if (ptr < dl->data)
    	ptr = dl->end;
	*ptr = insamp;
    dl->pointer = ptr;
}

/* dl_access(dl, position);
 * Returns sample "position" samples into delay-line's past.
 * Position "0" points to the most recently inserted sample.
 */
static inline short dl_access(DelayLine *dl, int position) {
    short *outloc = dl->pointer + position;
    while (outloc < dl->data)
    	outloc += dl->length;
    while (outloc > dl->end)
    	outloc -= dl->length;
    return *outloc;
}

/*
 *  Right-going delay line:
 *  -->---->---->--- 
 *  x=0
 *  (pointer)
 *  Left-going delay line:
 *  --<----<----<--- 
 *  x=0
 *  (pointer)
 */

/* rg_dl_access(dl, position);
 * Returns spatial sample at location "position", where position zero
 * is equal to the current upper delay-line pointer position (x = 0).
 * In a right-going delay-line, position increases to the right, and
 * delay increases to the right => left = past and right = future.
 */
static inline short rg_dl_access(DelayLine *dl, int position) {
    return dl_access(dl, position);
}

/* lg_dl_access(dl, position);
 * Returns spatial sample at location "position", where position zero
 * is equal to the current lower delay-line pointer position (x = 0).
 * In a left-going delay-line, position increases to the right, and
 * delay DEcreases to the right => left = future and right = past.
 */
static inline short lg_dl_access(DelayLine *dl, int position) {
    return dl_access(dl, position);
}

static DelayLine *upper_rail,*lower_rail;


static inline int initString(double amplitude, double pitch,
                             double pick, double pickup) {
    int i, rail_length = SRATE/pitch/2 + 1;
	/* 
	 * Round pick position to nearest spatial sample.
	 * A pick position at x = 0 is not allowed. 
	 */
    int pickSample = MAX(rail_length * pick, 1); 
	double upslope = amplitude/pickSample;
    double downslope = amplitude/(rail_length - pickSample - 1);
    double initial_shape[rail_length];

    upper_rail = initDelayLine(rail_length);
    lower_rail = initDelayLine(rail_length);

#ifdef DEBUG
    initial_shape[pickSample] = 1;
#else
    for (i = 0; i < pickSample; i++)
    	initial_shape[i] = upslope * i;
    for (i = pickSample; i < rail_length; i++)
    	initial_shape[i] = downslope * (rail_length - 1 - i);
#endif

    /*
     * Initial conditions for the ideal plucked string.
     * "Past history" is measured backward from the end of the array.
     */
    setDelayLine(lower_rail, initial_shape, 0.5);
    setDelayLine(upper_rail, initial_shape, 0.5);

    return pickup * rail_length;
}

static inline void freeString(void) {
    freeDelayLine(upper_rail);
    freeDelayLine(lower_rail);
}

static inline short bridgeReflection(int insamp) {
    static short state = 0; /* filter memory */
    /* Implement a one-pole lowpass with feedback coefficient = 0.5 */
    /* outsamp = 0.5 * outsamp + 0.5 * insamp */
    short outsamp = (state >> 1) + (insamp >> 1);
    state = outsamp;
    return outsamp;
}

static inline short nextStringSample(int pickup_loc) {
    short yp0,ym0,ypM,ymM;
    short outsamp, outsamp1;

    /* Output at pickup location */
    outsamp  = rg_dl_access(upper_rail, pickup_loc);
    outsamp1 = lg_dl_access(lower_rail, pickup_loc);
	outsamp += outsamp1;

    ym0 = lg_dl_access(lower_rail, 1);     /* Sample traveling into "bridge" */
    ypM = rg_dl_access(upper_rail, upper_rail->length - 2); /* Sample to "nut" */

    ymM = -ypM;                    /* Inverting reflection at rigid nut */
    yp0 = -bridgeReflection(ym0);  /* Reflection at yielding bridge */

    /* String state update */
    rg_dl_update(upper_rail, yp0); /* Decrement pointer and then update */
    lg_dl_update(lower_rail, ymM); /* Update and then increment pointer */

    return outsamp;
}

/* Utility for writing a mono sound to a sound file on a NeXT machine */
#import <sound/sound.h>
static int writeSound(char *name, short *soundData, int sampleCount) {
    int i, err;
    short *data;
    SNDSoundStruct *sound;
    SNDAlloc(&sound, sampleCount * sizeof(short), SND_FORMAT_LINEAR_16,
             SRATE,1,4);
    data = (short *) ((char *)sound + sound->dataLocation);
    for (i = 0; i < sampleCount; i++)
    	data[i] = soundData[i];
    err = SNDWriteSoundfile(name,sound);
    if(err)
    	fprintf(stderr,"*** Could not write sound file %s\n",name);
    else
    	printf("File %s written.\n",name);
    return err;
}

static void writeString(void) {
    int i, sampleCount = upper_rail->length;
    short data[sampleCount];

    for (i = 0; i < sampleCount; i++)
    	data[i] = rg_dl_access(upper_rail,i);
    writeSound("upper.snd", data, sampleCount);

    for (i = 0; i < sampleCount; i++)
    	data[i] = lg_dl_access(lower_rail,i);
    writeSound("lower.snd", data, sampleCount);

    for (i = 0; i < sampleCount; i++)
    	data[i] = rg_dl_access(upper_rail,  i) + lg_dl_access(lower_rail, i);
    writeSound("string.snd", data, sampleCount);
}

void main (int argc, char *argv[]) {
    int i, sampleCount;
    short *data;
    double amp, duration, pitch, pick, pickup, writesample;
    int pickupSample;

    if (argc != 8) {
        fprintf(stderr, "Usage: %s amp(<1.0) pitch(Hz) pickPosition(<1.0) "
                "pickupPosition(<1.0) duration(sec) writeSamp out.snd\n",
                argv[0]);
        fprintf(stderr, "example: %s .5 100 .1 .2 1 -1 test.snd\n", argv[0]);
        exit(1);
    }

    sscanf(argv[1],"%lf",&amp);
    sscanf(argv[2],"%lf",&pitch);
    sscanf(argv[3],"%lf",&pick);
    sscanf(argv[4],"%lf",&pickup);
    sscanf(argv[5],"%lf",&duration);
    sscanf(argv[6],"%lf",&writesample);

    sampleCount = duration * SRATE;
    data = (short *) malloc(sampleCount * sizeof(short));

    pickupSample = initString(amp, pitch, pick, pickup);

    for (i = 0; i < sampleCount; i++) {
        if (i == writesample) {
            printf("Writing string snapshot at sample %d\n",i);
            writeString();
        }
        data[i] = nextStringSample(pickupSample);
  	}
    
	writeSound(argv[7], data, sampleCount);
    freeString();
    exit(0);
}

