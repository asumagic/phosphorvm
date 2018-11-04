## Oddities in community documentation and (un)solved mysteries

There are a few informations on documentation you may find online I found to be incorrect or incomplete, or are lacking detail when further analyzing some `data.win`.  
I am trying to list all of those I encountered.

### Bytecode ver `0xF`+ `pushvar`

Buckle up, that's a big one.

More than just meant to push "other" variables as pointed out by [this page](https://pcy.ulyssis.be/undertale/decompilation-corrected), it appears that `0xC3` is behaving very strangely.

Here are some observations:
1. Certain values, such as `fps`, `room`, `room_width`, `room_height`, `application_surface`, `id`, `x`, `y` and more importantly `argument`, `argument*` and `argument_count` values are *always* pushed to the stack using `0xC3`.
2. The variables appearing as `0xC3` operands seem to have the first unknown value set to `-1` and the second unknown value set to `-6`. That is, assuming instance types, respectively `InstanceType::self` (in the above link) and `InstanceType::unknown` (refer to the link)?!
3. It should be noted that the second `VariableDefinition` unknown value occasionally accepts strictly positive values and sometimes is set to `0` (e.g. the peculiar `arguments` variable that appears a lot). This surely indicates there is a specific meaning to it we haven't caught yet.
4. References for those variables always seem to have `VariableType::Normal`.
5. It *is* possible to write to certain of those values. Yet, there is only one `pop` instruction. There is surely information kept in `VariableDefinition` for this... and it could very well be the second unknown value in `VariableDefinition`.
6. The name for those variables is *never* duplicated!

The following is the parsed `VARI` chunk executed on the DELTARUNE `data.win` file:

```
        Variable sprite_index        , instance type -1 (unknown=fffffffa).
        Variable argument0           , instance type -1 (unknown=fffffffa).
        Variable argument1           , instance type -1 (unknown=fffffffa).
        Variable argument2           , instance type -1 (unknown=fffffffa).
        Variable argument3           , instance type -1 (unknown=fffffffa).
        Variable room                , instance type -1 (unknown=fffffffa).
        Variable y                   , instance type -1 (unknown=fffffffa).
        Variable x                   , instance type -1 (unknown=fffffffa).
        Variable alarm               , instance type -1 (unknown=fffffffa).
        Variable image_index         , instance type -1 (unknown=fffffffa).
        Variable argument4           , instance type -1 (unknown=fffffffa).
        Variable argument5           , instance type -1 (unknown=fffffffa).
        Variable argument6           , instance type -1 (unknown=fffffffa).
        Variable argument7           , instance type -1 (unknown=fffffffa).
        Variable argument8           , instance type -1 (unknown=fffffffa).
        Variable argument9           , instance type -1 (unknown=fffffffa).
        Variable argument10          , instance type -1 (unknown=fffffffa).
        Variable fps                 , instance type -1 (unknown=fffffffa).
        Variable id                  , instance type -1 (unknown=fffffffa).
        Variable image_speed         , instance type -1 (unknown=fffffffa).
        Variable sprite_width        , instance type -1 (unknown=fffffffa).
        Variable sprite_height       , instance type -1 (unknown=fffffffa).
        Variable image_blend         , instance type -1 (unknown=fffffffa).
        Variable application_surface , instance type -1 (unknown=fffffffa).
        Variable visible             , instance type -1 (unknown=fffffffa).
        Variable xstart              , instance type -1 (unknown=fffffffa).
        Variable ystart              , instance type -1 (unknown=fffffffa).
        Variable image_xscale        , instance type -1 (unknown=fffffffa).
        Variable image_yscale        , instance type -1 (unknown=fffffffa).
        Variable depth               , instance type -1 (unknown=fffffffa).
        Variable image_angle         , instance type -1 (unknown=fffffffa).
        Variable speed               , instance type -1 (unknown=fffffffa).
        Variable room_width          , instance type -1 (unknown=fffffffa).
        Variable room_height         , instance type -1 (unknown=fffffffa).
        Variable argument11          , instance type -1 (unknown=fffffffa).
        Variable argument12          , instance type -1 (unknown=fffffffa).
        Variable argument13          , instance type -1 (unknown=fffffffa).
        Variable argument14          , instance type -1 (unknown=fffffffa).
        Variable argument            , instance type -1 (unknown=fffffffa).
        Variable argument_count      , instance type -1 (unknown=fffffffa).
        Variable working_directory   , instance type -1 (unknown=fffffffa).
        Variable view_current        , instance type -1 (unknown=fffffffa).
        Variable direction           , instance type -1 (unknown=fffffffa).
        Variable vspeed              , instance type -1 (unknown=fffffffa).
        Variable gravity             , instance type -1 (unknown=fffffffa).
        Variable hspeed              , instance type -1 (unknown=fffffffa).
        Variable image_alpha         , instance type -1 (unknown=fffffffa).
        Variable persistent          , instance type -1 (unknown=fffffffa).
        Variable friction            , instance type -1 (unknown=fffffffa).
        Variable gravity_direction   , instance type -1 (unknown=fffffffa).
        Variable room_persistent     , instance type -1 (unknown=fffffffa).
        Variable image_number        , instance type -1 (unknown=fffffffa).
        Variable bbox_bottom         , instance type -1 (unknown=fffffffa).
        Variable bbox_right          , instance type -1 (unknown=fffffffa).
        Variable bbox_top            , instance type -1 (unknown=fffffffa).
        Variable bbox_left           , instance type -1 (unknown=fffffffa).
        Variable keyboard_lastkey    , instance type -1 (unknown=fffffffa).
        Variable object_index        , instance type -1 (unknown=fffffffa).
        Variable path_position       , instance type -1 (unknown=fffffffa).
        Variable path_speed          , instance type -1 (unknown=fffffffa).
        Variable mask_index          , instance type -1 (unknown=fffffffa).
        Variable room_speed          , instance type -1 (unknown=fffffffa).
```

**Hypothesis:**

It appears that `0xC3` is meant for special variables. There is no real common thing between all of those variables.

However, because of `6.` we know that they are surely meant to be unique. Yet `argument0` is definitively not going to be a global value, because it would get overriden just at the following call (and, nope, it doesn't get "saved" by being pushed temporarily to the stack when it happens).

As for `2.` and `3.`: If the second unknown value is *really* an `InstanceType`.  
Instance IDs seem to begin at 10000 and this value is usually below 100 when it's >0.  
I suspect it is some sort of location argument. It should be checked what types of `push` are used for variables that have this value `>0`. After a few quick tests (which requires deeper checking!) it appears that only `pushloc` use this. Very interestingly, this could be a way to pool local variables.

Because of all of this, I believe that `0xC3` is meant to push a variable **that has an entirely unique behavior**.

That theory seems to fit in the pieces nicely. However, I am not sure why it would have the first unknown value set to `InstanceType::self` - perhaps because it doesn't matter.