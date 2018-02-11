#ifndef PERSON_H
#define PERSON_H

#include <stdint.h>

struct person {
    char name[48];
    uint32_t age;
    uint32_t id;
    char control;
    float salary;
};

#endif /* PERSON_H */
