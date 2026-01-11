use std::io::Write;

use koopa::ir::ValueKind;

const WRITE_UNABLE_ERROR: &str = "Cannot write to the destination file.";
const INDENT: &str = "\t";

pub struct CodeGenCtx<'a, W: Write> {
    pub out: &'a mut W,
    pub prog: &'a koopa::ir::Program,
    pub cur_func: Option<koopa::ir::Function>,
}

impl<'a, W: Write> CodeGenCtx<'a, W> {
    pub fn func(&self) -> &'a koopa::ir::FunctionData {
        self.prog
            .func(self.cur_func.expect("No current function set."))
    }
}

pub trait GenerateAsm {
    fn generate<W: Write>(&self, ctx: &mut CodeGenCtx<'_, W>);
}

impl GenerateAsm for koopa::ir::Program {
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
    fn generate<W: Write>(&self, ctx: &mut CodeGenCtx<'_, W>) {
        for (&val, _inst_node) in self.insts() {
            let val_data = ctx.func().dfg().value(val);
            val_data.generate(ctx);
        }
    }
}

impl GenerateAsm for koopa::ir::entities::ValueData {
    fn generate<W: Write>(&self, ctx: &mut CodeGenCtx<'_, W>) {
        match self.kind() {
            ValueKind::Integer(int) => {
                writeln!(ctx.out, "{}li a0, {}", INDENT, int.value()).expect(WRITE_UNABLE_ERROR);
            }
            ValueKind::Return(ret) => {
                if let Some(ret_exp_val) = ret.value() {
                    ctx.func().dfg().value(ret_exp_val).generate(ctx);
                }
                writeln!(ctx.out, "{}ret", INDENT).expect(WRITE_UNABLE_ERROR);
            }
            _ => unreachable!("Not implemented yet"),
        }
    }
}
