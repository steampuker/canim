#ifndef CANIM_TIMELINE_H
#define CANIM_TIMELINE_H

typedef struct CanimTimeline CanimTimeline;

typedef void canim_callback_t(double progress, double actual_time);

CanimTimeline* canimTimelineCreate(int pagination_sec);
void canimTimelineDestroy(CanimTimeline* timeline);

void canimTimelineSetLength(CanimTimeline *timeline, double length);
double canimTimelineGetLength(CanimTimeline *timeline);
void canimTimelineComputeLength(CanimTimeline *timeline);

char canimTimelineIterate(CanimTimeline *timeline, double seconds);
void canimTimelineAddEntry(CanimTimeline *timeline, double start, double end, void (*callback)(double, double));

#endif
