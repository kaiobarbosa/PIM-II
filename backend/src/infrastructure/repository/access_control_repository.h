#ifndef ACCESS_CONTROL_REPOSITORY_H
#define ACCESS_CONTROL_REPOSITORY_H

int get_user_access_context(const char *user_id, char *profile,
                            unsigned long profile_capacity, char branch_id[7]);
int user_can_create_product_in_branch(const char *user_id,
                                      const char *branch_id);
int user_can_withdraw_stock_from_branch(const char *user_id,
                                        const char *branch_id);
int user_can_sell_product(const char *user_id, const char *product_id,
                          const char *branch_id, const char *crf);

#endif
