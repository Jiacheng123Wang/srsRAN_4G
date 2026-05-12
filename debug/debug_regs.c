#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <complex.h>

/* --- Mocks and Definitions to make it standalone --- */

#define SRSRAN_API
#define SRSRAN_SUCCESS 0
#define SRSRAN_ERROR -1
#define SRSRAN_ERROR_INVALID_INPUTS -2

#define INFO(...) printf("[INFO] " __VA_ARGS__)
#define DEBUG(...) printf("[DEBUG] " __VA_ARGS__)
#define ERROR(...) printf("[ERROR] " __VA_ARGS__)

#define SRSRAN_NRE 12
#define SRSRAN_CP_NORM 0
#define SRSRAN_CP_EXT 1
#define SRSRAN_CP_ISNORM(cp) (cp == SRSRAN_CP_NORM)
#define SRSRAN_CP_ISEXT(cp) (cp == SRSRAN_CP_EXT)

typedef float cf_t; // Using float for simplicity in debug, or _Complex float
#define _Complex float

typedef enum { SRSRAN_PHICH_NORM = 0, SRSRAN_PHICH_EXT } srsran_phich_length_t;
typedef enum { SRSRAN_PHICH_R_1_6 = 0, SRSRAN_PHICH_R_1_2, SRSRAN_PHICH_R_1, SRSRAN_PHICH_R_2 } srsran_phich_r_t;

typedef enum { SRSRAN_FDD = 0, SRSRAN_TDD = 1 } srsran_frame_type_t;

typedef struct {
    uint32_t nof_prb;
    uint32_t nof_ports;
    uint32_t id;
    uint8_t  cp; // Using uint8_t to match enum/int usage
    srsran_phich_length_t phich_length;
    srsran_phich_r_t      phich_resources;
    srsran_frame_type_t   frame_type;
} srsran_cell_t;

typedef struct {
    uint32_t k[4];
    uint32_t k0;
    uint32_t l;
    bool     assigned;
} srsran_regs_reg_t;

typedef struct {
    uint32_t            nof_regs;
    srsran_regs_reg_t** regs;
} srsran_regs_ch_t;

typedef struct {
    srsran_cell_t cell;
    uint32_t      max_ctrl_symbols;
    uint32_t      ngroups_phich;
    uint32_t      ngroups_phich_m1;
    srsran_phich_r_t      phich_res;
    srsran_phich_length_t phich_len;
    srsran_regs_ch_t  pcfich;
    srsran_regs_ch_t* phich;
    srsran_regs_ch_t  pdcch[3];
    uint32_t           phich_mi;
    uint32_t           nof_regs;
    srsran_regs_reg_t* regs;
} srsran_regs_t;

#define REGS_PHICH_NSYM 12
#define REGS_PHICH_REGS_X_GROUP 3
#define REGS_PCFICH_NSYM 16
#define REGS_PCFICH_NREGS 4
#define REGS_RE_X_REG 4
#define SRSRAN_PHICH_EXT 1
#define SRSRAN_PHICH_NORM 0

/* Mocking srsran_symbol_sz which is used in regs_reg_init and srsran_regs_init_opts */
int srsran_symbol_sz(uint32_t nof_prb) {
    return nof_prb * 12; 
}

/* --- Implementation copied from regs.c --- */

#define REG_IDX(r, i, n) r->k[i] + r->l * n * SRSRAN_NRE

/* Forward Declarations */
srsran_regs_reg_t* regs_find_reg(srsran_regs_t* h, uint32_t k, uint32_t l);
int regs_put_reg(srsran_regs_reg_t* reg, cf_t* reg_data, cf_t* slot_symbols, uint32_t nof_prb);
int regs_add_reg(srsran_regs_reg_t* reg, cf_t* reg_data, cf_t* slot_symbols, uint32_t nof_prb);
int regs_get_reg(srsran_regs_reg_t* reg, cf_t* slot_symbols, cf_t* reg_data, uint32_t nof_prb);
int regs_reset_reg(srsran_regs_reg_t* reg, cf_t* slot_symbols, uint32_t nof_prb);
void regs_pdcch_free(srsran_regs_t* h);
void srsran_regs_free(srsran_regs_t* h);
uint32_t srsran_regs_phich_ngroups(srsran_regs_t* h);

srsran_regs_reg_t* regs_find_reg(srsran_regs_t* h, uint32_t k, uint32_t l) {
    uint32_t i;
    for (i = 0; i < h->nof_regs; i++) {
        if (h->regs[i].l == l && h->regs[i].k0 == k) {
            return &h->regs[i];
        }
    }
    return NULL;
}

int regs_put_reg(srsran_regs_reg_t* reg, cf_t* reg_data, cf_t* slot_symbols, uint32_t nof_prb) {
    uint32_t i;
    for (i = 0; i < REGS_RE_X_REG; i++) {
        slot_symbols[REG_IDX(reg, i, nof_prb)] = reg_data[i];
    }
    return REGS_RE_X_REG;
}

int regs_add_reg(srsran_regs_reg_t* reg, cf_t* reg_data, cf_t* slot_symbols, uint32_t nof_prb) {
    uint32_t i;
    for (i = 0; i < REGS_RE_X_REG; i++) {
        slot_symbols[REG_IDX(reg, i, nof_prb)] += reg_data[i];
    }
    return REGS_RE_X_REG;
}

int regs_get_reg(srsran_regs_reg_t* reg, cf_t* slot_symbols, cf_t* reg_data, uint32_t nof_prb) {
    uint32_t i;
    for (i = 0; i < REGS_RE_X_REG; i++) {
        reg_data[i] = slot_symbols[REG_IDX(reg, i, nof_prb)];
    }
    return REGS_RE_X_REG;
}

int regs_reset_reg(srsran_regs_reg_t* reg, cf_t* slot_symbols, uint32_t nof_prb) {
    uint32_t i;
    for (i = 0; i < REGS_RE_X_REG; i++) {
        slot_symbols[REG_IDX(reg, i, nof_prb)] = 0;
    }
    return REGS_RE_X_REG;
}

void regs_pdcch_free(srsran_regs_t* h) {
    int i;
    for (i = 0; i < 3; i++) {
        if (h->pdcch[i].regs) {
            free(h->pdcch[i].regs);
            h->pdcch[i].regs = NULL;
        }
    }
}

#define PDCCH_NCOLS 32
const uint8_t PDCCH_PERM[PDCCH_NCOLS] = {1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31,
                                         0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30};

int regs_pdcch_init(srsran_regs_t* h) {
    int i, m, cfi, nof_ctrl_symbols;
    int ret = SRSRAN_ERROR;
    int nrows, ndummy, j;
    uint32_t k, kp;
    srsran_regs_reg_t** tmp = NULL;

    memset(&h->pdcch, 0, sizeof(srsran_regs_ch_t) * 3);

    for (cfi = 0; cfi < 3; cfi++) {
        if (h->cell.nof_prb <= 10) {
            nof_ctrl_symbols = cfi + 2;
        } else {
            nof_ctrl_symbols = cfi + 1;
        }

        tmp = malloc(sizeof(srsran_regs_reg_t*) * h->nof_regs);
        if (!tmp) {
            perror("malloc");
            goto clean_and_exit;
        }

        m = 0;
        for (i = 0; i < h->nof_regs; i++) {
            if (h->regs[i].l < nof_ctrl_symbols && !h->regs[i].assigned) {
                tmp[m] = &h->regs[i];
                m++;
            }
        }

        h->pdcch[cfi].nof_regs = m;
        h->pdcch[cfi].regs = malloc(sizeof(srsran_regs_reg_t*) * h->pdcch[cfi].nof_regs);
        if (!h->pdcch[cfi].regs) {
            perror("malloc");
            goto clean_and_exit;
        }

        nrows  = (h->pdcch[cfi].nof_regs - 1) / PDCCH_NCOLS + 1;
        ndummy = PDCCH_NCOLS * nrows - h->pdcch[cfi].nof_regs;
        if (ndummy < 0) ndummy = 0;

        k = 0;
        for (j = 0; j < PDCCH_NCOLS; j++) {
            for (i = 0; i < nrows; i++) {
                if (i * PDCCH_NCOLS + PDCCH_PERM[j] >= ndummy) {
                    m = i * PDCCH_NCOLS + PDCCH_PERM[j] - ndummy;
                    if (k < h->cell.id) {
                        kp = (h->pdcch[cfi].nof_regs + k - (h->cell.id % h->pdcch[cfi].nof_regs)) % h->pdcch[cfi].nof_regs;
                    } else {
                        kp = (k - h->cell.id) % h->pdcch[cfi].nof_regs;
                    }
                    h->pdcch[cfi].regs[m] = tmp[kp];
                    k++;
                }
            }
        }
        h->pdcch[cfi].nof_regs = (h->pdcch[cfi].nof_regs / 9) * 9;
        INFO("Init PDCCH REG space CFI %d. %d useful REGs\n", cfi + 1, h->pdcch[cfi].nof_regs);
        free(tmp);
        tmp = NULL;
    }

    ret = SRSRAN_SUCCESS;
clean_and_exit:
    if (tmp) free(tmp);
    if (ret == SRSRAN_ERROR) regs_pdcch_free(h);
    return ret;
}

int srsran_regs_pdcch_nregs(srsran_regs_t* h, uint32_t cfi) {
    if (cfi < 1 || cfi > 3) return SRSRAN_ERROR;
    return (int)h->pdcch[cfi - 1].nof_regs;
}

int srsran_regs_pdcch_ncce(srsran_regs_t* h, uint32_t cfi) {
    int nregs = srsran_regs_pdcch_nregs(h, cfi);
    return (nregs > 0) ? (uint32_t)(nregs / 9) : SRSRAN_ERROR;
}

int regs_phich_init(srsran_regs_t* h, uint32_t phich_mi, bool mbsfn_or_sf1_6_tdd) {
    float ng;
    uint32_t i, ni, li, n[3], nreg, mi;
    srsran_regs_reg_t** regs_phich[3];
    int ret = SRSRAN_ERROR;

    for (i = 0; i < 3; i++) regs_phich[i] = NULL;

    switch (h->phich_res) {
        case SRSRAN_PHICH_R_1_6: ng = (float)1 / 6; break;
        case SRSRAN_PHICH_R_1_2: ng = (float)1 / 2; break;
        case SRSRAN_PHICH_R_1:   ng = 1; break;
        case SRSRAN_PHICH_R_2:   ng = 2; break;
        default: ng = 0; break;
    }
    h->ngroups_phich_m1 = (int)ceilf(ng * ((float)h->cell.nof_prb / 8));
    h->ngroups_phich    = (int)phich_mi * h->ngroups_phich_m1;
    h->phich            = malloc(sizeof(srsran_regs_ch_t) * h->ngroups_phich);
    if (!h->phich) return -1;

    for (i = 0; i < h->ngroups_phich; i++) {
        h->phich[i].nof_regs = REGS_PHICH_REGS_X_GROUP;
        h->phich[i].regs     = malloc(sizeof(srsran_regs_reg_t*) * REGS_PHICH_REGS_X_GROUP);
        if (!h->phich[i].regs) goto clean_and_exit;
    }

    memset(n, 0, sizeof(n));
    for (i = 0; i < h->nof_regs; i++) {
        if (h->regs[i].l < 3 && !h->regs[i].assigned) {
            n[h->regs[i].l]++;
        }
    }

    for (i = 0; i < 3; i++) {
        regs_phich[i] = malloc(n[i] * sizeof(srsran_regs_reg_t*));
        if (!regs_phich[i]) goto clean_and_exit;
    }

    memset(n, 0, sizeof(n));
    for (i = 0; i < h->nof_regs; i++) {
        if (h->regs[i].l < 3 && !h->regs[i].assigned) {
            regs_phich[h->regs[i].l][n[h->regs[i].l]++] = &h->regs[i];
        }
    }

    nreg = 0;
    for (mi = 0; mi < h->ngroups_phich; mi++) {
        for (i = 0; i < 3; i++) {
            if (h->phich_len == SRSRAN_PHICH_EXT && mbsfn_or_sf1_6_tdd) {
                li = (mi / 2 + i + 1) % 2;
            } else {
                li = i;
            }
            if (h->phich_len == SRSRAN_PHICH_EXT && mbsfn_or_sf1_6_tdd) {
                ni = ((h->cell.id * n[li] / n[1]) + mi + i * n[li] / 3) % n[li];
            } else {
                ni = ((h->cell.id * n[li] / n[0]) + mi + i * n[li] / 3) % n[li];
            }
            h->phich[mi].regs[i]           = regs_phich[li][ni];
            h->phich[mi].regs[i]->assigned = true;
            nreg++;
        }
    }

    ret = SRSRAN_SUCCESS;
clean_and_exit:
    if (ret == SRSRAN_ERROR && h->phich) {
        for (i = 0; i < h->ngroups_phich; i++) {
            if (h->phich[i].regs) free(h->phich[i].regs);
        }
        free(h->phich);
    }
    for (i = 0; i < 3; i++) if (regs_phich[i]) free(regs_phich[i]);
    return ret;
}

void regs_phich_free(srsran_regs_t* h) {
    uint32_t i;
    for (i = 0; i < h->ngroups_phich; i++) {
        if (h->phich[i].regs) {
            free(h->phich[i].regs);
            h->phich[i].regs = NULL;
        }
    }
    free(h->phich);
    h->phich = NULL;
}

uint32_t srsran_regs_phich_ngroups(srsran_regs_t* h) {
    return h->ngroups_phich;
}

int regs_pcfich_init(srsran_regs_t* h) {
    uint32_t i, k_hat, k;
    srsran_regs_ch_t* ch = &h->pcfich;

    ch->regs = malloc(sizeof(srsran_regs_reg_t*) * REGS_PCFICH_NREGS);
    if (!ch->regs) return SRSRAN_ERROR;
    ch->nof_regs = REGS_PCFICH_NREGS;

    k_hat = (SRSRAN_NRE / 2) * (h->cell.id % (2 * h->cell.nof_prb));
    for (i = 0; i < REGS_PCFICH_NREGS; i++) {
        k = (k_hat + (i * h->cell.nof_prb / 2) * (SRSRAN_NRE / 2)) % (h->cell.nof_prb * SRSRAN_NRE);
        ch->regs[i] = regs_find_reg(h, k, 0);
        if (!ch->regs[i]) return SRSRAN_ERROR;
        if (ch->regs[i]->assigned) return SRSRAN_ERROR;
        ch->regs[i]->assigned = true;
    }
    return SRSRAN_SUCCESS;
}

void regs_pcfich_free(srsran_regs_t* h) {
    if (h->pcfich.regs) {
        free(h->pcfich.regs);
        h->pcfich.regs = NULL;
    }
}

int regs_reg_init(srsran_regs_reg_t* reg, uint32_t symbol, uint32_t nreg, uint32_t k0, uint32_t maxreg, uint32_t vo) {
    uint32_t i, j, z;
    reg->l = symbol;
    reg->assigned = false;
    if (maxreg == 2) {
        reg->k0 = k0 + nreg * 6;
        j = z = 0;
        for (i = 0; i < vo; i++) { reg->k[j] = k0 + nreg * 6 + i; j++; }
        for (i = 0; i < 2; i++) { reg->k[j] = k0 + nreg * 6 + i + vo + 1; j++; }
        z = j;
        for (i = 0; i < 4 - z; i++) { reg->k[j] = k0 + nreg * 6 + vo + 3 + i + 1; j++; }
        return (j == 4) ? SRSRAN_SUCCESS : SRSRAN_ERROR;
    } else if (maxreg == 3) {
        reg->k0 = k0 + nreg * 4;
        for (i = 0; i < 4; i++) reg->k[i] = k0 + nreg * 4 + i;
        return SRSRAN_SUCCESS;
    }
    return SRSRAN_ERROR;
}

int srsran_regs_init_opts(srsran_regs_t* h, srsran_cell_t cell, uint32_t phich_mi, bool mbsfn_or_sf1_6_tdd) {
    int ret = SRSRAN_ERROR;
    uint32_t i, k, j[4], jmax = 0, prb = 0;
    uint32_t n[4], vo;
    uint32_t max_ctrl_symbols;

    if (h == NULL) return SRSRAN_ERROR;
    memset(h, 0, sizeof(srsran_regs_t));
    max_ctrl_symbols = cell.nof_prb <= 10 ? 4 : 3;
    vo = cell.id % 3;
    h->cell = cell;
    h->max_ctrl_symbols = max_ctrl_symbols;
    h->phich_res = cell.phich_resources;
    h->phich_len = cell.phich_length;

    h->nof_regs = 0;
    for (i = 0; i < max_ctrl_symbols; i++) {
        uint32_t n_sym;
        if (i == 0) n_sym = 2;
        else if (i == 1) n_sym = (cell.nof_ports == 4) ? 2 : 3;
        else if (i == 2) n_sym = 3;
        else n_sym = (cell.cp == SRSRAN_CP_NORM) ? 3 : 2;
        
        h->nof_regs += cell.nof_prb * n_sym;
        n[i] = n_sym;
    }

    h->regs = malloc(sizeof(srsran_regs_reg_t) * h->nof_regs);
    if (!h->regs) return SRSRAN_ERROR;

    memset(j, 0, sizeof(j));
    k = i = prb = jmax = 0;
    while (k < h->nof_regs) {
        if (n[i] == 3 || (n[i] == 2 && jmax != 1)) {
            if (regs_reg_init(&h->regs[k], i, j[i], prb * SRSRAN_NRE, n[i], vo)) {
                return SRSRAN_ERROR;
            }
            j[i]++;
            k++;
        }
        i++;
        if (i == max_ctrl_symbols) {
            i = 0;
            jmax++;
        }
        if (jmax == 3) {
            prb++;
            memset(j, 0, sizeof(j));
            jmax = 0;
        }
    }

    if (regs_pcfich_init(h)) goto clean_and_exit;
    h->phich_mi = phich_mi;
    if (phich_mi > 0) {
        if (regs_phich_init(h, phich_mi, mbsfn_or_sf1_6_tdd)) goto clean_and_exit;
    }
    if (regs_pdcch_init(h)) goto clean_and_exit;

    ret = SRSRAN_SUCCESS;
clean_and_exit:
    if (ret != SRSRAN_SUCCESS) srsran_regs_free(h);
    return ret;
}

void srsran_regs_free(srsran_regs_t* h) {
    if (h->regs) free(h->regs);
    regs_pcfich_free(h);
    regs_phich_free(h);
    regs_pdcch_free(h);
    memset(h, 0, sizeof(srsran_regs_t));
}

/* --- Main Function for Debugging --- */

int main() {
    srsran_regs_t h;
    srsran_cell_t cell;

    // Test Case: nof_prb = 6, cell_id = 150, cp = normal (0)
    cell.nof_prb = 6;
    cell.nof_ports = 4;
    cell.id = 150;
    cell.cp = SRSRAN_CP_NORM;
    cell.phich_length = SRSRAN_PHICH_NORM;
    cell.phich_resources = SRSRAN_PHICH_R_1_6;
    cell.frame_type = 0; // FDD

    printf("=== Starting Debugging with: PRB=%u, CellID=%u, CP=%s ===\n", 
           cell.nof_prb, cell.id, SRSRAN_CP_ISNORM(cell.cp) ? "Normal" : "Extended");

    if (srsran_regs_init_opts(&h, cell, 1, false) != SRSRAN_SUCCESS) {
        printf("Failed to initialize regs\n");
        return 1;
    }

    // 1. PHICH Debug
    printf("\n--- [PHICH REG/RE INDEX] ---\n");
    printf("PHICH Groups: %u\n", srsran_regs_phich_ngroups(&h));
    for (uint32_t g = 0; g < srsran_regs_phich_ngroups(&h); g++) {
        printf("Group %u:\n", g);
        for (uint32_t r = 0; r < h.phich[g].nof_regs; r++) {
            srsran_regs_reg_t* reg = h.phich[g].regs[r];
            printf("  REG %u: l=%u, k=[%u, %u, %u, %u]\n", 
                   r, reg->l, reg->k[0], reg->k[1], reg->k[2], reg->k[3]);
        }
    }

    // 2. PCFICH Debug
    printf("\n--- [PCFICH REG/RE INDEX] ---\n");
    for (uint32_t i = 0; i < h.pcfich.nof_regs; i++) {
        srsran_regs_reg_t* reg = h.pcfich.regs[i];
        printf("  REG %u: l=%u, k=[%u, %u, %u, %u]\n", 
               i, reg->l, reg->k[0], reg->k[1], reg->k[2], reg->k[3]);
    }

    // 3. PDCCH Debug for different CFI
    for (uint32_t cfi = 1; cfi <= 3; cfi++) {
        printf("\n--- [PDCCH REG/RE INDEX] CFI=%u ---\n", cfi);
        uint32_t nregs = srsran_regs_pdcch_nregs(&h, cfi);
        uint32_t ncce = srsran_regs_pdcch_ncce(&h, cfi);
        printf("  NREGs: %u, NCCEs: %u\n", nregs, ncce);
        for (uint32_t i = 0; i < nregs; i++) {
            srsran_regs_reg_t* reg = h.pdcch[cfi-1].regs[i];
            printf("  REG %u: l=%u, k=[%u, %u, %u, %u]\n", 
                   i, reg->l, reg->k[0], reg->k[1], reg->k[2], reg->k[3]);
        }
    }

    srsran_regs_free(&h);
    printf("\n=== Debugging Finished ===\n");
    return 0;
}