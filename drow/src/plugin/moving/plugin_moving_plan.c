#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_json.h"
#include "plugin_moving_plan_i.h"
#include "plugin_moving_plan_track_i.h"
#include "plugin_moving_plan_point_i.h"
#include "plugin_moving_plan_node_i.h"
#include "plugin_moving_plan_segment_i.h"

/*plan */
plugin_moving_plan_t
plugin_moving_plan_create(plugin_moving_module_t module, ui_data_src_t src) {
    plugin_moving_plan_t plan;

    if (ui_data_src_type(src) != ui_data_src_type_moving_plan) {
        CPE_ERROR(
            module->m_em, "create plan at %s: src not moving_plan!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    if (ui_data_src_product(src)) {
        CPE_ERROR(
            module->m_em, "create plan at %s: product already loaded!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    plan = mem_alloc(module->m_alloc, sizeof(struct plugin_moving_plan));
    if (plan == NULL) {
        CPE_ERROR(
            module->m_em, "create plan at %s: alloc plan fail!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    plan->m_module = module;
    plan->m_src = src;
    bzero(&plan->m_data, sizeof(plan->m_data));
    plan->m_track_count = 0;
    plan->m_node_count = 0;
    
    TAILQ_INIT(&plan->m_tracks);
    TAILQ_INIT(&plan->m_nodes);
    
    ui_data_src_set_product(src, plan);

    return plan;
}

void plugin_moving_plan_free(plugin_moving_plan_t plan) {
    plugin_moving_module_t module = plan->m_module;

    while(!TAILQ_EMPTY(&plan->m_tracks)) {
        plugin_moving_plan_track_free(TAILQ_FIRST(&plan->m_tracks));
    }
    assert(plan->m_track_count == 0);
    
    while(!TAILQ_EMPTY(&plan->m_nodes)) {
        plugin_moving_plan_node_free(TAILQ_FIRST(&plan->m_nodes));
    }
    assert(plan->m_node_count == 0);

    mem_free(module->m_alloc, plan);
}

MOVING_PLAN *
plugin_moving_plan_data(plugin_moving_plan_t plan) {
    return &plan->m_data;
}

ui_data_src_t plugin_moving_plan_src(plugin_moving_plan_t plan) {
    return plan->m_src;
}

static plugin_moving_plan_track_t plugin_moving_plan_tracks_next(struct plugin_moving_plan_track_it * it) {
    plugin_moving_plan_track_t * data = (plugin_moving_plan_track_t *)(it->m_data);
    plugin_moving_plan_track_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_plan);

    return r;
}

void plugin_moving_plan_tracks(plugin_moving_plan_track_it_t it, plugin_moving_plan_t plan) {
    *(plugin_moving_plan_track_t *)(it->m_data) = TAILQ_FIRST(&plan->m_tracks);
    it->next = plugin_moving_plan_tracks_next;
}

static plugin_moving_plan_node_t plugin_moving_plan_nodes_next(struct plugin_moving_plan_node_it * it) {
    plugin_moving_plan_node_t * data = (plugin_moving_plan_node_t *)(it->m_data);
    plugin_moving_plan_node_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_plan);

    return r;
}

void plugin_moving_plan_nodes(plugin_moving_plan_node_it_t it, plugin_moving_plan_t plan) {
    *(plugin_moving_plan_node_t *)(it->m_data) = TAILQ_FIRST(&plan->m_nodes);
    it->next = plugin_moving_plan_nodes_next;
}

void plugin_moving_plan_print(write_stream_t s, int dent, plugin_moving_plan_t plan) {
    plugin_moving_module_t module = plan->m_module;
    struct plugin_moving_plan_track_it track_it;
    plugin_moving_plan_track_t track;
    struct plugin_moving_plan_node_it node_it;
    plugin_moving_plan_node_t node;
    
    stream_putc_count(s, ' ', dent);
    dr_json_print(s, &plan->m_data, sizeof(plan->m_data), module->m_meta_moving_plan, DR_JSON_PRINT_MINIMIZE, NULL);
    stream_printf(s, "\n");
    
    stream_putc_count(s, ' ', dent);
    stream_printf(s, "tracks\n");
    plugin_moving_plan_tracks(&track_it, plan);
    while((track = plugin_moving_plan_track_it_next(&track_it))) {
        struct plugin_moving_plan_point_it point_it;
        plugin_moving_plan_point_t point;

        stream_putc_count(s, ' ', dent + 4);
        dr_json_print(s, &track->m_data, sizeof(track->m_data), module->m_meta_moving_plan_track, DR_JSON_PRINT_MINIMIZE, NULL);
        stream_printf(s, "\n");

        stream_putc_count(s, ' ', dent + 4);
        stream_printf(s, "points\n");
        plugin_moving_plan_track_points(&point_it, track);
        while((point = plugin_moving_plan_point_it_next(&point_it))) {
            stream_putc_count(s, ' ', dent + 8);
            dr_json_print(s, &point->m_data, sizeof(point->m_data), module->m_meta_moving_plan_point, DR_JSON_PRINT_MINIMIZE, NULL);
            stream_printf(s, "\n");
        }        
    }

    stream_putc_count(s, ' ', dent);
    stream_printf(s, "nodes\n");
    plugin_moving_plan_nodes(&node_it, plan);
    while((node = plugin_moving_plan_node_it_next(&node_it))) {
        struct plugin_moving_plan_segment_it segment_it;
        plugin_moving_plan_segment_t segment;

        stream_putc_count(s, ' ', dent + 4);
        dr_json_print(s, &node->m_data, sizeof(node->m_data), module->m_meta_moving_plan_node, DR_JSON_PRINT_MINIMIZE, NULL);
        stream_printf(s, "\n");

        stream_putc_count(s, ' ', dent + 4);
        stream_printf(s, "segments\n");
        plugin_moving_plan_node_segments(&segment_it, node);
        while((segment = plugin_moving_plan_segment_it_next(&segment_it))) {
            stream_putc_count(s, ' ', dent + 8);
            dr_json_print(s, &segment->m_data, sizeof(segment->m_data), module->m_meta_moving_plan_segment, DR_JSON_PRINT_MINIMIZE, NULL);
            stream_printf(s, "\n");
        }        
    }
}

const char * plugin_moving_plan_dump(mem_buffer_t buffer, plugin_moving_plan_t plan) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);

    plugin_moving_plan_print((write_stream_t)&stream, 0, plan);
    stream_putc((write_stream_t)&stream, 0);

    return (const char *)mem_buffer_make_continuous(buffer, 0);
}
