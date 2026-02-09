use koopa::ir::{
    BasicBlock, BinaryOp, Function, FunctionData, Program, Value,
    builder::{BasicBlockBuilder, LocalInstBuilder, ValueBuilder},
};

use crate::c_ast::{Block, CompUnit, Exp, FuncDef, PrimaryExp, Stmt, UnaryExp, UnaryOp};

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
        self.lower_unary_exp(func, &exp.unary_exp)
    }

    pub fn lower_primary_exp(&mut self, func: Function, p_exp: &PrimaryExp) -> Value {
        match p_exp {
            PrimaryExp::ExpPrimary { exp } => self.lower_exp(func, exp),
            PrimaryExp::NumberPrimary { num } => self.construct_i32(func, *num)
        }
    }

    pub fn lower_unary_exp(&mut self, func: Function, unary_exp: &UnaryExp) -> Value {
        match unary_exp {
            UnaryExp::PrimaryUnary { primary_exp } => self.lower_primary_exp(func, primary_exp),
            UnaryExp::OpUnaryUnary {
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
