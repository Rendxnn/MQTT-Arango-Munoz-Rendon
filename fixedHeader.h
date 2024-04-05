#ifndef FIXEDHEADER_H
#define FIXEDHEADER_H

struct fixed_header {
    unsigned char type: 4;
    unsigned char flags: 4;
    int remaining_length;
};

#endif