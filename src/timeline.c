#include "core/timeline.h"
#include "raylib/raylib.h"
#include "utils/altarr.h"

struct Entry {
    canim_callback_t *callback;
    double start, end;
};
altarr_typedef(struct Entry, Entry);

struct Page { altarr_t(Entry) entries; };
altarr_typedef(struct Page, Page);
altarr_typedef(int);

struct SparsePageArray {
    altarr_t(int) sparse;
    altarr_t(Page) dense;
};

static inline bool initSparsePages(struct SparsePageArray *sparse) {
    sparse->dense = altarrCreate(Page);
    sparse->sparse = altarrCreate(int);
    return altarrValid(sparse->dense) && altarrValid(sparse->sparse);
}

static inline struct Page* getSparsePage(struct SparsePageArray *sparse, int at) {
    return &altarrAt(sparse->dense, altarrAt(sparse->sparse, at));
}

static inline void addSparsePage(struct SparsePageArray *sparse, struct Page *page, int at) {
    if(at >= altarrLength(sparse->sparse))
        altarrResize(sparse->sparse, at + 1);

    altarrPush(sparse->dense, *page);
    altarrAt(sparse->sparse, at) = altarrLength(sparse->dense) - 1;
}

struct CanimTimeline {
    struct SparsePageArray pages;
    double length;
    int pagination;
};

void canimTimelineSetLength(CanimTimeline *timeline, double length) { timeline->length = length; }
double canimTimelineGetLength(CanimTimeline *timeline) { return timeline->length; }

CanimTimeline* canimTimelineCreate(int pagination_sec)
{
    CanimTimeline* timeline = MemAlloc(sizeof *timeline);
    if(!timeline) return NULL;

    if(!initSparsePages(&timeline->pages)) return MemFree(timeline), NULL;

    timeline->pagination = pagination_sec;
    return timeline;
}

void canimTimelineDestroy(CanimTimeline *timeline)
{
    if(!timeline) return;

    for(int i = 0; i < altarrLength(timeline->pages.dense); ++i)
        altarrDestroy(altarrAt(timeline->pages.dense, i).entries);

    altarrDestroy(timeline->pages.dense);
    altarrDestroy(timeline->pages.sparse);

    MemFree(timeline);
}

char canimTimelineIterate(CanimTimeline *timeline, double seconds)
{
    if(seconds >= timeline->length)
        return 0;

    int page_id = (int)seconds / timeline->pagination;
    if(page_id >= altarrLength(timeline->pages.sparse))
        return 1;

    struct Page *current = getSparsePage(&timeline->pages, page_id);

    for(int i = 0; i < altarrLength(current->entries); ++i) {
        struct Entry *entry = &altarrAt(current->entries, i);
        if(seconds >= entry->start && seconds <= entry->end)
            entry->callback((seconds - entry->start) / entry->end, seconds);
    }

    return 1;
}

void canimTimelineAddEntry(CanimTimeline *timeline, double start, double end, canim_callback_t *callback)
{
    assert(timeline->pagination > 0);
    int page_start = (int)start / timeline->pagination;
    int page_end = (int)end / timeline->pagination;

    struct Entry entry = {.start = start, .end = end, .callback = callback };

    for(int i = page_start; i <= page_end; ++i) {
        if(page_end >= altarrLength(timeline->pages.sparse))
            addSparsePage(&timeline->pages, &(struct Page){.entries = altarrCreate(Entry)}, i);

        altarrPush(getSparsePage(&timeline->pages, i)->entries, entry);
    }
}

void canimTimelineComputeLength(CanimTimeline *timeline)
{
    double max_length = 0.0;
    if(altarrLength(timeline->pages.sparse) == 0)
        return;

    struct Page *last_page = getSparsePage(&timeline->pages, altarrLength(timeline->pages.sparse) - 1);

    for(int i = 0; i < altarrLength(last_page->entries); ++i) {
        double this_length = altarrAt(last_page->entries, i).end;
        max_length = this_length > max_length ? this_length : max_length;
    }

    timeline->length = max_length;
}
