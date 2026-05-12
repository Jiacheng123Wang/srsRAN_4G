#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// --- 基础定义 (模拟 srsRAN) ---
#define SRSRAN_NRE 12          // 每个 PRB 的子载波数量
#define REGS_RE_X_REG 4        // 每个 REG 包含的有效 RE 数量

typedef struct {
    uint32_t k0;               // 逻辑起始位置
    uint32_t l;                // 符号索引 (0, 1, 2...)
    bool     assigned;         // 是否已被分配 (给 PCFICH/PHICH)
    uint32_t k[REGS_RE_X_REG]; // 存储 4 个避开 CRS 后的物理 RE 索引
} srsran_regs_reg_t;

/**
 * @brief 初始化单个 REG，并根据 3GPP 规范跳过 CRS 占用的位置
 * * @param reg   指向 REG 结构体的指针
 * @param l     符号索引 (Symbol Index)
 * @param k0    物理起始子载波索引
 * @param pci   物理小区标识 (用于计算 v_shift)
 */
void regs_reg_init(srsran_regs_reg_t* reg, uint32_t l, uint32_t k0, uint32_t pci) {
    reg->l = l;
    reg->k0 = k0;
    reg->assigned = false;

    // 3GPP TS 36.211: v_shift = PCI mod 6
    uint32_t v_shift = pci % 6;
    
    uint32_t re_count = 0;
    uint32_t current_k = k0;

    // 在 Symbol 0 且天线端口 > 1 时，每 6 个子载波有 2 个 CRS
    // 位置位于: k mod 6 = v_shift 和 (v_shift + 3) mod 6
    while (re_count < REGS_RE_X_REG) {
        bool is_crs = false;
        if (l == 0) {
            if ((current_k % 6 == v_shift) || (current_k % 6 == (v_shift + 3) % 6)) {
                is_crs = true;
            }
        }

        if (!is_crs) {
            reg->k[re_count] = current_k;
            re_count++;
        }
        current_k++;
    }
}

// --- 测试主程序 ---
int main() {
    uint32_t pci = 150;     // 设定 PCI 为 150 (v_shift = 0)
    uint32_t nof_prb = 6;   // 1.4MHz 带宽
    uint32_t n0 = nof_prb * 2; // Symbol 0 的 REG 总数 (每个 PRB 2 个 REG)

    // 分配内存
    srsran_regs_reg_t* regs_s0 = (srsran_regs_reg_t*)malloc(sizeof(srsran_regs_reg_t) * n0);

    printf("/* LTE 1.4MHz PHY REG Initialization (PCI: %u, Symbol 0) */\n", pci);
    printf("/* CRS positions: k mod 6 = %u and %u */\n\n", pci % 6, (pci % 6 + 3) % 6);
    printf("%-10s | %-10s | %-20s\n", "REG Index", "Logic k0", "Physical RE Indices");
    printf("------------------------------------------------------------\n");

    // 初始化 Symbol 0 的所有 REG
    for (uint32_t i = 0; i < n0; i++) {
        // 在 1.4MHz 下，每个 PRB 有 2 个 REG
        // 每个 REG 的物理跨度约为 6 个子载波
        uint32_t k0 = i * 6; 
        
        regs_reg_init(&regs_s0[i], 0, k0, pci);

        // 打印结果
        printf("REG %-6u | k0=%-7u | [%u, %u, %u, %u]\n", 
               i, 
               regs_s0[i].k0, 
               regs_s0[i].k[0], 
               regs_s0[i].k[1], 
               regs_s0[i].k[2], 
               regs_s0[i].k[3]);
    }

    free(regs_s0);
    return 0;
}