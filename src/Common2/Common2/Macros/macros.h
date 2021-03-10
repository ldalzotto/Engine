#pragma once

#define COMA ,
#define SEMICOLON ;
#define STR(V1) #V1
#define CONCAT_2(V1, V2) V1##V2
#define CONCAT_3(V1, V2, V3) V1##V2##V3

#define LINE_RETURN \n
#define MULTILINE(...) #__VA_ARGS__

#define cast(Type, Value) ((Type)(Value))
#define castv(Type, Value) *(Type*)(&Value)

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

#define vector_loop(VectorVariable, Iteratorname)                                                                                                                                                      \
    uimax Iteratorname = 0;                                                                                                                                                                            \
    Iteratorname < (VectorVariable)->Size;                                                                                                                                                             \
    Iteratorname++
#define vector_loop_reverse(VectorVariable, Iteratorname)                                                                                                                                              \
    uimax Iteratorname = (VectorVariable)->Size - 1;                                                                                                                                                   \
    Iteratorname != ((uimax)-1);                                                                                                                                                                       \
    --Iteratorname

#define pool_loop(PoolVariable, Iteratorname)                                                                                                                                                          \
    uimax Iteratorname = 0;                                                                                                                                                                            \
    Iteratorname < (PoolVariable)->get_size();                                                                                                                                                         \
    Iteratorname++

#define varyingvector_loop(VaryingVectorVariable, Iteratorname)                                                                                                                                        \
    uimax Iteratorname = 0;                                                                                                                                                                            \
    Iteratorname < (VaryingVectorVariable)->get_size();                                                                                                                                                \
    Iteratorname++