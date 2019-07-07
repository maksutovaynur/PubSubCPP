//
// Created by aynur on 12.03.19.
//

#ifndef PUBSUBCPP_DEBUG_HPP
#define PUBSUBCPP_DEBUG_HPP

unsigned int make_flag(unsigned int bitpos) {
    return (unsigned int)(1 << bitpos);
}

unsigned int DF1 = make_flag(1);
unsigned int DF2 = make_flag(2);
unsigned int DF3 = make_flag(3);
unsigned int DF4 = make_flag(4);
unsigned int DF5 = make_flag(5);
unsigned int DF6 = make_flag(6);
unsigned int DF7 = make_flag(7);
unsigned int DF8 = make_flag(8);

unsigned int ALL = DF1 & DF2 & DF3 & DF4 & DF5 & DF6 & DF7 & DF8;
#endif //PUBSUBCPP_DEBUG_HPP
