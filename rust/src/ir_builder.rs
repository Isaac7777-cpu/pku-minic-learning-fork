use koopa::ir::{
    BasicBlock, BinaryOp, Function, FunctionData, Program, Value,
    builder::{BasicBlockBuilder, LocalInstBuilder, ValueBuilder},
};

use crate::c_ast::{
    AddExp, AddOp, Block, CompUnit, Exp, FuncDef, MulExp, MulOp, PrimaryExp, Stmt, UnaryExp,
    UnaryOp,
};

pub struct LowerCtx {
    pub program: Program,
    cur_bb: Option<BasicBlock>,
}

impl LowerCtx {
    pub fn new() -> Self {
        Self {
            program: Program::new(),
            cur_bb: None,
        }
    }

    pub fn build_program_from_comp_unit(&mut self, comp_unit: CompUnit) {
        self.lower_func_def(&comp_unit.func_def);
    }

    pub fn lower_func_def(&mut self, func_def: &FuncDef) {
        let func_data = self.func_header(func_def);
        let func = self.program.new_func(func_data);
        self.lower_function_body(func, &func_def.block);
    }

    pub fn lower_function_body(&mut self, func: Function, block: &Block) {
        let f: &mut FunctionData = self.program.func_mut(func);

        let entry: BasicBlock = f.dfg_mut().new_bb().basic_block(Some("%entry".into()));
        f.layout_mut()
            .bbs_mut()
            .push_key_back(entry)
            .expect("We should only have one basic block for now.");

        self.cur_bb = Some(entry);

        self.lower_block(func, block);

        self.cur_bb = None;
    }

    pub fn lower_block(&mut self, func: Function, block: &Block) {
        self.lower_stmt(func, &block.stmt);
    }

    pub fn lower_stmt(&mut self, func: Function, stmt: &Stmt) {
        let return_val = self.lower_exp(func, &stmt.exp);
        self.emit_return_inst(func, return_val);
    }

    pub fn lower_exp(&mut self, func: Function, exp: &Exp) -> Value {
        self.lower_add_exp(func, &exp.add_exp)
    }

    pub fn lower_primary_exp(&mut self, func: Function, p_exp: &PrimaryExp) -> Value {
        match p_exp {
            PrimaryExp::Exp { exp } => self.lower_exp(func, exp),
            PrimaryExp::Number { num } => self.construct_i32(func, *num),
        }
    }

    pub fn lower_unary_exp(&mut self, func: Function, unary_exp: &UnaryExp) -> Value {
        match unary_exp {
            UnaryExp::Primary { primary_exp } => self.lower_primary_exp(func, primary_exp),
            UnaryExp::OpUnary {
                unary_op,
                unary_exp,
            } => {
                let u_exp_val = self.lower_unary_exp(func, unary_exp);
                match unary_op {
                    UnaryOp::PLUS => u_exp_val,
                    UnaryOp::MINUS => {
                        let zero = self.construct_i32(func, 0);
                        self.emit_binary(func, BinaryOp::Sub, zero, u_exp_val)
                    }
                    UnaryOp::BANG => {
                        let zero = self.construct_i32(func, 0);
                        self.emit_binary(func, BinaryOp::Eq, u_exp_val, zero)
                    }
                    UnaryOp::TILDE => {
                        let all_one = self.construct_i32(func, -1);
                        self.emit_binary(func, BinaryOp::Xor, u_exp_val, all_one)
                    }
                }
            }
        }
    }

    pub fn lower_mul_exp(&mut self, func: Function, mul_exp: &MulExp) -> Value {
        match mul_exp {
            MulExp::Unary { unary_exp } => self.lower_unary_exp(func, unary_exp),
            MulExp::MulOpUnary {
                mul_exp,
                op,
                unary_exp,
            } => {
                let mul_exp_val = self.lower_mul_exp(func, mul_exp);
                let unary_exp_val = self.lower_unary_exp(func, unary_exp);
                match op {
                    MulOp::STAR => {
                        self.emit_binary(func, BinaryOp::Mul, mul_exp_val, unary_exp_val)
                    }
                    MulOp::SLASH => {
                        self.emit_binary(func, BinaryOp::Div, mul_exp_val, unary_exp_val)
                    }
                    MulOp::PERCENT => {
                        self.emit_binary(func, BinaryOp::Mod, mul_exp_val, unary_exp_val)
                    }
                }
            }
        }
    }

    pub fn lower_add_exp(&mut self, func: Function, add_exp: &AddExp) -> Value {
        match add_exp {
            AddExp::Mul { mul_exp } => self.lower_mul_exp(func, mul_exp),
            AddExp::AddOpMul {
                add_exp,
                op,
                mul_exp,
            } => {
                let add_exp_val = self.lower_add_exp(func, add_exp);
                let mul_exp_val = self.lower_mul_exp(func, mul_exp);
                match op {
                    AddOp::PLUS => self.emit_binary(func, BinaryOp::Add, add_exp_val, mul_exp_val),
                    AddOp::MINUS => self.emit_binary(func, BinaryOp::Sub, add_exp_val, mul_exp_val),
                }
            }
        }
    }

    pub fn emit_return_inst(&mut self, func: Function, return_val: Value) {
        let f = self.program.func_mut(func);
        let ret_inst = f.dfg_mut().new_value().ret(Some(return_val));
        self.push_inst(func, ret_inst);
    }

    pub fn emit_binary(
        &mut self,
        func: Function,
        binary_op: BinaryOp,
        lhs: Value,
        rhs: Value,
    ) -> Value {
        let f = self.program.func_mut(func);
        let binary_inst = f.dfg_mut().new_value().binary(binary_op, lhs, rhs);
        self.push_inst(func, binary_inst);
        binary_inst
    }

    pub fn construct_i32(&mut self, func: Function, n: i32) -> Value {
        let f = self.program.func_mut(func);
        f.dfg_mut().new_value().integer(n)
    }

    fn func_header(&self, func_def: &FuncDef) -> FunctionData {
        let name = if func_def.ident.starts_with('@') {
            func_def.ident.clone()
        } else {
            format!("@{}", func_def.ident)
        };
        FunctionData::new(name, Vec::new(), func_def.func_type.to_koopa_type())
    }

    fn push_inst(&mut self, func: Function, inst: Value) {
        let bb = self.cur_bb.expect("no current block set.");
        let f = self.program.func_mut(func);
        f.layout_mut()
            .bb_mut(bb)
            .insts_mut()
            .push_key_back(inst)
            .expect("Duplicated instruction node.");
    }
}

impl Default for LowerCtx {
    fn default() -> Self {
        Self::new()
    }
}
