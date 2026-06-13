#ifndef CANIM_TIMELINE_H
#define CANIM_TIMELINE_H

#include "canim.h"

typedef struct CanimTimeline CanimTimeline;

CanimTimeline* canimTimelineCreate(int pagination_sec);
void canimTimelineDestroy(CanimTimeline* timeline);

void canimTimelineSetLength(CanimTimeline *timeline, double length);
double canimTimelineGetLength(CanimTimeline *timeline);
void canimTimelineComputeLength(CanimTimeline *timeline);

char canimTimelineIterate(CanimTimeline *timeline, double seconds);
void canimTimelineAddEntry(CanimTimeline *timeline, CanimAnimation *callback);

#endif
