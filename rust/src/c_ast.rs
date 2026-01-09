use koopa::ir::Type;

#[derive(Debug)]
pub struct CompUnit {
    pub func_def: FuncDef,
}

#[derive(Debug)]
pub struct FuncDef {
    pub func_type: FuncType,
    pub ident: String,
    pub block: Block,
}

#[derive(Debug)]
pub enum FuncType {
    Int,
}

impl FuncType {
    pub fn to_koopa_type(&self) -> Type {
        match self {
            Self::Int => Type::get_i32(),
        }
    }
}

#[derive(Debug)]
pub struct Block {
    pub stmt: Stmt,
}

#[derive(Debug)]
pub struct Stmt {
    pub num: Number,
}

pub type Number = i32;
