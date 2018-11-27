#include "pvm/bc/names.hpp"

const std::unordered_map<std::string_view, SpecialVar> special_var_names{
    {"argument0", SpecialVar::argument0},
    {"argument1", SpecialVar::argument1},
    {"argument2", SpecialVar::argument2},
    {"argument3", SpecialVar::argument3},
    {"argument4", SpecialVar::argument4},
    {"argument5", SpecialVar::argument5},
    {"argument6", SpecialVar::argument6},
    {"argument7", SpecialVar::argument7},
    {"argument8", SpecialVar::argument8},
    {"argument9", SpecialVar::argument9},
    {"argument10", SpecialVar::argument10},
    {"argument11", SpecialVar::argument11},
    {"argument12", SpecialVar::argument12},
    {"argument13", SpecialVar::argument13},
    {"argument14", SpecialVar::argument14},
    {"argument15", SpecialVar::argument15},

    {"argument_count", SpecialVar::argument_count},

    {"id", SpecialVar::id}};
