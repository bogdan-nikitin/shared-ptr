#include "control_block.h"

void detail::control_block_base::check_lifetime() {
  if (strong_ref_cnt + weak_ref_cnt == 0) {
    delete this;
  }
}

void detail::control_block_base::dec_strong() {
  --strong_ref_cnt;
  if (strong_ref_cnt == 0) {
    clear();
  }
  check_lifetime();
}

void detail::control_block_base::dec_weak() {
  --weak_ref_cnt;
  check_lifetime();
}
