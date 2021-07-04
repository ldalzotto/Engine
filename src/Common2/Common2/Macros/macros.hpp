#pragma once

#define COMA ,
#define SEMICOLON ;
#define STR(V1) #V1
#define CONCAT_2(V1, V2) V1##V2
#define CONCAT_3(V1, V2, V3) V1##V2##V3

#define LINE_RETURN \n
#define MULTILINE(...) #__VA_ARGS__
#define MULTILINE_(...) MULTILINE(__VA_ARGS__)

#define loop(Iteratorname, BeginNumber, EndNumber)                                                                                                                                                     \
    uimax Iteratorname = BeginNumber;                                                                                                                                                                  \
    Iteratorname < EndNumber;                                                                                                                                                                          \
    Iteratorname++
#define loop_int16(Iteratorname, BeginNumber, EndNumber)                                                                                                                                               \
    int16 Iteratorname = BeginNumber;                                                                                                                                                                  \
    Iteratorname < EndNumber;                                                                                                                                                                          \
    Iteratorname++

#define loop_int8(Iteratorname, BeginNumber, EndNumber)                                                                                                                                                \
    int8 Iteratorname = BeginNumber;                                                                                                                                                                   \
    Iteratorname < EndNumber;                                                                                                                                                                          \
    Iteratorname++

#define loop_reverse(Iteratorname, BeginNumber, EndNumber)                                                                                                                                             \
    uimax Iteratorname = EndNumber - 1;                                                                                                                                                                \
    Iteratorname != ((uimax)BeginNumber - 1);                                                                                                                                                          \
    --Iteratorname
