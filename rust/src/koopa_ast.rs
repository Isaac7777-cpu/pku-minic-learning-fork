use koopa::ir::{
    BasicBlock, Function, FunctionData, Program, Value,
    builder::{BasicBlockBuilder, LocalInstBuilder, ValueBuilder},
};

use crate::c_ast::{Block, CompUnit, FuncDef, Stmt};

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
        // self.emit_return_inst(func, stmt.num);
    }

    fn emit_return_inst(&mut self, func: Function, num: i32) {
        let f = self.program.func_mut(func);

        let numeric_node = f.dfg_mut().new_value().integer(num);
        let ret_inst = f.dfg_mut().new_value().ret(Some(numeric_node));

        self.push_inst(func, ret_inst);
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
