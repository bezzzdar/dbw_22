#define INSTRUCTION_LIST(_)                                                    \
  _(BAN, "ban", INT)                                                           \
  _(PROMOTE, "promote", INT)                                                   \
  _(ADD_POINTS, "add_points", INT_INT)                                         \
  _(SUB_POINTS, "sub_points", INT_INT)                                         \
  _(KEK, "kek", INT_INT_INT)                                                   \
  _(STATISTICS, "statistics", INT)                                             \
  _(SETSTATE, "state", INT_INT)                                                \
  _(PREPOD, "prepod", INT_INT)                                                 \
  _(RETURN, "return", INT)
