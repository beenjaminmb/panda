
#include "qemu/osdep.h"
#include "cpu.h"
#include <stdint.h>
#include "panda/plugin.h"
#include "panda/callback_support.h"
#include "panda/common.h"

#include "panda/rr/rr_log.h"
#include "exec/cpu-common.h"
#include "exec/ram_addr.h"

void panda_callbacks_hd_transfer(CPUState *cpu, Hd_transfer_type type, uint64_t src_addr, uint64_t dest_addr, uint32_t num_bytes)
{
    if (rr_mode == RR_REPLAY) {
        panda_cb_list *plist;
        for (plist = panda_cbs[PANDA_CB_REPLAY_HD_TRANSFER];
             plist != NULL;
             plist = panda_cb_list_next(plist)) {
				if (plist->enabled) plist->entry.replay_hd_transfer(cpu, type, src_addr, dest_addr, num_bytes);
        }
    }
}

void panda_callbacks_handle_packet(CPUState *cpu, uint8_t *buf, size_t size, uint8_t direction, uint64_t old_buf_addr) {
    if (rr_mode == RR_REPLAY) {
        panda_cb_list *plist;
        for (plist = panda_cbs[PANDA_CB_REPLAY_HANDLE_PACKET];
             plist != NULL;
             plist = panda_cb_list_next(plist)) {
				if (plist->enabled) plist->entry.replay_handle_packet(cpu, buf, size, direction, old_buf_addr);
        }
    }
}
void panda_callbacks_net_transfer(CPUState *cpu, Net_transfer_type type, uint64_t src_addr, uint64_t dst_addr, uint32_t num_bytes) {
    if (rr_mode == RR_REPLAY) {
        panda_cb_list *plist;
        for (plist = panda_cbs[PANDA_CB_REPLAY_NET_TRANSFER];
             plist != NULL;
             plist = panda_cb_list_next(plist)) {
				if (plist->enabled) plist->entry.replay_net_transfer(cpu, type, src_addr, dst_addr, num_bytes);
        }
    }
}

// These are used in exec.c
void panda_callbacks_before_dma(CPUState *cpu, hwaddr addr1, const uint8_t *buf, hwaddr l, int is_write) {
    if (rr_mode == RR_REPLAY) {
        panda_cb_list *plist;
        for (plist = panda_cbs[PANDA_CB_REPLAY_BEFORE_DMA];
             plist != NULL; plist = panda_cb_list_next(plist)) {
			if (plist->enabled) plist->entry.replay_before_dma(cpu, is_write, (uint8_t *) buf, (uint64_t) addr1, l);
        }
    }
}

void panda_callbacks_after_dma(CPUState *cpu, hwaddr addr1, const uint8_t *buf, hwaddr l, int is_write) {
    if (rr_mode == RR_REPLAY) {
        panda_cb_list *plist;
       for (plist = panda_cbs[PANDA_CB_REPLAY_AFTER_DMA];
            plist != NULL; plist = panda_cb_list_next(plist)) {
			if (plist->enabled) plist->entry.replay_after_dma(cpu, is_write, (uint8_t *) buf, (uint64_t) addr1, l);
        }
    }
}

// These are used in cpu-exec.c
void panda_callbacks_before_block_exec(CPUState *cpu, TranslationBlock *tb) {
    panda_cb_list *plist;
    for (plist = panda_cbs[PANDA_CB_BEFORE_BLOCK_EXEC];
         plist != NULL; plist = panda_cb_list_next(plist)) {
		if (plist->enabled) plist->entry.before_block_exec(cpu, tb);
    }
}


void panda_callbacks_after_block_exec(CPUState *cpu, TranslationBlock *tb) {
    panda_cb_list *plist;
    for (plist = panda_cbs[PANDA_CB_AFTER_BLOCK_EXEC];
         plist != NULL; plist = panda_cb_list_next(plist)) {
		if (plist->enabled) plist->entry.after_block_exec(cpu, tb);
    }
}


void panda_callbacks_before_block_translate(CPUState *cpu, target_ulong pc) {
    panda_cb_list *plist;
    for (plist = panda_cbs[PANDA_CB_BEFORE_BLOCK_TRANSLATE];
         plist != NULL; plist = panda_cb_list_next(plist)) {
		if (plist->enabled) plist->entry.before_block_translate(cpu, pc);
    }
}


void panda_callbacks_after_block_translate(CPUState *cpu, TranslationBlock *tb) {
    panda_cb_list *plist;
    for (plist = panda_cbs[PANDA_CB_AFTER_BLOCK_TRANSLATE];
         plist != NULL; plist = panda_cb_list_next(plist)) {
       if (plist->enabled) plist->entry.after_block_translate(cpu, tb);
    }
}

void panda_before_find_fast(void) {
    if (panda_plugin_to_unload){
        panda_plugin_to_unload = false;
        int i;
        for (i = 0; i < MAX_PANDA_PLUGINS; i++){
            if (panda_plugins_to_unload[i]){
                panda_do_unload_plugin(i);
                panda_plugins_to_unload[i] = false;
            }
        }
    }
    if (panda_flush_tb()) {
        tb_flush(first_cpu);
    }
}


bool panda_callbacks_after_find_fast(CPUState *cpu, TranslationBlock *tb, bool bb_invalidate_done, bool *invalidate) {
    panda_cb_list *plist;
    if (!bb_invalidate_done) {
        for(plist = panda_cbs[PANDA_CB_BEFORE_BLOCK_EXEC_INVALIDATE_OPT];
            plist != NULL; plist = panda_cb_list_next(plist)) {
            *invalidate |=
                plist->entry.before_block_exec_invalidate_opt(cpu, tb);
        }
        return true;
    }
    return false;
}


// These are used in target-i386/translate.c
bool panda_callbacks_insn_translate(CPUState *env, target_ulong pc) {
    panda_cb_list *plist;
    bool panda_exec_cb = false;
    for(plist = panda_cbs[PANDA_CB_INSN_TRANSLATE]; plist != NULL;
        plist = panda_cb_list_next(plist)) {
        if (plist->enabled) panda_exec_cb |= plist->entry.insn_translate(env, pc);
    }
    return panda_exec_cb;
}

bool panda_callbacks_after_insn_translate(CPUState *env, target_ulong pc) {
    panda_cb_list *plist;
    bool panda_exec_cb = false;
    for(plist = panda_cbs[PANDA_CB_AFTER_INSN_TRANSLATE]; plist != NULL;
        plist = panda_cb_list_next(plist)) {
        if (plist->enabled) panda_exec_cb |= plist->entry.after_insn_translate(env, pc);
    }
    return panda_exec_cb;
}

static inline hwaddr get_paddr(CPUState *cpu, target_ulong addr, void *ram_ptr) {
    if (!ram_ptr) {
        return panda_virt_to_phys(cpu, addr);
    }

    ram_addr_t offset = 0;
    RAMBlock *block = qemu_ram_block_from_host(ram_ptr, false, &offset);
    if (!block) {
        return panda_virt_to_phys(cpu, addr);
    } else {
        assert(block->mr);
        return block->mr->addr + offset;
    }
}

// These are used in softmmu_template.h
// ram_ptr is a possible pointer into host memory from the TLB code. Can be NULL.
void panda_callbacks_before_mem_read(CPUState *env, target_ulong pc,
                                     target_ulong addr, uint32_t data_size,
                                     void *ram_ptr) {
    panda_cb_list *plist;
    for(plist = panda_cbs[PANDA_CB_VIRT_MEM_BEFORE_READ]; plist != NULL;
        plist = panda_cb_list_next(plist)) {
        if (plist->enabled) plist->entry.virt_mem_before_read(env, env->panda_guest_pc, addr,
                                          data_size);
    }
    if (panda_cbs[PANDA_CB_PHYS_MEM_BEFORE_READ]) {
        hwaddr paddr = get_paddr(env, addr, ram_ptr);
        for(plist = panda_cbs[PANDA_CB_PHYS_MEM_BEFORE_READ]; plist != NULL;
            plist = panda_cb_list_next(plist)) {
            if (plist->enabled) plist->entry.phys_mem_before_read(env, env->panda_guest_pc, paddr,
                                              data_size);
        }
    }
}


void panda_callbacks_after_mem_read(CPUState *env, target_ulong pc,
                                    target_ulong addr, uint32_t data_size,
                                    uint64_t result, void *ram_ptr) {
    panda_cb_list *plist;
    for(plist = panda_cbs[PANDA_CB_VIRT_MEM_AFTER_READ]; plist != NULL;
        plist = panda_cb_list_next(plist)) {
        if (plist->enabled) plist->entry.virt_mem_after_read(env, env->panda_guest_pc, addr,
                                         data_size, &result);
    }
    if (panda_cbs[PANDA_CB_PHYS_MEM_AFTER_READ]) {
        hwaddr paddr = get_paddr(env, addr, ram_ptr);
        for(plist = panda_cbs[PANDA_CB_PHYS_MEM_AFTER_READ]; plist != NULL;
            plist = panda_cb_list_next(plist)) {
            if (plist->enabled) plist->entry.phys_mem_after_read(env, env->panda_guest_pc, paddr,
                                             data_size, &result);
        }
    }
}


void panda_callbacks_before_mem_write(CPUState *env, target_ulong pc,
                                      target_ulong addr, uint32_t data_size,
                                      uint64_t val, void *ram_ptr) {
    panda_cb_list *plist;
    for(plist = panda_cbs[PANDA_CB_VIRT_MEM_BEFORE_WRITE]; plist != NULL;
        plist = panda_cb_list_next(plist)) {
        if (plist->enabled) plist->entry.virt_mem_before_write(env, env->panda_guest_pc, addr,
                                           data_size, &val);
    }
    if (panda_cbs[PANDA_CB_PHYS_MEM_BEFORE_WRITE]) {
        hwaddr paddr = get_paddr(env, addr, ram_ptr);
        for(plist = panda_cbs[PANDA_CB_PHYS_MEM_BEFORE_WRITE]; plist != NULL;
            plist = panda_cb_list_next(plist)) {
            if (plist->enabled) plist->entry.phys_mem_before_write(env, env->panda_guest_pc, paddr,
                                               data_size, &val);
        }
    }
}


void panda_callbacks_after_mem_write(CPUState *env, target_ulong pc,
                                     target_ulong addr, uint32_t data_size,
                                     uint64_t val, void *ram_ptr) {
    panda_cb_list *plist;
    for(plist = panda_cbs[PANDA_CB_VIRT_MEM_AFTER_WRITE]; plist != NULL;
        plist = panda_cb_list_next(plist)) {
        if (plist->enabled) plist->entry.virt_mem_after_write(env, env->panda_guest_pc, addr,
                                          data_size, &val);
    }
    if (panda_cbs[PANDA_CB_PHYS_MEM_AFTER_WRITE]) {
        hwaddr paddr = get_paddr(env, addr, ram_ptr);
        for(plist = panda_cbs[PANDA_CB_PHYS_MEM_AFTER_WRITE]; plist != NULL;
            plist = panda_cb_list_next(plist)) {
            if (plist->enabled) plist->entry.phys_mem_after_write(env, env->panda_guest_pc, paddr,
                                              data_size, &val);
        }
    }
}


// vl.c
void panda_callbacks_after_machine_init(void) {
    panda_cb_list *plist;

    //printf ("entering panda_callbacks_after_machine_init:  panda_cbs[PANDA_CB_AFTER_MACHINE_INIT=%d] = %" PRIx64 "\n", 
//            PANDA_CB_AFTER_MACHINE_INIT, (uint64_t) panda_cbs[PANDA_CB_AFTER_MACHINE_INIT]);

    for(plist = panda_cbs[PANDA_CB_AFTER_MACHINE_INIT]; plist != NULL;
        plist = panda_cb_list_next(plist)) {
        if (plist->enabled) plist->entry.after_machine_init(first_cpu);
    }
}

void panda_callbacks_during_machine_init(MachineState *machine) {
    panda_cb_list *plist;
    for(plist = panda_cbs[PANDA_CB_DURING_MACHINE_INIT]; plist != NULL;
	plist = panda_cb_list_next(plist)) {
	if (plist->enabled) plist->entry.during_machine_init(machine);
    }
}

void panda_callbacks_top_loop(void) {
    panda_cb_list *plist;
    for(plist = panda_cbs[PANDA_CB_TOP_LOOP]; plist != NULL;
        plist = panda_cb_list_next(plist)) {
        if (plist->enabled) plist->entry.top_loop(first_cpu);
    }
}

// target-i386/misc_helpers.c
void panda_callbacks_cpuid(CPUState *env) {
    panda_cb_list *plist;
    for(plist = panda_cbs[PANDA_CB_GUEST_HYPERCALL]; plist != NULL; plist = panda_cb_list_next(plist)) {
        if (plist->enabled) plist->entry.guest_hypercall(env);
    }
}


void panda_callbacks_cpu_restore_state(CPUState *env, TranslationBlock *tb) {
    panda_cb_list *plist;
    for(plist = panda_cbs[PANDA_CB_CPU_RESTORE_STATE]; plist != NULL;
        plist = panda_cb_list_next(plist)) {
        if (plist->enabled) plist->entry.cpu_restore_state(env, tb);
    }
}


void panda_callbacks_asid_changed(CPUState *env, target_ulong old_asid, target_ulong new_asid) {
    panda_cb_list *plist;
    for(plist = panda_cbs[PANDA_CB_ASID_CHANGED]; plist != NULL; plist = panda_cb_list_next(plist)) {
        if (plist->enabled) plist->entry.asid_changed(env, old_asid, new_asid);
    }
}

void panda_callbacks_serial_receive(CPUState *cpu, uint64_t fifo_addr,
                                    uint8_t value)
{
    if (rr_mode == RR_REPLAY) {
        panda_cb_list *plist;
        for (plist = panda_cbs[PANDA_CB_REPLAY_SERIAL_RECEIVE]; plist != NULL;
             plist = panda_cb_list_next(plist)) {
            if (plist->enabled) plist->entry.replay_serial_receive(cpu, fifo_addr, value);
        }
    }
}

void panda_callbacks_serial_read(CPUState *cpu, uint64_t fifo_addr,
                                 uint32_t port_addr, uint8_t value)
{
    if (rr_mode == RR_REPLAY) {
        panda_cb_list *plist;
        for (plist = panda_cbs[PANDA_CB_REPLAY_SERIAL_READ]; plist != NULL;
             plist = panda_cb_list_next(plist)) {
            if (plist->enabled) plist->entry.replay_serial_read(cpu, fifo_addr, port_addr, value);
        }
    }
}

void panda_callbacks_serial_send(CPUState *cpu, uint64_t fifo_addr,
                                 uint8_t value)
{
    if (rr_mode == RR_REPLAY) {
        panda_cb_list *plist;
        for (plist = panda_cbs[PANDA_CB_REPLAY_SERIAL_SEND]; plist != NULL;
             plist = panda_cb_list_next(plist)) {
            if (plist->enabled) plist->entry.replay_serial_send(cpu, fifo_addr, value);
        }
    }
}

void panda_callbacks_serial_write(CPUState *cpu, uint64_t fifo_addr,
                                  uint32_t port_addr, uint8_t value)
{
    if (rr_mode == RR_REPLAY) {
        panda_cb_list *plist;
        for (plist = panda_cbs[PANDA_CB_REPLAY_SERIAL_WRITE]; plist != NULL;
             plist = panda_cb_list_next(plist)) {
            if (plist->enabled) plist->entry.replay_serial_write(cpu, fifo_addr, port_addr, value);
        }
    }
}


/*
typedef enum RunState {
    RUN_STATE_DEBUG = 0,
    RUN_STATE_INMIGRATE = 1,
    RUN_STATE_INTERNAL_ERROR = 2,
    RUN_STATE_IO_ERROR = 3,
    RUN_STATE_PAUSED = 4,
    RUN_STATE_POSTMIGRATE = 5,
    RUN_STATE_PRELAUNCH = 6,
    RUN_STATE_FINISH_MIGRATE = 7,
    RUN_STATE_RESTORE_VM = 8,
    RUN_STATE_RUNNING = 9,
    RUN_STATE_SAVE_VM = 10,
    RUN_STATE_SHUTDOWN = 11,
    RUN_STATE_SUSPENDED = 12,
    RUN_STATE_WATCHDOG = 13,
    RUN_STATE_GUEST_PANICKED = 14,
    RUN_STATE_COLO = 15,
    RUN_STATE__MAX = 16,
} RunState;
*/

extern bool panda_exit_loop;
extern bool panda_stopped;
int runstate_is_running(void);

void panda_callbacks_main_loop_wait(void) {
    panda_cb_list *plist;
//    printf ("In panda_callbacks_main_loop_wait\n");
    int n = 0;
    for (plist = panda_cbs[PANDA_CB_MAIN_LOOP_WAIT]; plist != NULL;
         plist = panda_cb_list_next(plist)) {
        plist->entry.main_loop_wait();
        n ++;
    }
//    printf ("... %d callbacks.  panda_exit_loop=%d runstate_is_running=%d panda_stopped=%d\n", n, panda_exit_loop, runstate_is_running(), panda_stopped);

    if (panda_exit_loop) {
        //       printf ("Clearing panda_exit_loop\n");
        panda_exit_loop = false;
    }
}

void panda_callbacks_pre_shutdown(void) {
    panda_cb_list *plist;
    //printf ("In panda_callbacks_pre_shutdown\n");
    int n = 0;
    for (plist = panda_cbs[PANDA_CB_PRE_SHUTDOWN]; plist != NULL;
         plist = panda_cb_list_next(plist)) {
        plist->entry.main_loop_wait();
        n ++;
    }
}
