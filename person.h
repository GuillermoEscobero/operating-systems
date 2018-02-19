/**
 * Definition of personal data for PA1 - Operating Systems 17/18 
 * 
 * DO NOT MODIFY
 *
 * Data types valid as of Feb 2018 @guernika.lab.inf.uc3m.es with GCC (Debian 6.3.0-18) 6.3.0 20170516
 */

typedef struct {
  char          name[48];  /* 48-byte string */
  unsigned int  age;       /* 4-byte unsigned integer between 0 and 150, included */
  unsigned int  id;        /* 4-byte unsigned integer between 0 and 99999999, included */
  char          id_ctrl;   /* Character */
  double        salary;    /* 8-byte decimal between 0.0 and 100000.0, included */
} Person;