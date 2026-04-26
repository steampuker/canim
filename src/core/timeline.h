#ifndef CANIM_TIMELINE_H
#define CANIM_TIMELINE_H

typedef struct CanimTimeline CanimTimeline;

typedef void (*canim_callback_t)(double progress, double actual_time);
typedef void (*canim_init_t)(void);

CanimTimeline* canimTimelineCreate(int pagination_sec);
void canimTimelineDestroy(CanimTimeline* timeline);

void canimTimelineSetLength(CanimTimeline *timeline, double length);
double canimTimelineGetLength(CanimTimeline *timeline);
void canimTimelineComputeLength(CanimTimeline *timeline);

char canimTimelineIterate(CanimTimeline *timeline, double seconds);
void canimTimelineAddEntry(CanimTimeline *timeline, double start, double end, canim_callback_t callback, canim_init_t birth, canim_init_t death);

#endif
