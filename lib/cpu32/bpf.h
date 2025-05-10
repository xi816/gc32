// Basic processor functions
struct govnodate {
  short year;
  char  month;
  char  day;
};
typedef struct govnodate govnodate;

govnodate govnodate_convert(unsigned short date) {
  return (govnodate){
    .year = ((date & 0b1111111000000000)>>9) + 1970,
    .month = ((date & 0b0000000111100000)>>5) + 1,
    .day = (date & 0b0000000000011111) + 1
  };
}

U16 GC_GOVNODATE() {
  time_t rawtm;
  struct tm* localtm;

  /*             Year   Mon  Day    */
  /*             /-----\/--\/---\   */
  /* Govndate 2: 0000000000000000   */
  time(&rawtm);
  localtm = localtime(&rawtm);
  return ((localtm->tm_year-70)<<9) + (localtm->tm_mon<<5) + (localtm->tm_mday-1);
}

U0 fatal(char* msg) {
  printf("gc32: \033[91mcannot operate\033[0m, error:\n  %s", msg);
}
