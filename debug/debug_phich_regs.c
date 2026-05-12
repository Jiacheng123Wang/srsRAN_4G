#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

// 模拟 srsRAN 的宏和常量
#define SRSRAN_NRE 12
#define SRSRAN_SUCCESS 0
#define SRSRAN_ERROR -1
#define REGS_PHICH_REGS_X_GROUP 3
#define SRSRAN_PHICH_NORM 1
#define SRSRAN_PHICH_EXT 2
#define SRSRAN_CP_ISEXT(cp) ((cp) == 1)
#define SRSRAN_CP_ISNORM(cp) ((cp) == 0)

typedef float cf_t;

typedef struct {
    uint32_t k0;
    uint32_t l;
    bool assigned;
    uint32_t k[4];
} srsran_regs_reg_t;

typedef struct {
    uint32_t nof_regs;
    srsran_regs_reg_t** regs;
} srsran_regs_ch_t;

typedef struct {
    uint32_t nof_prb;
    uint32_t id;
    uint32_t phich_resources;
    uint32_t phich_length;
    uint32_t ports;
    uint32_t cp;
} srsran_cell_t;

typedef struct {
    srsran_cell_t cell;
    uint32_t nof_regs;
    srsran_regs_reg_t* regs;
    srsran_regs_ch_t* pcfich; 
    srsran_regs_ch_t* phich;
    int ngroups_phich_m1;
    int ngroups_phich;
    int phich_len;
    int phich_res;
} srsran_regs_t;

// 模拟 regs_find_reg
srsran_regs_reg_t* regs_find_reg(srsran_regs_t* h, uint32_t k, uint32_t l) {
    for (uint32_t i = 0; i < h->nof_regs; i++) {
        if (h->regs[i].l == l && h->regs[i].k0 == k) {
            return &h->regs[i];
        }
    }
    return NULL;
}

// 模拟 regs_reg_init
int regs_reg_init(srsran_regs_reg_t* reg, uint32_t symbol, uint32_t nreg, uint32_t k0, uint32_t maxreg, uint32_t vo) {
    reg->l = symbol;
    reg->assigned = false;
    if (maxreg == 2) {
        reg->k0 = k0 + nreg * 6;
    } else {
        reg->k0 = k0 + nreg * 4;
    }
    return SRSRAN_SUCCESS;
}

// PCFICH 初始化 (简化版，用于占用 REGs)
int regs_pcfich_init(srsran_regs_t* h) {
    uint32_t i;
    uint32_t k_hat, k;
    srsran_regs_ch_t* ch = h->pcfich;

    ch->nof_regs = 4;
    ch->regs = malloc(sizeof(srsran_regs_reg_t*) * ch->nof_regs);

    k_hat = (SRSRAN_NRE / 2) * (h->cell.id % (2 * h->cell.nof_prb));
    for (i = 0; i < ch->nof_regs; i++) {
        k = (k_hat + (i * h->cell.nof_prb / 2) * (SRSRAN_NRE / 2)) % (h->cell.nof_prb * SRSRAN_NRE);
        ch->regs[i] = regs_find_reg(h, k, 0);
        if (ch->regs[i]) {
            ch->regs[i]->assigned = true;
        }
    }
    return SRSRAN_SUCCESS;
}

// PHICH 初始化
int regs_phich_init(srsran_regs_t* h, uint32_t phich_mi, bool mbsfn_or_sf1_6_tdd) {
    float ng;
    uint32_t i, ni, li, n[3], nreg, mi;
    srsran_regs_reg_t** regs_phich[3];

    // 简化 ng 计算
    ng = 1.0f; 
    h->ngroups_phich_m1 = (int)ceilf(ng * ((float)h->cell.nof_prb / 8));
    h->ngroups_phich = (int)phich_mi * h->ngroups_phich_m1;
    h->phich = malloc(sizeof(srsran_regs_ch_t) * h->ngroups_phich);

    for (i = 0; i < h->ngroups_phich; i++) {
        h->phich[i].nof_regs = REGS_PHICH_REGS_X_GROUP;
        h->phich[i].regs = malloc(sizeof(srsran_regs_reg_t*) * REGS_PHICH_REGS_X_GROUP);
    }

    // Step 2 & 3: Count and collect unassigned REGs
    bzero(n, sizeof(n));
    for (i = 0; i < h->nof_regs; i++) {
        if (h->regs[i].l < 3 && !h->regs[i].assigned) {
            n[h->regs[i].l]++;
        }
    }

    for (i = 0; i < 3; i++) {
        regs_phich[i] = malloc(n[i] * sizeof(srsran_regs_reg_t*));
    }

    bzero(n, sizeof(n));
    for (i = 0; i < h->nof_regs; i++) {
        if (h->regs[i].l < 3 && !h->regs[i].assigned) {
            regs_phich[h->regs[i].l][n[h->regs[i].l]++] = &h->regs[i];
        }
    }

    nreg = 0;
    printf("Starting PHICH mapping...\n");
    printf("Mapping Unit (mi) | Symbol (i) | REG index in regs_phich | RE index (k0) | RE symbol (l)\n");
    printf("--------------------------------------------------------------------------------------\n");

    for (mi = 0; mi < h->ngroups_phich; mi++) {
        for (i = 0; i < 3; i++) {
            // Step 7
            if (h->phich_len == SRSRAN_PHICH_NORM) {
                li = 0;
            } else if (h->phich_len == SRSRAN_PHICH_EXT && mbsfn_or_sf1_6_tdd) {
                li = (mi / 2 + i + 1) % 2;
            } else {
                li = i;
            }

            // Step 8
            if (h->phich_len == SRSRAN_PHICH_EXT && mbsfn_or_sf1_6_tdd) {
                ni = ((h->cell.id * n[li] / n[1]) + mi + i * n[li] / 3) % n[li];
            } else {
                ni = ((h->cell.id * n[li] / n[0]) + mi + i * n[li] / 3) % n[li];
            }

            h->phich[mi].regs[i] = regs_phich[li][ni];
            h->phich[mi].regs[i]->assigned = true;

            // 打印调试信息
            // 我们需要找到这个 regs_phich[li][ni] 在原始 h->regs 中的 index
            // 但题目要求的是 RE index (k0) 和 symbol (l)
            printf("%16u | %10u | %24u | %13u | %10u\n", 
                   mi, i, ni, h->phich[mi].regs[i]->k0, h->phich[mi].regs[i]->l);
            
            nreg++;
        }
    }

    if (SRSRAN_CP_ISEXT(h->cell.cp)) {
        h->ngroups_phich *= 2;
    }

    for (i = 0; i < 3; i++) {
        free(regs_phich[i]);
    }

    return SRSRAN_SUCCESS;
}

int main() {
    srsran_regs_t h;
    memset(&h, 0, sizeof(h));

    h.cell.nof_prb = 100;
    h.cell.id = 1;
    h.cell.cp = 0; // Normal CP
    h.phich_len = SRSRAN_PHICH_NORM;

    // 模拟初始化所有 REGs
    uint32_t n0 = h.cell.nof_prb * 2;
    uint32_t n1 = h.cell.nof_prb * 3;
    uint32_t n2 = h.cell.nof_prb * 3;
    h.nof_regs = n0 + n1 + n2;
    h.regs = malloc(sizeof(srsran_regs_reg_t) * h.nof_regs);

    uint32_t k_idx = 0;
    // Symbol 0
    for (uint32_t j = 0; j < n0; j++) {
        uint32_t prb = j / 2;
        uint32_t nreg = j % 2;
        regs_reg_init(&h.regs[k_idx], 0, nreg, prb * SRSRAN_NRE, 2, 0);
        k_idx++;
    }
    // Symbol 1
    for (uint32_t j = 0; j < n1; j++) {
        uint32_t prb = j / 3;
        uint32_t nreg = j % 3;
        regs_reg_init(&h.regs[k_idx], 1, nreg, prb * SRSRAN_NRE, 3, 0);
        k_idx++;
    }
    // Symbol 2
    for (uint32_t j = 0; j < n2; j++) {
        uint32_t prb = j / 3;
        uint32_t nreg = j % 3;
        regs_reg_init(&h.regs[k_idx], 2, nreg, prb * SRSRAN_NRE, 3, 0);
        k_idx++;
    }

    // 分配 PCFICH 指针
    h.pcfich = malloc(sizeof(srsran_regs_ch_t));
    memset(h.pcfich, 0, sizeof(srsran_regs_ch_t));

    // 1. 先初始化 PCFICH 占用 REGs
    regs_pcfich_init(&h);

    // 2. 再初始化 PHICH
    regs_phich_init(&h, 1, false);

    // 清理
    free(h.regs);
    free(h.pcfich->regs);
    free(h.pcfich);
    for(int i=0; i<h.ngroups_phich; i++) {
        free(h.phich[i].regs);
    }
    free(h.phich);

    return 0;
}