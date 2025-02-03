#ifndef CMDLINE_H_
#define CMDLINE_H_


struct ush_cmdline {
    unsigned int length;

    /* typing */
    char current;
    unsigned int cursor;

#ifdef CONFIG_USH_VIMODE
    bool insertmode;
#endif

    char cmdline[];
};


struct ush_cmdline *
cmdline_create(unsigned short maxchars);


int
cmdline_dispose(struct ush_cmdline *cl);


#endif  // CMDLINE_H_
