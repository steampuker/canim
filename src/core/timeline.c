#include "./timeline.h"

#include "raylib/raylib.h"
#include "altarr.h"

struct Entry {
    canim_callback_t callback;
    canim_init_t birth, death;
    float start, end;
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

static inline void addSparsePages(struct SparsePageArray *sparse, int to) {
    int old_length = altarrLength(sparse->sparse);
    altarrResize(sparse->sparse, to + 1);

    for(int i = old_length; i <= to; ++i) {
        struct Page new_page = {.entries = altarrCreate(Entry)};
        altarrPush(sparse->dense, new_page);
        altarrAt(sparse->sparse, i) = altarrLength(sparse->dense) - 1;
    }
}

struct CanimTimeline {
    struct SparsePageArray pages;
    double length;
    unsigned pagination;

    int last_page;
};

void canimTimelineSetLength(CanimTimeline *timeline, double length) { timeline->length = length; }
double canimTimelineGetLength(CanimTimeline *timeline) { return timeline->length; }

CanimTimeline* canimTimelineCreate(int pagination_sec)
{
    CanimTimeline* timeline = MemAlloc(sizeof *timeline);
    if(!timeline) return NULL;

    if(!initSparsePages(&timeline->pages)) return MemFree(timeline), NULL;

    timeline->pagination = pagination_sec;
    timeline->last_page = -1;
    return timeline;
}

void canimTimelineDestroy(CanimTimeline *timeline)
{
    if(!timeline) return;

    for(int i = timeline->last_page > 0 ? timeline->last_page : 0; i < altarrLength(timeline->pages.sparse); ++i) {
        struct Page *page = getSparsePage(&timeline->pages, i);
        for(int j = 0; j < altarrLength(page->entries); ++j)
            altarrAt(page->entries, j).death ? altarrAt(page->entries, j).death() : 0;
    }

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

    if(page_id > timeline->last_page) {
        for(int i = 0; i < altarrLength(current->entries); ++i)
            altarrAt(current->entries, i).birth ? altarrAt(current->entries, i).birth() : 0;

        if(timeline->last_page >= 0) {
            struct Page *last = getSparsePage(&timeline->pages, timeline->last_page);

            for(int i = 0; i < altarrLength(last->entries); ++i)
                altarrAt(last->entries, i).death ? altarrAt(last->entries, i).death() : 0;
        }

        timeline->last_page = page_id;
    }

    for(int i = 0; i < altarrLength(current->entries); ++i) {
        struct Entry *entry = &altarrAt(current->entries, i);
        if(seconds >= entry->start && seconds <= entry->end)
            entry->callback((seconds - entry->start) / entry->end, seconds - entry->start);
    }

    return 1;
}

void canimTimelineAddEntry(CanimTimeline *timeline, double start, double end, canim_callback_t callback, canim_init_t birth, canim_init_t death)
{
    assert(timeline->pagination > 0);

    if(start > end)
        return;

    struct Entry entry = {.start = start, .end = end, .callback = callback, .birth = birth, .death = death};
    unsigned page_start = (unsigned)start / timeline->pagination;
    unsigned page_end = (unsigned)end / timeline->pagination;

    if(page_end >= altarrLength(timeline->pages.sparse))
        addSparsePages(&timeline->pages, page_end);

    if(page_start == page_end) {
        altarrPush(getSparsePage(&timeline->pages, page_end)->entries, entry);
        return;
    }

    entry.death = 0;
    altarrPush(getSparsePage(&timeline->pages, page_start)->entries, entry);

    entry.birth = 0;
    for(int i = page_start + 1; i < page_end; ++i)
        altarrPush(getSparsePage(&timeline->pages, i)->entries, entry);

    entry.death = death;
    altarrPush(getSparsePage(&timeline->pages, page_end)->entries, entry);
}

void canimTimelineComputeLength(CanimTimeline *timeline)
{
    if(altarrLength(timeline->pages.sparse) == 0)
        return;

    double max_length = 0.0;

    struct Page *last_page = getSparsePage(&timeline->pages, altarrLength(timeline->pages.sparse) - 1);

    for(int i = 0; i < altarrLength(last_page->entries); ++i) {
        double this_length = altarrAt(last_page->entries, i).end;
        max_length = this_length > max_length ? this_length : max_length;
    }

    timeline->length = max_length;
}
