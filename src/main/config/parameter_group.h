/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

typedef uint16_t pgn_t;

// parameter group registry flags
typedef enum {
    PGRF_NONE = 0,
    PGRF_CLASSIFICATON_BIT = (1 << 0),
} pgRegistryFlags_e;

// parameter group classification
typedef enum {
    PGC_SYSTEM = 0,
    PGC_PROFILE = 1,
} pgClassification_e;

typedef struct pgRegistry_s {
    // Base of the group in RAM.
    uint8_t *base;
    // The pointer to update after loading the record into ram.
    uint8_t **ptr;
    // Size of the group in RAM.
    uint16_t size;
    // The parameter group number.
    pgn_t pgn;
    // The in-memory format number.  Bump when making incompatible
    // changes to the PG.
    uint8_t format;
    // see pgRegistryFlags_e
    uint8_t flags;
} pgRegistry_t;


#define PG_PACKED __attribute__((packed))

extern const pgRegistry_t __pg_registry_start[];
extern const pgRegistry_t __pg_registry_end[];

// Helper to iterate over the PG register.  Cheaper than a visitor
// style callback.
#define PG_FOREACH(_name) \
    for (const pgRegistry_t *(_name) = __pg_registry_start; (_name) < __pg_registry_end; (_name)++)

#define PG_FOREACH_PROFILE(_name)                                    \
    PG_FOREACH(_name)                                                \
        if(((_name)->flags & PGRF_CLASSIFICATON_BIT) != PGC_PROFILE) \
            continue;                                                \
        else                                                         \
            /**/

#define PG_DECLARE(_type, _name)                \
    extern _type _name                          \
    /**/

#define PG_DECLARE_ARR(_type, _size, _name)     \
    extern _type _name[_size]                   \
    /**/

#define PG_DECLARE_PROFILE(_type, _name)        \
    extern _type *_name                         \
    /**/

#define PG_REGISTER_ATTRIBUTES __attribute__ ((section(".pg_registry"), used, aligned(4)))

// Register config
#define PG_REGISTER(_type, _name, _pgn, _format)                        \
    _type _name;                                                        \
    static const pgRegistry_t _name##Registry PG_REGISTER_ATTRIBUTES = { \
        .base = (uint8_t*)&_name,                                       \
        .ptr = 0,                                                       \
        .size = sizeof(_name),                                          \
        .pgn = _pgn,                                                    \
        .format = _format,                                              \
        .flags = PGC_SYSTEM,                                            \
    }                                                                   \
    /**/

// Register config
#define PG_REGISTER_ARR(_type, _size, _name, _pgn, _format)             \
    _type _name[_size];                                                 \
    static const pgRegistry_t _name##Registry PG_REGISTER_ATTRIBUTES = { \
        .base = (uint8_t*)&_name,                                       \
        .ptr = 0,                                                       \
        .size = sizeof(_name),                                          \
        .pgn = _pgn,                                                    \
        .format = _format,                                              \
        .flags = PGC_SYSTEM,                                            \
    }                                                                   \
    /**/


#define PG_REGISTER_PROFILE(_type, _name, _pgn, _format)                \
    STATIC_UNIT_TESTED _type _name##Storage[MAX_PROFILE_COUNT];         \
    _type *_name;                                                       \
    static const pgRegistry_t _name##Registry PG_REGISTER_ATTRIBUTES = { \
        .base = (uint8_t*)&_name##Storage,                              \
        .ptr = (uint8_t **)&_name,                                      \
        .size = sizeof(_type),                                          \
        .pgn = _pgn,                                                    \
        .format = _format,                                              \
        .flags = PGC_PROFILE,                                           \
    }                                                                   \
    /**/


typedef uint8_t (*pgMatcherFuncPtr)(const pgRegistry_t *candidate, const void *criteria);

const pgRegistry_t* pgFind(pgn_t pgn);
const pgRegistry_t* pgMatcher(pgMatcherFuncPtr matcher, const void *criteria);
void pgLoad(const pgRegistry_t* reg, const void *from, int size);
void pgResetAll(uint8_t profileCount);
void pgActivateProfile(uint8_t profileIndexToActivate);
