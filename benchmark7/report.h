#ifndef REPORT_H
#define REPORT_H

#include "encoder.h"
#include "parser.h"
#include "planner.h"
#include "table.h"
#include "util.h"

void report_summary(const AtlasSpec *spec, const TilePlan *plan, const TileTable *table, const EncodeStats *stats, const Trace *trace);

#endif
