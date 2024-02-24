#include <string.h>
#include <stdlib.h>

#include "a_star.h"

int directions[8][2] = {
    {1, 0},
    {0, 1},
    {1, 1},
    {-1, 0},
    {0, -1},
    {-1, -1},
    {-1, 1},
    {1, -1},
};

static bool _are_coords_valid(coords_t coords, int max_x, int max_h) {
    int x = coords.x;
    int y = coords.y;
    return (x >= 0 && y >= 0 && x < max_x && y < max_h);
}

static bool _are_coords_equal(coords_t a, coords_t b) {
    return (a.x == b.x && a.y == b.y);
}

// This function is responsile for estimating the cost of moving from one
// cell to another. It accounts for diagonal movement, however its "base cost"
// is 14 [about sqrt(2) * 10] compared to 10 [1 * 10] for linear movement. The
// function always yields the smallest possible cost estimate.
static int _heuristic_cost(coords_t a, coords_t b) {
    enum { DIAGONAL_COST = 14, STRAIGHT_COST = 10 };

    // Taking a diagonal steps towards the target is always
    // optimal; thus we must "fit in" as much of them as possible.
    int diagonal_steps = MIN(ABS(a.x - b.x), ABS(a.y - b.y));
    int straight_steps = MAX(ABS(a.x - b.x), ABS(a.y - b.y)) - diagonal_steps;

    int cost = diagonal_steps * DIAGONAL_COST + straight_steps * STRAIGHT_COST;
    return cost;
}

// Uses a* star algorithm to compute the best "step" from source to destination
// which is then returned. Thus it works well for a dynamic enviorment where
// the destination position can change every tick.
//
// WARNING: The algorithm assumes there exists a valid path between the two
// coordinates.
coords_t compute_next_position(int *map, int w, int h, coords_t source, coords_t destination, bool(*is_traversable)(int)) {
    typedef struct node_t node_t;
    struct node_t {
        int g, h, f; // costs
        int x, y;    // position
        node_t *next, *prev;
        node_t *parent;
    };

    AS_ASSERT(_are_coords_valid(source, w, h));
    AS_ASSERT(_are_coords_valid(destination, w, h));

    AS_ASSERT(is_traversable(map[source.y * h + source.x]));
    AS_ASSERT(is_traversable(map[destination.y * h + destination.x]));

    if (_are_coords_equal(source, destination))
        return source;

    bool *closed = malloc(sizeof(*closed) * w * h);
    memset(closed, false, sizeof(*closed) * w * h);

    // Don't forget to free the stuff, otherwise we will quickly
    // run out of memory
    node_t *open = malloc(sizeof(node_t));
    *open = (node_t){
        .h = _heuristic_cost(source, destination), .g = 0, .f = _heuristic_cost(source, destination),
        .x = source.x, .y = source.y,
        .next = NULL, .prev = NULL, .parent = NULL
    };

    // This is necessary to track all nodes allocated,
    // because it is impossible (or maybe it is?) to
    // free memory "on the go" due to several problems.
    // The array tracks all pointers that were allocated
    // so that when we exit we can free everything.
    void **history = malloc(sizeof(*history) * w * h);
    history[0] = open;
    int his_index = 1;

    while (open != NULL) {
        // (i) Find the node with the lowest f-cost
        node_t *cur = NULL;
        for (node_t *it = open; it; it = it->next) {
            if (cur == NULL || it->f < cur->f || (it->f == cur-> f && it->h < cur->h)) {
                cur = it;
            }
        }

        // (ii) Set the current node to closed and remove it from open
        closed[cur->y * h + cur->x] = true;
        if (cur->prev)
            cur->prev->next = cur->next;
        if (cur->next)
            cur->next->prev = cur->prev;
        if (cur == open)
            open = cur->next;

        // (iii) Check if we have found the path
        if (cur->x == destination.x && cur->y == destination.y) {
            // Now if we did indeed find the path, we are currently
            // on the last node, so we need to retrace out path
            // back to the original node and return the node before it.
            node_t *last = NULL;
            node_t *current = cur;
            while (current->parent) {
                last = current;
                current = current->parent;
            }
            coords_t coords = (coords_t){last->x, last->y};

            // Free all used memory
            for (int i = 0; i < his_index; ++i) {
                free(history[i]);
            }
            free(closed);
            free(history);

            return coords;
        }

        // (iv) Loop through neighbours of the current node
        for (int i = 0; i < 8; ++i) {
            int dx = directions[i][0];
            int dy = directions[i][1];

            int new_x = cur->x + dx;
            int new_y = cur->y + dy;

            if (!_are_coords_valid((coords_t){new_x, new_y}, w, h)
                || !is_traversable(map[new_y * h + new_x])
                || closed[new_y * h + new_x]) {
                continue;
            }

            bool is_open = false;
            node_t *neighbour = NULL;
            for (node_t *it = open; it; it = it->next) {
                if (it->x == new_x && it->y == new_y) {
                    is_open = true;
                    neighbour = it;
                }
            }

            int new_gcost = cur->g + _heuristic_cost((coords_t){cur->x, cur->y}, (coords_t){new_x, new_y});
            if (!is_open) {
                node_t *neighbour = malloc(sizeof(*neighbour));
                *neighbour = (node_t){
                    .g = new_gcost, .h = _heuristic_cost((coords_t){new_x, new_y}, destination),
                    .f = new_gcost + _heuristic_cost((coords_t){new_x, new_y}, destination),
                    .x = new_x, .y = new_y,
                    .next = open, .prev = NULL, .parent = cur
                };
                if (open)
                    open->prev = neighbour;
                open = neighbour;
                history[his_index++] = open;

            } else if (neighbour->g > new_gcost) {
                int new_fcost = new_gcost + _heuristic_cost((coords_t){new_x, new_y}, destination);
                neighbour->g = new_gcost;
                neighbour->f = new_fcost;
                neighbour->parent = cur;
            }
        }
    }

    return source;
}
