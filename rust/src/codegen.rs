use std::{collections::HashMap, io::Write};

use koopa::ir::{BinaryOp, Value, ValueKind};

use crate::{
    reg_alloc::{NaiveRegAlloc, RegAllocator},
    register::Reg,
    riscv::AsmInst,
};

const WRITE_UNABLE_ERROR: &str = "Cannot write to the destination file.";
const INDENT: &str = "\t";

pub struct CodeGenCtx<'a, W: Write> {
    reg_allocator: NaiveRegAlloc,
    val_reg_dict: HashMap<Value, Reg>,
    cur_func: Option<koopa::ir::Function>,
    out: &'a mut W,
    prog: &'a koopa::ir::Program,
}

impl<'a, W: Write> CodeGenCtx<'a, W> {
    pub fn new(writer: &'a mut W, program: &'a koopa::ir::Program) -> Self {
        Self {
            reg_allocator: NaiveRegAlloc::new(),
            cur_func: None,
            out: writer,
            prog: program,
            val_reg_dict: HashMap::new(),
        }
    }

    pub fn func(&self) -> &'a koopa::ir::FunctionData {
        self.prog
            .func(self.cur_func.expect("No current function set."))
    }

    pub fn acquire_any_reg(&mut self) -> Reg {
        self.reg_allocator.get_avail().unwrap()
    }

    pub fn acquire_reg(&mut self, reg: Reg) -> Option<Reg> {
        self.reg_allocator.acquire(reg)
    }

    pub fn emit_inst(&mut self, inst: AsmInst) {
        writeln!(self.out, "{inst}").expect(WRITE_UNABLE_ERROR);
    }

    pub fn emit_binary(&mut self, op: BinaryOp, dst: Reg, lhs: Reg, rhs: Reg) {
        match op {
            BinaryOp::Eq => {
                self.emit_inst(AsmInst::Xor {
                    rd: dst,
                    rs1: lhs,
                    rs2: rhs,
                });
                self.emit_inst(AsmInst::Seqz { rd: dst, rs: dst });
            }
            BinaryOp::Add => self.emit_inst(AsmInst::Add {
                rd: dst,
                rs1: lhs,
                rs2: rhs,
            }),
            BinaryOp::Sub => self.emit_inst(AsmInst::Sub {
                rd: dst,
                rs1: lhs,
                rs2: rhs,
            }),
            BinaryOp::Mul => self.emit_inst(AsmInst::Mul {
                rd: dst,
                rs1: lhs,
                rs2: rhs,
            }),
            BinaryOp::Div => self.emit_inst(AsmInst::Div {
                rd: dst,
                rs1: lhs,
                rs2: rhs,
            }),
            BinaryOp::Mod => self.emit_inst(AsmInst::Mod {
                rd: dst,
                rs1: lhs,
                rs2: rhs,
            }),
            BinaryOp::Xor => self.emit_inst(AsmInst::Xor {
                rd: dst,
                rs1: lhs,
                rs2: rhs,
            }),
            _ => todo!("unimplemented BinaryOp: {:?}", op),
        }
    }
}

pub trait GenerateAsm {
    type Return;
    fn generate<W: Write>(&self, ctx: &mut CodeGenCtx<'_, W>) -> Self::Return;
}

impl GenerateAsm for koopa::ir::Program {
    type Return = ();

    fn generate<W: Write>(&self, ctx: &mut CodeGenCtx<'_, W>) {
        writeln!(ctx.out, "{}.text", INDENT).expect(WRITE_UNABLE_ERROR);
        writeln!(ctx.out, "{}.global main", INDENT).expect(WRITE_UNABLE_ERROR);
        for &func in self.func_layout() {
            ctx.cur_func = Some(func);
            self.func(func).generate(ctx);
        }
    }
}

impl GenerateAsm for koopa::ir::FunctionData {
    type Return = ();

    fn generate<W: Write>(&self, ctx: &mut CodeGenCtx<'_, W>) {
        let raw = self.name();
        let name = raw
            .strip_prefix('@')
            .or_else(|| raw.strip_prefix('%'))
            .unwrap_or(raw);

        writeln!(ctx.out, "{}:", name).expect(WRITE_UNABLE_ERROR);

        for (_bb, bb_node) in self.layout().bbs() {
            bb_node.generate(ctx);
        }
    }
}

impl GenerateAsm for koopa::ir::layout::BasicBlockNode {
    type Return = ();

    fn generate<W: Write>(&self, ctx: &mut CodeGenCtx<'_, W>) {
        for (&val, _inst_node) in self.insts() {
            val.generate(ctx);
        }
    }
}

// impl GenerateAsm for koopa::ir::entities::ValueData {
impl GenerateAsm for koopa::ir::Value {
    type Return = Option<Reg>;
    fn generate<W: Write>(&self, ctx: &mut CodeGenCtx<'_, W>) -> Option<Reg> {
        // Check if this value is already parsed and we can then return.
        if let Some(saved_reg) = ctx.val_reg_dict.get(self) {
            return Some(*saved_reg);
        }

        let val_data = ctx.func().dfg().value(*self);
        match val_data.kind() {
            ValueKind::Integer(num) => {
                if num.value() == 0 {
                    return Some(Reg::Zero);
                }

                let dst = ctx.acquire_any_reg();
                ctx.emit_inst(AsmInst::Li {
                    rd: dst,
                    imm: num.value(),
                });
                Some(dst)
            }
            ValueKind::Return(ret) => {
                if let Some(ret_exp_val) = ret.value() {
                    // Parse the sub-tree
                    let src_reg = ret_exp_val.generate(ctx);
                    if let Some(ret_reg) = src_reg {
                        let reg_a0 = Reg::A(0);
                        if ret_reg.xnum() != reg_a0.xnum() {
                            let a0 = ctx
                                .acquire_reg(reg_a0)
                                .expect("Cannot arrange a0, must have been used by others");
                            ctx.emit_inst(AsmInst::Mv {
                                rd: a0,
                                rs: ret_reg,
                            });
                        }
                    }
                }
                ctx.emit_inst(AsmInst::Ret);
                None
            }
            ValueKind::Binary(bin) => {
                let lhs_reg = bin
                    .lhs()
                    .generate(ctx)
                    .expect("Binary operations require resulting register from sub-operations.");
                let rhs_reg = bin
                    .rhs()
                    .generate(ctx)
                    .expect("Binary operations require resulting register from sub-operations.");

                // Pick the destination so that we can use as least as possible.
                let lhs_val_data = ctx.func().dfg().value(bin.lhs());
                let rhs_val_data = ctx.func().dfg().value(bin.rhs());
                let dst = match (lhs_val_data.kind(), rhs_val_data.kind()) {
                    (ValueKind::Integer(n), _) if n.value() != 0 => lhs_reg,
                    (_, ValueKind::Integer(n)) if n.value() != 0 => rhs_reg,
                    _ => ctx.acquire_any_reg(),
                };
                ctx.emit_binary(bin.op(), dst, lhs_reg, rhs_reg);
                // match bin.op() {
                //     BinaryOp::Add => {
                //         ctx.emit_binary(BinaryOp::Add, dst, lhs_reg, rhs_reg);
                //     }
                //     BinaryOp::Sub => {
                //         ctx.emit_binary(BinaryOp::Sub, dst, lhs_reg, rhs_reg);
                //     }
                //     BinaryOp::Eq => {
                //         ctx.emit_binary(BinaryOp::Eq, dst, lhs_reg, rhs_reg);
                //     }
                //     BinaryOp::Xor => {
                //         ctx.emit_binary(BinaryOp::Xor, dst, lhs_reg, rhs_reg);
                //     }
                //     _ => {
                //         unreachable!("Other operations are not yet implemetned")
                //     }
                // }
                ctx.val_reg_dict.insert(*self, dst);
                Some(dst)
            }
            _ => unreachable!("Not implemented yet"),
        }
    }
}
