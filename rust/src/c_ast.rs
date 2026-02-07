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
    pub exp: Exp,
}

#[derive(Debug)]
pub struct Exp {
    pub unary_exp: Box<UnaryExp>,
}

#[derive(Debug)]
pub enum PrimaryExp {
    ExpPrimary { exp: Box<Exp> },
    NumberPrimary { num: Number },
}

pub type Number = i32;

#[derive(Debug)]
pub enum UnaryExp {
    PrimaryUnary {
        primary_exp: Box<PrimaryExp>,
    },
    OpUnaryUnary {
        unary_op: UnaryOp,
        unary_exp: Box<UnaryExp>,
    },
}

#[derive(Debug)]
pub enum UnaryOp {
    PLUS,
    MINUS,
    BANG,
}
