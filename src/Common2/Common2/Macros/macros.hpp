#pragma once

#define COMA ,
#define SEMICOLON ;
#define STR(V1) #V1
#define CAT_2(V1, V2) V1##V2
#define CAT_2_(V1, V2) CAT_2(V1, V2)
#define CAT_3(V1, V2, V3) CAT_2_(V1, CAT_2(V2, V3))
#define CAT_3_(V1, V2, V3) CAT_3(V1, V2, V3)

#define LINE_RETURN \n
#define MULTILINE(...) #__VA_ARGS__

#define loop(Iteratorname, BeginNumber, EndNumber)                                                                                                                                                     \
    uimax Iteratorname = BeginNumber;                                                                                                                                                                  \
    Iteratorname < EndNumber;                                                                                                                                                                          \
    Iteratorname++
#define loop_int16(Iteratorname, BeginNumber, EndNumber)                                                                                                                                               \
    int16 Iteratorname = BeginNumber;                                                                                                                                                                  \
    Iteratorname < EndNumber;                                                                                                                                                                          \
    Iteratorname++

#define loop_reverse(Iteratorname, BeginNumber, EndNumber)                                                                                                                                             \
    uimax Iteratorname = EndNumber - 1;                                                                                                                                                                \
    Iteratorname != ((uimax)BeginNumber - 1);                                                                                                                                                          \
    --Iteratorname

#define CB(name) const name##&