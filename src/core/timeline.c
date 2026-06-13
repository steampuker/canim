#include "./timeline.h"

#include "canim.h"
#include "raylib/raylib.h"
#include "altarr.h"

typedef struct Entry {
    CanimAnimation animation;
    CanimAnimationFrame frame;
} Entry;

altarr_typedef(Entry);

typedef struct Page { altarr_t(Entry) entries; } Page;
altarr_typedef(Page);

struct CanimTimeline {
    altarr_t(Page) pages;
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

    timeline->pages = altarrCreate(Page);
    if(!altarrValid(timeline->pages)) return MemFree(timeline), NULL;

    timeline->pagination = pagination_sec;
    timeline->last_page = -1;
    return timeline;
}

void canimTimelineDestroy(CanimTimeline *timeline)
{
    if(!timeline) return;

    for(int i = timeline->last_page > 0 ? timeline->last_page : 0; i < altarrLength(timeline->pages); ++i) {
        Page *page = &altarrAt(timeline->pages, i);
        for(int j = 0; j < altarrLength(page->entries); ++j) {
            struct Entry *entry = &altarrAt(page->entries, j);
            entry->animation.deinit ? entry->animation.deinit(entry->frame.data) : 0;
        }
    }

    for(int i = 0; i < altarrLength(timeline->pages); ++i)
        altarrDestroy(altarrAt(timeline->pages, i).entries);

    altarrDestroy(timeline->pages);

    MemFree(timeline);
}

char canimTimelineIterate(CanimTimeline *timeline, double seconds)
{
    if(seconds >= timeline->length)
        return 0;

    int page_id = (int)seconds / timeline->pagination;

    if(page_id >= altarrLength(timeline->pages))
        return 1;

    struct Page *current = &altarrAt(timeline->pages, page_id);

    if(page_id > timeline->last_page) {
        for(int i = 0; i < altarrLength(current->entries); ++i) {
            Entry *entry = &altarrAt(current->entries, i);
            if(entry->animation.init)
            entry->frame.data = entry->animation.init();
        }

        if(timeline->last_page >= 0) {
            struct Page *last = &altarrAt(timeline->pages, timeline->last_page);

            for(int j = 0; j < altarrLength(last->entries); ++j) {
                struct Entry *entry = &altarrAt(last->entries, j);
                entry->animation.deinit ? entry->animation.deinit(entry->frame.data) : 0;
            }
        }

        timeline->last_page = page_id;
    }

    for(int i = 0; i < altarrLength(current->entries); ++i) {
        struct Entry *entry = &altarrAt(current->entries, i);
        if(seconds < entry->animation.start || seconds > (entry->animation.start + entry->animation.duration))
            continue;

        entry->frame.progress = (seconds - entry->animation.start) / entry->animation.duration;
        entry->frame.global_progress = seconds - entry->animation.start;

        entry->animation.callback(&entry->frame);
    }

    return 1;
}

void canimTimelineAddEntry(CanimTimeline *timeline, CanimAnimation *callback)
{
    assert(timeline->pagination > 0);

    Entry entry = { *callback, {.animation_length = callback->duration} };
    unsigned page_start = (unsigned)callback->start / timeline->pagination;
    unsigned page_end = (unsigned)(callback->start + callback->duration) / timeline->pagination;

    if(page_end >= altarrLength(timeline->pages)) {
        size_t old_length = altarrLength(timeline->pages);
        altarrResize(timeline->pages, page_end + 1);
        for(size_t i = old_length; i < altarrLength(timeline->pages); ++i)
            altarrAt(timeline->pages, i).entries = altarrCreate(Entry);
    }

    if(page_start == page_end) {
        altarrPush(altarrAt(timeline->pages, page_end).entries, entry);
        return;
    }

    Entry clone = entry;
    clone.animation.deinit = 0;
    altarrPush(altarrAt(timeline->pages, page_start).entries, entry);

    clone.animation.init = 0;
    for(int i = page_start + 1; i < page_end; ++i)
        altarrPush(altarrAt(timeline->pages, i).entries, entry);

    entry.animation.deinit = clone.animation.deinit;
    altarrPush(altarrAt(timeline->pages, page_end).entries, entry);
}

void canimTimelineComputeLength(CanimTimeline *timeline)
{
    if(altarrLength(timeline->pages) == 0)
        return;

    double max_length = 0.0;
    struct Page *last_page = &altarrAt(timeline->pages, altarrLength(timeline->pages) - 1);

    for(int i = 0; i < altarrLength(last_page->entries); ++i) {
        Entry entry = altarrAt(last_page->entries, i);
        double this_length = entry.animation.start + entry.animation.duration;
        max_length = this_length > max_length ? this_length : max_length;
    }

    timeline->length = max_length;
}
