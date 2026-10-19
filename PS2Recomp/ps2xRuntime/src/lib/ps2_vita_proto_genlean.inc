// manager_sub_0110DD58 as the lean-GPR pass emits it (generated).
[[gnu::noinline]] static void protoGenLean_110dd58(uint8_t* rdram, R5900Context* ctx, PS2Runtime *runtime) {
    uint64_t gr2 = Ps2LoadGpr64(ctx, 2), gr3 = Ps2LoadGpr64(ctx, 3), gr4 = Ps2LoadGpr64(ctx, 4), gr5 = Ps2LoadGpr64(ctx, 5), gr6 = Ps2LoadGpr64(ctx, 6), gr7 = Ps2LoadGpr64(ctx, 7), gr8 = Ps2LoadGpr64(ctx, 8), gr31 = Ps2LoadGpr64(ctx, 31);
#ifdef PS2_FUNCTION_LOG_TRACKER
    PS_LOG_ENTRY("manager_sub_0110DD58_0x110dd58");
#endif

    switch (ctx->pc) {
        case 0x110dd70u: goto label_110dd70;
        case 0x110dd80u: goto label_110dd80;
        case 0x110dd88u: goto label_110dd88;
        case 0x110dd90u: goto label_110dd90;
        case 0x110dda8u: goto label_110dda8;
        default: break;
    }

    ctx->pc = 0x110dd58u;

label_110dd58:
    // 0x110dd58: 0x61080  sll         $v0, $a2, 2
    gr2 = PS2_SX32((int32_t)SLL32(static_cast<uint32_t>(gr6), 2));
    // 0x110dd5c: 0xc8182a  slt         $v1, $a2, $t0
    gr3 = static_cast<uint64_t>(((int64_t)static_cast<int64_t>(gr6) < (int64_t)static_cast<int64_t>(gr8)) ? 1 : 0);
    // 0x110dd60: 0x10600007  beqz        $v1, . + 4 + (0x7 << 2)
    ctx->pc = 0x110DD60u;
    {
        const bool branch_taken_0x110dd60 = (gr3 == GPR_U64(ctx, 0));
        ctx->pc = 0x110DD64u;
        ctx->in_delay_slot = true;
        ctx->branch_pc = 0x110DD60u;
        // 0x110dd64: 0x471021  addu        $v0, $v0, $a3 (Delay Slot)
        gr2 = PS2_SX32((int32_t)ADD32(static_cast<uint32_t>(gr2), static_cast<uint32_t>(gr7)));
        ctx->in_delay_slot = false;
        if (branch_taken_0x110dd60) {
            ctx->pc = 0x110DD80u;
            goto label_110dd80;
        }
    }
    ctx->pc = 0x110DD68u;
    // 0x110dd68: 0xac860010  sw          $a2, 0x10($a0)
    ctx->pc = 0x110dd68u;
    WRITE32(ADD32(static_cast<uint32_t>(gr4), 16), static_cast<uint32_t>(gr6));
    // 0x110dd6c: 0x8c420000  lw          $v0, 0x0($v0)
    ctx->pc = 0x110dd6cu;
    gr2 = PS2_SX32((int32_t)READ32(ADD32(static_cast<uint32_t>(gr2), 0)));
label_110dd70:
    // 0x110dd70: 0x0  nop
    // NOP
    // 0x110dd74: 0x5040fff8  beql        $v0, $zero, . + 4 + (-0x8 << 2)
    ctx->pc = 0x110DD74u;
    {
        const bool branch_taken_0x110dd74 = (gr2 == GPR_U64(ctx, 0));
        if (branch_taken_0x110dd74) {
            ctx->pc = 0x110DD78u;
            ctx->in_delay_slot = true;
            ctx->branch_pc = 0x110DD74u;
            // 0x110dd78: 0x24c60001  addiu       $a2, $a2, 0x1 (Delay Slot)
            gr6 = PS2_SX32((int32_t)ADD32(static_cast<uint32_t>(gr6), 1));
            ctx->in_delay_slot = false;
            ctx->pc = 0x110DD58u;
            if (protoCheckpoint()) {
                { { Ps2StoreGpr64(ctx, 2, gr2); Ps2StoreGpr64(ctx, 3, gr3); Ps2StoreGpr64(ctx, 5, gr5); Ps2StoreGpr64(ctx, 6, gr6); } return; }
            }
            goto label_110dd58;
        }
    }
    ctx->pc = 0x110DD7Cu;
    // 0x110dd7c: 0x40282d  daddu       $a1, $v0, $zero
    gr5 = static_cast<uint64_t>((uint64_t)gr2 + (uint64_t)GPR_U64(ctx, 0));
label_110dd80:
    // 0x110dd80: 0x3e00008  jr          $ra
    ctx->pc = 0x110DD80u;
    {
        const uint32_t jumpTarget = static_cast<uint32_t>(gr31);
        ctx->pc = 0x110DD84u;
        ctx->in_delay_slot = true;
        ctx->branch_pc = 0x110DD80u;
        // 0x110dd84: 0xa0102d  daddu       $v0, $a1, $zero (Delay Slot)
        gr2 = static_cast<uint64_t>((uint64_t)gr5 + (uint64_t)GPR_U64(ctx, 0));
        ctx->in_delay_slot = false;
        ctx->pc = jumpTarget;
        #if defined(PS2X_STRICT_RETURN_DIAGNOSTICS) && PS2X_STRICT_RETURN_DIAGNOSTICS
        { Ps2StoreGpr64(ctx, 2, gr2); Ps2StoreGpr64(ctx, 3, gr3); Ps2StoreGpr64(ctx, 5, gr5); Ps2StoreGpr64(ctx, 6, gr6); }
        (void)runtime->dispatchGuestBranch(rdram, ctx, jumpTarget, 0x110DD80u, 0u, PS2Runtime::GuestBranchKind::Return, "JR $ra");
        { gr2 = Ps2LoadGpr64(ctx, 2); gr3 = Ps2LoadGpr64(ctx, 3); gr4 = Ps2LoadGpr64(ctx, 4); gr5 = Ps2LoadGpr64(ctx, 5); gr6 = Ps2LoadGpr64(ctx, 6); gr7 = Ps2LoadGpr64(ctx, 7); gr8 = Ps2LoadGpr64(ctx, 8); gr31 = Ps2LoadGpr64(ctx, 31); }
        { { Ps2StoreGpr64(ctx, 2, gr2); Ps2StoreGpr64(ctx, 3, gr3); Ps2StoreGpr64(ctx, 5, gr5); Ps2StoreGpr64(ctx, 6, gr6); } return; }
        #else
        ctx->pc = jumpTarget;
        { { Ps2StoreGpr64(ctx, 2, gr2); Ps2StoreGpr64(ctx, 3, gr3); Ps2StoreGpr64(ctx, 5, gr5); Ps2StoreGpr64(ctx, 6, gr6); } return; }
        #endif
    }
    ctx->pc = 0x110DD88u;
label_110dd88:
    // 0x110dd88: 0x3e00008  jr          $ra
    ctx->pc = 0x110DD88u;
    {
        const uint32_t jumpTarget = static_cast<uint32_t>(gr31);
        ctx->pc = 0x110DD8Cu;
        ctx->in_delay_slot = true;
        ctx->branch_pc = 0x110DD88u;
        // 0x110dd8c: 0x102d  daddu       $v0, $zero, $zero (Delay Slot)
        gr2 = static_cast<uint64_t>((uint64_t)GPR_U64(ctx, 0) + (uint64_t)GPR_U64(ctx, 0));
        ctx->in_delay_slot = false;
        ctx->pc = jumpTarget;
        #if defined(PS2X_STRICT_RETURN_DIAGNOSTICS) && PS2X_STRICT_RETURN_DIAGNOSTICS
        { Ps2StoreGpr64(ctx, 2, gr2); Ps2StoreGpr64(ctx, 3, gr3); Ps2StoreGpr64(ctx, 5, gr5); Ps2StoreGpr64(ctx, 6, gr6); }
        (void)runtime->dispatchGuestBranch(rdram, ctx, jumpTarget, 0x110DD88u, 0u, PS2Runtime::GuestBranchKind::Return, "JR $ra");
        { gr2 = Ps2LoadGpr64(ctx, 2); gr3 = Ps2LoadGpr64(ctx, 3); gr4 = Ps2LoadGpr64(ctx, 4); gr5 = Ps2LoadGpr64(ctx, 5); gr6 = Ps2LoadGpr64(ctx, 6); gr7 = Ps2LoadGpr64(ctx, 7); gr8 = Ps2LoadGpr64(ctx, 8); gr31 = Ps2LoadGpr64(ctx, 31); }
        { { Ps2StoreGpr64(ctx, 2, gr2); Ps2StoreGpr64(ctx, 3, gr3); Ps2StoreGpr64(ctx, 5, gr5); Ps2StoreGpr64(ctx, 6, gr6); } return; }
        #else
        ctx->pc = jumpTarget;
        { { Ps2StoreGpr64(ctx, 2, gr2); Ps2StoreGpr64(ctx, 3, gr3); Ps2StoreGpr64(ctx, 5, gr5); Ps2StoreGpr64(ctx, 6, gr6); } return; }
        #endif
    }
    ctx->pc = 0x110DD90u;
label_110dd90:
    // 0x110dd90: 0xcc400040  pref        0x00, 0x40($v0)
    // PREF instruction (ignored)
    // 0x110dd94: 0x8ca30008  lw          $v1, 0x8($a1)
    ctx->pc = 0x110dd94u;
    gr3 = PS2_SX32((int32_t)READ32(ADD32(static_cast<uint32_t>(gr5), 8)));
    // 0x110dd98: 0x1460000c  bnez        $v1, . + 4 + (0xC << 2)
    ctx->pc = 0x110DD98u;
    {
        const bool branch_taken_0x110dd98 = (gr3 != GPR_U64(ctx, 0));
        ctx->pc = 0x110DD9Cu;
        ctx->in_delay_slot = true;
        ctx->branch_pc = 0x110DD98u;
        // 0x110dd9c: 0x60102d  daddu       $v0, $v1, $zero (Delay Slot)
        gr2 = static_cast<uint64_t>((uint64_t)gr3 + (uint64_t)GPR_U64(ctx, 0));
        ctx->in_delay_slot = false;
        if (branch_taken_0x110dd98) {
            ctx->pc = 0x110DDCCu;
            goto label_110ddcc;
        }
    }
    ctx->pc = 0x110DDA0u;
    // 0x110dda0: 0x8c850004  lw          $a1, 0x4($a0)
    ctx->pc = 0x110dda0u;
    gr5 = PS2_SX32((int32_t)READ32(ADD32(static_cast<uint32_t>(gr4), 4)));
    // 0x110dda4: 0x8c820010  lw          $v0, 0x10($a0)
    ctx->pc = 0x110dda4u;
    gr2 = PS2_SX32((int32_t)READ32(ADD32(static_cast<uint32_t>(gr4), 16)));
label_110dda8:
    // 0x110dda8: 0x24420001  addiu       $v0, $v0, 0x1
    gr2 = PS2_SX32((int32_t)ADD32(static_cast<uint32_t>(gr2), 1));
    // 0x110ddac: 0x21880  sll         $v1, $v0, 2
    gr3 = PS2_SX32((int32_t)SLL32(static_cast<uint32_t>(gr2), 2));
    // 0x110ddb0: 0x1045fff5  beq         $v0, $a1, . + 4 + (-0xB << 2)
    ctx->pc = 0x110DDB0u;
    {
        const bool branch_taken_0x110ddb0 = (gr2 == gr5);
        ctx->pc = 0x110DDB4u;
        ctx->in_delay_slot = true;
        ctx->branch_pc = 0x110DDB0u;
        // 0x110ddb4: 0xac820010  sw          $v0, 0x10($a0) (Delay Slot)
        WRITE32(ADD32(static_cast<uint32_t>(gr4), 16), static_cast<uint32_t>(gr2));
        ctx->in_delay_slot = false;
        if (branch_taken_0x110ddb0) {
            ctx->pc = 0x110DD88u;
            if (protoCheckpoint()) {
                { { Ps2StoreGpr64(ctx, 2, gr2); Ps2StoreGpr64(ctx, 3, gr3); Ps2StoreGpr64(ctx, 5, gr5); Ps2StoreGpr64(ctx, 6, gr6); } return; }
            }
            goto label_110dd88;
        }
    }
    ctx->pc = 0x110DDB8u;
    // 0x110ddb8: 0x8c820000  lw          $v0, 0x0($a0)
    ctx->pc = 0x110ddb8u;
    gr2 = PS2_SX32((int32_t)READ32(ADD32(static_cast<uint32_t>(gr4), 0)));
    // 0x110ddbc: 0x621021  addu        $v0, $v1, $v0
    gr2 = PS2_SX32((int32_t)ADD32(static_cast<uint32_t>(gr3), static_cast<uint32_t>(gr2)));
    // 0x110ddc0: 0x8c420000  lw          $v0, 0x0($v0)
    ctx->pc = 0x110ddc0u;
    gr2 = PS2_SX32((int32_t)READ32(ADD32(static_cast<uint32_t>(gr2), 0)));
    // 0x110ddc4: 0x5040fff8  beql        $v0, $zero, . + 4 + (-0x8 << 2)
    ctx->pc = 0x110DDC4u;
    {
        const bool branch_taken_0x110ddc4 = (gr2 == GPR_U64(ctx, 0));
        if (branch_taken_0x110ddc4) {
            ctx->pc = 0x110DDC8u;
            ctx->in_delay_slot = true;
            ctx->branch_pc = 0x110DDC4u;
            // 0x110ddc8: 0x8c820010  lw          $v0, 0x10($a0) (Delay Slot)
            gr2 = PS2_SX32((int32_t)READ32(ADD32(static_cast<uint32_t>(gr4), 16)));
            ctx->in_delay_slot = false;
            ctx->pc = 0x110DDA8u;
            if (protoCheckpoint()) {
                { { Ps2StoreGpr64(ctx, 2, gr2); Ps2StoreGpr64(ctx, 3, gr3); Ps2StoreGpr64(ctx, 5, gr5); Ps2StoreGpr64(ctx, 6, gr6); } return; }
            }
            goto label_110dda8;
        }
    }
    ctx->pc = 0x110DDCCu;
label_110ddcc:
    // 0x110ddcc: 0x3e00008  jr          $ra
    ctx->pc = 0x110DDCCu;
    {
        const uint32_t jumpTarget = static_cast<uint32_t>(gr31);
        ctx->pc = jumpTarget;
        #if defined(PS2X_STRICT_RETURN_DIAGNOSTICS) && PS2X_STRICT_RETURN_DIAGNOSTICS
        { Ps2StoreGpr64(ctx, 2, gr2); Ps2StoreGpr64(ctx, 3, gr3); Ps2StoreGpr64(ctx, 5, gr5); Ps2StoreGpr64(ctx, 6, gr6); }
        (void)runtime->dispatchGuestBranch(rdram, ctx, jumpTarget, 0x110DDCCu, 0u, PS2Runtime::GuestBranchKind::Return, "JR $ra");
        { gr2 = Ps2LoadGpr64(ctx, 2); gr3 = Ps2LoadGpr64(ctx, 3); gr4 = Ps2LoadGpr64(ctx, 4); gr5 = Ps2LoadGpr64(ctx, 5); gr6 = Ps2LoadGpr64(ctx, 6); gr7 = Ps2LoadGpr64(ctx, 7); gr8 = Ps2LoadGpr64(ctx, 8); gr31 = Ps2LoadGpr64(ctx, 31); }
        { { Ps2StoreGpr64(ctx, 2, gr2); Ps2StoreGpr64(ctx, 3, gr3); Ps2StoreGpr64(ctx, 5, gr5); Ps2StoreGpr64(ctx, 6, gr6); } return; }
        #else
        ctx->pc = jumpTarget;
        { { Ps2StoreGpr64(ctx, 2, gr2); Ps2StoreGpr64(ctx, 3, gr3); Ps2StoreGpr64(ctx, 5, gr5); Ps2StoreGpr64(ctx, 6, gr6); } return; }
        #endif
    }
    ctx->pc = 0x110DDD4u;
    // 0x110ddd4: 0x0  nop
    // NOP
    ctx->pc = 0x110ddd8u;
    { Ps2StoreGpr64(ctx, 2, gr2); Ps2StoreGpr64(ctx, 3, gr3); Ps2StoreGpr64(ctx, 5, gr5); Ps2StoreGpr64(ctx, 6, gr6); }
    ps2TransferTo(rdram, ctx, runtime, 0x110ddd8u);
    { gr2 = Ps2LoadGpr64(ctx, 2); gr3 = Ps2LoadGpr64(ctx, 3); gr4 = Ps2LoadGpr64(ctx, 4); gr5 = Ps2LoadGpr64(ctx, 5); gr6 = Ps2LoadGpr64(ctx, 6); gr7 = Ps2LoadGpr64(ctx, 7); gr8 = Ps2LoadGpr64(ctx, 8); gr31 = Ps2LoadGpr64(ctx, 31); }
    { Ps2StoreGpr64(ctx, 2, gr2); Ps2StoreGpr64(ctx, 3, gr3); Ps2StoreGpr64(ctx, 5, gr5); Ps2StoreGpr64(ctx, 6, gr6); }
}
